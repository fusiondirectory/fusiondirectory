<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

require_once('jsonRPCClient.php');

class supportDaemon
{
    private $s_host       = "";
    private $config;
    
    private $s_error      = "";
    private $b_error      = FALSE;


    public function __construct()
    {
        $this->config = session::global_get('config');
        /* This should only be the case if we call this from setup.
        __autoload() 
        */
        if(!is_object($this->config)) { return; }
        # load from config, store statically
        $this->s_host = $this->config->get_cfg_value("argonautServer");
/*
      $this->is_configured = TRUE;
*/
    }
  
    public function is_available() {
        try {
            $client = new jsonRPCClient($this->s_host);
            $ok = "OK";
            $test = $client->echo($ok);
        } catch (Exception $e) {
            $this->set_error(nl2br($e->getMessage()));
            return(FALSE);
        }
        if ($test == $ok) {
            $this->reset_error();
            return(TRUE);
        } else {
            $this->set_error("Received $test, expected $ok");
            return(FALSE);
        }
    }
  
	/* Allows simply appending a new DaemonEvent 
	*/
	public function append($event)
	{
		if(!($event instanceof DaemonEvent)) {
            return(FALSE);
		}

		/* Add to queue if new 
		 */
		if($event->is_new()) {

            $request_answer = FALSE;
            if($event->get_type() == SCHEDULED_EVENT) {
                $action = $event->get_schedule_action();
            } elseif($event->get_type() == TRIGGERED_EVENT) {
                $action = $event->get_trigger_action();
            } else {
                trigger_error("Unknown type of queue event given.");
                return(FALSE);
            }
		  
			/* Get event informations, like targets..
			*/
			$targets    = $event->get_targets();
			$data       = $event->save();
            
            /* Put timestamp back to UTC time
             */
            timezone::get_default_timezone();
            if($event->get_timestamp(FALSE) != 0) {
                $data_json['timestamp'] = $event->get_timestamp(FALSE);
                $data_json['periodic'] = $data['periodic'];
            }
            $data_json['args'] = $data;
            unset($data_json['args']['timestamp']);
            unset($data_json['args']['periodic']);

			/* Append an entry
			*/
			try {
				$client = new jsonRPCClient($this->s_host);
				$client->action($action,$targets,$data_json);
			} catch (Exception $e) {
				$this->set_error(nl2br($e->getMessage()));
				return(FALSE);
			}
            $this->reset_error();
            return(TRUE);
		} else {

            /* Updated edited entry. */
            $id                 = $event->get_id();
            $data               = $event->save();
            return($this->update_entries(array($id),$data));
		}

		return(FALSE);
	}
    
  /*! \brief  Sets an error message, which can be returned with get_error().
    @param  string  The Error message,
   */
  private function set_error($str)
  {
    $this->b_error = TRUE;
    $this->s_error = $str;
  }


  /*! \brief  Sets an error message, which can be returned with get_error().
    @param  string  The Error message,
   */
  private function reset_error()
  {
    $this->b_error = FALSE;
    $this->s_error = "";
  }


  /*! \brief  Checks if an error occured.
    @return boolean returns TRUE or FALSE, whether there is an error or not.
   */
  public function is_error()
  {
    return($this->b_error);
  }


  /*! \brief  Returns the last error. 
    @return Returns the last error.
   */
  public function get_error()
  {
    $str = $this->s_error;
    $ret = "";
    if(is_string($str)){
      $ret = $str;
    }else{
      foreach($str as $msg){
        $ret .= $msg." ";
      }
    }
    $ret = str_replace(" ","&nbsp;",$ret);
    return($ret);
  }
  
    private function format_entries($entries) {
        timezone::get_default_timezone();
        foreach($entries as &$entry) {
            $entry_r['TIMESTAMP'] = DaemonEvent::_timestamp_to_event($entry['data']['timestamp']);
            $entry_r['MACADDRESS'] = $entry['target'];
            
            // TODO : do an ldap search only if we haven't done it for the same mac before
            $ldap = $this->config->get_ldap_link();
            $ldap->cd($this->config->current['BASE']);
            $ldap->search("(macAddress=".$entry_r['MACADDRESS'].")",array('cn'));
            $ldap_infos = $ldap->fetch();
            $ldap_infos['cn'][0];
            $entry_r['PLAINNAME'] = $ldap_infos['cn'][0];
            
            $entry_r['ERROR'] = $entry['error'];
            $entry_r['PROGRESS'] = $entry['progress'];
            
            $entry_r['PERIODIC'] = $entry['data']['periodic'];
            $entry_r['STATUS'] = $entry['status'];
            $entry_r['SUBSTATUS'] = $entry['substatus'];
            $entry_r['ID'] = $entry['id'];
            $entry_r['HEADERTAG'] = $entry['action'];
            $entries_r[]=$entry_r;
        }
        return $entries_r;
    }
  
  
    /*! \brief  Returns an array containing all queued entries.
    @return Array All queued entries as an array.
   */
    public function get_queued_entries($event_types = array("*"),$from=-1,$to=-1,$sort="timestamp DESC")
    {
        timezone::get_default_timezone();
        try {
            $client = new jsonRPCClient($this->s_host);
            $entries = $client->get_entries();
        } catch (Exception $e) {
            $this->set_error(nl2br($e->getMessage()));
            return(FALSE);
        }
        $this->get_entries();
        $entries = $this->format_entries($entries);
        $this->reset_error();
        return($entries);
    }
    
    /*! \brief  Returns an entry containing all requested ids.
    @param  Array   The IDs of the entries we want to return.
    @return Array   Of the requested entries. 
    */
    public function get_entries_by_id($ids)
    {
        return $this->get_entries("get_entries_by_id",$ids);
    }
    
    /*! \brief  Removes a set of entries from the GOsa support queue. 
    @param  Array The IDs to remove.
    @return Boolean True on success.
    */
    public function remove_entries($ids)
    {
        if(!is_array($ids)){
          trigger_error("Requires an array as parameter.");
          return;
        }
        
        try {
            $client = new jsonRPCClient($this->s_host);
            $entries = $client->remove_entries($ids);
        } catch (Exception $e) {
            $this->set_error(nl2br($e->getMessage()));
            return(FALSE);
        }
        $this->reset_error();
        return(TRUE);
    }

    /*! \brief  Removes an entry from the GOsa support queue. 
    @param  Integer The ID of the entry we want to remove.
    @return Boolean True on success.
    */
    public function remove_entry($id)
    {
        return($this->remove_entries(array($id)));
    }
    
    /*! \brief  Returns an entry from the gosaSupportQueue
    @param  Integer The ID of the entry we want to return.
    @return Array   Of the requested entry. 
    */
    public function get_entry_by_id($id)
    {
        return $this->get_entries_by_id(array($id));
    }
    
    /*! \brief  Updates an entry with a set of new values, 
    @param  Integer The ID of the entry, we want to update.
    @param  Array   The variables to update.   
    @return Boolean Returns TRUE on success. 
    */
    public function update_entries($ids,$data)
    {
        if(!is_array($ids)){
          trigger_error("Requires an array as first parameter.");
          return;
        }

        if(!is_array($data)){
          trigger_error("Requires an array as second parameter.");
          return;
        }
        
        // won't be implemented
    }
    
    public function process_entries_now($ids)
    {
        if(!is_array($ids)){
          trigger_error("Requires an array as first parameter.");
          return;
        }
        
        try {
            $client = new jsonRPCClient($this->s_host);
            $entries = $client->process_entries_now($ids);
        } catch (Exception $e) {
            $this->set_error(nl2br($e->getMessage()));
            return(FALSE);
        }
        $this->reset_error();
        return(TRUE);
    }

    public function is_currently_installing($mac) {
        $e_types = DaemonEvent::get_event_types(USER_EVENT | SYSTEM_EVENT | HIDDEN_EVENT);
        
        $evts = $this->get_entries_by_mac(array($mac));
        foreach($evts as $evt) {
            if(isset($e_types['QUEUED'][$evt['HEADERTAG']]) && $evt['STATUS'] == "processing" && 
                in_array($e_types['QUEUED'][$evt['HEADERTAG']],array("DaemonEvent_reinstall","DaemonEvent_update"))) {
                // Why do we check HEADERTAG? processing status should be sufficient.
                return(TRUE);
            }
        }
        return(FALSE);
    }
    
