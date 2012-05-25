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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class recoveryConfig extends plugin
{
  var $plHeadline     = "Password recovery";
  var $plDescription  = "Manage password recovery settings";
  var $plIcon  = "plugins/password/images/password.png";

  var $cli_summary      = "This plugin is used to configure the password recovery system.";
  var $cli_description  = "";
  var $cli_parameters   = array("one" => "one is nice", "two" => "two is better");

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("recoveryConfig");
  var $attributes       = array("passwordRecoveryActivated","passwordRecoveryEmail",
                                "passwordRecoveryMailSubject","passwordRecoveryMailBody",
                                "passwordRecoveryMail2Subject","passwordRecoveryMail2Body",
                                "passwordRecoveryValidity","passwordRecoverySalt",);
  var $StatusFlag       = "";

  /* This class can't be assigned twice so it conflicts with itsself */
  var $conflicts        = array("recoveryConfig");

  var $DisplayName      = "";
  var $dn               = NULL;
  var $acl;
  var $cn               = "";

  var $passwordRecoveryActivated    = FALSE;
  var $passwordRecoveryEmail        = "to.be@chang.ed";
  var $passwordRecoveryMailSubject;
  var $passwordRecoveryMailBody;
  var $passwordRecoveryMail2Subject;
  var $passwordRecoveryMail2Body;
  var $passwordRecoveryValidity     = 10;
  var $passwordRecoverySalt         = "SomethingSecretAndVeryLong";

  function recoveryConfig(&$config,$dn)
  {
    $this->passwordRecoveryMailSubject  = _("[FusionDirectory] Password recovery link");
    $this->passwordRecoveryMailBody     = _("Hello,\n\nHere are your informations : \n - Login : %s\n - Link : %s\n\nThis link is only valid for 10 minutes.");
    $this->passwordRecoveryMail2Subject = _("[FusionDirectory] Password recovery successful");
    $this->passwordRecoveryMail2Body    = _("Hello,\n\nYour password has been changed.\nYour login is still %s.");

    plugin::plugin($config,$dn);
    $this->DisplayName = _("Password recovery settings");
    $this->passwordRecoveryActivated = ($this->passwordRecoveryActivated == "TRUE"); // conversion from LDAP format to PHP boolean
  }


  function execute()
  {
    if(isset($_POST['recoveryConfigPosted'])) {
      $this->save_object();
      $this->save();
    }

    $smarty = get_smarty();

    $smarty->assign("recoveryConfigACL", $this->getacl("recoveryConfig")) ;
    $attributes= array();
    foreach ($this->attributes as $attribute) {
      $smarty->assign("$attribute" , $this->$attribute);
      $attributes[$attribute] = $this->$attribute;
    }
    $smarty->assign("attributes",$attributes);

    return $smarty->fetch(get_template_path("recoveryConfig.tpl",TRUE,dirname(__FILE__)));
  }

  function save()
  {
    plugin::save();

    $this->attrs['passwordRecoveryActivated'] = ($this->passwordRecoveryActivated?"TRUE":"FALSE"); // conversion from PHP format to LDAP boolean
    /* Check if this is a new entry ... add/modify */
    $ldap = $this->config->get_ldap_link();
    $ldap->cat($this->dn,array("objectClass"));
    if ($ldap->count()) {
      $ldap->cd($this->dn);
      $ldap->modify($this->attrs);
    } else {
      $ldap->cd($this->dn);
      $ldap->add($this->attrs);
    }
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }
    if($this->initially_was_account) {
      $this->handle_post_events("modify");
      new log("modify","server/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    } else {
      $this->handle_post_events("add");
      new log("create","server/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }
  }

  function save_object()
  {
    if (isset($_POST['recoveryConfigPosted'])) {
      plugin::save_object();
      if ($this->acl_is_writeable('passwordRecoveryActivated')) {
        $this->passwordRecoveryActivated = isset($_POST['passwordRecoveryActivated']);
      }
    }
  }

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Password recovery config"),
          "plDescription" => _("Password recovery config"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 91,
          "plSection"     => array("administration"),
          "plCategory"    => array("recovery"),

          "plProvidedAcls"=> array()
          ));
  }
}

?>
