<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

class departmentManagement extends management
{
  // Tab definition
  protected $tabClass     = "deptabs";
  protected $tabType      = "DEPARTMENTTABS";
  protected $aclCategory  = "department";
  protected $aclPlugin    = "generic";
  protected $objectName   = "department";

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Departments"),
      "plDescription" => _("Manage departments"),
      "plIcon"        => "plugins/departments/images/icon.png",
      "plSection"     => array("admin" => array("name" => _("Administration"), "priority" => 0)),
      "plCategory"    => array("department"),
      "plPriority"    => 0,

      "plProvidedAcls" => array()
    );
  }

  function __construct($config, $ui)
  {
    $this->config = $config;
    $this->ui     = $ui;

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")) {
      $filter = session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path("dep-filter.xml", TRUE));
    }
    $filter->setObjectStorage(array(''));

    // Build headpage
    $headpage = new listing(get_template_path("dep-list.xml", TRUE));
    $headpage->registerElementFilter("depLabel", "departmentManagement::filterDepLabel");
    $headpage->setFilter($filter);
    $this->setFilter($filter);

    parent::__construct($config, $ui, "departments", $headpage);

    $this->registerAction("open",                   "openEntry");
    $this->registerAction("new_domain",             "newEntry");
    $this->registerAction("new_country",            "newEntry");
    $this->registerAction("new_locality",           "newEntry");
    $this->registerAction("new_dcObject",           "newEntry");
    $this->registerAction("new_organization",       "newEntry");
    $this->registerAction("new_organizationalUnit", "newEntry");
    $this->registerAction("performRecMove",         "performRecMove");
    $this->registerAction("tagDepartment",          "tagDepartment");
  }

  // Inject additional actions here.
  function detectPostActions()
  {
    $actions = management::detectPostActions();
    if (isset($_GET['PerformRecMove'])) {
      $actions['action'] = "performRecMove";
    }
    if (isset($_GET['TagDepartment'])) {
      $actions['action'] = "tagDepartment";
    }
    return $actions;
  }

  // Action handler which allows department tagging - Creates the iframe contents.
  function tagDepartment()
  {
    $plugname = $this->last_tabObject->base_name;
    $this->last_tabObject->by_object[$plugname]->tag_objects();
    exit();
  }

  // Overridden new handler - We've different types of departments to create!
  function newEntry($action = "", $target = array(), $all = array(), $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    $types  = $this->get_support_departments();
    $type   = preg_replace("/^new_/", "", $action);
    return management::newEntry($action, $target, $all, $this->tabClass, $types[$type]['TAB'], $this->aclCategory);
  }

  // Overridden edit handler - We've different types of departments to edit!
  function editEntry($action = "", $target = array(), $all = array(), $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    $types    = $this->get_support_departments();
    $headpage = $this->getHeadpage();
    $type     = $headpage->getType($target[0]);
    return management::editEntry($action, $target, $all, $this->tabClass, $types[$type]['TAB'], $this->aclCategory);
  }


  // Overriden save handler - We've to take care about the department tagging here.
  protected function saveChanges()
  {
    $str = management::saveChanges();
    if (!empty($str)) {
      return $str;
    }

    $this->refreshDeps();
  }


  function refreshDeps()
  {
    global $config;
    $config->get_departments();
    $config->make_idepartments();
    $this->config = $config;
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
  protected function removeEntryRequested($action = "", $target = array(), $all = array())
  {
    $target = array_remove_entries(array($this->config->current['BASE']), $target);
    return management::removeEntryRequested($action, $target, $all);
  }


  // A filter which allows to open a department by clicking on the departments name.
  static function filterDepLabel($row, $dn, $params, $ou, $pid, $base)
  {
    $ou = $ou[0];
    if ($dn == $base) {
      $ou = ".";
    }
    $dn = LDAP::fix(func_get_arg(1));
    return "<a href='?plug=".$_GET['plug']."&amp;PID=$pid&amp;act=listing_open_$row' title='$dn'>$ou</a>";
  }


  // Finally remove departments and update departmnet browsers
  function removeEntryConfirmed($action = "", $target = array(), $all = array(), $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    management::removeEntryConfirmed($action, $target, $all, $altTabClass, $altTabType, $altAclCategory);
    $this->refreshDeps();
  }

  /*! \brief  Returns information about all container types that FusionDirectory can handle.
    @return Array   Informations about departments supported by FusionDirectory.
   */
  public static function get_support_departments()
  {
    /* Domain
     */
    $types = array();
    $types['domain']['ACL']       = "domain";
    $types['domain']['CLASS']     = "domain";
    $types['domain']['ATTR']      = "dc";
    $types['domain']['TAB']       = "DOMAINTABS";
    $types['domain']['OC']        = "domain";
    $types['domain']['IMG']       = "plugins/departments/images/domain.png";
    $types['domain']['IMG_FULL']  = "plugins/departments/images/domain.png";
    $types['domain']['TITLE']     = _("Domain");
    $types['domain']['TPL']       = "domain.tpl";

    /* Domain component
     */
    $types['dcObject']['ACL']       = "dcObject";
    $types['dcObject']['CLASS']     = "dcObject";
    $types['dcObject']['ATTR']      = "dc";
    $types['dcObject']['TAB']       = "DCOBJECTTABS";
    $types['dcObject']['OC']        = "dcObject";
    $types['dcObject']['IMG']       = "plugins/departments/images/dc.png";
    $types['dcObject']['IMG_FULL']  = "plugins/departments/images/dc.png";
    $types['dcObject']['TITLE']     = _("Domain Component");
    $types['dcObject']['TPL']       = "dcObject.tpl";

    /* Country object
     */
    $types['country']['ACL']      = "country";
    $types['country']['CLASS']    = "country";
    $types['country']['TAB']      = "COUNTRYTABS";
    $types['country']['ATTR']     = "c";
    $types['country']['OC']       = "country";
    $types['country']['IMG']      = "plugins/departments/images/country.png";
    $types['country']['IMG_FULL'] = "plugins/departments/images/country.png";
    $types['country']['TITLE']    = _("Country");
    $types['country']['TPL']      = "country.tpl";

    /* Locality object
     */
    $types['locality']['ACL']       = "locality";
    $types['locality']['CLASS']     = "locality";
    $types['locality']['TAB']       = "LOCALITYTABS";
    $types['locality']['ATTR']      = "l";
    $types['locality']['OC']        = "locality";
    $types['locality']['IMG']       = "plugins/departments/images/locality.png";
    $types['locality']['IMG_FULL']  = "plugins/departments/images/locality.png";
    $types['locality']['TITLE']     = _("Locality");
    $types['locality']['TPL']       = "locality.tpl";

    /* Organization
     */
    $types['organization']['ACL']       = "organization";
    $types['organization']['CLASS']     = "organization";
    $types['organization']['TAB']       = "ORGANIZATIONTABS";
    $types['organization']['ATTR']      = "o";
    $types['organization']['OC']        = "organization";
    $types['organization']['IMG']       = "plugins/departments/images/organization.png";
    $types['organization']['IMG_FULL']  = "plugins/departments/images/organization.png";
    $types['organization']['TITLE']     = _("Organization");
    $types['organization']['TPL']       = "organization.tpl";

    /* Department
     */
    $types['organizationalUnit']['ACL']       = "department";
    $types['organizationalUnit']['CLASS']     = "department";
    $types['organizationalUnit']['TAB']       = "DEPARTMENTTABS";
    $types['organizationalUnit']['ATTR']      = "ou";
    $types['organizationalUnit']['OC']        = "organizationalUnit";
    $types['organizationalUnit']['IMG']       = "images/lists/folder.png";//plugins/departments/images/department.png";
    $types['organizationalUnit']['IMG_FULL']  = "images/lists/folder-full.png";//:wplugins/departments/images/department.png";
    $types['organizationalUnit']['TITLE']     = _("Department");
    $types['organizationalUnit']['TPL']       = "generic.tpl";
    return $types;
  }
}
?>
