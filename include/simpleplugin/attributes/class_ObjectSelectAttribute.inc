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
 * \file class_ObjectSelectAttribute.inc
 * Source code for ObjectSelectAttribute
 */

/*! \brief This class allows to handle a select attribute which allow to choose an object
 *
 */
class ObjectSelectAttribute extends SelectAttribute
{
  protected $objectTypes;
  protected $storeAttr;
  protected $displayAttr;
  protected $objectFilter = '';

  function __construct (string $label, string $description, string $ldapName, bool $required, array $objectTypes, string $storeAttr = 'dn', string $displayAttr = 'nameAttr', $objectFilter = '', $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, [], '', NULL, $acl);
    $this->objectTypes  = $objectTypes;
    $this->storeAttr    = $storeAttr;
    $this->displayAttr  = $displayAttr;
    $this->objectFilter = $objectFilter;
  }

  function setParent (simplePlugin &$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      $this->updateChoices();
    }
  }

  function updateChoices ()
  {
    $choices = [];
    $outputs = [];
    foreach ($this->objectTypes as $objectType) {
      try {
        $infos = objects::infos($objectType);
        $realStoreAttr = $this->storeAttr;
        if (in_array($this->storeAttr, ['nameAttr','mainAttr'])) {
          $realStoreAttr = $infos[$this->storeAttr];
        }
        $realDisplayAttr = $this->displayAttr;
        if (in_array($this->displayAttr, ['nameAttr','mainAttr'])) {
          $realDisplayAttr = $infos[$this->displayAttr];
        }
        $attributes = [
          $realStoreAttr    => (($realStoreAttr == 'dn') ? 'raw' : '1'),
          $realDisplayAttr  => (($realDisplayAttr == 'dn') ? 'raw' : '1'),
        ];
        $objects = objects::ls($objectType, $attributes, NULL, $this->objectFilter);
        foreach (array_column($objects, $realStoreAttr) as $choice) {
          $choices[] = $choice;
        }
        foreach (array_column($objects, $realDisplayAttr) as $output) {
          $outputs[] = $output;
        }
      } catch (NonExistingObjectTypeException $e) {
        continue;
      }
    }
    $this->setChoices($choices, $outputs);
  }
}
