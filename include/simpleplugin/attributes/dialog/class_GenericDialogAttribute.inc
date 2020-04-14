<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2020  FusionDirectory

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
 * \brief Base class for DialogAttribute using a GenericDialog derived dialog
 */
class GenericDialogAttribute extends DialogAttribute
{
  protected $displays     = [];
  protected $store_attr   = 'dn';
  protected $display_attr = 'cn';

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $store_attr = 'dn', $display_attr = 'cn', $acl = '')
  {
    $this->store_attr   = $store_attr;
    $this->display_attr = $display_attr;
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
  }

  function getFilterBlackList ()
  {
    return [$this->store_attr => $this->getValue()];
  }

  function addValue (string $dn, $attrs = NULL)
  {
    if (
        ($this->isTemplate() && ($attrs === NULL)) ||
        ($this->store_attr == 'dn')
      ) {
      $value = $dn;
    } else {
      $value = $attrs[$this->store_attr][0];
    }
    if (!in_array($value, $this->value)) {
      end($this->value);
      $new_key = key($this->value) + 1;
      $this->value[$new_key] = $value;
      $this->fillDisplayValueFrom($new_key, $attrs);
    }
  }

  protected function removeValue ($row)
  {
    unset($this->displays[$row]);
    unset($this->value[$row]);
  }

  protected function sortValues ()
  {
    asort($this->value);
  }

  function getDisplayValues ()
  {
    foreach (array_keys($this->value) as $i) {
      if (!isset($this->displays[$i])) {
        $this->fillDisplayValue($i);
      }
    }
    return $this->displays;
  }

  function setDisplayValues (array $array)
  {
    $this->displays = $array;
  }

  protected function fillDisplayValue ($i)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $value = $this->value[$i];
    if ($this->store_attr == 'dn') {
      $ldap->cat($value, $this->ldapAttributesToGet());
    } else {
      $ldap->cd($config->current['BASE']);
      $ldap->search('('.$this->store_attr.'='.ldap_escape_f($value).')', $this->ldapAttributesToGet());
    }
    $attrs = $ldap->fetch(TRUE);
    if (empty($attrs) && $this->isTemplate()) {
      $this->fillDisplayValueFrom($i, NULL);
    } else {
      $this->fillDisplayValueFrom($i, $attrs);
    }
  }

  protected function ldapAttributesToGet ()
  {
    return [$this->display_attr];
  }

  protected function fillDisplayValueFrom ($i, $attrs)
  {
    if ($this->isTemplate() && ($attrs === NULL)) {
      $this->displays[$i] = $this->value[$i];
    } elseif (!isset($attrs[$this->display_attr])) {
      unset($this->value[$i]);
    } else {
      if ($this->display_attr == 'dn') {
        $this->displays[$i] = $attrs['dn'];
      } else {
        $this->displays[$i] = $attrs[$this->display_attr][0];
      }
    }
  }

  function foreignKeyUpdate ($oldvalue, $newvalue, array $source)
  {
    foreach ($this->value as $key => &$value) {
      if (($source['FIELD'] == 'dn') && ($source['MODE'] == 'move')) {
        if ($newvalue === NULL) {
          if (preg_match('/'.preg_quote($oldvalue, '/').'$/', $value)) {
            $this->removeValue($key);
          }
        } else {
          $value = preg_replace('/'.preg_quote($oldvalue, '/').'$/', $newvalue, $value, -1, $count);
          if ($count > 0) {
            /* Update display */
            $this->fillDisplayValue($key);
          }
        }
      } elseif ($value == $oldvalue) {
        if ($newvalue === NULL) {
          $this->removeValue($key);
        } elseif ($source['MODE'] == 'copy') {
          $this->value[] = $newvalue;
        } elseif ($source['MODE'] == 'move') {
          $value = $newvalue;
          /* Update display */
          $this->fillDisplayValue($key);
        }
      }
    }
    unset($value);
  }
}
