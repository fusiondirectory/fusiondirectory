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

class ogrouptabs extends simpleTabs_noSpecial
{
  protected $groupObjects = NULL;
  protected $removed_tabs = [];

  function __construct ($type, $dn, $copied_object = NULL)
  {
    parent::__construct($type, $dn, $copied_object);

    $this->loadTabs($this->getBaseObject()->gosaGroupObjects);
  }

  public function loadTabs (string $gosaGroupObjects = NULL)
  {
    global $config;

    if ($gosaGroupObjects === NULL) {
      $gosaGroupObjects = $this->getBaseObject()->getGroupObjectTypes();
    }

    if ($this->groupObjects == $gosaGroupObjects) {
      return;
    }
    $this->groupObjects = $gosaGroupObjects;
    $objects = preg_replace('/[\[\]]/', '', $gosaGroupObjects);

    $tabs = [];
    if (strlen($objects)) {
      if (isset($config->data['TABS']['OGROUP-DYNAMICTABS'])) {
        $dtabs  = $config->data['TABS']['OGROUP-DYNAMICTABS'];
      } else {
        $dtabs  = [];
      }
      $tabs   = FALSE;
      for ($i = 0; $i < strlen($objects); $i++) {
        if (in_array($objects[$i], ['I','G'])) {
          /* Ignore unknown objects and groups */
          continue;
        }
        $otype = ogroup::$objectTypes[$objects[$i]];
        $otabs = [];
        if (isset($config->data['TABS'][strtoupper($otype).'TABS'])) {
          $otabs = array_uintersect(
            $dtabs,
            $config->data['TABS'][strtoupper($otype).'TABS'],
            function ($a, $b)
            {
              return strcmp($a['CLASS'], $b['CLASS']);
            }
          );
        }
        if (isset($config->data['TABS']['OGROUP-'.strtoupper($otype).'TABS'])) {
          $otabs = array_merge(
            $otabs,
            $config->data['TABS']['OGROUP-'.strtoupper($otype).'TABS']
          );
        }
        if ($tabs === FALSE) {
          $tabs = $otabs;
        } else {
          $tabs = array_uintersect(
            $tabs,
            $otabs,
            function ($a, $b)
            {
              return strcmp($a['CLASS'], $b['CLASS']);
            }
          );
        }
      }
    }
    $tabs = array_merge($config->data['TABS']['OGROUPTABS'], (array)$tabs);
    $tabs = array_map(
      function ($a)
      {
        return $a['CLASS'];
      },
      $tabs
    );

    foreach ($this->by_object as $tab => $object) {
      if (!in_array($tab, $tabs)) {
        $this->delTab($tab);
      }
    }
    foreach ($tabs as $class) {
      if (class_available($class) && !isset($this->by_object["$class"])) {
        $this->addTab($class);
      }
    }
    $this->addSpecialTabs();
  }

  function addTab ($class)
  {
    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $class, 'Adding tab ');
    $plInfos = pluglist::pluginInfos($class);
    $this->by_name[$class]  = $plInfos['plShortName'];
    $this->plNotify[$class] = FALSE;
    if (isset($this->removed_tabs[$class])) {
      $this->by_object[$class] = $this->removed_tabs[$class];
      unset($this->removed_tabs[$class]);
    } else {
      $this->by_object[$class] = new $class($this->dn, $this->getBaseObject(), $this, FALSE);
      $this->by_object[$class]->set_acl_category($this->acl_category);
    }
  }

  function delTab ($class)
  {
    $this->removed_tabs["$class"] = $this->by_object["$class"];
    unset($this->by_object["$class"]);
    unset($this->by_name["$class"]);
    unset($this->plNotify["$class"]);
  }

  protected function renderTabList (bool $disabled = FALSE): string
  {
    $this->loadtabs();
    return parent::renderTabList($disabled);
  }

  /*!
   * \brief Returns the list of tabs which may appear for a given object type
   */
  public static function getPotentialTabList (string $type, array $infos): array
  {
    global $config;

    /* prefix will be OGROUP- for ogroup */
    $tabList  = parent::getPotentialTabList($type, $infos);
    $prefix   = strtoupper($type).'-';
    foreach ($config->data['TABS'] as $tabgroup => $tabs) {
      if (strpos($tabgroup, $prefix) === 0) {
        $tabList = array_merge($tabList, $tabs);
      }
    }
    return $tabList;
  }

  /*!
   * \brief Sets the active tabs from this instance to an other one. Used by templates
   */
  public function setActiveTabs (&$tabObject)
  {
    $tabObject->loadTabs($this->groupObjects);
    foreach ($this->by_object as $class => $plugin) {
      if ($plugin->isActive()) {
        $tabObject->by_object[$class]->is_account = $plugin->is_account;
      }
    }
  }

  function save ()
  {
    $errors = parent::save();

    foreach ($this->removed_tabs as $key => &$obj) {
      if (isset($this->by_object[$key])) {
        continue;
      }
      logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $key, 'Removing');

      $obj->dn = $this->dn;
      $tabErrors = $obj->remove(FALSE);
      if (!empty($tabErrors)) {
        $errors = array_merge($errors, $tabErrors);
      }
    }
    unset($obj);

    return $errors;
  }

  public function update (): bool
  {
    parent::update();

    /* Update reference, transfer variables */
    $baseobject = $this->getBaseObject();
    foreach ($this->by_object as $name => $obj) {

      /* Don't touch base object */
      if ($name != 'ogroup') {
        $obj->uid       = $baseobject->cn;
        $obj->cn        = $baseobject->cn;
        $obj->sn        = $baseobject->cn;
        $obj->givenName = $baseobject->cn;

        $this->by_object[$name] = $obj;
      }
    }

    return TRUE;
  }
}
