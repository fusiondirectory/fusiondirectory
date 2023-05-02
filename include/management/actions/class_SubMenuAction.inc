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
 * \brief Action which unfold a submenu
 */
class SubMenuAction extends Action
{
  protected $actions  = [];
  protected $handlers = [];

  function __construct (string $name, $label, $icon, array $actions, bool $inmenu = TRUE)
  {
    parent::__construct($name, $label, $icon, '0', FALSE, [], $inmenu, FALSE);
    $this->actions = $actions;
    foreach ($this->actions as $action) {
      $names = $action->listActions();
      foreach ($names as $name) {
        $this->handlers[$name] = $action;
      }
    }
  }

  function addAction (Action $action)
  {
    $this->actions[] = $action;
    $names = $action->listActions();
    foreach ($names as $name) {
      $this->handlers[$name] = $action;
    }
    /* Rerun registerAction process */
    $this->parent->registerAction($this);
  }

  function setParent (management $parent)
  {
    parent::setParent($parent);
    foreach ($this->actions as $action) {
      $action->setParent($parent);
    }
  }

  function listActions (): array
  {
    return array_keys($this->handlers);
  }

  function execute (management $management, array $action)
  {
    if (isset($action['subaction']) && isset($this->handlers[$action['action'].'_'.$action['subaction']])) {
      return $this->handlers[$action['action'].'_'.$action['subaction']]->execute($management, $action);
    } elseif (isset($this->handlers[$action['action']])) {
      return $this->handlers[$action['action']]->execute($management, $action);
    }
  }

  function fillMenuItems (array &$actions)
  {
    if (!$this->inmenu) {
      return;
    }

    $subactions = [];
    foreach ($this->actions as $action) {
      $action->fillMenuItems($subactions);
    }

    if (!empty($subactions)) {
      $actions[] = [
        'name'      => $this->name,
        'icon'      => $this->icon,
        'label'     => $this->label,
        'separator' => $this->separator,
        'actions'   => $subactions
      ];
    }
  }

  function renderColumnIcons (ListingEntry $entry): string
  {
    return '';
  }
}
