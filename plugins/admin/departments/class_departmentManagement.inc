<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class departmentManagement extends simpleManagement
{
  protected $objectTypes;

  protected $departmentBrowser      = FALSE;
  protected $departmentRootVisible  = FALSE;
  protected $baseMode               = TRUE;

  /* Return plugin information for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Departments'),
      'plTitle'       => _('Manage departments'),
      'plDescription' => _('Manage departments, countries, domain components, domains, localities and organization nodes,'),
      'plIcon'        => 'geticon.php?context=places&icon=folder&size=48',
      'plSection'     => array('accounts' => array('name' => _('Users and groups'), 'priority' => 0)),
      'plPriority'    => 0,
      'plManages'     => static::getDepartmentTypes(),

      'plProvidedAcls' => array()
    );
  }

  function __construct()
  {
    $this->objectTypes  = static::getDepartmentTypes();
    $this->listXMLPath  = get_template_path('dep-list.xml', TRUE, dirname(__FILE__));
    parent::__construct();
    $this->headpage->registerElementFilter("depLabel", "departmentManagement::filterDepLabel");
    $this->registerAction("open", "openEntry");
  }

  // Overriden save handler - We've to take care about the department tagging here.
  protected function saveChanges()
  {
    $str = parent::saveChanges();
    if (!empty($str)) {
      return $str;
    }

    $this->refreshDeps();
  }

  function refreshDeps()
  {
    global $config, $ui;
    $config->get_departments();
    $config->make_idepartments();
    $ui->reset_acl_cache();
    $headpage     = $this->getHeadpage();
    $headpage->refreshBasesList();
  }

  // An action handler which enables to switch into deparmtment by clicking the names.
  function openEntry($action, $entry)
  {
    $headpage = $this->getHeadpage();
    $headpage->setBase(array_pop($entry));
  }

  // Overridden remove request method - Avoid removal of the ldap base.
  protected function removeEntryRequested($action, array $target, array $all)
  {
    global $config;
    $target = array_remove_entries(array($config->current['BASE']), $target);
    return parent::removeEntryRequested($action, $target, $all);
  }

  // A filter which allows to open a department by clicking on the departments name.
  static function filterDepLabel($row, $dn, $params, $ou, $pid, $base)
  {
    $ou = $ou[0];
    if ($dn == $base) {
      $ou = ".";
    }
    $dn = func_get_arg(1);
    return "<a href='?plug=".$_GET['plug']."&amp;PID=$pid&amp;act=listing_open_$row' title='$dn'>$ou</a>";
  }

  // Finally remove departments and update departmnet browsers
  function removeEntryConfirmed($action, array $target, array $all)
  {
    parent::removeEntryConfirmed($action, $target, $all);
    $this->refreshDeps();
  }

  static function getDepartmentTypes()
  {
    return array('DEPARTMENT','COUNTRY','DCOBJECT','DOMAIN','LOCALITY','ORGANIZATION');
  }
}
?>
