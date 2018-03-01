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

  protected $minTargets;
  protected $maxTargets;

  function __construct($name, $label, $icon, $targets, $callable, array $acl = array(), $inmenu = TRUE, $inline = TRUE)
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
    $this->acl      = $acl;

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

  function getName()
  {
    return $this->name;
  }

  function getLabel()
  {
    return $this->label;
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
    $actions[] = array(
      'name'  => $this->name,
      'icon'  => $this->icon,
      'label' => $this->label
    );
  }

  function getRowClasses(ListingEntry $entry)
  {
    return array();
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

    // TODO? Skip entry if the pseudo filter does not fit
    // TODO? Skip depending on type

    // Render
    return '<input type="image" src="'.htmlentities($this->icon, ENT_COMPAT, 'UTF-8').'"'.
            ' title="'.$this->label.'" alt="'.$this->label.'" name="listing_'.$this->name.'_'.$entry->row.'"/>';
  }

  function hasPermission($dn, $type, $template)
  {
    global $ui;

    $infos  = objects::infos($type);
    foreach ($this->acl as $acl) {
      if (preg_match('/([a-zA-Z0-9]+):([rwcdm]+)/', $acl, $m)) {
        /* attribute:rw */
        $module   = $ui->getAttributeCategory($type, $m[1]);
        $checkAcl = $ui->get_permissions($dn, $module, $m[1]);
        $acl      = $m[2];
      } else {
        if ($template) {
          $module = $infos['aclCategory'].'/template';
        } else {
          $module = $infos['aclCategory'].'/'.$infos['mainTab'];
        }
        $checkAcl = $ui->get_permissions($dn, $module, '0');
      }

      // Split up remaining part of the acl and check rights
      $parts = str_split($acl);
      foreach ($parts as $part) {
        if (strpos($checkAcl, $part) === FALSE) {
          return FALSE;
        }
      }
    }

    return TRUE;
  }
}

/*!
 * \brief Action which unfold a submenu
 */
class ActionSubMenu extends Action
{
  protected $actions  = array();
  protected $handlers = array();

  function __construct($name, $label, $icon, array $actions, $inmenu = TRUE)
  {
    parent::__construct($name, $label, $icon, '0', FALSE, array(), $inmenu, FALSE);
    $this->actions = $actions;
    foreach ($this->actions as $action) {
      $names = $action->listActions();
      foreach ($names as $name) {
        $this->handlers[$name] = $action;
      }
    }
  }

  function listActions()
  {
    return array_keys($this->handlers);
  }

  function execute($management, $action)
  {
    if (isset($action['subaction']) && isset($this->handlers[$action['action'].'_'.$action['subaction']])) {
      return $this->handlers[$action['action'].'_'.$action['subaction']]->execute($management, $action);
    } elseif (isset($this->handlers[$action['action']])) {
      return $this->handlers[$action['action']]->execute($management, $action);
    }
  }

  function fillMenuItems(&$actions)
  {
    if (!$this->inmenu) {
      return;
    }
    $subactions = array();
    foreach ($this->actions as $action) {
      $action->fillMenuItems($subactions);
    }
    $actions[] = array(
      'name'    => $this->name,
      'icon'    => $this->icon,
      'label'   => $this->label,
      'actions' => $subactions
    );
  }

  function renderColumnIcons(ListingEntry $entry)
  {
  }
}

/*!
 * \brief Action hidden from both column and menu
 */
class HiddenAction extends Action
{
  function __construct($name, $callable, array $acl = array())
  {
    parent::__construct($name, '', '', '0', $callable, $acl, FALSE, FALSE);
  }
}
