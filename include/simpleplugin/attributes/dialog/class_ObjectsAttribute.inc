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

  function __construct (string $label, string $description, string $ldapName, bool $required, array $objectTypes, array $defaultValue = [], string $store_attr = 'dn', string $display_attr = 'nameAttr', array $filterElementDefinitions = NULL, string $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $store_attr, $display_attr, $acl);
    $attributes = [
      'objectClass' => '*',
      'dn'          => 'raw',
    ];
    if (!in_array($store_attr, ['dn','nameAttr','mainAttr'])) {
      $attributes[$store_attr] = '*';
    }
    if (!in_array($display_attr, ['dn','nameAttr','mainAttr'])) {
      $attributes[$display_attr] = '*';
    }
    $specialAttributes = array_intersect([$store_attr,$display_attr], ['nameAttr','mainAttr']);
    foreach ($objectTypes as $type) {
      try {
        if (!empty($specialAttributes)) {
          $infos = objects::infos($type);
          foreach ($specialAttributes as $attribute) {
            $attributes[$infos[$attribute]] = '*';
          }
        }
        $filterAttributes = objects::getFilterObject($type)->listUsedAttributes();
        foreach ($filterAttributes as $attribute) {
          if ($attribute != 'dn') {
            $attributes[$attribute] = '*';
          }
        }
      } catch (NonExistingObjectTypeException $e) {
        continue;
      }
    }
    $this->selectManagementParameters = [$objectTypes,TRUE,$attributes];
    $this->filterElementDefinitions   = $filterElementDefinitions;
  }

  protected function ldapAttributesToGet (): array
  {
    return array_keys($this->selectManagementParameters[2]);
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

  protected function fillDisplayValueFrom ($i, $attrs)
  {
    if ($attrs) {
      $objectType = NULL;
      if (is_array($attrs)) {
        foreach ($this->selectManagementParameters[0] as $type) {
          try {
            if (objects::isOfType($attrs, $type)) {
              $objectType = $type;
            }
          } catch (NonExistingObjectTypeException $e) {
            continue;
          }
        }
      } else {
        $objectType = $attrs->getTemplatedType();
      }
      if ($objectType !== NULL) {
        $infos = objects::infos($objectType);
        if (in_array($this->display_attr, ['nameAttr','mainAttr'])) {
          $display = $attrs[$infos[$this->display_attr]][0];
        } else {
          $display = $attrs[$this->display_attr][0];
        }
        $this->displays[$i] = sprintf(_('%s (%s)'), trim($display), $infos['name']);
      }
      if (!isset($this->displays[$i])) {
        trigger_error('Unkown type for "'.$this->value[$i].'"');
        $this->displays[$i] = sprintf(_('Unknown type : %s'), $this->value[$i]);
      }
    } elseif (($attrs === NULL) && $this->isTemplate()) {
      $this->displays[$i] = $this->value[$i];
    } else {
      $this->displays[$i] = sprintf(_('Non existing dn: %s'), $this->value[$i]);
    }
  }
}
