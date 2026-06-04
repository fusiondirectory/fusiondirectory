<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2012-2022 FusionDirectory

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

class tasks extends simplePlugin
{
  protected $displayHeader = FALSE;

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('Tasks'),
      'plDescription'  => _('Tasks'),
      'plObjectClass'  => ['fdTasks'],
      'plFilter'       => '(objectClass=fdTasks)',
      'plPriority'     => 41,
      'plObjectType'   => ['tasks' => [
        'name' => _('Tasks'),
        'ou'   => get_ou('tasksRDN'),
        'icon' => 'geticon.php?context=applications&icon=tasks&size=16',
      ]],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    // Build selectable minute options Minutes:00 .. Minutes:59
    $minuteOptions = array_map(function ($m) {
      return sprintf('Minutes:%02d', $m);
    }, range(0, 59));

    return [
      // Attributes are grouped by section
      'tasks'       => [
        'name'  => _('Tasks Generic'),
        'attrs' => [
          new StringAttribute(
            _('Task Name'), _('Name for this task'),
            'cn', TRUE
          ),
          new StringAttribute(
            _('Description'), _('What is this task doing'),
            'description', FALSE
          ),
          new GeneralizedTimeDateAttribute(
            _('Schedule'), '',
            'fdTasksScheduleDate', FALSE
          ),
          new GeneralizedTimeDateAttribute(
            _('StartDate'), _('Start Date And Time Of A Task'),
            'fdTasksCreationDate', TRUE
          ),
          new GeneralizedTimeDateAttribute(
            _('LastActivation'), _('Last activation date'),
            'fdTasksLastActivation', FALSE, ''
          ),
          new GeneralizedTimeDateAttribute(
            _('LastExecution'), _('Last Executed - processed date'),
            'fdTasksLastExec', FALSE, ''
          ),
          new GeneralizedTimeDateAttribute(
            _('NextExecution'), _('Next Executed - processed date'),
            'fdTasksNextExec', FALSE, ''
          ),
          new HiddenAttribute('fdTasksStatus', TRUE, '1', '', 'Status', 'Status of the task'),
        ]
      ],
      'subTasks'    => [
        'name'  => _('Creation of Sub Tasks - Starting this task'),
        'attrs' => [
          new BooleanAttribute(
            _('Activate SubTasks'), _('Trigger the creation of this task and related subtasks'),
            'fdSubTasksActivation', FALSE
          ),
        ]
      ],
      'taskSetting' => [
        'name'  => _('Advanced settings'),
        'attrs' => [
          new BooleanAttribute(
            _('Only with new members'), _('Allows creation of sub-tasks for "NEW MEMBERS" only. (Case of Dynamic Group)'),
            'fdTasksUpdatable', FALSE, TRUE
          ),
          new BooleanAttribute(
            _('Repeatable Task'), _('Set the task to be repeatable.'),
            'fdTasksRepeatable', FALSE
          ),
          new SelectAttribute(
            _('Repeatable Schedule'), _('Select the desired schedule.'),
            'fdTasksRepeatableSchedule', TRUE,
            array_merge(['Yearly', 'Monthly', 'Weekly', 'Daily', 'Hourly'], $minuteOptions),
            'Daily',
            array_merge(['Yearly', 'Monthly', 'Weekly', 'Daily', 'Hourly'], $minuteOptions)
          ),
        ]
      ],
    ];
  }


  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['fdTasksRepeatable']->setManagedAttributes(
      [
        'disable' => [
          FALSE => [
            'fdTasksRepeatableSchedule',
          ]
        ]
      ]
    );

    $this->attributesAccess['fdSubTasksActivation']->setInLdap(FALSE);

    $this->attributesAccess['fdTasksCreationDate']->setVisible(FALSE);
    $this->attributesAccess['fdTasksLastActivation']->setVisible(FALSE);
    $this->attributesAccess['fdTasksLastExec']->setVisible(FALSE);
    $this->attributesAccess['fdTasksNextExec']->setVisible(FALSE);
  }

  function save (): array
  {
    // Verification if the bool of activation is ticked and activate the last activation accordingly.
    if ($this->fdSubTasksActivation === TRUE) {
      $currentDateTime             = date("Y-m-d H:i:s");
      $this->fdTasksLastActivation = LdapGeneralizedTime::toString(new DateTime($currentDateTime, new DateTimeZone('UTC')));
    }
    return parent::save();
  }

  /**
   * @param array $listOfDN
   * @param string $attributeType
   * @param array|NULL $attrs
   * @param string $taskType
   * @return void
   * Note : $taskType is present to define the object name related to life cycle (core) and any new plugins added to tasks.
   */
  public function createSlaveTasks (array $listOfDN, string $attributeType, ?array $attrs = NULL, string $taskType = ''): void
  {
    global $config;
    $ldap = $config->get_ldap_link();

    // Take the attribute from the other tabs - attribute cannot be null or unset by default
    $schedule = $this->fdTasksScheduleDate ?? NULL;
    // Verify if members can have multiple sub-tasks for that main task.
    // When TRUE, only new members get subtasks, or members with subtasks in state 3 (nothing to process)
    $newMemberOnly = $this->fdTasksUpdatable;

    // remove 'dn' keeping only 'cn'
    $rmDn = preg_replace('/(?=,).*/', '', $this->dn);
    // only take the cn without dc
    preg_match('/cn=(.*)/', $rmDn, $matches);

    if (!empty($listOfDN)) {
      // Condition allowing the creation of subtasks for existing members
      if ($newMemberOnly === TRUE) {
        $ldap->cd($config->current['BASE']);
        $filter = '(&(objectClass=fdTasksGranular)(fdTasksGranularMaster=' . $this->dn . '))';
        $ldap->search($filter, ['fdTasksGranularStatus', $attributeType]);

        // The while loop is important to get all info from ldap into the array.
        while ($info = $ldap->fetch()) {
          $subTasks[] = $info;
        }

        if (!empty($subTasks)) {
          $membersDN = [];
          $completedMembers = [];

          // Recuperate members DN from the ldap search.
          foreach ($subTasks as $subTask) {
            $status = isset($subTask['fdTasksGranularStatus'][0]) ? $subTask['fdTasksGranularStatus'][0] : '1';

            // For lifeCycle we check DN and for mailObject we check mail
            switch ($attributeType) {
              case 'fdTasksGranularDN':
                // If status is 3 (nothing to process), allow creating a new subtask
                // For all other statuses, add to membersDN to prevent duplicate subtask creation
                if ($status !== '3') {
                  $membersDN[] = $subTask['fdTasksGranularDN'][0];
                } else {
                  $completedMembers[] = $subTask['fdTasksGranularDN'][0];
                }
                break;
              case 'fdTasksGranularMail':
                // If status is 3 (nothing to process), allow creating a new subtask
                // For all other statuses, add to membersDN to prevent duplicate subtask creation
                if ($status !== '3') {
                  $membersDN[] = $subTask['fdTasksGranularMail'][0];
                } else {
                  $completedMembers[] = $subTask['fdTasksGranularMail'][0];
                }
                break;
            }
          }

          // Verify the DN differences and only keep those without tasks or with "nothing to process" (status 3) tasks
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
        $uniqueID  = str_replace(".", "_", $timestamp); // Remove . with _ for correct CN

        // Array matches come from preg_match function above with rmDn
        $subTaskName = $matches[1] . '-SubTask-' . $uniqueID;

        // Define the type of the granular task based on the attribute type passed to this method.
        $prepData = NULL;
        switch ($attributeType) {
          case 'fdTasksGranularMail':
            $prepData['tasksGranular'] = [
              "fdTasksGranularMail"     => $dn,
              "fdTasksGranularType"     => 'Mail Object',
              "fdTasksGranularRef"      => $attrs['ref'],
              "fdTasksGranularMailFrom" => $attrs['from'],
              "fdTasksGranularMailBCC"  => $attrs['bcc']
            ];
            break;
          case 'fdTasksGranularDN' :
            $prepData['tasksGranular'] = [
              "fdTasksGranularDN"   => $dn,
              "fdTasksGranularType" => $taskType,
              // Verification as 'ref' could potentially not be present depending on the logic of the main task.
              "fdTasksGranularRef"  => !empty($attrs['ref'][$dn]) ? array_values($attrs['ref'][$dn]) : [],
              // Verification if any helper reference must be added to help further backend processing.
              "fdTasksGranularHelper"  => !empty($attrs['helper']) ? array_values($attrs['helper']) : [],
            ];
            break;
        }

        // Common attributes to be filled for object tasksGranular.
        $defaultData['tasksGranular'] = [
          "cn"                        => $subTaskName,
          "fdTasksGranularMaster"     => $this->dn,
          "fdTasksGranularSchedule"   => $schedule,
          "fdTasksGranularCreationDate" => date("YmdHis"),
        ];
        // Simply merged the common values and the custom ones depending on the attribute type passed.
        $values['tasksGranular'] = array_merge($prepData['tasksGranular'], $defaultData['tasksGranular']);

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

        if (!empty($errors)) {
          msg_dialog::displayChecks($errors);
        }
      }
    }
  }

  /**
   * @param array $groups
   * @return array
   */
  public static function extractMembersFromGroups (array $groups): array
  {
    global $config;

    $ldap         = $config->get_ldap_link();
    $listMemberDN = [];

    if (!empty($groups)) {
      // Verify if  the values received is a member or a group and collect the members DN
      foreach ($groups as $group) {
        if (strpos($group, "ou=groups") !== FALSE) {

          // Position ldap to the dn required (limit search).
          $ldap->cd($group);
          $filter = '(|(objectClass=groupOfUrls)(objectClass=groupOfNames))';
          $attrs  = ['member'];

          $ldap->search($filter, $attrs);
          $info = $ldap->fetch();

          // Verify if the group is not empty of members
          if (!empty($info['member'])) {
            //unset the count from the array
            unset($info['member']['count']);
            foreach ($info['member'] as $memberDN) {
              $listMemberDN[] = $memberDN;
            }
            // case of member not within a group or dyngroup
          }
        } else {
          $listMemberDN[] = $group; //Here group is indeed a sole user
        }
        // Make sure no duplicate can happens, case of same member in an existing group.
        $listMemberDN = array_unique($listMemberDN);
      }

      // Iterate on the DN list to remove any members representing a group (members of that potential groups were extracted).
      foreach ($listMemberDN as $key => $value) {
        if (strpos($value, 'ou=groups') !== FALSE) {
          unset($listMemberDN[$key]);
        }
      }
    }
    return $listMemberDN;
  }

}
