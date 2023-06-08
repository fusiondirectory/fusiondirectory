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
        'name'  => _('Tasks Generic | Saving will create un-modifiable subtasks'),
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
    ];
  }


  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['fdSubTasksActivation']->setInLdap(FALSE);
  }

}