    public function get_entries_by_mac($macs) {
        return $this->get_entries("get_entries_by_mac",$macs);
    }
    
    private function get_entries($func,$address) {
        if(!is_array($address)){
          trigger_error("Requires an array as parameter.");
          return;
        }
        
        try {
            $client = new jsonRPCClient($this->s_host);
            $entries = $client->$func($address);
        } catch (Exception $e) {
            $this->set_error(nl2br($e->getMessage()));
            return(FALSE);
        }
        $entries = $this->format_entries($entries);
        $this->reset_error();
        return($entries);
    }
    
    /*! \brief  Returns log file informations for a given mac address 
      @param  $mac The mac address to fetch logs for.
      @retrun Array A Multidimensional array containing log infos.
        MAC_00_01_6C_9D_B9_FA['install_20080311_090900'][0]=debconf.log
        MAC_00_01_6C_9D_B9_FA['install_20080311_090900'][1]=syslog.log
                               install_20080313_144450   ...
    */
    public function get_log_info_for_mac($mac)
    {
        // FIXME : this function needs to look into logs on the local disk
        return array();
    }
    
    /*! \brief  Returns true if an argonaut daemon is running on the pc
      @param  $mac The mac address to ping
    */
    public function ping($mac) {
        try {
            $client = new jsonRPCClient($this->s_host);
            $this->reset_error();
            return($client->ping($mac));
        } catch (Exception $e) {
            $this->set_error(nl2br($e->getMessage()));
            return(FALSE);
        }
    }
};

class gosaSupportDaemon
{
  private $s_host       = "";
  private $i_port       = 0;
  private $s_encryption_key = "";

  private $o_sock       = NULL;
  private $f_timeout    = 2;
  private $s_error      = "";
  private $b_error      = FALSE;

  private $is_connected   = FALSE;
  private $is_configured  = FALSE;
  protected $use_alternative_xml_parse_method = FALSE;

  public function get_host()
  {
    return($this->s_host);
  }

  public function get_port()
  {
    return($this->i_port);
  }
 
  /*! \brief  Creates a new gosaSupportDaemon object.
    @param string   Host    The Host where the daemon is running on.  
    @param integer  Port    The port which the daemon use.
    @param string   Key     The encryption string.
    @param boolean  Connect Directly connect to daemon socket.
    @param float    Timeout The timelimit for all socket actions.
   */
  public function __construct($connect=TRUE,$timeout=null)
  {
/*
      debug_print_backtrace();
*/
    #FIXME: bad idea about referencing global variables from within classes
    global $config;
    if(!isset($config) || !$config){
      $config = session::global_get('config');
    }

    // Detect timeout 
    if($timeout == null){
      $timeout = $config->get_cfg_value("goto-si-Timeout",15);
    }

    /* This should only be the case if we call this from setup.
        __autoload() 
     */
    if(!is_object($config)) { return; }

    # load from config, store statically
    if ($config->get_cfg_value("goto-si-URI") != ""){

      if ($this->s_host == ""){
        $this->s_host= preg_replace("/^.*@([^:]+):.*$/", "$1", $config->get_cfg_value("goto-si-URI"));
        $this->i_port= preg_replace("/^.*@[^:]+:(.*)$/", "$1", $config->get_cfg_value("goto-si-URI"));
        $this->s_encryption_key = preg_replace("/^(.*)@[^:]+:.*$/", "$1", $config->get_cfg_value("goto-si-URI"));
      }
      $this->is_configured = TRUE;
      $this->f_timeout = $timeout;
      if($connect){
        $this->connect();
      }
    }
  }


  public function is_configured()
  {
    return($this->is_configured);
  }


  /*! \brief  Establish daemon connection. 
    @return boolean Returns true if the connection was succesfully established. 
   */
  public function connect()
  {
    if(!empty($this->s_host) && !empty($this->i_port)){
      $this->o_sock = new Socket_Client($this->s_host,$this->i_port,TRUE,$this->f_timeout);
      if($this->o_sock->connected()){ 
        $this->o_sock->setEncryptionKey($this->s_encryption_key); 
        $this->is_connected = TRUE;
      }else{
        $this->set_error($this->o_sock->get_error());
        $this->disconnect();
        new log("debug","gosaSupportDaemon::connect()", "Cannot connect to si-server", array(),$this->get_error());
      }
    }else{
      $this->set_error(msgPool::cmdnotfound("goto-si-URI",_("Service GOto-si")));
    }
    return($this->is_connected);
  }

  
  /*! \brief  Returns TRUE whether we are connected or not 
      @return BOOLEAN  Returns TRUE when connected else FALSE
   */
  public function is_connected()
  {
    return($this->is_connected);
  }
 

  /*! \brief  */
  public function get_hosts_with_module($mod)
  {
    $data = array("module_name" => $mod);
    $res = $this->send_data("gosa_get_hosts_with_module",$this->s_host.":".$this->i_port,$data,TRUE);
    $hosts = array();
    if(isset($res['XML'][0])){
      foreach($res['XML'][0] as $name => $data){
        if(preg_match("/^ANSWER[0-9]*$/",$name)){
          if(isset($data[0]['MAC'][0]['VALUE']) && $data[0]['MAC'][0]['VALUE'] != ""){
            $hosts[] = $data[0]['MAC'][0]['VALUE'];
          } elseif(isset($data[0]['IP'][0]['VALUE']) && $data[0]['IP'][0]['VALUE'] != "") {
            $hosts[] = $data[0]['IP'][0]['VALUE'];
          }
        }
      }
    }

    if(count($hosts) == 0){
      @DEBUG(DEBUG_SI, __LINE__, "<b>".__CLASS__."::".__FUNCTION__."</b>" , 
        __FILE__, "<font color='red'><i>Found: 0</i></font>", $info=$mod);
    }else{
      @DEBUG(DEBUG_SI, __LINE__, "<b>".__CLASS__."::".__FUNCTION__."</b>" , 
        __FILE__, "<i>Found: ".count($hosts)."</i>", $info=$mod);
    }

    return($hosts);
  }


  /*! \brief  Disconnect from gosa daemon.
   */
  public function disconnect()
  {
    $this->o_sock->close();
    $this->is_connected = FALSE;
  }


  /*! \brief  Sets an error message, which can be returned with get_error().
    @param  string  The Error message,
   */
  private function set_error($str)
  {
    /******
      Debug handling
     ******/
    $debug = debug_backtrace();
    $file = __FILE__;
    $function = __FUNCTION__;
    $line = __LINE__;
    $class = __CLASS__;
    foreach($debug as $info){
      if(!in_array($info['function'],array("send_data","_send","set_error","connect"))){
        $file = $info['file'];
        $line = $info['line'];
        $class = get_class($this);
        $function = $info['function'];
        break;
      }
    }
    @DEBUG(DEBUG_SI, $line, "<b>".$class."::".$function."</b>" , $file, "<font color='red'><i>".htmlentities($str)."</i></font>", $info="");

    /******
      Set error string.
     ******/
  
    $this->b_error = TRUE;
    $this->s_error = $str;
  }


  /*! \brief  Sets an error message, which can be returned with get_error().
    @param  string  The Error message,
   */
  private function reset_error()
  {
    $this->b_error = FALSE;
    $this->s_error = "";
  }


  /*! \brief  Checks if an error occured.
    @return boolean returns TRUE or FALSE, whether there is an error or not.
   */
  public function is_error()
  {
    return($this->b_error);
  }


  /*! \brief  Returns the last error. 
    @return Returns the last error.
   */
  public function get_error()
  {
    $str = $this->s_error;
    $ret = "";
    if(is_string($str)){
      $ret = $str;
    }else{
      foreach($str as $msg){
        $ret .= $msg." ";
      }
    }
    $ret = str_replace(" ","&nbsp;",$ret);
    return($ret);
  }


  public function FAI_get_kernels($release)
  {
    $xml_msg = 
      "<xml>".
      "<header>gosa_get_available_kernel</header>".
      "<source>GOSA</source>".
      "<target>GOSA</target>".
      "<fai_release>".$release."</fai_release>".
      "</xml>";

    $ret = array();
    if($this->connect()){
      $entries = $this->_send($xml_msg,TRUE);

      /* Check if returned values represent a valid answer */
      if(isset($entries['XML']) && is_array($entries['XML'])){
        if(isset($entries['XML'])){
          $ret = $entries['XML'];
          foreach($ret as $key => $entry){
            if(!preg_match("/^answer/i",$key)){
              unset($ret[$key]);
            }
          }
        }
      }
    }
    return($ret);
  }


