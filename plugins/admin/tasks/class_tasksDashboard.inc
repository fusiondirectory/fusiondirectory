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

  // Actions collumn are removed compared to parent.
  public static $columns = [
    ['Column', ['attributes' => 'cn', 'label' => 'Tasks']],
    ['Column', ['attributes' => 'fdTasksGranularMaster', 'label' => 'Master Task']],
    ['Column', ['attributes' => 'fdTasksGranularType', 'label' => 'Types']],
    ['Column', ['attributes' => 'fdTasksGranularMail', 'label' => 'Email']],
    ['Column', ['attributes' => 'fdTasksGranularSchedule', 'label' => 'Schedule']],
    ['Column', ['attributes' => 'fdTasksGranularStatus', 'label' => 'Status']],
  ];

  // No modification tools are required.
  protected $skipCpHandler        = TRUE;
  public static $skipSnapshots    = TRUE;
  public static $skipTemplates    = TRUE;
  protected $skipConfiguration    = TRUE;

  static function plInfo ()
  {
    return [
      'plShortName'   => _('Tasks Dashboard'),
      'plDescription' => _('Provide a reporting mechanism for tasks'),
      'plTitle'       => _('Tasks Dashboard'),
      'plIcon'        => 'geticon.php?context=applications&icon=argonaut&size=48',
      'plSection'     => ['reporting' => ['name' => _('Reporting'), 'priority' => 40]],
      'plPriority'    => 10,
      'plManages'     => ['tasksGranular'],
      'plProvidedAcls' => [],
    ];
  }

  protected function setUpFilter (array $filterElementDefinitions)
  {
    $this->filter   = new tasksDashboardFilter($this, NULL, $filterElementDefinitions);
  }

  // Allowing us to add additional actions and remove unrequired ones.
  protected function configureActions ()
  {
        // Leaving empty as no default action allowed.
  }
  // End of Class
}

// extending the class allows us to remove the rendering of filter.
class tasksDashboardFilter extends managementFilter
{
    // Filter will be
}
