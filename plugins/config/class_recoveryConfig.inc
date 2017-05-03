<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2016  FusionDirectory

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
  var $objectclasses  = array('fdPasswordRecoveryConf');

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Password recovery'),
      'plDescription'   => _('Settings for the password recovery feature'),
      'plObjectType'    => array('configuration'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array (
      'settings' => array (
        'name'      => _('Password recovery settings'),
        'class'     => array ('alone'),
        'template'  => get_template_path('simpleplugin_section_alone.tpl'),
        'attrs'     => array (
          new BooleanAttribute (
            _('Activate password recovery'),
            _('Whether to activate or not password recovery feature'),
            'fdPasswordRecoveryActivated',
            FALSE
          ),
          new MailAttribute (
            _('Sender email address'),
            _('Email address from which mails will be sent'),
            'fdPasswordRecoveryEmail',
            TRUE,
            'to.be@chang.ed'
          ),
          new IntAttribute (
            _('Link validity (minutes)'),
            _('Number of minutes before a recovery link expires'),
            'fdPasswordRecoveryValidity',
            TRUE,
            0, FALSE, 10
          ),
          new StringAttribute (
            _('Salt for tokens'),
            _('Just a security measure, you can put anything in there, even random characters'),
            'fdPasswordRecoverySalt',
            TRUE,
            "SomethingSecretAndVeryLong"
          ),
          new BooleanAttribute (
            _('Allow the use of alternate addresses'),
            _('Users will also be able to enter one of theirs alternate addresses to recover their password'),
            'fdPasswordRecoveryUseAlternate',
            FALSE
          ),
          new StringAttribute (
            _('Login attribute'), _('Usually uid, but you can use something else for instance in case of SSO'),
            'fdPasswordRecoveryLoginAttribute', TRUE,
            'uid'
          ),
        )
      ),
      'first_email' => array (
        'name' => _('First email'),
        'attrs' => array (
          new StringAttribute (
            _('Subject'),
            _('Subject of the first email'),
            'fdPasswordRecoveryMailSubject',
            TRUE,
            _("[FusionDirectory] Password recovery link")
          ),
          new TextAreaAttribute (
            _('Body (first %s is login, second is link)'),
            _('Body of the first email, sent when the user ask for a new password. Use %s for the login and the recovery link.'),
            'fdPasswordRecoveryMailBody',
            TRUE,
            _("Hello,\n\nHere is your information: \n - Login : %s\n - Link : %s\n\nThis link is only valid for 10 minutes.")
          )
        )
      ),
      'second_email' => array (
        'name' => _('Second email'),
        'attrs' => array (
          new StringAttribute (
            _('Subject'),
            _('Subject of the second email'),
            'fdPasswordRecoveryMail2Subject',
            TRUE,
            _("[FusionDirectory] Password recovery successful")
          ),
          new TextAreaAttribute (
            _('Body (%s is login)'),
            _('Body of the second email, sent to confirm the password has been changed. Use %s for the user login.'),
            'fdPasswordRecoveryMail2Body',
            TRUE,
           _("Hello,\n\nYour password has been changed.\nYour login is still %s.")
          )
        )
      )
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    // Check if token branch is here
    $ldap = $config->get_ldap_link();
    $tokenBranch = get_ou('recoveryTokenRDN').get_ou('fusiondirectoryRDN').$config->current['BASE'];
    $ldap->cat($tokenBranch, array('dn'));
    if (!$ldap->count()) {
      $ldap->cd($config->current['BASE']);
      $ldap->create_missing_trees($tokenBranch);
      if (!$ldap->success()) {
        msg_dialog::display(
          _('LDAP error'),
          msgPool::ldaperror($ldap->get_error(), $tokenBranch, LDAP_MOD, get_class()),
          LDAP_ERROR
        );
      }
      fusiondirectory_log('Created token branch '.$tokenBranch);
    }

    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['fdPasswordRecoveryActivated']->setManagedAttributes(
      array(
        'disable' => array(
          FALSE => array(
            'fdPasswordRecoveryEmail','fdPasswordRecoveryValidity','fdPasswordRecoverySalt',
            'fdPasswordRecoveryMailSubject','fdPasswordRecoveryMailBody',
            'fdPasswordRecoveryMail2Subject','fdPasswordRecoveryMail2Body'
          )
        )
      )
    );
  }
}
?>
