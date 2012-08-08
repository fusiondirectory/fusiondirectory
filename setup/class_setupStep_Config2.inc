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


class Step_Config2 extends setup_step
{
  var $governmentmode = FALSE;
  var $sambaidmapping = FALSE;
  var $header_image = "images/setup/server.png";

  var $cyrusunixstyle = FALSE;
  var $mail           = "none";
  var $mail_attrib    = "";
  var $mail_methods   = array();
  var $mail_settings = array("vacationdir"        => VACATION_DIR,
                             "vacationdir_active" => FALSE);

  var $generic_settings  = array( "wws_ou"        => "ou=computers",
                                  "wws_ou_active" => FALSE,
                                  "snapshot_active"     => FALSE,
                                  "snapshot_base"       => "ou=snapshots,%base%",
                                  "snapshot_user"       => "%admin%",
                                  "snapshot_password"   => "",
                                  "snapshot_server"     => "%connection%");

  var $samba_settings    = array( "samba_sid"        => "0-815-4711",
                                  "samba_sid_active" => FALSE,
                                  "samba_rid"        => 1000,
                                  "smbhash"          => 'SMBHASH',
                                  "samba_rid_active" => FALSE);

  var $sambaDomain_found    = FALSE;
  var $timezone       = "Europe/Paris";
  var $timezones      = array();

  var $attributes = array("mail_attrib","sambaidmapping","cyrusunixstyle","mail","timezone");
  var $called = FALSE;

  function Step_Config2()
  {
    $this->update_strings();
    $this->mail_methods = $this->get_available_mail_classes();

    /* Look for samba password generation method */
    if(file_exists(MKNTPWD)){
      $pwdhash = MKNTPWD;
    } elseif (preg_match("/^Usage: fusiondirectory-mkntpassword <password>/", shell_exec (MKNTPWD))){
      $pwdhash = MKNTPWD;
    } else {
      $pwdhash = 'perl -MCrypt::SmbHash -e "print join(q[:], ntlmgen \$ARGV[0]), $/;"';
    }

    $this->timezones = timezone::_get_tz_zones();
    $this->samba_settings['smbhash'] = $pwdhash;
  }


  function update_strings()
  {
    $this->s_title      = _("FusionDirectory settings 2/3");
    $this->s_title_long = _("FusionDirectory settings 2/3");
    $this->s_info       = _("Customize special parameters");
  }


  function execute()
  {
    global $BASE_DIR;
    $this->mail_methods = $this->get_available_mail_classes();
    /* On first call check for rid/sid base */
    $cv = $this->parent->captured_values;
    $ldap_l = new LDAP($cv['admin'],
        $cv['password'],
        $cv['connection'],
        FALSE,
        $cv['tls']);

    $ldap = new ldapMultiplexer($ldap_l);

    $ldap->cd($cv['base']);
    $ldap->search("(objectClass=sambaDomain)",array("dn"));
    if($ldap->count()){
      $this->sambaDomain_found =TRUE;
    }else{
      $this->sambaDomain_found =FALSE;
    }

    if(!$this->called){
      if(!$this->sambaDomain_found){
        $this->samba_settings['samba_sid_active'] = TRUE;
        $this->samba_settings['samba_rid_active'] = TRUE;
      }

      $this->called = TRUE;
    }

    /* Update snapshot values, with already collected values */
    foreach($this->generic_settings as $key => $value){
      foreach(array("snapshot_base","snapshot_user","snapshot_server") as $attr){
        foreach($this->parent->captured_values as $replace_name => $replace_value){
          if(is_string($replace_value))  {
            $this->generic_settings[$attr] = preg_replace("/%".$replace_name."%/",$replace_value,$this->generic_settings[$attr]);
          }
        }
      }
    }

    $smarty = get_smarty();
    $smarty->assign("sambaDomain_found",  $this->sambaDomain_found);
    $smarty->assign("generic_settings",  reverse_html_entities($this->generic_settings));
    $smarty->assign("mail_settings",     reverse_html_entities($this->mail_settings));
    $smarty->assign("mail_methods",      reverse_html_entities($this->mail_methods));
    $smarty->assign("samba_settings",    reverse_html_entities($this->samba_settings));

    $smarty->assign("bool",array(FALSE => _("No"), TRUE => _("Yes")));
    foreach($this->attributes as $attr){
      $smarty->assign($attr,reverse_html_entities($this->$attr));
    }
    $smarty->assign("timezone"     ,$this->timezone);
    $smarty->assign("timezones"    ,$this->timezones);
    return $smarty->fetch("$BASE_DIR/setup/setup_config2.tpl");
  }

