<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

/*! \brief   multiPlugin
    This class provides the functionality to have multiple plugins in a single tab
 */

class multiPlugin extends simplePlugin
{
  /* attribute list for save action */
  var $objectclasses        = [];
  protected $ignore_account = TRUE;
  var $plugin               = [];

  var $tabClass = '';

  static function getAttributesInfo (): array
  {
    return [];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL)
  {
    global $config;
    parent::__construct($dn, $object, $parent, FALSE);

    $plInfos = pluglist::pluginInfos(get_class($this));

    /* Load accounts */
    foreach ($config->data['TABS'][$this->tabClass] as $plug) {
      if (!plugin_available($plug['CLASS'])) {
        continue;
      }
      $name = $plug['CLASS'];
      $this->plugin[$name] = new $name($dn, $object, $parent);

      /* Acl base && category configuration,
          these settings will be overloaded in main.inc,
          if we are editing ourself */
      $this->plugin[$name]->set_acl_category($plInfos['plCategory'][0]);
    }
  }

  function aclHasPermissions (): bool
  {
    foreach ($this->plugin as $plug) {
      if (!$plug->aclHasPermissions()) {
        /* If one of our plugins has no permission handling (always shown), we do not either */
        return FALSE;
      }
    }

    return TRUE;
  }

  function aclGetPermissions ($attribute = '0', string $base = NULL, bool $skipWrite = FALSE): string
  {
    if (isset($this->parent) && isset($this->parent->ignoreAcls) && $this->parent->ignoreAcls) {
      return 'cdmr'.($skipWrite ? '' : 'w');
    }

    $skipWrite |= $this->readOnly();
    if ($base === NULL) {
      $base = $this->getAclBase();
    }

    $acl = '';
    foreach ($this->plugin as $plug) {
      $acl .= $plug->aclGetPermissions($attribute, $base, $skipWrite);
    }

    return implode('', array_unique(str_split($acl)));
  }

  function execute (): string
  {
    $display = "";

    /* Do we represent a valid account? */
    if ($this->parent === NULL) {
      $enabled = FALSE;
      foreach ($this->plugin as &$plug) {
        if ($plug->is_account) {
          $enabled = TRUE;
          break;
        }
      }
      unset($plug);
      if (!$enabled) {
        $display = '<img alt="'._('Error').'" src="geticon.php?context=status&amp;icon=dialog-error&amp;size=16" align="middle"/>&nbsp;<b>'.
                    msgPool::noValidExtension('').'</b>';
        return $display;
      }
    }

    /* Display plugins */
    $readOnly = $this->readOnly();

    foreach ($this->plugin as $plug) {
      $plug->read_only = $readOnly;
      $display .= $plug->execute();
    }

    return $display;
  }


  /* Save data to object */
  function save_object ()
  {
    foreach ($this->plugin as &$plug) {
      $plug->save_object();
    }
    unset($plug);
  }

  function check (): array
  {
    $errors = parent::check();

    foreach ($this->plugin as &$plug) {
      if ($plug->isActive()) {
        $tmp    = $plug->check();
        $errors = array_merge($errors, $tmp);
      }
    }
    unset($plug);

    return $errors;
  }

  function set_acl_category (string $cat)
  {
    parent::set_acl_category($cat);
    foreach ($this->plugin as &$plug) {
      $plug->set_acl_category($cat);
    }
    unset($plug);
  }

  public function setNeedEditMode (bool $bool)
  {
    parent::setNeedEditMode($bool);
    foreach ($this->plugin as &$plug) {
      $plug->setNeedEditMode($bool);
    }
    unset($plug);
  }

  /* Save to LDAP */
  function save (): array
  {
    $errors = [];
    /* Save objects */
    foreach ($this->plugin as &$plug) {
      $plug->dn = $this->dn;
      if ($plug->isActive()) {
        $result = $plug->save();
      } else {
        $result = $plug->remove(FALSE);
      }
      if (!empty($result)) {
        $errors = array_merge($errors, $result);
      }
    }
    unset($plug);
    return $errors;
  }

  function remove (bool $fulldelete = FALSE): array
  {
    $errors = [];
    /* Remove objects */
    foreach ($this->plugin as &$plug) {
      $plug->dn = $this->dn;
      $result = $plug->remove($fulldelete);
      if (!empty($result)) {
        $errors = array_merge($errors, $result);
      }
    }
    unset($plug);
    return $errors;
  }

  function adapt_from_template (array $attrs, array $skip = [])
  {
    /* Adapt objects */
    foreach ($this->plugin as &$plug) {
      $plug->dn = $this->dn;
      $plug->adapt_from_template($attrs, $skip);
    }
    unset($plug);
  }

  function resetCopyInfos ()
  {
    $this->dn = 'new';
    foreach ($this->plugin as &$plug) {
      $plug->resetCopyInfos();
    }
    unset($plug);
  }
}
