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
  protected $removed_tabs = array();

  function __construct($type, $dn, $copied_object = NULL)
  {
    parent::__construct($type, $dn, $copied_object);

    $this->loadtabs($this->getBaseObject()->gosaGroupObjects);
  }

  function loadtabs($gosaGroupObjects)
  {
    if ($this->groupObjects == $gosaGroupObjects) {
      return;
    }
    $this->groupObjects = $gosaGroupObjects;
    $objects = preg_replace('/[\[\]]/', '', $gosaGroupObjects);

    $tabs = array();
    if (strlen($objects)) {
      if (isset($this->config->data['TABS']['OGROUP-DYNAMICTABS'])) {
        $dtabs  = $this->config->data['TABS']['OGROUP-DYNAMICTABS'];
      } else {
        $dtabs  = array();
      }
      $tabs   = FALSE;
      for ($i = 0; $i < strlen($objects); $i++) {
        if ($objects[$i] == 'I') {
          continue;
        }
        $otype = ogroup::$objectTypes[$objects[$i]];
        $otabs = array();
        if (isset($this->config->data['TABS'][strtoupper($otype).'TABS'])) {
          $otabs = array_uintersect(
            $dtabs,
            $this->config->data['TABS'][strtoupper($otype).'TABS'],
            function ($a, $b)
            {
              return strcmp($a['CLASS'], $b['CLASS']);
            }
          );
        }
        if (isset($this->config->data['TABS']['OGROUP-'.strtoupper($otype).'TABS'])) {
          $otabs = array_merge(
            $otabs,
            $this->config->data['TABS']['OGROUP-'.strtoupper($otype).'TABS']
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
    $tabs = array_merge($this->config->data['TABS']['OGROUPTABS'], (array)$tabs);
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
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $class, "Adding tab ");
    $plInfos = pluglist::pluginInfos($class);
    $this->by_name["$class"]            = $plInfos['plShortName'];
    if (isset($this->removed_tabs["$class"])) {
      $this->by_object["$class"] = $this->removed_tabs["$class"];
      unset($this->removed_tabs["$class"]);
    } else {
      $this->by_object["$class"]          = new $class($this->config, $this->dn, $this->getBaseObject());
      $this->by_object["$class"]->parent  = &$this;
      $this->by_object["$class"]->set_acl_category($this->acl_category);
    }
  }

  function delTab ($class)
  {
    $this->removed_tabs["$class"] = $this->by_object["$class"];
    unset($this->by_object["$class"]);
    unset($this->by_name["$class"]);
  }

  function gen_tabs($disabled = FALSE)
  {
    $this->loadtabs($this->getBaseObject()->getGroupObjectTypes());
    return parent::gen_tabs($disabled);
  }

  function check($ignore_account = FALSE)
  {
    return parent::check(FALSE);
  }

  function save()
  {
    foreach ($this->removed_tabs as $key => &$obj) {
      if (isset($this->by_object[$key])) {
        continue;
      }
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $key, "Removing");

      $obj->dn = $this->dn;
      $obj->remove_from_parent();
    }
    unset($obj);

    parent::save();
  }

  function save_object($save_current = FALSE)
  {
    parent::save_object($save_current);

    /* Update reference, transfer variables */
    $baseobject = $this->getBaseObject();
    foreach ($this->by_object as $name => $obj) {

      /* Don't touch base object */
      if ($name != 'ogroup') {
        $obj->parent    = &$this;
        $obj->uid       = $baseobject->cn;
        $obj->cn        = $baseobject->cn;
        $obj->sn        = $baseobject->cn;
        $obj->givenName = $baseobject->cn;

        $this->by_object[$name] = $obj;
      }
    }

    /* Update parent in base object */
    $this->by_object['ogroup']->parent = &$this;
  }
}
?>
