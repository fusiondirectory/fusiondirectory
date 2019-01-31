<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2018-2019  FusionDirectory

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

class LockAction extends Action
{
  function __construct($name, $targets, $callable, array $acl = array(), $inmenu = TRUE, $inline = TRUE)
  {
    parent::__construct(
      $name,
      array('lock' => _('Lock users'), 'unlock' => _('Unlock users')),
      array(
        'lock'    => 'geticon.php?context=status&icon=object-locked&size=16',
        'unlock'  => 'geticon.php?context=status&icon=object-unlocked&size=16',
      ),
      $targets, $callable, $acl, $inmenu, $inline);
  }

  function isLocked(ListingEntry $entry)
  {
    if (isset($entry['userPassword']) && preg_match('/^\{[^\}]/', $entry['userPassword'])) {
      return (preg_match('/^[^\}]*+\}!/', $entry['userPassword']) === 1);
    } elseif ((strtolower($entry->type) == 'user') && !isset($entry['userPassword'])) {
      /* Empty lockable password */
      return FALSE;
    }
    return NULL;
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
      'name'      => $this->name.'_lock',
      'icon'      => $this->icon['lock'],
      'label'     => $this->label['lock'],
      'enabled'   => $this->isEnabledFor(),
      'separator' => $this->separator,
    );

    $actions[] = array(
      'name'      => $this->name.'_unlock',
      'icon'      => $this->icon['unlock'],
      'label'     => $this->label['unlock'],
      'enabled'   => $this->isEnabledFor(),
      'separator' => FALSE,
    );
  }

  function fillRowClasses(&$classes, ListingEntry $entry)
  {
    if ($this->isLocked($entry) === TRUE) {
      $classes[] = 'entry-locked';
    }
  }

  function renderColumnIcons(ListingEntry $entry)
  {
    if (!$this->inline) {
      return '';
    }

    // Skip the entry completely if there's no permission to execute it
    if (!$this->hasPermission($entry->dn, $entry->getTemplatedType(), $entry->isTemplate())) {
      return '<img src="images/empty.png" alt=" " class="center optional"/>';
    }

    $lockStatus = $this->isLocked($entry);
    if ($lockStatus === NULL) {
      return '<img src="images/empty.png" alt=" " class="center optional"/>';
    } elseif ($lockStatus) {
      // Render
      return '<input type="image" src="'.htmlentities($this->icon['lock'], ENT_COMPAT, 'UTF-8').'"'.
              ' title="'.$this->label['unlock'].'" alt="'.$this->label['unlock'].'" name="listing_'.$this->name.'_unlock_'.$entry->row.'"/>';
    } else {
      return '<input type="image" src="'.htmlentities($this->icon['unlock'], ENT_COMPAT, 'UTF-8').'"'.
              ' title="'.$this->label['lock'].'" alt="'.$this->label['lock'].'" name="listing_'.$this->name.'_lock_'.$entry->row.'"/>';
    }
  }
}