  public function FAI_get_package_sections($release)
  {
    $xml_msg = "<xml><header>gosa_query_packages_list</header><target>GOSA</target><source>GOSA</source>".
      "<select>distinct section</select>".
      "<where><clause><phrase><distribution>".$release."</distribution></phrase></clause></where></xml>";

    $ret = array();
    if($this->connect()){
      $entries = $this->_send($xml_msg,TRUE);
      if(isset($entries['XML']) && is_array($entries['XML'])){

        /* Unset header tags */
        foreach(array("HEADER","SOURCE","TARGET","SESSION_ID") as $type){
          if(isset($entries['XML'][$type])){
            unset($entries['XML'][$type]);
          }
        }
        $ret = $entries['XML'];
      }
    }
    return($ret);
  }


  public function FAI_get_packages($release,$attrs,$package,$from=-1,$to=-1)
  {
    $ret = array();

    /* Check Parameter */
    if(!is_array($attrs) || !count($attrs)){
      trigger_error("Second parameter must be an array. With at least one attribute name.");
      return($ret);
    }

    /* Check Parameter */
    if(!is_array($package)){
      trigger_error("Third parameter must be an array. With at least one attribute name.");
      return($ret);
    }

    /* Create list of attributes to fetch */
    $attr = ""; 
    foreach($attrs as $at){
      $attr.= "<select>".$at."</select>";
    }

    /* If no package is given, search for all */
    if(!count($package)) $package = array("%");

    /* Create limit tag */
    if($from == -1){
      $limit =""; 
    }else{
      $limit = "<limit><from>".$from."</from><to>".$to."</to></limit>";
    }

    /* Create list of attributes to fetch */
    $pkgs = ""; 
    foreach($package as $pkg){
      $pkgs .="<phrase><operator>like</operator><package>".$pkg."</package></phrase>";
    }

    $xml_msg = "<xml><header>gosa_query_packages_list</header><target>GOSA</target><source>GOSA</source>".
      $attr.
      "<where>
      <clause><phrase><distribution>".$release."</distribution></phrase></clause>
      <clause><connector>OR</connector>
      ".$pkgs."
      </clause>
      </where>".
      $limit.
      "</xml>";

    if($this->connect()){
      $entries = $this->_send($xml_msg,TRUE);
      if(isset($entries['XML']) && is_array($entries['XML'])){

        /* Check if returned values represent a valid answer */
        if(isset($entries['XML'])){

          /* Unset header tags */
          foreach(array("HEADER","SOURCE","TARGET","SESSION_ID") as $type){
            if(isset($entries['XML'][$type])){
              unset($entries['XML'][$type]);
            }
          }
          $ret = $entries['XML'];
        }
      }
    }
    return($ret);

    
  }


  public function FAI_get_server($name = "")
  {

    $xml_msg = "<xml><header>gosa_query_fai_server</header><target>GOSA</target><source>GOSA</source></xml>";
    $ret = array();
    if($this->connect()){

      /* Check if returned values represent a valid answer */
      $entries = $this->_send($xml_msg,TRUE);
      if(isset($entries['XML']) && is_array($entries['XML'])){

        /* Unset header tags */
        foreach(array("HEADER","SOURCE","TARGET","SESSION_ID") as $type){
          if(isset($entries['XML'][$type])){
            unset($entries['XML'][$type]);
          }
        }
        $ret = $entries['XML']; 
      }
    }
    return($ret);
  }


  public function FAI_get_classes($name)
  {
    $xml_msg = "<xml><header>gosa_query_fai_release</header><target>GOSA</target><source>GOSA</source>".
                  "<where><clause><phrase><fai_release>".$name."</fai_release></phrase></clause></where></xml>";;
    $ret = array();
    if($this->connect()){

      $entries = $this->_send($xml_msg,TRUE);
      if(isset($entries['XML']) && is_array($entries['XML'])){

        /* Unset header tags */
        foreach(array("HEADER","SOURCE","TARGET","SESSION_ID") as $type){
          if(isset($entries['XML'][$type])){
            unset($entries['XML'][$type]);
          }
        }
        $ret = $entries['XML']; 
      }
    }
    return($ret);
  }


  /*! \brief  Returns an array containing all queued entries.
    @return Array All queued entries as an array.
   */
  public function get_queued_entries($event_types = array("*"),$from=-1,$to=-1,$sort="timestamp DESC")
  {
/*
    $ret = array();
*/

/*
    debug_print_backtrace();
    * filter goto events
*/
    timezone::get_default_timezone();
    
    try {
        $client = new jsonRPCClient($this->s_host);
        $entries = $client->get_entries();
    } catch (Exception $e) {
        $this->set_error(nl2br($e->getMessage()));
        return(FALSE);
    }
    foreach($entries as &$entry) {
        $entry['TIMESTAMP'] = DaemonEvent::_timestamp_to_event($entry['data']['timestamp']);
        $entry['MACADDRESS'] = $entry['targets'][0];
        $entry['PERIODIC'] = $entry['data']['periodic'];
        $entry['STATUS'] = $entry['status'];
    }
    $this->reset_error();
    return($entries);
    
    /*
    $tags = "";
    foreach($event_types as $type){
      $tags .= "<phrase><headertag>".$type."</headertag></phrase>";
    }
    if(count($event_types) > 1){
      $tags = "<connector>or</connector>".$tags;
    }
    if(count($event_types)){
      $tags = "<where><clause>".$tags."</clause></where>";
    }

    $xml_msg = 
      "<xml>
      <header>gosa_query_jobdb</header>
      <target>GOSA</target>
      <source>GOSA</source>
      ".$tags."

      <orderby>".$sort."</orderby>";
    if($from != -1 && $to != -1){
      $xml_msg.= "
        <limit>
        <from>".$from."</from>
        <to>".$to."</to>
        </limit>";
    }
    $xml_msg.= "
      </xml>";
      */

/*
    if($this->connect()){
      $entries = $this->_send($xml_msg,TRUE);
      if(isset($entries['XML']) && is_array($entries['XML'])){

        // Unset header tags
        foreach(array("HEADER","SOURCE","TARGET","SESSION_ID") as $type){
          unset($entries['XML'][$type]);
        }
        $ret = $entries['XML']; 
      }
    }
*/

    
    return($ret);
  }


  /*! \brief  Checks if the given ids are used queue ids.
    @param  Array   The ids we want to check..
    @return Array   An array containing all ids as index and TRUE/FALSE as value. 
   */
  public function ids_exist($ids)
  {
    if(!is_array($ids)){
      trigger_error("Requires an array as parameter.");
      return;
    }

    $ret = array();

    $xml_msg = "<xml>
      <header>gosa_query_jobdb</header>
      <target>GOSA</target>
      <source>GOSA</source>
      <where>
      <clause>
      <connector>or</connector>";
    foreach($ids as $id){
      $xml_msg .= "<phrase>
        <operator>eq</operator>
        <id>".$id."</id>
        </phrase>";
    }
    $xml_msg .= "</clause>
      </where>
      </xml>";

    if($this->connect()){
      $entries = $this->_send($xml_msg,TRUE);
      if(isset($entries['XML']) && is_array($entries['XML'])){
        foreach($entries['XML'] as $entry){
          if(is_array($entry) && array_key_exists("ID",$entry)){
            $ret[] = $entry['ID'];
          }
        }
      }
    }
    return($ret);
  }


  /*! \brief  Returns an entry containing all requested ids.
    @param  Array   The IDs of the entries we want to return.
    @return Array   Of the requested entries. 
   */
  public function get_entries_by_mac($macs)
  {
    if(!is_array($macs)){
      trigger_error("Requires an array as parameter.");
      return;
    }

    $ret = array();

    $xml_msg = "<xml>
      <header>gosa_query_jobdb</header>
      <target>GOSA</target>
      <source>GOSA</source>
      <where>
      <clause>
      <connector>or</connector>";
    foreach($macs as $mac){
      $xml_msg .= "<phrase>
        <operator>eq</operator>
        <macaddress>".$mac."</macaddress>
        </phrase>";
    }
    $xml_msg .= "</clause>
      </where>
      </xml>";

    if($this->connect()){
      $entries = $this->_send($xml_msg,TRUE);
      if(isset($entries['XML'])){
        foreach($entries['XML'] as $name => $entry){
          if(preg_match("/^ANSWER[0-9]*$/",$name)){
            $ret[$name] = $entry;
          }
        }
      }
    }
    return($ret);
  }


