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

  /* Object types this action is present on */
  protected $validTypes;

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

  function __construct (string $name, $label, $icon, string $targets, $callable, array $acls = [], bool $inmenu = TRUE, bool $inline = TRUE, array $validTypes = [])
  {
    if ($targets == '0') {
      $inline = FALSE;
    }

    $this->name       = $name;
    $this->label      = $label;
    $this->icon       = $icon;
    $this->targets    = $targets;
    $this->callable   = $callable;
    $this->inmenu     = $inmenu;
    $this->inline     = $inline;
    $this->validTypes = array_map('strtoupper', $validTypes);
    $this->acl        = [];
    /*
     * acl may be of the form:
     * acl (ex: 'd')
     * attribute:acl (ex: 'userPassword:r')
     * category/class/acl (ex: 'user/template/r')
     * category/class/attribute:acl (ex: 'user/user/userPassword:r')
     * /class/acl (ex: '/SnapshotHandler/c')
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
      $this->acl[] = [
        'category'  => $category,
        'class'     => $class,
        'attribute' => $attribute,
        'acl'       => str_split($acl),
      ];
    }

    switch ($this->targets) {
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
        throw new FusionDirectoryException('Invalid targets value for action '.$this->name.': '.$this->targets);
    }
  }

  function setParent (management $parent)
  {
    $this->parent = $parent;
  }

  function getName (): string
  {
    return $this->name;
  }

  function getLabel ()
  {
    return $this->label;
  }

  function setSeparator (bool $bool)
  {
    $this->separator = $bool;
  }

  function setEnableFunction (callable $callable)
  {
    $this->enabledCallable = $callable;
  }

  function setInMenu (bool $inmenu)
  {
    $this->inmenu = $inmenu;
  }

  function listActions (): array
  {
    return [$this->name];
  }

  function execute (management $management, array $action)
  {
    if ($this->callable === FALSE) {
      return;
    }
    foreach ($action['targets'] as $targetDn) {
      if (!$this->hasPermission($management->listing->getEntry($targetDn))) {
        throw new FusionDirectoryException(sprintf(_('You are not allowed to execute action "%s" on target "%s"'), $this->name, $targetDn));
      }
    }
    if (count($action['targets']) < $this->minTargets) {
      throw new FusionDirectoryException(sprintf(_('Not enough targets (%d) passed for action "%s"'), count($action['targets']), $this->name));
    }
    if (($this->maxTargets !== FALSE) && (count($action['targets']) > $this->maxTargets)) {
      throw new FusionDirectoryException(sprintf(_('Too many targets (%d) passed for action "%s"'), count($action['targets']), $this->name));
    }
    $func = $this->callable;
    if (!is_array($func)) {
      $func = [$management, $func];
    }
    return call_user_func($func, $action);
  }

  function fillMenuItems (array &$actions)
  {
    if (!$this->inmenu) {
      return;
    }

    if (!$this->hasPermission()) {
      return;
    }

    $actions[] = [
      'name'      => $this->name,
      'icon'      => $this->icon,
      'label'     => $this->label,
      'enabled'   => $this->isEnabledFor(),
      'separator' => $this->separator,
    ];
  }

  function fillRowClasses (array &$classes, ListingEntry $entry)
  {
  }

  function renderColumnIcons (ListingEntry $entry): string
  {
    if (!$this->inline) {
      return '';
    }

    if (!empty($this->validTypes) && !($entry->isTemplate() && in_array('TEMPLATE', $this->validTypes)) && !in_array($entry->type, $this->validTypes)) {
      return '';
    }

    // Skip the entry completely if there's no permission to execute it
    if (!$this->hasPermission($entry)) {
      return '<img src="images/empty.png" alt=" " class="optional"/>';
    }

    if (!$this->isEnabledFor($entry)) {
      return '<img src="'.htmlescape($this->icon.'&disabled=1').'"'.
              ' title="'.htmlescape($this->label).'" alt="'.htmlescape($this->label).'"/>';
    }

    // Render
    return '<input type="image" src="'.htmlescape($this->icon).'"'.
            ' title="'.htmlescape($this->label).'" alt="'.htmlescape($this->label).'" name="listing_'.$this->name.'_'.$entry->row.'"/>';
  }

  function isEnabledFor (ListingEntry $entry = NULL): bool
  {
    if (isset($this->enabledCallable)) {
      return call_user_func($this->enabledCallable, $this->name, $entry);
    }
    return TRUE;
  }

  function hasPermission (ListingEntry $entry = NULL): bool
  {
    global $ui;

    if ($entry === NULL) {
      $dn       = $this->parent->listing->getBase();
      $types    = $this->parent->objectTypes;
      $template = FALSE;
    } else {
      $dn       = $entry->aclBase;
      $types    = [$entry->getTemplatedType()];
      $template = $entry->isTemplate();
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
            /* Class with empty category may be used in special cases like '/SnapshotHandler/c'*/
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
