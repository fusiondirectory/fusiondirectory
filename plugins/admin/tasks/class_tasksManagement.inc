<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2022  FusionDirectory

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

class tasksManagement extends management
{

  protected $skipCpHandler    = TRUE;

  /*
  *  LinkColumn exists by class collumn
  *  TasksColumn is a new class for Tasks bassed on argonaut column definition
  */
  public static $columns = [
    ['LinkColumn',  ['attributes' => 'cn', 'label' => 'Tasks']],
    //below fdTasksMailObject must be change to have a type defined within task creation (new ldap attributes ?hidden)
    ['TasksColumn', ['attributes' => 'fdTasksMailObject', 'label' => 'Types']],
    ['TasksColumn', ['attributes' => 'fdTasksScheduleDate', 'label' => 'Schedule']],
    //below fdTasksStatus attribute needs to be calculated based on the type of the task.
    ['TasksColumn', ['attributes' => 'fdTasksStatus', 'label' => 'Status']],
    ['Column', ['attributes' => 'fdTasksCreationDate', 'label' => 'Creation Date']],
    ['ActionsColumn', ['label' => 'Actions']],
  ];

  static function plInfo ()
  {
    return [
      'plShortName'   => _('Tasks'),
      'plDescription' => _('Provide a mechanism to create and manage tasks'),
      'plTitle'       => _('Tasks'),
      'plIcon'        => 'geticon.php?context=applications&icon=argonaut&size=48',
      'plSection'     => 'conf',
      'plPriority'    => 40,
      'plManages'     => ['tasks'],
      'plProvidedAcls' => [],
    ];
  }

  protected function setUpListing ()
  {
    /* Set baseMode to FALSE */
    $this->listing  = new managementListing($this, FALSE);
  }

}
