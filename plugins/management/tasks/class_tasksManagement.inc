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
  protected $skipCpHandler        = TRUE;
  public static $skipSnapshots    = TRUE;
  public static $skipTemplates    = TRUE;

  public static $columns = [
    ['ObjectTypeColumn', []],
    ['LinkColumn',  ['attributes' => 'cn', 'label' => 'Tasks']],
    ['LinkColumn',  ['attributes' => 'description',  'label' => 'Description']],
    ['LdapGeneralizedTimeColumn', ['attributes' => 'fdTasksCreationDate', 'label' => 'Creation Date']],
    ['LdapGeneralizedTimeColumn', ['attributes' => 'fdTasksLastActivation', 'label' => 'Activation']],
    ['LdapGeneralizedTimeColumn', ['attributes' => 'fdTasksLastExec', 'label' => 'Last Execution']],
    ['LdapGeneralizedTimeColumn', ['attributes' => 'fdTasksNextExec', 'label' => 'Next Execution']],
    ['ActionsColumn', ['label' => 'Actions']],
  ];

  static function plInfo ()
  {
    return [
      'plShortName'   => _('Tasks Creation'),
      'plDescription' => _('Manage the creation of tasks'),
      'plIcon'        => 'geticon.php?context=applications&icon=tasks&size=48',
      'plSection'     => ['workflow' => ['name' => _('Workflow'), 'priority' => 2]],
      'plPriority'    => 1,
      'plManages'     => ['tasks'],
      'plProvidedAcls' => [],
    ];
  }

  function __construct ()
  {
    parent::__construct();

    $status    = [
      'Completed'    => [
        'name'        => 'Completed tasks',
       'filter'      => '(fdTasksStatus=2)',
        'icon'        => 'geticon.php?context=actions&icon=add&size=16',
      ],
      'Scheduled'    => [
        'name'        => 'Scheduled tasks',
        'filter'      => '(fdTasksStatus=1)',
        'icon'        => 'geticon.php?context=actions&icon=system-update&size=16',
      ],
      'Nothing'    => [
        'name'        => 'Nothing to process',
        'filter'      => '(fdTasksStatus=3)',
        'icon'        => 'geticon.php?context=actions&icon=nothing-to-process&size=16',
      ],
    ];

    $this->filter->addElement(new CheckBoxesFilterElement($this->filter, _('Status'), $status));
  }

}
