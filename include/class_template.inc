<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014-2016  FusionDirectory

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

/* Class for applying a template */
class template
{
  protected $type;
  protected $dn;
  protected $needed;
  protected $attrs;
  protected $tabObject;
  protected $attributes;

  protected $applied = FALSE;

  function __construct($type, $dn, $targetdn = NULL)
  {
    $this->type = $type;
    $this->dn   = $dn;
    list($this->attrs, $depends) = plugin::tpl_fetch_template($this->dn);
    $this->needed     = plugin::tpl_needed_attrs($this->attrs, $depends);
    if ($targetdn === NULL) {
      $this->tabObject  = objects::create($this->type);
    } else {
      trigger_error("This should not be used for now");
      $this->tabObject  = objects::open($this->dn, $this->type);
    }
    $tempTabObject    = objects::open($this->dn, $this->type); /* Used to know which tab is activated */
    $this->attributes = array();
    foreach ($tempTabObject->by_object as $class => &$plugin) {
      if ($plugin->is_account || $plugin->ignore_account) {
        $this->tabObject->by_object[$class]->is_account = $plugin->is_account;
        $this->attributes[$class] = array();
        $attrs = array_unique(array_merge($this->tabObject->by_object[$class]->getRequiredAttributes(), $this->needed));
        foreach ($attrs as $attr) {
          if (isset($this->attrs[$attr])) {
            continue;
          }
          if (isset($this->tabObject->by_object[$class]->attributesAccess[$attr])) {
            $this->attributes[$class][] = $attr;
          }
        }
      }
    }
  }

  /* Used when you need to re-apply the same template with different values */
  function reset()
  {
    list($this->attrs, ) = plugin::tpl_fetch_template($this->dn);
    $this->tabObject  = objects::create($this->type);
    $tempTabObject    = objects::open($this->dn, $this->type); /* Used to know which tab is activated */
    foreach ($tempTabObject->by_object as $class => &$plugin) {
      if ($plugin->is_account || $plugin->ignore_account) {
        $this->tabObject->by_object[$class]->is_account = $plugin->is_account;
      }
    }
    $this->applied = FALSE;
  }

  function getDn()
  {
    return $this->dn;
  }

  function getBase()
  {
    $infos = objects::infos($this->type);
    return dn2base($this->dn, 'ou=templates,'.$infos['ou']);
  }

  function getNeeded()
  {
    return $this->attributes;
  }

  function serialize()
  {
    $attributes = array();
    foreach ($this->tabObject->by_object as $class => &$plugin) {
      if (!isset($this->attributes[$class])) {
        continue;
      }
      $attributes[$class] = array('name' => $this->tabObject->by_name[$class], 'attrs' => array());
      foreach ($this->attributes[$class] as $attr) {
        $plugin->attributesAccess[$attr]->serializeAttribute($attributes[$class]['attrs']);
      }
    }

    return $attributes;
  }

  function deserialize($values)
  {
    foreach ($values as $class => $class_values) {
      $this->tabObject->by_object[$class]->deserializeValues($class_values);
    }
  }

  function save_object()
  {
    foreach ($this->tabObject->by_object as $class => &$plugin) {
      $plugin->save_object();
    }
    unset($plugin);
  }

  function execute()
  {
    $smarty   = get_smarty();
    $sections = array();
    $posted   = array();
    foreach ($this->tabObject->by_object as $class => &$plugin) {
      if (!isset($this->attributes[$class])) {
        continue;
      }
      $attributes = array();
      foreach ($this->attributes[$class] as $attr) {
        if ($plugin->attributesAccess[$attr]->getAclInfo() !== FALSE) {
          // We assign ACLs so that attributes can use them in their template code
          $smarty->assign($plugin->attributesAccess[$attr]->getAcl()."ACL", $plugin->getacl($plugin->attributesAccess[$attr]->getAcl()));
        }
        $plugin->attributesAccess[$attr]->renderAttribute($attributes, FALSE);
      }

      $smarty->assign('section', $class);
      $smarty->assign('sectionId', $class);
      $smarty->assign('sectionClasses', ' fullwidth');
      $smarty->assign('attributes', $attributes);

      $posted[]   = $class.'_posted';
      $sections[] = $smarty->fetch(get_template_path('simpleplugin_section.tpl'));
    }
    unset($plugin);

    $smarty->assign('sections', $sections);
    $smarty->assign('hiddenPostedInput', $posted);
    $smarty->assign('focusedField', '');

    return $smarty->fetch(get_template_path('simpleplugin.tpl'));
  }

  /* Apply template and current values to an object and returns it for saving or edition
   * Cannot be called twice! If you need to, call reset between calls */
  function apply($targetdn = NULL)
  {
    if ($targetdn !== NULL) {
      $this->tabObject = objects::open($targetdn, $this->type);
    } elseif ($this->applied) {
      trigger_error('Templates can’t be applied twice without calling reset before');
      return;
    }

    foreach ($this->tabObject->by_object as $class => &$plugin) {
      if (!isset($this->attributes[$class])) {
        continue;
      }
      foreach ($this->attributes[$class] as $attr) {
        $plugin->attributesAccess[$attr]->fillLdapValue($this->attrs);
      }
    }
    unset($plugin);
    foreach ($this->tabObject->by_object as $class => &$plugin) {
      if (!isset($this->attributes[$class])) {
        continue;
      }
      foreach ($this->attributes[$class] as $attr) {
        $plugin->attributesAccess[$attr]->fillLdapValueHook($this->attrs);
      }
    }
    unset($plugin);
    foreach ($this->attrs as &$array) {
      if (!is_array($array)) {
        $array = array($array);
      }
      if (!isset($array['count'])) {
        $array['count'] = count($array);
      }
    }
    unset($array);

    $this->attrs = plugin::tpl_parse_attrs($this->attrs);
    $this->tabObject->adapt_from_template($this->attrs, call_user_func_array('array_merge', $this->attributes));
    $this->tabObject->getBaseObject()->base = $this->getBase();

    $this->applied = TRUE;
    return $this->tabObject;
  }
}
