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

/*! \brief This class allows to handle an attribute for selecting objects
 *
 * It looks like a SetAttribute, but clicking "Add" will open a dialog that allow to select one or more objects.
 * It stores their dn as values, but displays the cn.
 *
 */
class ObjectsAttribute extends GenericDialogAttribute
{
  protected $dialogClass  = 'GenericSelectManagementDialog';

  protected $selectManagementParameters;
  protected $filterElementDefinitions;

  function __construct (string $label, string $description, string $ldapName, bool $required, array $objectTypes, array $defaultValue = [], string $store_attr = 'dn', string $display_attr = 'cn', array $filterElementDefinitions = NULL, string $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $store_attr, $display_attr, $acl);
    $this->selectManagementParameters = [
      $objectTypes,
      TRUE,
      [
        'objectClass' => '*',
        $store_attr   => (($store_attr == 'dn') ? 'raw' : '*'),
        $display_attr => (($display_attr == 'dn') ? 'raw' : '*'),
      ]
    ];
    $this->filterElementDefinitions = $filterElementDefinitions;
  }

  public function getSelectManagementParameters (): array
  {
    $parameters = array_merge(
      $this->selectManagementParameters,
      [
        $this->getFilterBlackList(),
        $this->getFilterWhiteList(),
      ]
    );
    if (isset($this->filterElementDefinitions)) {
      $parameters[] = $this->filterElementDefinitions;
    }
    return $parameters;
  }
}
