<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2024 FusionDirectory

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

class tasksLifeCycle extends simplePlugin
{
  protected $displayHeader = TRUE;
  /**
   * @var array|array[]
   */
  private array $subStates;

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('Tasks Life Cycle'),
      'plDescription'  => _('Tasks Life Cycle Object'),
      'plIcon'         => 'geticon.php?context=applications&icon=tasks&size=16',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdTasksLifeCycle'],
      'plFilter'       => '(objectClass=fdTasksLifeCycle)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => ['tasksMail'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo()),
      'plForeignKeys'  => [],
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'section1' => [
        'class' => ['fullwidth', 'information'],
        'name'  => _('We recommend the task redundancy to be set to daily, and the case “new members only” to be unpicked'),
        'attrs' => [
          // This hiddenAttribute is only present to show the information label on the section.
          new HiddenAttribute(
            'fdShowInformation', FALSE,
          ),
          // Following attributes will contain all the DNs required to be verified by Orchestrator.
          new HiddenArrayAttribute('fdTasksLifeCycleListOfDN', FALSE, ''),
        ]
      ],
      'section3' => [
        'name'  => _('Post resources details'),
        'attrs' => [
          new SelectAttribute(
            _('Resource'), _('Supann resources'),
            'fdTasksLifeCyclePostResource', TRUE
          ),
          new SelectAttribute(
            _('State'), _('Resource state'),
            'fdTasksLifeCyclePostState', TRUE
          ),
          new SelectAttribute(
            _('Sub state'), _('Resource sub state'),
            'fdTasksLifeCyclePostSubState', FALSE
          ),
          new IntAttribute(
            _('Extra days to add'), _('Extra days to be added after today'),
            'fdTasksLifeCyclePostEndDate', FALSE, '0', FALSE, '0'
          ),
        ]
      ],
      // Attributes are grouped by section
      'section2' => [
        'name'  => _('Pre / existing resources details'),
        'attrs' => [
          new SelectAttribute(
            _('Resource'), _('Supann resources'),
            'fdTasksLifeCyclePreResource', TRUE
          ),
          new SelectAttribute(
            _('State'), _('Resource state'),
            'fdTasksLifeCyclePreState', TRUE
          ),
          new SelectAttribute(
            _('Sub state'), _('Resource sub state'),
            'fdTasksLifeCyclePreSubState', FALSE
          ),
        ]
      ],
      'UserGroupSelection' => [
        'name' => _('Recipients Users and/or Groups'),
        'class' => ['fullwidth'],
        'attrs' => [
          new UsersGroupsRolesAttribute(
            _('Members'), _('Users or groups to assign to this task.'),
            'fdTasksLifeCycleMembers', TRUE
          ),
        ],
      ],
    ];
  }

  /**
   * @throws NonExistingLdapNodeException
   */
  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->attributesAccess['fdShowInformation']->setInLdap(FALSE);
    $this->setSupannStates();
  }

  /*
 *  Set attributes values with defined supann states from configuration
 */
  protected function setSupannStates () : void
  {
    global $config;

    // Define the mandatory ones and get the remaining from configuration.
    $resources = ['COMPTE' => _('Account'),'MAIL' => _('Mail')];

    foreach ($config->get_cfg_value('SupannRessourceLabels', []) as $line) {
      list($resource,$label) = explode(':', $line, 2);
      $resources[$resource] = $label;
    }

    $this->attributesAccess['fdTasksLifeCyclePreResource']->setChoices(array_keys($resources), array_values($resources));
    $this->attributesAccess['fdTasksLifeCyclePostResource']->setChoices(array_keys($resources), array_values($resources));

    $this->subStates = supannAccountStatus::getConfiguredSubstates(); //Keys are states and Values are subStates
    $this->attributesAccess['fdTasksLifeCyclePreState']->setChoices(array_keys($this->subStates));
    $this->attributesAccess['fdTasksLifeCyclePostState']->setChoices(array_keys($this->subStates));

    // Allows the sub states to be listed when state is modified.
    $this->attributesAccess['fdTasksLifeCyclePreState']->setSubmitForm('updateFieldsChoices');
    $this->attributesAccess['fdTasksLifeCyclePostState']->setSubmitForm('updateFieldsChoices');
    // This initial fill the list choices based on registered states.
    $this->updateFieldsChoices();

  }

  /* Update list of subStates which depends on the state selected */
  function updateFieldsChoices ()
  {
    // Update pre sub states
    $preSubStatesList = $this->subStates[$this->attributesAccess['fdTasksLifeCyclePreState']->getValue()] ?? [];
    $this->attributesAccess['fdTasksLifeCyclePreSubState']->setChoices(array_keys($preSubStatesList), array_values($preSubStatesList));
    // Update post sub states
    $postSubStatesList = $this->subStates[$this->attributesAccess['fdTasksLifeCyclePostState']->getValue()] ?? [];
    $this->attributesAccess['fdTasksLifeCyclePostSubState']->setChoices(array_keys($postSubStatesList), array_values($postSubStatesList));
  }

  /*
* Retrieve all the DNs from groups or members
*/
  public function getListOfDN () : void
  {
    global $config;
    $ldap = $config->get_ldap_link();

    // Get the members or groups selected
    $membersAndGroups = $this->attributesAccess['fdTasksLifeCycleMembers']->getValue();

    if (!empty($membersAndGroups)) {

      // Verify if  the values received is a member or a group and collect the members DN
      foreach ($membersAndGroups as $group) {
        if (strpos($group, "ou=groups") !== FALSE) {

          // Position ldap to the dn required (limit search).
          $ldap->cd($group);
          $filter = '(|(objectClass=groupOfUrls)(objectClass=groupOfNames))';
          $attrs = ['member'];
          $ldap->search($filter, $attrs);
          $info = $ldap->fetch();

          // Remove the DN of the group from the list of DN
          unset($membersAndGroups[$group]);

          // Add the member DN to the list of DN
          foreach ($info['member'] as $memberDN) {
            $membersAndGroups[] = $memberDN;
          }
        }
      }

      $this->attributesAccess['fdTasksLifeCycleListOfDN']->setValue(array_values($membersAndGroups));
    }
  }

  /*
 * Generate slave tasks, careful that main task cannot be changed cause subtasks are not updated.
 * It would be dangerous to edit subs tasks if some are under processed already.
 */
  public function generateSlaveTasks ()
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $listOfDN = $this->attributesAccess['fdTasksLifeCycleListOfDN']->getValue();

    // Take the attribute from the other tabs - attribute cannot be null or unset by default
    $schedule = $this->parent->getBaseObject()->fdTasksScheduleDate ?? NULL;

    // Verify if members can have multiple sub-tasks for that main task.
    $newMemberOnly = $this->parent->getBaseObject()->fdTasksUpdatable;

    // remove 'dn' keeping only 'cn'
    $rmDn = preg_replace('/(?=,).*/', '', $this->dn);
    // only take the cn without dc
    preg_match('/cn=(.*)/', $rmDn, $matches);

    if (!empty($listOfDN)) {
      // Condition allowing the creation of subtasks for existing members
      if ($newMemberOnly === TRUE) {
        $ldap->cd($config->current['BASE']);
        $filter = '(&(objectClass=fdTasksGranular)(fdTasksGranularMaster='.$this->dn.'))';

        $attrs = ['fdTasksLifeCycle'];
        $ldap->search($filter, $attrs);

        // The while loop is important to get all info from ldap into the array.
        while ($info = $ldap->fetch()) {
          $subTasks[] = $info;
        }

        if (!empty($subTasks)) {
          // Recuperate members DN from the ldap search.
          foreach ($subTasks as $subTask) {
            $membersDN[] = $subTask['fdTasksGranularDN'][0];
          }
          // Verify the DN differences and only keep those.
          if (!empty($membersDN)) {
            $listOfDN = array_diff($listOfDN, $membersDN);
          }
          // Simple re-index the array.
          $listOfDN = array_values($listOfDN);
        }
      }

      foreach ($listOfDN as $dn) {
        // Here we create the object taskGranular
        $tabObject = objects::create('TasksGranular');

        // Create a unique ID based on timestamp (Allowing duplicate subtasks for same members in case of repeat).
        $timestamp = microtime(TRUE);  // Get the current timestamp with microseconds
        $timestamp = (string)$timestamp; // Convert the float to a string, str_replace expect array or string.
        $uniqueID = str_replace(".", "_", $timestamp); // Remove . with _ for correct CN

        // Array matches come from preg_match function above with rmDn
        $subTaskName = $matches[1] . '-SubTask-' . $uniqueID;

        $values['tasksGranular'] = [
          "cn" => $subTaskName,
          "fdTasksGranularType" => 'Life Cycle',
          "fdTasksGranularMaster" => $this->dn,
          "fdTasksGranularDN" => $dn,
          "fdTasksGranularSchedule" => $schedule,
        ];

        foreach ($values as $tab => $tabvalues) {
          if (!isset($tabObject->by_object[$tab])) {
            echo "Error tab does not contains attributes values" . PHP_EOL;
          }
          $error = $tabObject->by_object[$tab]->deserializeValues($tabvalues);
          if ($error !== TRUE) {
            echo 'Error during deserializing' . $error . PHP_EOL;
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

  public function update (): bool
  {
    parent::update();

    // Take the list of DN based on the groups or members list selected and filled the related attributes
    $this->getListOfDN();

    return TRUE;
  }

  function save (): array
  {
    // Verify if this tasks has to be executed upon saving.
    $execTasks = $this->parent->getBaseObject()->fdSubTasksActivation ?? NULL;

    if ($execTasks) {
      $this->generateSlaveTasks();
    }

    return parent::save();
  }
}