  function save_object()
  {
    if(isset($_POST['step6_posted'])){

      if(isset($_POST['timezone']) && isset($this->timezones[$_POST['timezone']])){
        $this->timezone = $_POST['timezone'];
      }

      $old_mail = $this->mail;

      /* Get attributes */
      foreach($this->attributes as $attr){
        if(isset($_POST[$attr])){
          $this->$attr = validate(get_post($attr));
        }
      }

      /* Mail settings */
      if(isset($_POST['vacationdir_active'])){
        $this->mail_settings['vacationdir_active'] = TRUE;
        if(isset($_POST['vacationdir'])){
          $this->mail_settings['vacationdir'] = get_post('vacationdir');
        }
      }else{
        $this->mail_settings['vacationdir_active'] = FALSE;
      }

      if(isset($_POST['wws_ou_active'])){
        $this->generic_settings['wws_ou_active'] = TRUE;
        if(isset($_POST['wws_ou'])){
          $this->generic_settings['wws_ou'] = get_post('wws_ou');
        }
      }else{
        $this->generic_settings['wws_ou_active'] = FALSE;
      }

      if(isset($_POST['snapshot_active'])){
        $this->generic_settings['snapshot_active'] = TRUE;
        if(isset($_POST['snapshot_base'])){
          $this->generic_settings['snapshot_base'] = get_post('snapshot_base');
        }
        if(isset($_POST['snapshot_user'])){
          $this->generic_settings['snapshot_user'] = get_post('snapshot_user');
        }
        if(isset($_POST['snapshot_password'])){
          $this->generic_settings['snapshot_password'] = get_post('snapshot_password');
        }
        if(isset($_POST['snapshot_server'])){
          $this->generic_settings['snapshot_server'] = get_post('snapshot_server');
        }
      }else{
        $this->generic_settings['snapshot_active'] = FALSE;
      }

      /* Samba settings */
      if(isset($_POST['samba_sid_active'])){
        $this->samba_settings['samba_sid_active'] = TRUE;
        if(isset($_POST['samba_sid'])){
          $this->samba_settings['samba_sid'] = get_post('samba_sid');
        }
      }else{
        $this->samba_settings['samba_sid_active'] = FALSE;
      }
      if(isset($_POST['samba_rid_active'])){
        $this->samba_settings['samba_rid_active'] = TRUE;
        if(isset($_POST['samba_rid'])){
          $this->samba_settings['samba_rid'] = get_post('samba_rid');
        }
      }else{
        $this->samba_settings['samba_rid_active'] = FALSE;
      }
      if(isset($_POST['smbhash'])){
        $this->samba_settings['smbhash'] = get_post('smbhash');
      }

      if(!$this->sambaDomain_found){
        $this->samba_settings['samba_sid_active'] = TRUE;
        $this->samba_settings['samba_rid_active'] = TRUE;
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
    return($message);
  }


  /* Attributes that are interesting for configuration generation */
  function get_attributes()
  {
    $tmp = setup_step::get_attributes();
    foreach(array("samba_settings","generic_settings","mail_settings") as $attr){
      $tmp[$attr]= $this->$attr;
    }
    return($tmp);
  }


  /* Returns the classnames auf the mail classes */
  function get_available_mail_classes()
  {
    global $class_mapping;

    /* Check if we have the mail classes/plugins installed */
    if(!class_available("mailMethod")){
      return(array());
    }
    $methods = array();
    foreach($class_mapping as $name => $file){
      if(preg_match("/^mailMethod.*$/i",$name)){
        $name = preg_replace("/^mailMethod/","",$name);
        if(!empty($name)){
          $methods[$name]['name'] = $name;
        }
      }
    }
    return($methods);
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
