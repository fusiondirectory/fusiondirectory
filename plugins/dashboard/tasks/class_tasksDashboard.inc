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

class tasksDashboard extends management
{

  function __construct ($objectTypes = FALSE, array $filterElementDefinitions = [['TabFilterElement',    []],])
  {
    parent::__construct($objectTypes, $filterElementDefinitions);

    // Alllow the creation of custom filtering
    $this->setupFilters();
  }

  // Actions column are removed compared to parent.
  public static $columns = [
    ['TasksColumn', ['attributes' => 'fdTasksGranularMaster', 'label' => 'Master Task']],
    ['Column', ['attributes' => 'fdTasksGranularType', 'label' => 'Types']],
    ['Column', ['attributes' => 'fdTasksGranularMail', 'label' => 'Email']],
    ['TasksColumn', ['attributes' => 'fdTasksGranularSchedule', 'label' => 'Schedule']],
    ['TasksColumn', ['attributes' => 'fdTasksGranularStatus', 'label' => 'Status']],
    ['ActionsColumn', ['label' => 'Actions']],
  ];

  // No modification tools are required.
  protected $skipCpHandler        = TRUE;
  public static $skipSnapshots    = TRUE;
  public static $skipTemplates    = TRUE;

  static function plInfo ()
  {
    return [
      'plShortName'   => _('Tasks Dashboard'),
      'plDescription' => _('Provide a reporting mechanism for tasks'),
      'plTitle'       => _('Tasks Dashboard'),
      'plIcon'        => 'geticon.php?context=applications&icon=tasks-dashboard&size=48',
      'plSection'       => ['Dashboard' => ['name' => _('Dashboard'), 'priority' => 4]],
      'plPriority'    => 0,
      'plManages'     => ['tasksGranular'],
      'plProvidedAcls' => [],
    ];
  }

  // Allowing us to add additional actions and remove unrequired ones.
  protected function configureActions ()
  {
    $this->registerAction(
    new Action(
        'tasksRenew',
        _('Renew the selected task'),
        'geticon.php?context=actions&icon=view-refresh&size=16',
        '+',
        'tasksRenew'
        )
    );

    $this->actions['tasksRenew']->setSeparator(TRUE);
    $this->actions['tasksRenew']->setEnableFunction([$this, 'setTasksActionStatus']);

  }

  // This method is triggered by the user action click on tasksRenew
  function tasksRenew (array $action)
  {
    foreach ((array)$action['targets'] as $dn) {
      $this->tasksUpdate($dn);
    }
  }

  function tasksUpdate ($dn)
  {
    $tabObject    = objects::open($dn, 'tasksGranular');
    $tabObject->by_object['tasksGranular']->attributesAccess['fdTasksGranularStatus']->setValue(1);
    $errors = $tabObject->save();

    if (!empty($errors)) {
      $show_error = new SimplePluginError($this, htmlescape(sprintf(_('Error updating: "%s" !.'), $dn)));
      $show_error->display();
    }
  }

  // This method is triggered during initiation of the dashboard, allowing proper icons status
  function setTasksActionStatus () : BOOL
  {
    return  TRUE;
  }

  function setupFilters ()
  {
    // array of states allowing proper filtering
    $status    = [
      'Completed'    => [
        'name'        => 'Completed tasks',
        'filter'      => '(fdTasksGranularStatus=2)',
        'icon'        => 'geticon.php?context=actions&icon=add&size=16',
      ],
      'Scheduled'    => [
        'name'        => 'Scheduled tasks',
        'filter'      => '(fdTasksGranularStatus=1)',
        'icon'        => 'geticon.php?context=actions&icon=system-update&size=16',
      ],
      'Failed'    => [
        'name'      => 'Failed tasks',
        // filter status must be updated if the status strategy changes
        'filter'    => '(!(|(fdTasksGranularStatus=1)(fdTasksGranularStatus=2)))',
        'icon'      => 'geticon.php?context=actions&icon=remove&size=16',
      ],
    ];

    $this->filter->addElement(new CheckBoxesFilterElement($this->filter, _('Status'), $status, '|'));
  }

  // End of Class
}