  /*! \brief  Returns an entry containing all requested ids.
    @param  Array   The IDs of the entries we want to return.
    @return Array   Of the requested entries. 
   */
  public function get_entries_by_id($ids)
  {
    if(!is_array($ids)){
      trigger_error("Requires an array as parameter.");
      return;
    }

    $ret = array();

    $xml_msg = "<xml>
      <header>gosa_query_jobdb</header>
      <target>GOSA</target>
      <source>GOSA</source>
      <where>
      <clause>
      <connector>or</connector>";
    foreach($ids as $id){
      $xml_msg .= "<phrase>
        <operator>eq</operator>
        <id>".$id."</id>
        </phrase>";
    }
    $xml_msg .= "</clause>
      </where>
      </xml>";

    if($this->connect()){
      $entries = $this->_send($xml_msg,TRUE);
      if(isset($entries['XML'])){
        foreach($entries['XML'] as $name => $entry){
          if(preg_match("/^ANSWER[0-9]*$/",$name)){
            $ret[$name] = $entry;
          }
        }
      }
    }
    return($ret);
  }


  /*! \brief  Checks if the given id is in use.
    @param  Integer The ID of the entry.
    @return Boolean TRUE if entry exists. 
   */
  public function id_exists($id)
  {
    if(!is_numeric($id)){
      trigger_error("Requires an integer as parameter.");
      return;
    }


    $xml_msg = "<xml>
      <header>gosa_query_jobdb</header>
      <target>GOSA</target>
      <source>GOSA</source>
      <where>
      <clause>
      <phrase>
      <operator>eq</operator>
      <id>".$id."</id>
      </phrase>
      </clause>
      </where>
      </xml>";

    if($this->connect()){
      $entries = $this->_send($xml_msg,TRUE);
      if( isset($entries['XML']['HEADER']) && 
          $entries['XML']['HEADER']=="answer" && 
          isset($entries['XML']['ANSWER1'])){
        return(TRUE);
      }
    }
    return(FALSE);
  }


  /*! \brief  Returns an entry from the gosaSupportQueue
    @param  Integer The ID of the entry we want to return.
    @return Array   Of the requested entry. 
   */
  public function get_entry_by_id($id)
  {
    if(!is_numeric($id)){
      trigger_error("Requires an integer as parameter.");
      return;
    }
  
    $ret = array();
    $xml_msg = "<xml>
      <header>gosa_query_jobdb</header>
      <target>GOSA</target>
      <source>GOSA</source>
      <where>
      <clause>
      <phrase>
      <operator>eq</operator>
      <id>".$id."</id>
      </phrase>
      </clause>
      </where>
      </xml>";
    if($this->connect()){
      $entries = $this->_send($xml_msg,TRUE);
      if( isset($entries['XML']['HEADER']) &&
          $entries['XML']['HEADER']=="answer" &&
          isset($entries['XML']['ANSWER1'])){
        $ret = $entries['XML']['ANSWER1'];
      }
    }
    return($ret);
  }


  /*! \brief  Removes a set of entries from the GOsa support queue. 
    @param  Array The IDs to remove.
    @return Boolean True on success.
   */
  public function remove_entries($ids)
  {
    if(!is_array($ids)){
      trigger_error("Requires an array as parameter.");
      return;
    }


    $ret = array();

    $xml_msg = "<xml>
      <header>gosa_delete_jobdb_entry</header>
      <target>GOSA</target>
      <source>GOSA</source>
      <where>
      <clause>
      <connector>or</connector>";
    foreach($ids as $id){
      $xml_msg .= "<phrase>
        <operator>eq</operator>
        <id>".$id."</id>
        </phrase>";
    }
    $xml_msg .= "</clause>
      </where>
      </xml>";

    if($this->connect()){
      $entries = $this->_send($xml_msg,TRUE);
      if(isset($entries['XML']) || isset($entries['COUNT'])){
        new log("debug","DaemonEvent (IDS) ", "gosaSupportDaemon::remove_entries()", $ids,"SUCCESS");
        return(TRUE);
      }else{
        new log("debug","DaemonEvent (IDS) ", "gosaSupportDaemon::remove_entries()", $ids,"FAILED ".$this->get_error());
      }
    }
    return(FALSE);
  }



  /*! \brief  Removes an entry from the GOsa support queue. 
    @param  Integer The ID of the entry we want to remove.
    @return Boolean True on success.
   */
  public function remove_entry($id)
  {
    return($this->remove_entries(array($id)));
  }


  /*! \brief  Parses the given xml string into an array 
    @param  String XML string  
    @return Array Returns an array containing the xml structure. 
   */
  private function xml_to_array($xml,$alternative_method = FALSE)
  {
    $params = array();
    $level = array();
    $parser  = xml_parser_create_ns();
    xml_parse_into_struct($parser, $xml, $vals, $index);

    $err_id = xml_get_error_code($parser);
    if($err_id){
      xml_parser_free($parser);
    }else{
      xml_parser_free($parser);

      if($this->use_alternative_xml_parse_method) {
        $params = $this->build_xml_array($vals);
      } else {

        foreach ($vals as $xml_elem) {
          if ($xml_elem['type'] == 'open') {
            if (array_key_exists('attributes',$xml_elem)) {
              list($level[$xml_elem['level']],$extra) = array_values($xml_elem['attributes']);
            } else {
              $level[$xml_elem['level']] = $xml_elem['tag'];
            }
          }
          if ($xml_elem['type'] == 'complete') {

            $start_level = 1;
            $test2 = &$params;
            while($start_level < $xml_elem['level']) {
              $test2 = &$test2[$level[$start_level]];
              $start_level++;
            }

            /* Save tag attributes too. 
               e.g. <tag attr="val">
             */
            if(isset($xml_elem['attributes'])){
              foreach($xml_elem['attributes'] as $name => $value){
                $test2['ATTRIBUTES'][$name] = $value;
              }
            }

            if(!isset($test2[$xml_elem['tag']])){
              if(isset($xml_elem['value'])){
                $test2[$xml_elem['tag']] = $xml_elem['value'];
              }
            }else{
              if(!is_array($test2[$xml_elem['tag']])){
                $test2[$xml_elem['tag']] = array($test2[$xml_elem['tag']]);
              }
              $test2[$xml_elem['tag']][] = $xml_elem['value'];
            }
          }
        }
      }
    }

    if(!isset($params['XML'])){
      if (!array_key_exists('XML', $params)){
        $this->set_error(_("Cannot not parse XML!"));
      }
      $params = array("COUNT" => 0);
    }

    return($params); 
  }


  function build_xml_array(&$vals)
  {
    $array = array();
    while(count($vals)){
      $key = key($vals);
      $val = $vals[$key];
      unset($vals[$key]);
      if($val['type'] == "close"){
        return($array);
      }elseif($val['type']=="open"){
        $array[$val['tag']][] = $this->build_xml_array($vals);
      }elseif($val['type'] != "cdata"){
        $data = array("VALUE" => "","ATTRIBUTES" => "");
        foreach(array("value" => "VALUE", "attributes" => "ATTRIBUTES") as $name => $attr){
          if(isset($val[$name])){
            $data[$attr] = $val[$name];
          }
        }
        $array[$val['tag']][] = $data;
      }else{
#print_a($val);
      }
    }
    return($array);
  }






