<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2013  FusionDirectory

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

class recoveryConfig extends simplePlugin
{
  var $mainTab        = TRUE;
  var $displayHeader  = FALSE;

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("recoveryConfig");
  var $StatusFlag       = "";

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Password recovery config"),
      "plDescription" => _("Settings for the password recovery feature"),
      "plIcon"        => "plugins/password/images/iconRecovery.png",
      "plSelfModify"  => FALSE,
      "plSection"     => "addons",
      "plPriority"    => 9,
      "plCategory"    => array("recovery"),

      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array (
    'settings' => array (
        'name'  => _('Password recovery settings'),
        'attrs' => array (
          new BooleanAttribute (
            _('Activate password recovery'),
            _('Weither to activate or not password recovery feature'),
            'passwordRecoveryActivated',
            FALSE
          ),
          new MailAttribute (
            _('Sender email address'),
            _('Email address from which mails will be sent'),
            'passwordRecoveryEmail',
            FALSE,
            'to.be@chang.ed'
          ),
          new IntAttribute (
            _('Link validity (minutes)'),
            _('Number of minutes before a recovery link expires'),
            'passwordRecoveryValidity',
            FALSE,
            0, FALSE, 10
          ),
          new StringAttribute (
            _('Salt for tokens'),
            _('Just a security measure, you can put anything in there, even random characters'),
            'passwordRecoverySalt',
            FALSE,
            "SomethingSecretAndVeryLong"
          ),
          new BooleanAttribute (
            _('Allow the use of alternate addresses'),
            _('Users will also be able to enter one of theirs alternate addresses to recover their password'),
            'passwordRecoveryUseAlternate',
            FALSE
          ),
        )
      ),
      'first_email' => array (
        'name' => _('First email'),
        'attrs' => array (
          new StringAttribute (
            _('Subject'),
            _('Subject of the first email'),
            'passwordRecoveryMailSubject',
            FALSE,
            _("[FusionDirectory] Password recovery link")
          ),
          new TextAreaAttribute (
            _('Body (first %s is login, second is link)'),
            _('Body of the first email, sent when the user ask for a new password. Use %s for the login and the recovery link.'),
            'passwordRecoveryMailBody',
            FALSE,
            _("Hello,\n\nHere are your informations : \n - Login : %s\n - Link : %s\n\nThis link is only valid for 10 minutes.")
          )
        )
      ),
      'second_email' => array (
        'name' => _('Second email'),
        'attrs' => array (
          new StringAttribute (
            _('Subject'),
            _('Subject of the second email'),
            'passwordRecoveryMail2Subject',
            FALSE,
            _("[FusionDirectory] Password recovery successful")
          ),
          new TextAreaAttribute (
            _('Body (%s is login)'),
            _('Body of the second email, sent to confirm the password has been changed. Use %s for the user login.'),
            'passwordRecoveryMail2Body',
            FALSE,
           _("Hello,\n\nYour password has been changed.\nYour login is still %s.")
          )
        )
      )
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    // Check if token branch is here
    $ldap = $config->get_ldap_link();
    $tokenBranch = get_ou("tokenRDN").$config->current['BASE'];
    $ldap->cat($tokenBranch, array('dn'));
    if (!$ldap->count()) {
      $ldap->cd($config->current['BASE']);
      $ldap->create_missing_trees($tokenBranch);
      if (!$ldap->success()) {
        $display = msgPool::ldaperror($ldap->get_error(),
                                  $tokenBranch, LDAP_MOD, get_class());
      }
      fusiondirectory_log("Created token branch ".$tokenBranch);
    }

    parent::__construct($config, $dn, $object);
    $this->attributesInfo['settings']['attrs']['passwordRecoveryActivated']->setManagedAttributes(
      array(
        'disable' => array(
          FALSE => array(
            'passwordRecoveryEmail','passwordRecoveryValidity','passwordRecoverySalt',
            'passwordRecoveryMailSubject','passwordRecoveryMailBody',
            'passwordRecoveryMail2Subject','passwordRecoveryMail2Body'
          )
        )
      )
    );
  }
  function execute()
  {
    $smarty = get_smarty();
    parent::execute();
    if ($this->displayPlugin) {
      return $this->header.$smarty->fetch(get_template_path('recoveryConfig.tpl', TRUE, dirname(__FILE__)));
    } else {
      return $this->header;
    }
  }
}
?>
