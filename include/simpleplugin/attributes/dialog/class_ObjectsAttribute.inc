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
  protected $types = [];

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
    foreach ($objectTypes as $i => $type) {
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
        unset($objectTypes[$i]);
      }
    }
    $this->selectManagementParameters = [array_values($objectTypes),TRUE,$attributes];
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

  protected function fillDisplayValue ($i)
  {
    $value = $this->value[$i];

    // Fixing potentially visual for wildcard string
    if ($value === '*') {
      $this->displays[$i] = 'Any';
      $this->types[$i]    = FALSE;
      return;
    }

    try {
      if ($this->store_attr == 'dn') {
        $objects = objects::ls($this->selectManagementParameters[0], $this->selectManagementParameters[2], $value, '', FALSE, 'base');
      } else {
        $objects = objects::ls($this->selectManagementParameters[0], $this->selectManagementParameters[2], NULL, '('.$this->store_attr.'='.ldap_escape_f($value).')');
      }
    } catch (EmptyFilterException $e) {
      $objects = [];
    }
    if (empty($objects) && $this->isTemplate()) {
      $this->fillDisplayValueFrom($i, NULL);
    } else {
      $this->fillDisplayValueFrom($i, reset($objects));
    }
  }

  protected function fillDisplayValueFrom ($i, $attrs)
  {
    global $config;
    $defaultDn = 'uid=default,ou=nonexistent,' . $config->current['BASE'];

    $objectType = NULL; // <-- Add this line

    if ($attrs) {
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
        if (in_array($this->display_attr, ['nameAttr','mainAttr'])) {
          $infos = objects::infos($objectType);
          $display = $attrs[$infos[$this->display_attr]][0];
        } else {
          $display = $attrs[$this->display_attr][0];
        }
        $this->displays[$i] = trim($display);
        $this->types[$i]    = $objectType;
      }
      if (!isset($this->displays[$i])) {
        trigger_error('Unkown type for "'.$this->value[$i].'"');
        $this->displays[$i] = sprintf(_('Unknown type : %s'), $this->value[$i]);
        $this->types[$i]    = FALSE;
      }
    } elseif (($attrs === NULL) && $this->isTemplate()) {
      $this->displays[$i] = $this->value[$i];
      $this->types[$i]    = FALSE;
    } else {
      // Special handling for the default user DN
      if ($this->value[$i] === $defaultDn) {
        $this->displays[$i] = '<span class="default-user-dn">' . _('Default user placeholder') . '</span>';
        $this->types[$i]    = FALSE; // Mark type as FALSE for placeholder
      } else {
        // Handle other non-existing DNs by setting display and type to FALSE
        // This prevents errors in later code expecting these keys to be set.
        $this->displays[$i] = sprintf(_('[Non-existent DN: %s]'), $this->value[$i]); // Indicate non-existence clearly
        $this->types[$i]    = FALSE; // Mark type as FALSE
      }
    }
  }

  function renderOnlyFormInput (): string
  {
    if (($this->size < 15) && ($this->size < count($this->value))) {
      $this->size = min(15, count($this->value));
    }
    $id       = $this->getHtmlId();
    $display  = '<select multiple="multiple" name="row'.$id.'[]" id="row'.$id.'" size="'.$this->size.'"'.
                ($this->disabled ? ' disabled="disabled"' : '').
                ' >'."\n";
    foreach ($this->getDisplayValues() as $key => $value) {
      $infos = [];
      try {
        if ($this->types[$key] !== FALSE) {
          $infos = objects::infos($this->types[$key]);
        }
      } catch (NonExistingObjectTypeException $e) {
        /* Ignore non-existing types, plugins may have been removed */
      }
      if (isset($infos['icon'])) {
        $img = $infos['icon'];
      } else {
        $img = 'images/empty.png';
      }
      $display .= '<option value="'.$key.'" class="select"'.
                  ' style="background-image:url(\''.get_template_path($img).'\');"'.
                  '>'.$value."</option>\n";
    }
    $display .= '</select><br/>'."\n";
    return $display;
  }

  function setValue ($value)
  {
    global $config; // Needed for defaultDn comparison

    // Reset types and let parent handle initial population and display filling
    $this->types = [];
    parent::setValue($value);

    // Now, filter out entries that are marked as non-existent (type === FALSE)
    // unless it's the special default user placeholder.
    $defaultDn = 'uid=default,ou=nonexistent,' . $config->current['BASE'];
    $filteredValue = [];
    $filteredDisplays = [];
    $filteredTypes = [];

    foreach ($this->value as $key => $dn) {
      // Keep if type is not FALSE (valid object) OR if it's the default user DN
      if (isset($this->types[$key]) && ($this->types[$key] !== FALSE || $dn === $defaultDn)) {
        $filteredValue[$key] = $this->value[$key];
        $filteredDisplays[$key] = $this->displays[$key];
        $filteredTypes[$key] = $this->types[$key];
      }
      // Entries where $this->types[$key] is FALSE (and not the defaultDn) are skipped
    }

    // Replace the internal arrays with the filtered versions
    $this->value = $filteredValue;
    $this->displays = $filteredDisplays;
    $this->types = $filteredTypes;

    // Re-index arrays to ensure sequential keys if necessary for rendering or other logic
    // Although using keys directly might be fine depending on renderOnlyFormInput logic
    $this->value = array_values($this->value);
    $this->displays = array_values($this->displays);
    $this->types = array_values($this->types);
  }

  protected function removeValue ($row)
  {
    // Ensure keys are sequential after removal if using array_values in setValue
    parent::removeValue($row);
    // Re-index after removal
    $this->value = array_values($this->value);
    $this->displays = array_values($this->displays);
    $this->types = array_values($this->types);
  }
}