  /*! \brief  Updates an entry with a set of new values, 
    @param  Integer The ID of the entry, we want to update.
    @param  Array   The variables to update.   
    @return Boolean Returns TRUE on success. 
   */
  public function update_entries($ids,$data)
  {
    if(!is_array($ids)){
      trigger_error("Requires an array as first parameter.");
      return;
    }

    if(!is_array($data)){
      trigger_error("Requires an array as second parameter.");
      return;
    }

    $attr = "";
    foreach($data as $key => $value){
      $key = strtolower($key);
      if(is_array($value)){
        foreach($value as $sub_value){
          $attr.= "<$key>".strtolower($sub_value)."</$key>\n";
        }
      }else{
        $attr.= "<$key>".strtolower($value)."</$key>\n";
      }
    }

    $xml_msg = "<xml>
      <header>gosa_update_status_jobdb_entry</header>
      <target>GOSA</target>
      <source>GOSA</source>
      <where>
      <clause>
      <connector>or</connector>";
    foreach($ids as $id){
      $xml_msg .= "<phrase>
        <operator>eq</operator>
        <id>".$id."</id>
        </phrase>";
    }
    $xml_msg .= "</clause>
      </where>
      <update>
      ".$attr." 
      </update>
      </xml>";

    if($this->connect()){
      $entries = $this->_send($xml_msg,TRUE);
      if(isset($entries['XML'])){
        if(isset($entries['XML']['ERROR_STRING'])) {
          $this->set_error($entries['XML']['ERROR_STRING']);
          new log("debug","DaemonEvent (IDS) ", "gosaSupportDaemon::update_entries()", $ids,"FAILED setting (".$attr.") error was ".$this->get_error());
          return(FALSE);
        }
        new log("debug","DaemonEvent (IDS) ", "gosaSupportDaemon::update_entries()", $ids,"SUCCESS");
        return(TRUE);
      }
    }
    return(FALSE);
  }


  /*! \brief  Returns the number of currently queued objects.
      @return Integer  
   */
  public function number_of_queued_entries($event_types)
  {
    $tags = "";
    foreach($event_types as $type){
      $tags .= "<phrase><headertag>".$type."</headertag></phrase>";
    }
    if(count($event_types) > 1){
      $tags = "<connector>or</connector>".$tags;
    }
    if(count($event_types)){
      $tags = "<where><clause>".$tags."</clause></where>";
    }


    $xml_msg =
      "<xml>".
      "<header>gosa_query_jobdb</header>".
      "<target>GOSA</target>".
      "<source>GOSA</source>".
      "<select> count ID</select>".
      $tags.
      "</xml>";

    $xml_msg ="<xml><header>gosa_count_jobdb</header><target>GOSA</target><source>GOSA</source></xml>";
    $this->connect();
    if($this->connect()){
      $entries = $this->_send($xml_msg,TRUE);
      if($this->o_sock->is_error()){
        $this->set_error($this->o_sock->get_error());
        return(0);
      }
      if(isset($entries['XML'])){
        return($entries['XML']['COUNT']);
      }
    }
    return(-1);
  } 


  public function send_data($header, $to, $data= array(), $answer_expected = FALSE)
  {
    $xml_message= "";

    /* Prepare data */
    foreach ($data as $key => $value){
      if(is_array($value)){
        foreach($value as $sub_value){
          $xml_message.= "<$key>$sub_value</$key>";
        }
      }else{
        $xml_message.= "<$key>$value</$key>";
      }
    }

    /* Multiple targets? */
    if (!is_array($to)){
      $to_targets= array($to);
    } else {
      $to_targets= $to;
    }

    /* Build target strings */
    $target ="";
    foreach($to_targets as $to){
      $target.= "<target>$to</target>";
    }

    return $this->_send("<xml><header>$header</header><source>GOSA</source>$target".$xml_message."</xml>",$answer_expected);
  }


  /* Allows simply appending a new DaemonEvent 
   */
  public function append($event, $skip_add_mac = FALSE)
  {
    if(!($event instanceof DaemonEvent)){
      return(FALSE);
    }
  

    /* Add to queue if new 
     */
    if($event->is_new()){

      $request_answer = FALSE;
      if($event->get_type() == SCHEDULED_EVENT){
        $action = $event->get_schedule_action();
      }elseif($event->get_type() == TRIGGERED_EVENT){
        $action = $event->get_trigger_action();
      }else{
        trigger_error("Unknown type of queue event given.");
        return(FALSE);
      }
      

		/* Get event informations, like targets..
		*/
		$targets    = $event->get_targets();
		$data       = $event->save();

		/* Append an entry
		*/
        try {
			$client = new jsonRPCClient('http://10.10.11.100:8080');//$s_host.":".$i_port
			$client->action($action,$targets,$data);
		} catch (Exception $e) {
			$this->set_error(nl2br($e->getMessage()));
			return(FALSE);
		}

/*      
 * 	foreach($targets as $target){
        if(!$skip_add_mac){
          $data['macaddress'] = $target;
        }
        $this->send_data($action,$target,$data,$request_answer);
*/

/*
        if($this->is_error()){
          return(FALSE);
        }
      }
*/
      return(TRUE);
    }else{

      /* Updated edited entry.
       */
      $id                 = $event->get_id();
      $data               = $event->save();
      return($this->update_entries(array($id),$data));
    }

    return(FALSE);
  }


  /*! \brief  Returns an array containing all queued entries.
    @return Array All queued entries as an array.
   */
  public function _send($data, $answer_expected= FALSE)
  {

    $ret = array();
    if(!$this->connect()){
      return($ret);
    }
  
    $this->reset_error();

    /******
      Debug handling
     ******/
    $debug = debug_backtrace();
    $file = __FILE__;
    $function = __FUNCTION__;
    $line = __LINE__;
    $class = __CLASS__;
    foreach($debug as $info){
      if(!in_array($info['function'],array("send_data","_send"))){
        $file = $info['file'];
        $line = $info['line'];
        $class = get_class($this);
        $function = $info['function'];
        break;
      }
    }
    @DEBUG(DEBUG_SI, $line, "<b>".$class."::".$function."</b>" , $file, "<i>".htmlentities($data)."</i>", $info="");

    $start = microtime(1);

    /*******
      Start sending data 
     *******/
    if($this->connect()){
      $this->o_sock->write($data);
      if ($answer_expected){
        $str = trim($this->o_sock->read());

        /* Check if something went wrong while reading */
        if($this->o_sock->is_error()){
          $this->set_error($this->o_sock->get_error());
          @DEBUG(DEBUG_SI, $line, "<b>".$class."::".$function."</b>" , $file, sprintf('%.7f', microtime(1) - $start) , "<b>FAILED Duration:</b>");
          return($ret);
        }

        $entries = $this->xml_to_array($str);
        if(isset($entries['XML']) && is_array($entries['XML'])){
          $ret = $entries;
          if($this->use_alternative_xml_parse_method) {

            // --------- Seems broken, check for 'ERROR' but using 'ERROR_STRING'
            if(isset($entries['XML'][0]['ERROR'][0]['VALUE']) && $entries['XML'][0]['ERROR'][0]['VALUE'] == "1"){
              $this->set_error($entries['XML'][0]['ERROR_STRING'][0]['VALUE']);
              new log("debug","DaemonEvent (IDS) ", "gosaSupportDaemon::_send()", 
                  array($data=>$data),"FAILED ".$this->get_error());
            // ---------
      
          
            }elseif(isset($entries['XML'][0]['ERROR'][0]['VALUE'])){
              $this->set_error($entries['XML'][0]['ERROR'][0]['VALUE']);
              new log("debug","DaemonEvent (IDS) ", "gosaSupportDaemon::_send()", 
                  array($data=>$data),"FAILED ".$this->get_error());
            }
          }else{
            if(isset($entries['XML']['ERROR_STRING'])) {
              $this->set_error($entries['XML']['ERROR_STRING']);
              new log("debug","DaemonEvent (IDS) ", "gosaSupportDaemon::_send()", 
                  array($data=>$data),"FAILED ".$this->get_error());
            }elseif(isset($entries['XML']['ERROR'])){
              $this->set_error($entries['XML']['ERROR']);
              new log("debug","DaemonEvent (IDS) ", "gosaSupportDaemon::_send()", 
                  array($data=>$data),"FAILED ".$this->get_error());
            }
          }
          new log("debug","DaemonEvent (IDS) ", "gosaSupportDaemon::_send()", 
              array($data=>$data),"SUCCESS");
        }
      }else{
        new log("debug","DaemonEvent (IDS) ", "gosaSupportDaemon::_send()", 
            array($data=>$data),"Fire & forget, not result.! ".$this->get_error());
      }
    }

    @DEBUG(DEBUG_SI, $line, "<b>".$class."::".$function."</b>" , $file, sprintf('%.7f', microtime(1) - $start) , "<b>Duration:</b>");
    return($ret);
  }


