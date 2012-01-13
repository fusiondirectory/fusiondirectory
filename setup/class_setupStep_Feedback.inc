<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007  Fabian Hickert
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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/


class Step_Feedback  extends setup_step
{
  var $languages      = array();
  var $header_image   = "images/setup/welcome.png";

  var $organization       = "";
  var $eMail              = "";
  var $name               = "";
  var $subscribe          = FALSE;
  var $use_gosa_announce  = FALSE;

  var $feedback_url         = "http://oss.gonicus.de/gosa-feedback/";
  var $get_started          = TRUE;
  var $problems_encountered = "";
  
  var $first_use            = TRUE;
  var $use_since            = "";

  var $distribution         = "";
  var $web_server           = "";
  var $php_version          = "";

  var $ldap_server          = "";
  var $object_count         = "";
    
  var $want_to_see_next     = "";

  var $features_used        = array();  
  var $initialized          = FALSE; 
  var $feedback_send        = FALSE;
  var $attributes = array("organization","eMail","name","subscribe","use_gosa_announce","get_started","problems_encountered","features_used",
                          "first_use","use_since","distribution","web_server","php_version","ldap_server","object_count","want_to_see_next");
  

  function Step_Feedback()
  {
    $this->is_enabled     = TRUE;
    $this->is_active      = TRUE;
    $this->update_strings(); 
  }


  function init()
  {
    $cv = $this->parent->captured_values;

    /* Initialize values */
    $this->use_since      = date("Y");
    $this->organization   = preg_replace("/^[^=]*+=(.*),.*$/","\\1",$cv['base']);
    $this->distribution   = "";
    $this->web_server     = $_SERVER['SERVER_SOFTWARE'];
    $this->php_version    = PHP_VERSION;

    /* Establish ldap connection */
    $cv = $this->parent->captured_values;
    $ldap_l = new LDAP($cv['admin'],
        $cv['password'],
        $cv['connection'],
        FALSE,
        $cv['tls']);

    $this->object_count= "";
    $ldap = new ldapMultiplexer($ldap_l);
    $ldap->cd($cv['base']);

    /* Preselect used features */
    $oc = $ldap->get_objectclasses();
    $this->features_used                = array();
    $this->features_used['posix']       = array("USED" => isset($oc['posixAccount']) , 
                                                "NAME" => _("UNIX accounts/groups"));
    $this->features_used['samba']       = array("USED" => isset($oc['sambaSamAccount']) || isset($oc['sambaAccount']) , 
                                                "NAME" => _("Samba management"));
    $this->features_used['mail']        = array("USED" => $cv['mail']  != "none", 
                                                "NAME" => _("Mailsystem management"));
    $this->features_used['fax']         = array("USED" => isset($oc['goFaxAccount']) , 
                                                "NAME" => _("FAX system administration"));
    $this->features_used['asterisk']    = array("USED" => isset($oc['goFonAccount']), 
                                                "NAME" => _("Asterisk administration"));
    $this->features_used['inventory']   = array("USED" => isset($oc['glpiAccount']) , 
                                                "NAME" => _("System inventory"));
    $this->features_used['system']      = array("USED" => FALSE , 
                                                "NAME" => _("System-/Configmanagement"));
    $this->features_used['addressbook'] = array("USED" => FALSE , 
                                                "NAME" => _("Addressbook"));
  }


