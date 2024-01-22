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
      'plShortName'   => _('Tasks'),
      'plDescription' => _('Tasks'),
      'plObjectClass' => ['fdTasks'],
      'plFilter'      => '(objectClass=fdTasks)',
      'plPriority'    => 41,
      'plObjectType'  => ['tasks' => [
        'name'        => _('Tasks'),
        'ou'          => get_ou('tasksRDN'),
        'icon'        => 'geticon.php?context=applications&icon=tasks&size=16',
      ]],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      // Attributes are grouped by section
      'tasks' => [
        'name'  => _('Tasks Generic'),
        'attrs' => [
        new StringAttribute(
          _('Task Name'), _('Name for this task'),
          'cn', TRUE
        ),
         new DateTimeAttribute(
           _('Schedule'), '',
           'fdTasksScheduleDate', FALSE
           ),

        new HiddenAttribute('fdTasksStatus', TRUE, '1', '', 'Status', 'Status of the task'),
        new HiddenAttribute('fdTasksLastExec', FALSE, '', '', 'LastExec', 'Last exec date'),
        new HiddenAttribute('fdTasksCreationDate', TRUE, date("Y-m-d h:i:sa"), '', 'StartDate', 'Start Date And Time Of A Task'),
        ]
      ],
      'subTasks' => [
        'name' => _('Creation of Sub Tasks - Starting this task'),
        'attrs' => [
          new BooleanAttribute(
            _('Activate SubTasks'), _('Trigger the creation of this task and related subtasks'),
            'fdSubTasksActivation', FALSE
          ),
        ]
      ],
      'taskSetting' => [
        'name' => _('Advanced settings'),
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
            'fdTasksRepeatableSchedule', FALSE, ['Yearly', 'Monthly', 'Weekly', 'Daily', 'Hourly'], 'Daily'
          ),
        ]
      ],
    ];
  }


  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
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
  }

  function save (): array
  {
    // Verification if the bool of activation is ticked and activate the last exec accordingly.
    if ($this->fdSubTasksActivation == TRUE) {
      $currentDateTime = date("Y-m-d h:i:sa", time());
      $this->fdTasksLastExec = $currentDateTime;
    }
    return parent::save();
  }
}
