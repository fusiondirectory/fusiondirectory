<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018  FusionDirectory

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

/*!
 * \brief Action base class
 */
class Action
{
  protected $name;
  protected $label;
  protected $icon;

  /* 0, 1, ?, + or * */
  protected $targets;

  protected $acl;

  /* Booleans */
  protected $inmenu;
  protected $inline;

  protected $callable;
  protected $enabledCallable;

  protected $minTargets;
  protected $maxTargets;

  protected $separator = FALSE;

  protected $parent;

  function __construct($name, $label, $icon, $targets, $callable, array $acls = array(), $inmenu = TRUE, $inline = TRUE)
  {
    if ($targets == '0') {
      $inline = FALSE;
    }

    $this->name     = $name;
    $this->label    = $label;
    $this->icon     = $icon;
    $this->targets  = $targets;
    $this->callable = $callable;
    $this->inmenu   = $inmenu;
    $this->inline   = $inline;
    $this->acl      = array();
    /*
     * acl may be of the form:
     * acl (ex: 'd')
     * attribute:acl (ex: 'userPassword:r')
     * category/class/acl (ex: 'user/template/r')
     * category/class/attribute:acl (ex: 'user/user/userPassword:r')
     * /class/acl (ex: '/snapshot/c')
     * /class/attribute:acl (ex: '/template/template_cn:w')
     * */
    foreach ($acls as $acl) {
      $category   = NULL;
      $class      = NULL;
      $attribute  = '0';
      if (strpos($acl, '/') !== FALSE) {
        list($category, $class, $acl) = explode('/', $acl, 3);
      }
      if (strpos($acl, ':') !== FALSE) {
        list($attribute, $acl) = explode(':', $acl, 2);
      }
      $this->acl[] = array(
        'category'  => $category,
        'class'     => $class,
        'attribute' => $attribute,
        'acl'       => str_split($acl),
      );
    }

    switch ($targets) {
      case '0':
        $this->minTargets = 0;
        $this->maxTargets = 0;
        break;
      case '1':
        $this->minTargets = 1;
        $this->maxTargets = 1;
        break;
      case '?':
        $this->minTargets = 0;
        $this->maxTargets = 1;
        break;
      case '+':
        $this->minTargets = 1;
        $this->maxTargets = FALSE;
        break;
      case '*':
        $this->minTargets = 0;
        $this->maxTargets = FALSE;
        break;
      default:
        throw new Exception('Invalid targets value for action '.$name.': '.$targets);
    }
  }

  function setParent(management $parent)
  {
    $this->parent = $parent;
  }

  function getName()
  {
    return $this->name;
  }

  function getLabel()
  {
    return $this->label;
  }

  function setSeparator($bool)
  {
    $this->separator = $bool;
  }

  function setEnableFunction(callable $callable)
  {
    $this->enabledCallable = $callable;
  }

  function listActions()
  {
    return array($this->name);
  }

  function execute($management, $action)
  {
    if ($this->callable === FALSE) {
      return;
    }
    if (count($action['targets']) < $this->minTargets) {
      throw new Exception('Not enough targets ('.count($action['targets']).') passed for action '.$name);
    }
    if (($this->maxTargets !== FALSE) && (count($action['targets']) > $this->maxTargets)) {
      throw new Exception('Too many targets ('.count($action['targets']).') passed for action '.$name);
    }
    $func = $this->callable;
    if (!is_array($func)) {
      $func = array($management, $func);
    }
    return call_user_func($func, $action);
  }

  function fillMenuItems(&$actions)
  {
    if (!$this->inmenu) {
      return;
    }

    if (!$this->hasPermission($this->parent->listing->getBase())) {
      return;
    }

    $actions[] = array(
      'name'      => $this->name,
      'icon'      => $this->icon,
      'label'     => $this->label,
      'enabled'   => $this->isEnabledFor(),
      'separator' => $this->separator,
    );
  }

  function fillRowClasses(&$classes, ListingEntry $entry)
  {
  }

  function renderColumnIcons(ListingEntry $entry)
  {
    if (!$this->inline) {
      return '';
    }

    // Skip the entry completely if there's no permission to execute it
    if (!$this->hasPermission($entry->dn, $entry->getTemplatedType(), $entry->isTemplate())) {
      return '<img src="images/empty.png" alt=" " class="optional"/>';
    }

    if (!$this->isEnabledFor($entry)) {
      return '<img src="'.htmlentities($this->icon.'&disabled=1', ENT_COMPAT, 'UTF-8').'"'.
              ' title="'.$this->label.'" alt="'.$this->label.'"/>';
    }

    // Render
    return '<input type="image" src="'.htmlentities($this->icon, ENT_COMPAT, 'UTF-8').'"'.
            ' title="'.$this->label.'" alt="'.$this->label.'" name="listing_'.$this->name.'_'.$entry->row.'"/>';
  }

  function isEnabledFor(ListingEntry $entry = NULL)
  {
    if (isset($this->enabledCallable)) {
      return call_user_func($this->enabledCallable, $this->name, $entry);
    }
    return TRUE;
  }

  function hasPermission($dn, $type = NULL, $template = FALSE)
  {
    global $ui;

    if ($type === NULL) {
      $types = $this->parent->objectTypes;
    } else {
      $types = array($type);
    }
    /*
     * if category is missing it’s deducted from type (all types are tested for menu actions)
     * if class is missing it’s deducted from attribute if present, otherwise it’s type mainTab
     * if attribute is missing 0 is used
     */
    foreach ($this->acl as $acl) {
      $checkAcl = '';
      if (!empty($acl['category'])) {
        $checkAcl = $ui->get_permissions($dn, $acl['category'].'/'.$acl['class'], $acl['attribute']);
      } elseif (empty($acl['class']) && ($acl['attribute'] != '0')) {
        foreach ($types as $type) {
          $module   = $ui->getAttributeCategory($type, $acl['attribute']);
          $checkAcl .= $ui->get_permissions($dn, $module, $acl['attribute']);
        }
      } else {
        foreach ($types as $type) {
          $infos = objects::infos($type);
          if (!empty($acl['class'])) {
            /* Class with empty category may be used in special cases like '/snapshot/c'*/
            $module = $infos['aclCategory'].'/'.$acl['class'];
          } elseif ($template) {
            $module = $infos['aclCategory'].'/template';
          } else {
            $module = $infos['aclCategory'].'/'.$infos['mainTab'];
          }
          $checkAcl .= $ui->get_permissions($dn, $module, $acl['attribute']);
        }
      }

      // Check rights
      foreach ($acl['acl'] as $part) {
        if (strpos($checkAcl, $part) === FALSE) {
          return FALSE;
        }
      }
    }

    return TRUE;
  }
}
