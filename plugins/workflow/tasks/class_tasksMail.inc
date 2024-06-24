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
      'plShortName'    => _('Tasks Mail'),
      'plDescription'  => _('Tasks Mail Object'),
      'plIcon'         => 'geticon.php?context=applications&icon=tasks&size=16',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdTasksMail'],
      'plFilter'       => '(objectClass=fdTasksMail)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => ['tasksLifeCycle'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo()),
      'plForeignKeys'  => [
        'fdTasksMailUsers' => [
          ['user', 'dn', 'fdTasksMailUsers=%oldvalue%', '*']
        ]
      ],
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      // Attributes are grouped by section
      'taskMail'           => [
        'name'  => _('Task Mail Object'),
        'attrs' => [
          new SelectAttribute(
            _('Mail Template'), _('Mail Template Object Selection'),
            'fdTasksMailObject', FALSE
          ),
          new HiddenArrayAttribute('fdTasksEmailsFromDN', FALSE, ''),
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
            'fdTasksMailUsers', TRUE
          ),
        ],
      ],
      'tasksMailType'      => [
        'name'  => _('Type of e-mail address desired'),
        'attrs' => [
          new SelectAttribute(
            _('Mail Type - If not found, priority will apply'), _('Mail Type Object Selection'),
            'fdTasksMailType', FALSE
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

    $this->attributesAccess['fdTasksMailType']->setChoices(array_keys($mailAttrTypes), array_values($mailAttrTypes));

  }

  /*
   * Must return bool to be compliant with the interface
   * Allows attributes values based on others to be updated before save.
   */
  public function update (): bool
  {
    parent::update();

    // get the value of type mail desired and if null set default
    $mailAttr = $this->attributesAccess['fdTasksMailType']->getValue();
    if (empty($mailAttr)) {
      $mailAttr = 'mail';
    }

    // create a method which return the objectype and attribute names required
    $mailObject = $this->getMailObject($mailAttr);
    // Get the members or groups selected
    $attributeValue = $this->attributesAccess['fdTasksMailUsers']->getValue();

    // Extract members from potential groups and dyn-groups.
    $listOfDN = tasks::extractMembersFromGroups($attributeValue);
    // send the objectype and attrs name to the below method
    $this->attributesAccess['fdTasksEmailsFromDN']->setValue(
      array_values(static::getEmailsFromSelectedDN($mailObject, $mailAttr, $listOfDN))
    );

    return TRUE;
  }

  /*
   * Return the objectype searched for by setEmailFromSelectedDN
   */
  public function getMailObject (string $mailAttr): string
  {
    switch ($mailAttr) {
      case 'mail' :
      case 'gosaMailAlternateAddress' :
      case 'gosaMailForwardingAddress':
        return 'gosaMailAccount';

      case 'supannAutreMail' :
      case 'supannMailPerso' :
      case 'supannMailPrive' :
        return 'supannPerson';

      default :
        return 'gosaMailAccount';
    }
  }

  /*
  * Retrieve registered emails for each DN passed.
  */
  public static function getEmailsFromSelectedDN ($mailObject, $mailAttr, $listOfDN): array
  {
    global $config;
    $ldap     = $config->get_ldap_link();
    $mailList = [];

    foreach ($listOfDN as $dn) {
      // Position ldap to the dn required (limit search).
      $ldap->cd($dn);

      // filter and attributes should be equals to the arguments passed to this method
      $filter = "(objectClass=$mailObject)";
      $attrs  = [$mailAttr];

      $ldap->search($filter, $attrs);
      $info = $ldap->fetch();

      if (!empty($info[$mailAttr][0])) {
        // In case of private supann mail, remove the prefix
        $mailList[] = preg_replace('/.+?(?=supann)/', '', $info[$mailAttr][0]);

        // Render the mailing list unique, somewhat mandatory when updating the members lists with dynGroups and members.
        $mailList = array_unique($mailList);
        // A possible enhancement is to recall itself with another mailObject / attr
      }
    }
    return $mailList;
  }

  function save (): array
  {
    $execTasks = $this->parent->getBaseObject()->fdSubTasksActivation ?? NULL;

    if ($execTasks) {
      $this->generateSlaveTasks();
    }

    return parent::save();
  }

  /*
   * Generate slave tasks, careful that main task cannot be changed cause subtasks are not updated.
   * It would be dangerous to edit subs tasks if some are under processed already.
   */
  public function generateSlaveTasks ()
  {
    $listOfDN      = $this->attributesAccess['fdTasksEmailsFromDN']->getValue();
    $attributeType = 'fdTasksGranularMail';

    // Ref is supposed to be the mail object CN in this class
    $attrs['ref']  = $this->attributesAccess['fdTasksMailObject']->getValue();
    $attrs['from'] = $this->attributesAccess['fdTasksEmailSender']->getValue();
    $attrs['bcc']  = $this->attributesAccess['fdTasksEmailBCC']->getValue();

    // Call the method from parent tasks object (first tab) to create sub-tasks.
    $this->parent->getBaseObject()->createSlaveTasks($listOfDN, $attributeType, $attrs);
  }
}
