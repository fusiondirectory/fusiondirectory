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

/*! \brief This class contains all the function needed by the argonaut
 * deployment system
 */
class supportDaemon
{
  private $s_host       = "";
  private $config;
  
  private $s_error      = "";
  private $b_error      = FALSE;

  private $logdir       = "";
  
  /*! \brief constructor
   * 
   */
  public function __construct()
  {
    $this->config = session::global_get('config');
    /* This should only be the case if we call this from setup.
    __autoload() 
    */
    if(!is_object($this->config)) { return; }
    $argonautHost = $this->get_argonaut_host();

    if ($argonautHost != "") {
      # load from config, store statically
      $this->s_host = $argonautHost;
      $this->logdir = $this->config->get_cfg_value("faiLogsDir");
      if($this->logdir == "") {
        $this->logdir = FAI_LOG_DIR;
      }
    }
  }

  /*! \brief Load argonaut server config from argonaut service in ldap
   * 
   *     
   */
  public function get_argonaut_host() {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=argonautServer)",array('cn','ipHostNumber','argonautPort'));
    if ($ldap->count() == 1) {
      $ldap_infos = $ldap->fetch();
      return "http://".$ldap_infos['ipHostNumber'][0].":".$ldap_infos['argonautPort'][0];
    }
    return "";
  }

  /*! \brief Test if the argonaut server is available
   * 
   * \return boolean TRUE if the server pings, FALSE otherwise    
   */
  public function is_available() {
    if($this->s_host == "") {
      $this->set_error("argonautServer not defined in config");
      return(FALSE);
    }
    try {
      $client = new jsonRPCClient($this->s_host);
      $ok = "OK";
      $test = $client->echo($ok);
      if ($test == $ok) {
        $this->reset_error();
        return(TRUE);
      } else {
        $this->set_error("Received $test, expected $ok");
        return(FALSE);
      }
    } catch (Exception $e) {
      $this->set_error(nl2br($e->getMessage()));
      return(FALSE);
    }
  }
  
  /*! \brief Allows simply appending a new DaemonEvent
   *  
   * \param string 'event' The event to append.
   * 
   * \return TRUE if correctly appended FALSE otherwise
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
   * 
   * \param  string 'str' The Error message,
   */
  private function set_error($str)
  {
    $this->b_error = TRUE;
    $this->s_error = $str;
  }


  /*! \brief  Resets the error message.
   * 
   */
  private function reset_error()
  {
    $this->b_error = FALSE;
    $this->s_error = "";
  }


  /*! \brief  Checks if an error occured.
   * 
   * \return boolean returns TRUE or FALSE, whether there is an error or not.
   */
  public function is_error()
  {
    return($this->b_error);
  }


  /*! \brief  Returns the last error.
   * 
   *  \return string Returns the last error.
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
  
  /*! \brief format queue entries
   * 
   * \param array 'entries' The entries to format
   * 
   * \return array The entries formatted
   */
  private function format_entries($entries) {
    timezone::get_default_timezone();
    $entries_r = array();
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
  
  
  /*! \brief Returns an array containing all queued entries.
   * 
   * \return array All queued entries as an array or FALSE if there is an error.
   */
  public function get_queued_entries() //$event_types = array("*"),$from=-1,$to=-1,$sort="timestamp DESC"
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
   *
   *  \param array 'ids' The IDs of the entries we want to return.
   *
   *  \return array  Array of the requested entries. 
  */
  public function get_entries_by_id($ids)
  {
    return $this->get_entries("get_entries_by_id",$ids);
  }
  
  /*! \brief  Removes a set of entries from the argonaut queue.
   * 
   * \param array 'ids' The IDs to remove.
   * 
   * \return boolean TRUE on success FALSE otherwise.
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

  /*! \brief Removes an entry from the argonaut queue.
   * 
   * \param integer 'id' The ID of the entry we want to remove.
   * 
   * \return Boolean TRUE on success.
  */
  public function remove_entry($id)
  {
    return($this->remove_entries(array($id)));
  }
  
  /*! \brief Returns an entry from the argonaut queue
   * 
   * \param integer 'id' The ID of the entry we want to return.
   * 
   * \return array  array of the requested entry. 
  */
  public function get_entry_by_id($id)
  {
    return $this->get_entries_by_id(array($id));
  }
  
  /*! \brief  Updates an entry with a set of new values, 
   * 
   * \param  integer 'ids' The ID of the entry, we want to update.
   * \param  array 'data'  The variables to update.   
   * 
   * \return boolean Returns TRUE on success. 
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
  
  /*! \brief  Ask argonaut to process a set of entries now
   * 
   * \param  array 'ids'  The ids of the entries to process immediatly
   * 
   * \return boolean Returns TRUE on success FALSE otherwise.
  */
  public function process_entries_now($ids)
  {
    if(!is_array($ids)){
      trigger_error("Requires an array as first parameter.");
      return;
    }
    
    try {
      $client = new jsonRPCClient($this->s_host);
      $entries = $client->process_entries_now($ids);
      $this->reset_error();
      return(TRUE);
    } catch (Exception $e) {
      $this->set_error(nl2br($e->getMessage()));
      return(FALSE);
    }
  }

  /*! \brief Check if a workstation is currently installing
   * 
   * \param  string 'mac' The mac address of the workstation
   * 
   * \return boolean Returns TRUE if currently installing FALSE otherwise.
   */
  public function is_currently_installing($mac) {
    $e_types = DaemonEvent::get_event_types(USER_EVENT | SYSTEM_EVENT | HIDDEN_EVENT);
    
    $evts = $this->get_entries_by_mac(array($mac));
    if($evts) {
      foreach($evts as $evt) {
        if(isset($e_types['QUEUED'][$evt['HEADERTAG']]) && $evt['STATUS'] == "processing" && 
          in_array($e_types['QUEUED'][$evt['HEADERTAG']],array("DaemonEvent_reinstall","DaemonEvent_update"))) {
          // Why do we check HEADERTAG? processing status should be sufficient.
          return(TRUE);
        }
      }
    }
    return(FALSE);
  }
  
  /*! \brief  Returns an array containing all entries concerning the macs passed.
   * 
   * \param array 'macs'  The mac addresses of the workstations for which we want entries.
   * 
   * \return array  The entries for the requested mac addresses. 
   */
  public function get_entries_by_mac($macs) {
    return $this->get_entries("get_entries_by_mac",$macs);
  }
  
  /*! \brief  Get entries from argonaut server with given function and format entries before to return them
   * 
   * \param function 'func'  The JSONRPC method to use for retrieving addresses
   * \param array 'address'  The mac addresses or ids of the entries we want.
   * 
   * \return Array   The entries for the requested addresses.
   */
  private function get_entries($func,$address) {
    if(!is_array($address)){
      trigger_error("Requires an array as parameter.");
      return;
    }
    
    try {
      $client = new jsonRPCClient($this->s_host);
      $entries = $client->$func($address);
      $entries = $this->format_entries($entries);
      $this->reset_error();
      return($entries);
    } catch (Exception $e) {
      $this->set_error(nl2br($e->getMessage()));
      return(FALSE);
    }
  }
  
  /*! \brief  Returns log file informations for a given mac address
   * 
   * \param  string 'mac' The mac address to fetch logs for.
   * \return Array A Multidimensional array containing log infos.
   * 
   * \code
   *  MAC_00_01_6C_9D_B9_FA['install_20080311_090900']['FILES'][0]=debconf.log
   *  MAC_00_01_6C_9D_B9_FA['install_20080311_090900']['FILES'][1]=syslog.log
   * \endcode
   */
  public function get_log_info_for_mac($mac)
  {
    # read mac directory
    $logfiledir = $this->logdir."/$mac/";
    if(!is_dir($logfiledir)) {
      return(FALSE);
    }
    $dates = scandir($logfiledir);
    if(!$dates) {
      /* removed because in this case we don't need to say that there is nothing */
      /*  $this->set_error("client fai log directory ".$this->logdir." do not exist"); */
      return(FALSE);
    }
    $ret = array();
    foreach ($dates as $date) {
      if($date == "." || $date =="..") {
          continue;
      }
      $ret[$date]['DATE_STR']  = $date; 
      $ret[$date]['REAL_DATE'] = strtotime(preg_replace("/[^0-9]*/","",$date));
      foreach(scandir("$logfiledir/$date") as $file) {
        if($file == "." || $file =="..") {
          continue;
        }
        $ret[$date]['FILES'][] = $file;
      }
    }
    return array($mac=>$ret);
  }
  
  /*! \brief  Get a precise log file on a certain date for a mac address
   * 
   * \param string 'mac' The mac address to fetch logs for.
   * \param string 'date' The date for the log to fetch.
   * \param string 'file' The log filename.
   *      * 
   */
  public function get_log_file($mac,$date,$file)
  {
    if(is_file("/$mac/$date/$file")) {
      return implode("",file($this->logdir."/$mac/$date/$file"));
    } else {
      return "";
    }
  }

  /*! \brief  Returns true if an argonaut daemon is running on the pc
   * 
   * \param string 'mac' The mac address to ping
   * 
   * \return boolean TRUE if the workstation is running an argonaut daemon
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
  
  /*! \brief  Get packages list for the given release
   * 
   * \param string  'release' The release we want the packages of. If empty all releases are used.
   * \param array   'attrs'   The attributes we want for each packages. 
   * (all attributes from Packages file can put, plus version, distribution, has_template and templace)
   * \param array   'filters' The filters we want to use. An or is performed between array cases.
   * \param integer 'from'    The indice of the package we want to start from
   * \param integer 'stop'    The indice of the package we want to stop to.
   * 
   * \return Array   List of packages that fit the given parameters.
  */
  public function FAI_get_packages($release,$attrs,$filters,$from=-1,$to=-1) {
    try {
      $client = new jsonRPCClient($this->s_host);
      $this->reset_error();
      $res = $client->get_packages($release,$attrs,$filters,$from,$to);
      $to_return = array();
      $attrs = array_map('strtoupper', $attrs);
      foreach ($res as $distribution => $packages) {
        $words = preg_split("/\//",$distribution);
        $distribution = $words[0];
        $section = $words[1];
        
        if(in_array("DISTRIBUTION",$attrs)) {
          foreach ($packages as &$package) {
            $package['DISTRIBUTION'] = $distribution;
          }
        }
        if(in_array("SECTION",$attrs)) {
          foreach ($packages as &$package) {
            $package['SECTION'] = $section;
          }
        }
        $to_return = array_merge($to_return,$packages);
      }
      return $to_return;
    } catch (Exception $e) {
      $this->set_error(nl2br($e->getMessage()));
      return array();
    }
  }

  /*! \brief Get all configured repository servers
   * 
   * \return array All configured repository servers
  */
  public function FAI_get_server() {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(FAIrepository=*)(objectClass=FAIrepositoryServer))",array("FAIrepository", "gosaUnitTag"));
    
    $repos = array();
    while($entry = $ldap->fetch()) {
      if(isset($entry['FAIrepository'])) {
        # Add an entry for each Repository configured for server
        unset($entry['FAIrepository']['count']); // FIXME
        foreach ($entry['FAIrepository'] as $repo) {
          $tmp = explode("|", $repo);
          $infos['URL'] = $tmp[0];
          $infos['PARENT_SERVER'] = $tmp[1];
          $infos['FAI_RELEASE'] = $tmp[2];
          $infos['SECTIONS'] = explode(",",$tmp[3]);
          $infos['INSTALL'] = $tmp[4];
          $infos['TAG'] = (isset($entry['gosaUnitTag'])?$entry['gosaUnitTag']:'');
          $repos[] = $infos;
        }
      }
    }
    return $repos;
  }
  
  /*! \brief  Get available kernels packages
   * 
   * \param string 'release' The release we want kernels of
   * 
   * \return array  All kernel packages for the given release
  */
  public function FAI_get_kernels($release)
  {
    $kernels = $this->FAI_get_packages($release,array("package"),array("linux-image"));
    foreach ($kernels as &$kernel) {
      $kernel = $kernel['PACKAGE'];
    }
    return $kernels;
  }

  /*! \brief  Get available profiles
   * 
   * \param string 'release' The release we want profiles for
   * 
   * \return array All FAI profiles for the given release
  */
  public function FAI_get_profiles($release)
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd("ou=$release,".get_ou("faiBaseRDN").$this->config->current['BASE']);
    $ldap->search("(&(objectClass=FAIprofile)(objectClass=FAIclass))",array('cn'));
    
    $res = array();
    while($entry = $ldap->fetch()) {
      $res[]=$entry['cn'][0];
    }
    
    return $res;
  }
  
  /*! \brief Manage services
   * 
   * \param string 'mac' The mac correspind to the host action
   * \param string 'service' The service to be managed
   * \param string 'action' The action to be done on the service
   * 
   * \return status or FALSE if there is an error
  */
  public function manage_service($mac,$service,$action) 
  {
    try {
      $client = new jsonRPCClient($this->s_host);
      $this->reset_error();
      $status = $client->action("manage_service",array($mac),array("args"=>array($service,$action)));
      return $status[0];
    } catch (Exception $e) {
      $this->set_error(nl2br($e->getMessage()));
      return(FALSE);
    }
  }
  
  /*! \brief Clean queue
   * 
   * \param array 'mac' The macs that have to be removed from the queue 
   * 
   * \return TRUE if success or FALSE otherwise
  */
  public function clean_queue_from_mac($mac)
  {
    try {
      $client = new jsonRPCClient($this->s_host);
      $this->reset_error();
      $taskids = $client->get_entries_by_mac(array($mac));
      $client->remove_entries($taskids);
      return(TRUE);
    } catch (Exception $e) {
      $this->set_error(nl2br($e->getMessage()));
      return(FALSE);
    }
  }
};


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
