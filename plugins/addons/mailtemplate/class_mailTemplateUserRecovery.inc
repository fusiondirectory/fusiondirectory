<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2018-2019 FusionDirectory

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

class mailTemplateUserRecovery extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Mail Template User Recovery'),
      'plDescription' => _('Mail Template User Recovery'),
      'plIcon'        => 'geticon.php?context=applications&icon=supann&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=supann-status&size=16',
      'plPriority'    => 31,
      'plObjectClass' => ['fdMailTemplateUserRecovery'],
      'plFilter'      => '(objectClass=fdMailTemplateUserRecovery)',
      'plObjectType'  => ['mailTemplate'],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return  [
      'first_email' => [
        'name' => _('First email'),
        'attrs' => [
          new StringAttribute(
            _('Subject'),
            _('Subject of the first email'),
            'fdMTPasswordRecoveryMailSubject',
            TRUE,
            _("[FusionDirectory] Password recovery link")
          ),
          new TextAreaAttribute(
            _('Body'),
            _('Body of the first email, sent when the user ask for a new password. Use %login% for the login and %link% for the recovery link.'),
            'fdMTPasswordRecoveryMailBody',
            TRUE,
            _("Hello,\n\nHere is your information: \n - Login : %login%\n - Link : %link%\n\nThis link is only valid for 10 minutes.")
          )
        ]
      ],
      'second_email' => [
        'name' => _('Second email'),
        'attrs' => [
          new StringAttribute(
            _('Subject'),
            _('Subject of the second email'),
            'fdMTPasswordRecoveryMail2Subject',
            TRUE,
            _("[FusionDirectory] Password recovery successful")
          ),
          new TextAreaAttribute(
            _('Body'),
            _('Body of the second email, sent to confirm the password has been changed. Use %login% for the user login.'),
            'fdMTPasswordRecoveryMail2Body',
            TRUE,
           _("Hello,\n\nYour password has been changed.\nYour login is still %login%.")
          )
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);
  }

  protected function shouldSave (): bool
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=fdMailTemplateUserRecovery))', ['cn','fdMTPasswordRecoveryMailSubject']);
    while ($attrs = $ldap->fetch()) {
      if (isset($this->attrs['fdMTPasswordRecoveryMailSubject'])) {
        $error = new FusionDirectoryError(htmlescape(sprintf(_('User Reminder Template Is Already Existing!'))));
        $error->display();

        return FALSE;
      }
    }

    return TRUE;
  }

}
