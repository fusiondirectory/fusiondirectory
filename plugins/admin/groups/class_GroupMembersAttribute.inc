<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2020  FusionDirectory

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

class GroupMembersAttribute extends ObjectsAttribute
{
  protected $typeToCodeMap;
  protected $childTypes;

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $store_attr = 'dn', $display_attr = 'cn', array $filterElementDefinitions = NULL, $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, array_values(ogroup::$objectTypes), $defaultValue, $store_attr, $display_attr, $filterElementDefinitions, $acl);
    $this->typeToCodeMap = array_flip(array_map('strtolower', ogroup::$objectTypes));
    $this->selectManagementParameters[2]['gosaGroupObjects'] = '*';
  }

  function getFilterBlackList ()
  {
    return [
      'dn' => array_merge($this->getValue(), $this->plugin->used_workstations, [$this->plugin->dn])
    ];
  }

  protected function typeToCode ($type): string
  {
    if ($type === FALSE) {
      return 'I';
    } else {
      return $this->typeToCodeMap[strtolower($type)];
    }
  }

  function listObjectTypes ()
  {
    /* Refresh types and displays */
    $this->getDisplayValues();
    /* Merge all object types codes and all child types */
    $codes = array_unique(
      array_merge(
        array_map([$this, 'typeToCode'], array_unique($this->types)),
        str_split(preg_replace('/[^a-z]/i', '', implode('', $this->childTypes)))
      )
    );
    sort($codes);
    return '['.implode('', $codes).']';
  }

  function getTypedValues ()
  {
    $values = $this->getValue();
    $ret    = [];
    foreach ($values as $i => $v) {
      $code = $this->typeToCode($this->types[$i]);
      if (!isset($ret[$code])) {
        $ret[$code] = [];
      }
      $ret[$code][] = $v;
    }

    return $ret;
  }

  protected function fillDisplayValueFrom ($i, $attrs)
  {
    parent::fillDisplayValueFrom($i, $attrs);
    $this->childTypes[$i] = ($attrs['gosaGroupObjects'][0] ?? '');
  }

  function setValue ($value)
  {
    $this->childTypes = [];
    parent::setValue($value);
  }

  protected function removeValue ($row)
  {
    parent::removeValue($row);
    unset($this->childTypes[$row]);
  }
}
