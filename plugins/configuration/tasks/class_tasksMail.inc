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
      'plIcon'        => 'geticon.php?context=applications&icon=tasks&size=16',
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
        'name'  => _('Sender email address'),
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
      ],
      'tasksMailType' => [
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
    $mailAttr  = $this->attributesAccess['fdTasksMailType']->getValue();
    if (empty($mailAttr)) {
      $mailAttr = 'mail';
    }

    // create a method which return the objectype and attribute names required
    $mailObject = $this->getMailObject($mailAttr);

    // send the objectype and attrs name to the below method
    $this->setEmailsFromSelectedDN($mailObject, $mailAttr);

    return TRUE;
  }

  /*
   * Return the objectype searched for by setEmailFromSelectedDN
   */
  public function getMailObject (string $mailAttr) : string
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
   * Populate the fdTasksEmailsFromDN attribute with related mails addresses.
   */
  public function setEmailsFromSelectedDN ($mailObject, $mailAttr) : void
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $attributeValue = $this->attributesAccess['fdTasksMailUsers']->getValue();

    if ($attributeValue && !empty($attributeValue)) {

      $mailList = [];

      foreach ($this->attributesAccess['fdTasksMailUsers']->getValue() as $dn) {

        $ldap->cd($dn);

        // filter and attributes should be equals to the arguments passed to this method
        $filter = "(objectClass=$mailObject)";
        $attrs = [$mailAttr];

        $ldap->search($filter, $attrs);
        $info = $ldap->fetch();

        if (!empty($info[$mailAttr][0]) && isset($info[$mailAttr][0])) {
          // In case of private supann mail, remove the prefix
          $mailList[] = preg_replace('/.+?(?=supann)/', '', $info[$mailAttr][0]);

          // A possible enhancement is to recall itself with another mailObject / attr
        }
      }
      $this->attributesAccess['fdTasksEmailsFromDN']->setValue(array_values($mailList));
    }
  }

  function save (): array
  {
    $execTasks = $this->parent->getBaseObject()->fdSubTasksActivation ?? NULL;

    if ($execTasks) {
      $this->generateSlaveTasks();
    }

    $errors = parent::save();
    return $errors;
  }

  /*
   * Generate slave tasks, carefull that main task cannot be changed cause subtasks are not updated.
   * It would be dangerous to edit subs tasks if some are under processed already.
   */
  public function generateSlaveTasks ()
  {
    $emails = $this->attributesAccess['fdTasksEmailsFromDN']->getValue();
    // Ref is supposed to be the mail object CN in this class
    $ref  = $this->attributesAccess['fdTasksMailObject']->getValue();
    $from = $this->attributesAccess['fdTasksEmailSender']->getValue();
    // Incremental id allowing creation of different CN for sub tasks
    $id = 0;

    // Take the attribute from the other tabs - attribute cannot be null or unset by default
    $schedule = $this->parent->getBaseObject()->fdTasksScheduleDate ?? NULL;

    if (isset($emails) && !empty($emails)) {
      foreach ($emails as $email) {
        $id ++;

        // Here we create the object taskGranular
        $tabObject = objects::create('TasksGranular');

        // remove 'dn' keeping only 'cn'
        $rmDn = preg_replace('/(?=,).*/', '', $this->dn);

        // only take the cn without dc
        preg_match('/cn=(.*)/', $rmDn, $matches);

        $subTaskName = $matches[1].'-SubTask-'.$id;

        $values['tasksGranular'] = [
          "cn"                      => $subTaskName,
          "fdTasksGranularType"     => 'Mail Object',
          "fdTasksGranularMaster"   => $this->dn,
          "fdTasksGranularMail"     => $email,
          "fdTasksGranularSchedule" => $schedule,
          "fdTasksGranularRef"      => $ref,
          "fdTasksGranularMailFrom" => $from
        ];

        foreach ($values as $tab => $tabvalues) {
          if (!isset($tabObject->by_object[$tab])) {
            echo "Error tab does not contains attributes values" .PHP_EOL;
          }
          $error = $tabObject->by_object[$tab]->deserializeValues($tabvalues);
          if ($error !== TRUE) {
            echo 'Error during deserializing' .$error .PHP_EOL;
          }

          $tabObject->current = $tab;
          $tabObject->update();
          $tabObject->loadTabs();
        }

        $errors = $tabObject->save();

        // Showing errors should be better, enhancement here required.
        if (!empty($errors)) {
          $show_error = new SimplePluginError($this, htmlescape(sprintf(_('Error : "%s", already exist ! Editing existing tasks is forbidden.'), $subTaskName)));
          $show_error->display();
        }
      }
    }
  }
}
