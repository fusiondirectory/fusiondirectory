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

class mailTask extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('Tasks Mail'),
      'plDescription'  => _('Tasks Mail Object'),
      'plIcon'         => 'geticon.php?context=applications&icon=internet-mail&size=48',
      'plSmallIcon'    => 'geticon.php?context=applications&icon=internet-mail&size=16',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdTasksMail'],
      'plFilter'       => '(objectClass=fdTasksMail)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => [ // The name of the class is required, not the object class.
        'archiveTask',
        'auditTask',
        'automaticGroupsTask',
        'dnsTask',
        'extractorTask',
        'lifeCycleTask',
        'notificationsTask',
        'reminderTask',
      ],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo()),
      'plForeignKeys'  => [
        'fdTasksMembers' => [
          ['user', 'dn', 'fdTasksMembers=%oldvalue%', '*']
        ]
      ],
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'warning' => [
        'class' => ['fullwidth', 'information'],
        'name'  => _('We recommend the task redundancy to be set to daily, and the case New Members Only to be checked'),
        'attrs' => [
          // This hiddenAttribute is only present to show the information label on the section.
          new HiddenAttribute(
            'fdShowInformation', FALSE,
          ),
        ]
      ],
      'taskMail' => [
        'name'  => _('Task Mail Object'),
        'attrs' => [
          new SelectAttribute(
            _('Mail Template'), _('Mail Template Object Selection'),
            'fdTasksMailObject', FALSE
          ),
        ]
      ],
      'From Component'     => [
        'name'  => _('Sender and BCC email address'),
        'attrs' => [
          new MailAttribute(
            _('Sender email address'),
            _('Email address from which mails will be sent'), 'fdTasksEmailSender', TRUE, 'to.be@chang.ed'),
          new MailAttribute(
            _('BCC email address'),
            _('Email address to put in BCC'), 'fdTasksEmailBCC', FALSE, ''),
        ]
      ],
      'UserGroupSelection' => [
        'name'  => _('Recipients Users and/or Groups'),
        'attrs' => [
          new UsersGroupsRolesAttribute(
            _('Members'), _('Users or groups to assign to this task.'),
            'fdTasksMembers', TRUE
          ),
        ],
      ],
      'tasksMailType'      => [
        'name'  => _('Type of e-mail address desired'),
        'attrs' => [
          new SelectAttribute(
            _('Mail Type - If not found, priority will apply'), _('Mail Type Object Selection'),
            'fdTasksEmailAttribute', FALSE
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

    $mailAttrTypes = [
      'mail'                      => 'mail - [gosaMailAccount primary]',
      'gosaMailAlternateAddress'  => 'gosaMailAlternateAddress',
      'gosaMailForwardingAddress' => 'gosaMailForwardingAddress',
      'supannAutreMail'           => 'supannAutreMail',
      'supannMailPerso'           => 'supannMailPerso',
      'supannMailPrive'           => 'supannMailPrive'
    ];

    $this->attributesAccess['fdTasksEmailAttribute']->setChoices(array_keys($mailAttrTypes), array_values($mailAttrTypes));

  }

  /*
   * Must return bool to be compliant with the interface
   * Allows attributes values based on others to be updated before save.
   */
  public function update (): bool
  {
    parent::update();
    return TRUE;
  }

  function save (): array
  {
    $execTasks = $this->parent->getBaseObject()->fdSubTasksActivation ?? NULL;

    if ($execTasks) {
      $this->generateSubTasks();
    }

    return parent::save();
  }

  /*
   * Generate slave tasks, careful that main task cannot be changed cause subtasks are not updated.
   * It would be dangerous to edit subs tasks if some are under processed already.
   */
  public function generateSubTasks ()
  {
    // Extract DNs from fdTasksMembers
    $attributeValue = $this->attributesAccess['fdTasksMembers']->getValue();
    $listOfDN = tasks::extractMembersFromGroups($attributeValue);
    $attributeType = 'fdSubtaskMemberDN';

    // Mailtemplate object
    $attrs['ref']  = $this->attributesAccess['fdTasksMailObject']->getValue();
    $attrs['from'] = $this->attributesAccess['fdTasksEmailSender']->getValue();
    $attrs['bcc']  = $this->attributesAccess['fdTasksEmailBCC']->getValue();

    // Call the method from parent tasks object (first tab) to create sub-tasks.
    $this->parent->getBaseObject()->createSubTasks($listOfDN, $attributeType, $attrs);
  }
}