  static function send($header, $to, $data= array(), $answer_expected = FALSE)
  {
    $xml_message= "";

    /* Get communication object */
    $d= new gosaSupportDaemon(TRUE,10);

    /* Prepare data */
    foreach ($data as $key => $value){
      if(is_array($value)){
        foreach($value as $sub_val){
          $xml_message.= "<$key>$sub_val</$key>";
        }
      }else{
        $xml_message.= "<$key>$value</$key>";
      }
    }

    /* Multiple targets? */
    if (!is_array($to)){
      $to_targets= array($to);
    } else {
      $to_targets= $to;
    }

    /* Build target strings */
    $target ="";
    foreach($to_targets as $to){
      $target.= "<target>$to</target>";
    }

    return $d->_send("<xml><header>$header</header><source>GOSA</source>$target".$xml_message."</xml>",$answer_expected);
  }


  /*! \brief  Removes all jobs from the queue that are tiggered with a specific macAddress.
      @param  String  $mac  The mac address for which we want to remove all jobs.      
   */
  function clean_queue_from_mac($mac)
  {
    global $config;
    if(!isset($config) || !$config){
      $config = session::global_get('config');
    }

    /* First of all we have to check which jobs are startet 
     *  for $mac 
     */
    $xml_msg ="<xml><header>gosa_query_jobdb</header><target>GOSA</target><source>GOSA</source><where><clause><phrase><macaddress>".$mac."</macaddress></phrase></clause></where></xml>";  
    
    new log("debug","DaemonEvent ", "gosaSupportDaemon::clean_queue_from_mac()", array($mac => $mac)," start cleaning.");
 
    $data = $this->_send($xml_msg,TRUE);
    if(is_array($data) && isset($data['XML'])){
      $already_aborted = FALSE;
      foreach($data['XML']  as $name => $entry){
        if(preg_match("/answer[0-9]*/i",$name)){
          $entry['STATUS'] = strtoupper($entry['STATUS']);
          switch($entry['STATUS']){

            case 'PROCESSING' :

              /* Send abort event, but only once 
               */
              if($already_aborted){
                break;
              }elseif(class_available("DaemonEvent_faireboot")){
                $already_aborted = TRUE;
                $tmp = new DaemonEvent_faireboot($config);
                $tmp->add_targets(array($mac));
                $tmp->set_type(TRIGGERED_EVENT);
                if(!$this->append($tmp)){
                  msg_dialog::display(_("Error"), sprintf(_("Cannot send abort event for entry %s!"),$entry['ID']) , ERROR_DIALOG);
                  new log("debug","DaemonEvent ", "gosaSupportDaemon::clean_queue_from_mac()", array($mac => $mac),
                      "FAILED, could not send 'DaemonEvent_faireboot' for entry ID (".$entry['ID'].") - ".$this->get_error());
                }else{
                  new log("debug","DaemonEvent ", "gosaSupportDaemon::clean_queue_from_mac()", array($mac => $mac),
                      "SUCCESS, send 'DaemonEvent_faireboot' for entry ID (".$entry['ID'].")");
                }
                ;break;
              }else{
                /* Couldn't find abort event, just remove entry */
              }

            case 'WAITING':
            case 'ERROR':
            default :
            
              /* Simply remove entries from queue. 
               *  Failed or waiting events, can be removed without any trouble.
               */ 
              if(!$this->remove_entries(array($entry['ID']))){
                msg_dialog::display(_("Error"), sprintf(_("Cannot remove entry %s!"),$entry['ID']) , ERROR_DIALOG);
              }
              ;break;
          }
    
        }
      }
    }
  }


  static function ping($target)
  {
    if (tests::is_mac($target)){
      /* Get communication object */
      $d= new gosaSupportDaemon(TRUE,2);
      $answer= $d->_send("<xml><header>gosa_ping</header><source>GOSA</source><target>$target</target></xml>", TRUE);
      return (count($answer) ? TRUE:FALSE);
    }
    return (FALSE);
  }



  /*! \brief  Returns a list of all configured principals. 
              (Uses the GOsa support daemon instead of the ldap database.)
      @return Array  A list containing the names of all configured principals.
   */
  public function krb5_list_principals($server)
  {
    $res = array();  

    /* Check if the given server is a valid mac address
     */
    if(!tests::is_mac($server)){
      trigger_error("The given server address '".$server."' is invalid, it must be a valid mac address");
      return($ret);
    }

    /* Prepare request event 
     */ 
    $xml_msg = 
      "<xml>".
      "<header>gosa_krb5_list_principals</header>".
      "<source>GOSA</source>".
      "<target>".$server."</target>".
      "</xml>";
    
    $tmp = $this->_send($xml_msg,TRUE);
    if(isset($tmp['XML']['PRINCIPAL'])){
      return($tmp['XML']['PRINCIPAL']);
    }else{
      return($res);
    }
  }


  /*! \brief  Returns the configuration settings for a given principal name. 
              (Uses the GOsa support daemon instead of the ldap database.)
      @pram   String The name of the requested principal. (e.g. peter@EXAMPLE.DE)
      @return Array  A list containing the names of all configured principals.
   */
  public function krb5_get_principal($server,$name)
  {
    $ret = array();

    /* Check if the given name is a valid request value 
     */
    if(!is_string($name) || empty($name)){
      trigger_error("The given principal name is not of type string or it is empty.");
      return($ret);
    }

    /* Check if the given server is a valid mac address
     */
    if(!tests::is_mac($server)){
      trigger_error("The given server address '".$server."' is invalid, it must be a valid mac address");
      return($ret);
    }

    /* Prepare request event 
     */ 
    $xml_msg = 
      "<xml>".
      "<header>gosa_krb5_get_principal</header>".
      "<principal>".$name."</principal>".
      "<source>GOSA</source>".
      "<target>".$server."</target>".
      "</xml>";

    $res = $this->_send($xml_msg,TRUE);
    if(isset($res['XML'])){
      return($res['XML']);
    }else{
      return($ret);
    }
  }


  /*! \brief  Creates a given principal with a set of configuration settings.
              For a list of configurable attributes have a look at 'krb5_get_principal()'.
              (Uses the GOsa support daemon instead of the ldap database.)
      @pram   String The name of the principal to update. (e.g. peter@EXAMPLE.DE)
      @return Boolean   TRUE on success else FALSE. 
   */
  public function krb5_add_principal($server,$name,$values)
  {
    $ret = FALSE;  

    /* Check if the given name is a valid request value 
     */
    if(!is_string($name) || empty($name)){
      trigger_error("The given principal name is not of type string or it is empty.");
      return($ret);
    }
    if(!is_array($values)){
      trigger_error("No valid update settings given. The parameter must be of type array and must contain at least one entry");
      return($ret);
    }

    /* Check if the given server is a valid mac address
     */
    if(!tests::is_mac($server)){
      trigger_error("The given server address '".$server."' is invalid, it must be a valid mac address");
      return($ret);
    }

    $attrs = "";
    foreach($values as $key => $value){
      if(empty($key) || is_numeric($key)){
        trigger_error("Invalid configuration attribute given '".$key."=".$value."'.");
        return($ret);
      }
      $key = strtolower($key);
      if(is_array($value)){
        foreach($value as $val){
          $attrs.= "<$key>$val</$key>\n";
        }
      }else{
        $attrs.= "<$key>$value</$key>\n";
      }
    }

    /* Prepare request event 
     */ 
    $xml_msg = 
      "<xml>".
      "<header>gosa_krb5_create_principal</header>".
      "<principal>".$name."</principal>".
      $attrs.
      "<source>GOSA</source>".
      "<target>".$server."</target>".
      "</xml>";

    return($this->_send($xml_msg,TRUE) == TRUE && !$this->is_error());
  }


  function krb5_ramdomize_key($server,$name)  
  {
    /* Prepare request event 
     */ 
    $xml_msg = 
      "<xml>".
      "<header>gosa_krb5_randomize_key</header>".
      "<principal>".$name."</principal>".
      "<source>GOSA</source>".
      "<target>".$server."</target>".
      "</xml>";

    return($this->_send($xml_msg,TRUE) == TRUE && !$this->is_error());
  }
  


