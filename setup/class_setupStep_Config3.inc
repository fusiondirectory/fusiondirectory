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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


class Step_Config3 extends setup_step
{
  var $header_image = "images/setup/server.png";

  var $errorlvl   = false;

  var $login_attributes = array("uid"       => "uid", 
                                "mail"      => "mail",
                                "uid,mail"  => "both");
  var $optional = array(
              "login_attribute" => "uid",
              "strict_units" => false,
              "list_summary" => true,
              "forceglobals" => true,
              "forcessl" => false,
              "ldapstats" => false,
              "warnssl" => true,
              "user_filter_cookie" => true,
              "ppd_path" => "/var/spool/ppd/",
              "ppd_path_active" => FALSE,
              "compile" =>  "/var/spool/fusiondirectory",
              "snmpcommunity" =>  "goto",
              "debuglevel" => 0,
              "session_lifetime" => 7200,
              "max_ldap_query_time" => "5.0",
              "max_ldap_query_time_active" => FALSE,

              "mailQueueScriptPath" => "/usr/bin/sudo /usr/local/sbin/mailqueue %action %id %server",
              "mailQueueScriptPath_active" => FALSE,

              "notifydir" => "",
              "notifydir_active" => FALSE,

              "gosaSupportURI" => "gosa-si-secret@server:20081",
              "gosaSupportTimeout" => 15,
              "gosaSupportURI_active" => FALSE,

              "uniq_identifier" => "entryCSN",
              "uniq_identifier_active" => TRUE,

              "kioskpath" => "/var/spool/kiosk",
              "kioskpath_active" => FALSE,

              "sudoou"   => "ou=sudoers",
              "sudoou_active" => FALSE,

              "noprimarygroup"  => FALSE);

  var $attributes = array("errorlvl");

  function Step_Config3()
  {
    $this->update_strings();
  }


  function update_strings()
  {
    $this->s_title      = _("FusionDirectory settings 3/3");
    $this->s_title_long = _("FusionDirectory settings 3/3");
    $this->s_info       = _("Tweak some FusionDirectory core behaviour");
  }
  

  function execute()
  {

    $cv = $this->parent->captured_values;
    $this->optional['sudoou'] = preg_replace("/%BASE%/",$cv['base'],$this->optional['sudoou']);
    $smarty = get_smarty();
    $smarty->assign("bool",array(FALSE => _("No"), TRUE => _("Yes")));
    $smarty->assign("uniq_identifiers",array( "entryCSN" => "entryCSN",
                                              "contextCSN"=>"contextCSN" ));

    
    $smarty->assign("optional", reverse_html_entities($this->optional));
    foreach($this->attributes as $attr){
      $smarty->assign($attr,reverse_html_entities($this->$attr));
    }
    $smarty->assign("login_attributes",$this->login_attributes);
    return($smarty -> fetch (get_template_path("../setup/setup_config3.tpl")));
  }


  function save_object()
  {
    if(isset($_POST['step6a_posted'])){

      /* Get attributes */
      foreach($this->attributes as $attr){
        if(isset($_POST[$attr])){
          $this->$attr = validate(get_post($attr));
        }
      }
      
      foreach(array("login_attribute","list_summary","strict_units","snmpcommunity","noprimarygroup","forceglobals","forcessl","ldapstats","user_filter_cookie","warnssl","compile","debuglevel","session_lifetime") as $name){
        if(isset($_POST[$name])){
          $this->optional[$name] = get_post($name);
        }
      } 

      if(isset($_POST['ppd_path_active'])){
        $this->optional['ppd_path_active'] = TRUE;
        if(isset($_POST['ppd_path'])){
          $this->optional['ppd_path'] = get_post('ppd_path');
        }
      }else{
        $this->optional['ppd_path_active'] = FALSE;
      }

      if(isset($_POST['max_ldap_query_time_active'])){
        $this->optional['max_ldap_query_time_active'] = TRUE;
        if(isset($_POST['max_ldap_query_time'])){
          $this->optional['max_ldap_query_time'] = get_post('max_ldap_query_time');
        }
      }else{
        $this->optional['max_ldap_query_time_active'] = FALSE;
      }

      if(isset($_POST['mailQueueScriptPath_active'])){
        $this->optional['mailQueueScriptPath_active'] = TRUE;
        if(isset($_POST['mailQueueScriptPath'])){
          $this->optional['mailQueueScriptPath'] = get_post('mailQueueScriptPath');
        }
      }else{
        $this->optional['mailQueueScriptPath_active'] = FALSE;
      }

      if(isset($_POST['kioskpath_active'])){
        $this->optional['kioskpath_active'] = TRUE;
        if(isset($_POST['kioskpath'])){
          $this->optional['kioskpath'] = get_post('kioskpath');
        }
      }else{
        $this->optional['kioskpath_active'] = FALSE;
      }

      if(isset($_POST['sudoou_active'])){
        $this->optional['sudoou_active'] = TRUE;
        if(isset($_POST['sudoou'])){
          $this->optional['sudoou'] = get_post('sudoou');
        }
      }else{
        $this->optional['sudoou_active'] = FALSE;
      }

      if(isset($_POST['notifydir_active'])){
        $this->optional['notifydir_active'] = TRUE;
        if(isset($_POST['notifydir'])){
          $this->optional['notifydir'] = get_post('notifydir');
        }
      }else{
        $this->optional['notifydir_active'] = FALSE;
      }

      if(isset($_POST['gosaSupportURI_active'])){
        $this->optional['gosaSupportURI_active'] = TRUE;
        if(isset($_POST['gosaSupportURI'])){
          $this->optional['gosaSupportURI'] = get_post('gosaSupportURI');
          $this->optional['gosaSupportTimeout'] = get_post('gosaSupportTimeout');
        }
      }else{
        $this->optional['gosaSupportURI_active'] = FALSE;
      }

      if(isset($_POST['uniq_identifier_active'])){
        $this->optional['uniq_identifier_active'] = TRUE;
        if(isset($_POST['uniq_identifier'])){
          $this->optional['uniq_identifier'] = get_post('uniq_identifier');
        }
      }else{
        $this->optional['uniq_identifier_active'] = FALSE;
      }
    }

    $tmp = $this->check();
    if(count($tmp) == 0){
      $this->is_completed = TRUE;
    }else{
      $this->is_completed = FALSE;
    }
  }


  function check()
  {
    $message = array();

    if(!is_numeric( $this->optional['session_lifetime'])){
      $message[] = _("Session lifetime must be a numeric value!");
    }

    if(!is_numeric( $this->optional['max_ldap_query_time'])){
      $message[] = _("Maximum LDAP query time must be a numeric value!");
    }
  
    return($message);
  }
  

  /* Attributes that are interesting for configuration generation */
  function get_attributes()
  {
    $tmp = setup_step::get_attributes();
    foreach(array("optional") as $attr){
      $tmp[$attr]= $this->$attr;
    }
    return($tmp);
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
