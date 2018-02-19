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

  /* 0, 1, + or * */
  protected $targets;

  /* Booleans */
  protected $inmenu;
  protected $inline;

  protected $callable;

  protected $minTargets;
  protected $maxTargets;

  function __construct($name, $label, $icon, $targets, $callable, $inmenu = TRUE, $inline = TRUE)
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

    switch ($targets) {
      case '0':
        $this->minTargets = 0;
        $this->maxTargets = 0;
        break;
      case '1':
        $this->minTargets = 1;
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

  function renderMenuItems()
  {
    if (!$this->inmenu) {
      return '';
    }
    return '<li id="actionmenu_'.$this->name.'">'
            .'<a href="#" onClick="'
              ."document.getElementById('actionmenu').value='".$this->name."';document.getElementById('exec_act').click();"
            .'">'
            .'<img src="'.htmlentities($this->icon, ENT_COMPAT, 'UTF-8').'" alt="'.$this->name.'" class="center">&nbsp;'.$this->label.'</a>'
            .'</li>'."\n";
  }
}

/*!
 * \brief Action which unfold a submenu
 */
class ActionSubMenu extends Action
{
  protected $actions = array();

  function __construct($name, $label, $icon, array $actions, $inmenu = TRUE)
  {
    parent::__construct($name, $label, $icon, '0', FALSE, $inmenu, FALSE);
    foreach ($actions as $action) {
      $names = $action->listActions();
      foreach ($names as $name) {
        $this->actions[$name] = $action;
      }
    }
  }

  function listActions()
  {
    $actions = array();
    foreach ($this->actions as $action) {
      $actions = array_merge($actions, $action->listActions());
    }
    return $actions;
  }

  function execute($management, $action)
  {
    return $this->actions[$action['action']]->execute($management, $action);
  }

  function renderMenuItems($level = 2)
  {
    if (!$this->inmenu) {
      return '';
    }
    $result = '';
    if (!empty($this->label)) {
      $img = '';
      if (!empty($this->icon)) {
        $img = '<img class="center" src="'.htmlentities($this->icon, ENT_COMPAT, 'UTF-8').'" alt="'.$this->label.'"/>&nbsp;';
      }
      $result .= '<li id="actionmenu_'.$this->name.'"><a href="#">'.$img.'&nbsp;'.$this->label.'&nbsp;<img src="images/forward-arrow.png" alt="forward arrow"/></a>'."\n";
    }

    $level++;
    $result .= '<ul class="level'.$level.'">'."\n";
    foreach ($this->actions as $action) {
      $result .= $action->renderMenuItems($level);
    }
    return $result.'</li></ul>'."\n";
  }
}