  /*! \brief  Updates a given principal with a set of configuration settings.
              For a list of configurable attributes have a look at 'krb5_get_principal()'.
              (Uses the GOsa support daemon instead of the ldap database.)
      @pram   String The name of the principal to update. (e.g. peter@EXAMPLE.DE)
      @return Boolean   TRUE on success else FALSE. 
   */
  public function krb5_set_principal($server,$name,$values)
  {
    $ret = FALSE;  

    /* Check if the given name is a valid request value 
     */
    if(!is_string($name) || empty($name)){
      trigger_error("The given principal name is not of type string or it is empty.");
      return($ret);
    }
    if(!is_array($values) || !count($values)){
      trigger_error("No valid update settings given. The parameter must be of type array and must contain at least one entry");
      return($ret);
    }

    /* Check if the given server is a valid mac address
     */
    if(!tests::is_mac($server)){
      trigger_error("The given server address '".$server."' is invalid, it must be a valid mac address");
      return($ret);
    }

    $attrs = "";
    foreach($values as $key => $value){
      if(empty($key) || is_numeric($key)){
        trigger_error("Invalid configuration attribute given '".$key."=".$value."'.");
        return($ret);
      }
      $key = strtolower($key);
      if(is_array($value)){
        foreach($value as $val){
          $attrs.= "<$key>$val</$key>\n";
        }
      }else{
        $attrs.= "<$key>$value</$key>\n";
      }
    }

    /* Prepare request event 
     */ 
    $xml_msg = 
      "<xml>".
      "<header>gosa_krb5_modify_principal</header>".
      "<principal>".$name."</principal>".
      $attrs.
      "<source>GOSA</source>".
      "<target>".$server."</target>".
      "</xml>";

    return($this->_send($xml_msg,TRUE) == TRUE && !$this->is_error());
  }


  /*! \brief  Removes the given principal.
              (Uses the GOsa support daemon instead of the ldap database.)
      @pram   String The name of the principal. (e.g. peter@EXAMPLE.DE)
      @return Boollean   TRUE on success else FALSE
   */
  public function krb5_del_principal($server,$name)
  {
    $ret = FALSE;  

    /* Check if the given name is a valid request value 
     */
    if(!is_string($name) || empty($name)){
      trigger_error("The given principal name is not of type string or it is empty.");
      return($ret);
    }

    /* Check if the given server is a valid mac address
     */
    if(!tests::is_mac($server)){
      trigger_error("The given server address '".$server."' is invalid, it must be a valid mac address");
      return($ret);
    }

    /* Prepare request event 
     */ 
    $xml_msg = 
      "<xml>".
      "<header>gosa_krb5_del_principal</header>".
      "<principal>".$name."</principal>".
      "<source>GOSA</source>".
      "<target>".$server."</target>".
      "</xml>";
    
    return($this->_send($xml_msg,TRUE) == TRUE && !$this->is_error());
  }


  /*! \brief  Returns a list of configured password policies.
              (Uses the GOsa support daemon instead of the ldap database.)
      @return Array A list of all configured password policies.
   */
  public function krb5_list_policies($server)
  {
    $res = array();  

    /* Check if the given server is a valid mac address
     */
    if(!tests::is_mac($server)){
      trigger_error("The given server address '".$server."' is invalid, it must be a valid mac address");
      return($ret);
    }

    /* Prepare request event 
     */ 
    $xml_msg = 
      "<xml>".
      "<header>gosa_krb5_list_policies</header>".
      "<source>GOSA</source>".
      "<target>".$server."</target>".
      "</xml>";
    
    $res = $this->_send($xml_msg,TRUE);
    
    /* Check if there are results for POLICY 
     */
    if(isset($res['XML']['POLICY'])){
      
      /* Ensure that we return an array 
       */
      $tmp = $res['XML']['POLICY'];
      if(!is_array($tmp)){
        $tmp = array($tmp);
      }
      return($tmp);
    }else{
      return(array());
    }
  }


  /*! \brief  Returns a list of configured password policies.
              (Uses the GOsa support daemon instead of the ldap database.)
      @return Array The policy settings for the given policy name.
   */
  public function krb5_get_policy($server,$name)
  {
    $ret = array();  

    /* Check if the given name is a valid request value 
     */
    if(!is_string($name) || empty($name)){
      trigger_error("The given policy name is not of type string or it is empty.");
      return($ret);
    }

    /* Check if the given server is a valid mac address
     */
    if(!tests::is_mac($server)){
      trigger_error("The given server address '".$server."' is invalid, it must be a valid mac address");
      return($ret);
    }

    /* Prepare request event 
     */ 
    $xml_msg = 
      "<xml>".
      "<header>gosa_krb5_get_policy</header>".
      "<policy>".$name."</policy>".
      "<source>GOSA</source>".
      "<target>".$server."</target>".
      "</xml>";

    /* Possible attributes */
    $attrs = array("MASK","POLICY","PW_HISTORY_NUM","PW_MAX_LIFE",
        "PW_MIN_CLASSES","PW_MIN_LENGTH","PW_MIN_LIFE","POLICY_REFCNT");

  
    $tmp = $this->_send($xml_msg,TRUE);
    if(isset($tmp['XML'])){
      foreach($attrs as $attr){
        if(isset($tmp['XML'][$attr])){
          $ret[$attr] = $tmp['XML'][$attr];
        }else{
          $ret[$attr] = "";
        }
      }
    }
    return($ret);
  }

  
  /*! \brief  Creates a new policy with a given set of configuration settings.
              For a list of configurable attributes have a look at 'krb5_get_policy()'.
              (Uses the GOsa support daemon instead of the ldap database.)
      @pram   String The name of the policy to update.
      @pram   Array  The attributes to update
      @return Boolean   TRUE on success else FALSE. 
   */
  public function krb5_add_policy($server,$name,$values)
  {
    $ret = FALSE;  

    /* Check if the given name is a valid request value 
     */
    if(!is_string($name) || empty($name)){
      trigger_error("The given policy name is not of type string or it is empty.");
      return($ret);
    }
    if(!is_array($values) || !count($values)){
      trigger_error("No valid policy settings given. The parameter must be of type array and must contain at least one entry");
      return($ret);
    }

    /* Check if the given server is a valid mac address
     */
    if(!tests::is_mac($server)){
      trigger_error("The given server address '".$server."' is invalid, it must be a valid mac address");
      return($ret);
    }


    /* Transform array into <xml>
     */
    $attrs = "";
    foreach($values as $id => $value){
      if(empty($id) || is_numeric($id)){
        trigger_error("Invalid policy configuration attribute given '".$id."=".$value."'.");
        return($ret);
      }
      $id = strtolower($id);
      $attrs.= "<$id>$value</$id>\n";
    }

    /* Prepare request event 
     */ 
    $xml_msg = 
      "<xml>".
      "<header>gosa_krb5_create_policy</header>".
      "<policy>".$name."</policy>".
      $attrs.
      "<source>GOSA</source>".
      "<target>".$server."</target>".
      "</xml>";

    return($this->_send($xml_msg,TRUE) == TRUE && !$this->is_error());
  }