  function update_strings()
  {
    $this->s_title      = _("Feedback");
    $this->s_info       = _("Get notifications or send feedback");
    $this->s_title_long = _("Notification and feedback");
  }

  
  function execute()
  {
    if(!$this->initialized){
      $this->initialized = TRUE;
      $this->init();
    }

    $additional_info ="";
    if(isset($_POST['send_feedback'])){

      $msgs = $this->check_feedback();
      if(count($msgs)){
        foreach($msgs as $msg){
          msg_dialog::display(_("Setup error"), $msg, ERROR_DIALOG);
        }
      }else{
        $str = $this->create_serialise_data_string(); 
        $feedback_url = $this->feedback_url."?data=".urlencode(base64_encode($str));

        $ret = @fopen($feedback_url,"r");

        if(!$ret){
          msg_dialog::display(_("Feedback error"), sprintf(_("Cannot send feedback to '%s': %s"), $this->feedback_url, $ret), ERROR_DIALOG);
        }else{
          $ret_value="";
          while($ret && !feof($ret)){
            $ret_value.= fgets($ret,256);
          }
          if(!preg_match("/successful/i",$ret_value)){
            msg_dialog::display(_("Feedback error"), _("Cannot send feedback: service temporarily unavailable"), ERROR_DIALOG);
          }else{
            $this->feedback_send = TRUE;
          }
        }
        @fclose($ret);
      }
    }

    $years = array();
    for($i = date("Y") ; $i >= 2001; $i--){
      $years[$i] = $i;
    }

    $this->is_completed=TRUE;
    $smarty = get_smarty();

    foreach($this->attributes as $attr){
      $smarty->assign($attr,   reverse_html_entities($this->$attr));
    }
    $smarty->assign("feedback_send",$this->feedback_send);
    $smarty->assign("years",$years);
    $smarty->assign("features_used",$this->features_used);

    if(function_exists("get_gosa_version")){
      $smarty->assign("version",get_gosa_version());
    }else{
      $smarty->assign("version","");
    }

    $smarty->assign("additional_info", reverse_html_entities($additional_info));
    $smarty->assign ("must", "<font class=\"must\">*</font>");
    return($smarty->fetch(get_template_path("setup_feedback.tpl",TRUE,dirname(__FILE__))));
  }


  function check_feedback()
  {
    $msgs = array();
    if($this->subscribe && (!tests::is_email($this->eMail) || empty($this->eMail))){
      $msgs[] = _("Please specify a valid email address.");
    }

    if(!$this->subscribe && !$this->use_gosa_announce){
      $msgs[] = _("You have to select at least one of both options, subscribe or send feedback.");
    }

    return($msgs);
  }
  

  function save_object()
  {
    if(isset($_POST['step_feedback'])){
      foreach($this->attributes as $attr){
        if(isset($_POST[$attr])){
          $this->$attr = get_post($attr);
        }
      }

      if(isset($_POST['subscribe'])){
        $this->subscribe = TRUE;
      }else{
        $this->subscribe = FALSE;
      }

      if(isset($_POST['use_gosa_announce'])){
        $this->use_gosa_announce = TRUE;
      }else{
        $this->use_gosa_announce = FALSE;
      }
    
      foreach($this->features_used as $key => $data){
        if(isset($_POST['feature_'.$key])){
          $this->features_used[$key]['USED'] = TRUE;
        }else{
          $this->features_used[$key]['USED'] = FALSE;
        }
      }
    }
  }


  function create_serialise_data_string()
  {
    $str = "#FEEDBACK ".date("H:i:s d:m:Y");
    if($this->subscribe){

      $str.= "\n\nSubscribe";
      $str .= "\nOrganization: ".$this->organization; 
      $str .= "\nName: ".$this->name;
      $str .= "\neMail: ".$this->eMail;
    }
  
    if($this->use_gosa_announce){

      $str.= "\n\nFeedback";    
  
      if($this->get_started) {
        $str .= "\nGet started: TRUE";
      }else{
        $str .= "\nGet started: FALSE";
      }
      $str .= "\nProblems : ".($this->problems_encountered);

      if($this->first_use){
        $str .= "\nFirst use: TRUE";
      }else{
        $str .= "\nFirst use: FALSE";
      }
      $str .= "\nUsing FusionDirectory since: ".$this->use_since;

      $str .="\nDistribution: ".$this->distribution;
      $str .="\nWeb Server: ".$this->web_server;
      $str .="\nPHP_Version: ".$this->php_version;
      if(function_exists("get_gosa_version")){
	$str .="\nFusionDirectory_Version: ".strip_tags(get_gosa_version());
      }
      $str .="\nLDAP Server: ".$this->ldap_server;
      $str .="\nObject count: ".$this->object_count;
      $str .="\nMissing features: ".($this->want_to_see_next);
    }

    $str .= "\n\nFeatures used";
    foreach($this->features_used as $name => $data){
      $str.="\n".$name.": ";
      if($data['USED']){
        $str .= "TRUE";
      }else{
        $str .= "FALSE";
      }
    }

    return($str);  
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
