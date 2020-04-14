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

/*! \brief This class allows to handle an attribute for selecting user or groups
 *
 * It looks like a SetAttribute, but clicking "Add" will open a dialog that allow to select one or more users or groups.
 * It stores their dn as values, but displays the cn.
 */
class UsersGroupsRolesAttribute extends ObjectsAttribute
{
  function __construct (string $label, string $description, string $ldapName, bool $required = FALSE, array $defaultValue = [], string $store_attr = 'dn', string $display_attr = 'cn', array $filterElementDefinitions = NULL, string $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, ['user', 'group', 'role', 'ogroup'], $defaultValue, $store_attr, $display_attr, $filterElementDefinitions, $acl);
    $this->selectManagementParameters[2]['cn']          = '*';
    $this->selectManagementParameters[2]['uid']         = '*';
    $this->selectManagementParameters[2]['objectClass'] = '*';
  }

  protected function ldapAttributesToGet ()
  {
    return array_values(array_unique([$this->store_attr, $this->display_attr, 'cn', 'uid', 'objectClass']));
  }

  protected function fillDisplayValueFrom ($i, $attrs)
  {
    if ($this->isTemplate() && ($attrs === NULL)) {
      $this->displays[$i] = $this->value[$i];
    } elseif (!isset($attrs[$this->display_attr])) {
      unset($this->value[$i]);
    } else {
      $cn = $attrs['cn'][0];
      if ($this->display_attr == 'dn') {
        $this->displays[$i] = $attrs['dn'];
      } elseif (in_array('posixGroup', $attrs['objectClass'])) {
        $this->displays[$i] = sprintf(_('POSIX group %s'), trim($cn));
      } elseif (in_array('organizationalRole', $attrs['objectClass'])) {
        $this->displays[$i] = sprintf(_('Role %s'), trim($cn));
      } elseif (in_array('groupOfNames', $attrs['objectClass'])) {
        $this->displays[$i] = sprintf(_('Group %s'), trim($cn));
      } elseif (isset($attrs['uid'])) {
        $this->displays[$i] = trim($attrs['uid'][0]);
      } else {
        $this->displays[$i] = $attrs[$this->display_attr][0];
      }
    }
  }
}