  /*! \brief  Updates a given policy with a set of configuration settings.
              For a list of configurable attributes have a look at 'krb5_get_policy()'.
              (Uses the GOsa support daemon instead of the ldap database.)
      @pram   String The name of the policy to update.
      @return Boolean   TRUE on success else FALSE. 
   */
  public function krb5_set_policy($server,$name,$values)
  {
    $ret = FALSE;  

    /* Check if the given name is a valid request value 
     */
    if(!is_string($name) || empty($name)){
      trigger_error("The given policy name is not of type string or it is empty.");
      return($ret);
    }
    if(!is_array($values) || !count($values)){
      trigger_error("No valid policy settings given. The parameter must be of type array and must contain at least one entry");
      return($ret);
    }

    /* Check if the given server is a valid mac address
     */
    if(!tests::is_mac($server)){
      trigger_error("The given server address '".$server."' is invalid, it must be a valid mac address");
      return($ret);
    }

    /* Transform array into <xml>
     */
    $attrs = "";
    foreach($values as $id => $value){
      if(preg_match("/^policy$/i",$id)) continue;
      if(empty($id) || is_numeric($id)){
        trigger_error("Invalid policy configuration attribute given '".$id."=".$value."'.");
        return($ret);
      }
      $id = strtolower($id);
      $attrs.= "<$id>$value</$id>\n";
    }

    /* Prepare request event 
     */ 
    $xml_msg = 
      "<xml>".
      "<header>gosa_krb5_modify_policy</header>".
      "<policy>".$name."</policy>".
      $attrs.
      "<source>GOSA</source>".
      "<target>".$server."</target>".
      "</xml>";

    return($this->_send($xml_msg,TRUE) == TRUE && !$this->is_error());
  }

  
  /*! \brief  Removes the given password policy. 
              (Uses the GOsa support daemon instead of the ldap database.)
      @return Boolean  TRUE on success else FALSE
   */
  public function krb5_del_policy($server,$name)
  {
    $ret = FALSE;

    /* Check if the given server is a valid mac address
     */
    if(!tests::is_mac($server)){
      trigger_error("The given server address '".$server."' is invalid, it must be a valid mac address");
      return($ret);
    }

    /* Check if the given name is a valid request value 
     */
    if(!is_string($name) || empty($name)){
      trigger_error("The given policy name is not of type string or it is empty.");
      return($ret);
    }

    /* Prepare request event 
     */ 
    $xml_msg = 
      "<xml>".
      "<header>gosa_krb5_del_policy</header>".
      "<policy>".$name."</policy>".
      "<source>GOSA</source>".
      "<target>".$server."</target>".
      "</xml>";
    return($this->_send($xml_msg,TRUE) == TRUE && !$this->is_error());
  }


  /*! \brief  Sets the password of for the given principal.
              (Uses the GOsa support daemon instead of the ldap database.)
      @param  String  The servers mac
      @param  String  The principals name
      @param  String  $the new password.   
      @return Boolean  TRUE on success else FALSE
   */
  public function krb5_set_password($server,$name,$password)
  {
    $ret = FALSE;

    /* Check if the given server is a valid mac address
     */
    if(!tests::is_mac($server)){
      trigger_error("The given server address '".$server."' is invalid, it must be a valid mac address");
      return($ret);
    }

    /* Check if the given name is a valid request value
     */
    if(!is_string($name) || empty($name)){
      trigger_error("The given principal name is not of type string or it is empty.");
      return($ret);
    }

    /* Prepare request event
     */
    $xml_msg =
      "<xml>".
      "<header>gosa_krb5_set_password</header>".
      "<principal>".$name."</principal>".
      "<password>".$password."</password>".
      "<source>GOSA</source>".
      "<target>".$server."</target>".
      "</xml>";
    return($this->_send($xml_msg,TRUE) == TRUE && !$this->is_error());
  }


  /*! \brief  Returns log file informations for a given mac address 
      @param  $mac The mac address to fetch logs for.
      @retrun Array A Multidimensional array containing log infos.
        MAC_00_01_6C_9D_B9_FA['install_20080311_090900'][0]=debconf.log
        MAC_00_01_6C_9D_B9_FA['install_20080311_090900'][1]=syslog.log
                               install_20080313_144450   ...
   */
  public function get_log_info_for_mac($mac)
  {
    $xml_msg = "
      <xml>
      <header>gosa_show_log_by_mac</header>
      <target>GOSA</target>
      <source>GOSA</source>
      <mac>".$mac."</mac>
      </xml>";

    $res = $this->_send($xml_msg,TRUE);
    $ret = array();
    if(isset($res['XML'])){

      /* Filter all entry that look like this 
          MAC_00_01_6C_9D_B9_FA
       */
      foreach($res['XML'] as $name => $entry){
        if(preg_match("/^MAC/",$name)){

          /* Get list of available log files 
           */
          if(!is_array($entry)){
            $entry = array($entry);
          }
          foreach($entry as $log_date){
            $xml_msg2 = "<xml> 
              <header>gosa_show_log_files_by_date_and_mac</header> 
              <target>GOSA</target> 
              <source>GOSA</source>                        
              <date>".$log_date."</date> 
              <mac>".$mac."</mac> 
              </xml>";
   
            $ret[$mac][$log_date] = array();
            $res = $this->_send($xml_msg2,TRUE);
            $ret[$mac][$log_date]['DATE_STR']  = $log_date; 
            $ret[$mac][$log_date]['REAL_DATE'] = strtotime(preg_replace("/[^0-9]*/","",$log_date));
            if(isset($res['XML']['SHOW_LOG_FILES_BY_DATE_AND_MAC'])){
              $ret[$mac][$log_date]['FILES']     = $res['XML']['SHOW_LOG_FILES_BY_DATE_AND_MAC'];
            }
          }
        }
      }
    }
    return($ret);
  }

  public function get_log_file($mac,$date,$file)
  {
    $xml_msg ="
      <xml> 
      <header>gosa_get_log_file_by_date_and_mac</header> 
      <target>GOSA</target> 
      <source>GOSA</source>
      <date>".$date."</date> 
      <mac>".$mac."</mac> 
      <log_file>".$file."</log_file>
      </xml>";

    $res = $this->_send($xml_msg,TRUE);
    if(isset($res['XML'][strtoupper($file)])){
      return(base64_decode($res['XML'][strtoupper($file)]));
    }
    return("");
  }





  /*****************
   * DAK - Functions 
   *****************/

  /*! \brief  Returns all currenlty queued entries for a given DAK repository 
      @param  ...
      @return Array   All queued entries.
   */
  public function DAK_keyring_entries($server)  
  {
    /* Ensure that we send the event to a valid mac address 
     */
    if(!is_string($server) || !tests::is_mac($server)){
      trigger_error("No valid mac address given '".$server."'.");
      return;
    }

    /* Create query
     */
    $xml_msg = "<xml> 
                  <header>gosa_get_dak_keyring</header> 
                  <target>".$server."</target> 
                  <source>GOSA</source>
                </xml>";
        
    $res = $this->_send($xml_msg,TRUE);

    /* Check if there are results for POLICY
     */
    if(isset($res['XML'])){
      $ret = array();
      foreach($res['XML'] as $key => $entry){
        if(preg_match("/^ANSWER/",$key)){
          $ret[] = $entry;
        }
      }
      return($ret);
    }else{
      return(array());
    }
  }


  /*! \brief  Imports the given key into the specified keyring (Servers mac address)
      @param  String  The servers mac address 
      @param  String  The gpg key.
      @return Boolean TRUE on success else FALSE 
   */
  public function DAK_import_key($server,$key)  
  {
    /* Ensure that we send the event to a valid mac address 
     */
    if(!is_string($server) || !tests::is_mac($server)){
      trigger_error("No valid mac address given '".$server."'.");
      return;
    }

    /* Check if there is some cleanup required before importing the key.
        There may be some Header lines like:
        -----BEGIN PGP PUBLIC KEY BLOCK-----   Version: GnuPG v1.4.6 (GNU/Linux)
     */
    if(preg_match("/BEGIN PGP PUBLIC KEY BLOCK/",$key)){

      /* Remove header */
      $key = preg_replace("/^.*\n\n/sim","",$key);
      /* Remove footer */
      $key = preg_replace("/-----.*$/sim","",$key);
    }elseif (!preg_match('%^[a-zA-Z0-9/+]*={0,2}$%', $key)) {
      
      /* Encode key if it is raw.
       */
      $key = base64_encode($key);
    }

    /* Create query
     */
    $xml_msg = "<xml> 
                  <header>gosa_import_dak_key</header> 
                  <target>".$server."</target> 
                  <key>".$key."</key> 
                  <source>GOSA</source>
                </xml>";
        
    $res = $this->_send($xml_msg,TRUE);
    return($this->is_error());
  }


  /*! \brief Removes a key from the keyring on the given server. 
      @param  String  The servers mac address 
      @param  String  The gpg key uid.
      @return Boolean TRUE on success else FALSE 
   */
  public function DAK_remove_key($server,$key)  
  {
    /* Ensure that we send the event to a valid mac address 
     */
    if(!is_string($server) || !tests::is_mac($server)){
      trigger_error("No valid mac address given '".$server."'.");
      return;
    }

    /* Create query
     */
    $xml_msg = "<xml> 
                  <header>gosa_remove_dak_key</header> 
                  <target>".$server."</target> 
                  <keyid>".$key."</keyid> 
                  <source>GOSA</source>
                </xml>";
       
    $res = $this->_send($xml_msg,TRUE);
    return($this->is_error());
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
