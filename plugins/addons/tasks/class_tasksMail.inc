<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2018-2022 FusionDirectory

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

class tasksMail extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Tasks Mail'),
      'plDescription' => _('Tasks Mail Object'),
      'plIcon'        => 'geticon.php?context=applications&icon=supann&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=supann-status&size=16',
      'plPriority'    => 42,
      'plObjectClass' => ['fdTasksMail'],
      'plFilter'      => '(objectClass=fdTasksMail)',
      'plObjectType'  => ['tasks'],
      'plConflicts'   => [''],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo()),
      'plForeignKeys'  => [
        'fdTasksMailUsers' => [
          ['user','dn','fdTasksMailUsers=%oldvalue%','*']
        ]
      ],
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      // Attributes are grouped by section
      'taskMail' => [
        'name'  => _('Task Mail Object'),
        'attrs' => [
          new SelectAttribute(
            _('Mail Template'), _('Mail Template Object Selection'),
            'fdTasksMailObject', FALSE
          ),
          new HiddenArrayAttribute('fdTasksEmailsFromDN', FALSE, ''),
          ]
      ],
      'From Component' => [
        'name'  => _('Sender Address'),
        'attrs' => [
           new MailAttribute(
             _('Sender email address'),
             _('Email address from which mails will be sent'), 'fdTasksEmailSender', TRUE, 'to.be@chang.ed'),
        ]
      ],
      'UserGroupSelection' => [
        'name'  => _('Recipients Users and/or Groups'),
        'attrs' => [
          new UsersGroupsRolesAttribute(
            _('Members'), _('Users or groups to assign to this task.'),
            'fdTasksMailUsers', TRUE
          ),
        ],
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    //Search within LDAP and retrieve all mail objects for current base.
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=fdMailTemplate))', ['cn']);
    $tmpSearch = [];
    while ($attrs = $ldap->fetch()) {
      $tmpSearch[$attrs['cn'][0]] = $attrs['cn'][0];
    }
    asort($tmpSearch);
    $this->attributesAccess['fdTasksMailObject']->setChoices(array_keys($tmpSearch), array_values($tmpSearch));

  }

  /*
   * Must return bool to be compliant with the interface
   * Allows attributes values based on others to be updated before save.
   */
  public function update (): bool
  {
    parent::update();
    $this->setEmailsFromSelectedDN();

    return TRUE;
  }

   /*
   * Populate the fdTasksEmailsFromDN attribute with related mails addresses.
   */
  public function setEmailsFromSelectedDN () : void
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $attributeValue = $this->attributesAccess['fdTasksMailUsers']->getValue();

    if ($attributeValue && !empty($attributeValue)) {

      $mailList = [];

      foreach ($this->attributesAccess['fdTasksMailUsers']->getValue() as $dn) {

        $ldap->cd($dn);
        $filter = "(objectClass=gosaMailAccount)";
        $attrs = ["mail"];

        $ldap->search($filter, $attrs);
        $info = $ldap->fetch();

        // To be developped properly to take Supann and other Mail attributes into consideration.
        if (!empty($info["mail"][0]) && isset($info["mail"][0])) {
          $mailList[] = $info["mail"][0];
        }
      }
      $this->attributesAccess['fdTasksEmailsFromDN']->setValue(array_values($mailList));
    }
  }

}
