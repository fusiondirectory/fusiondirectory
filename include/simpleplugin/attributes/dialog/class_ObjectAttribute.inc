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
 * \brief Attribute storing the dn of an object which can be selected through a dialog
 */
class ObjectAttribute extends DialogButtonAttribute
{
  protected $objectType;
  protected $storeAttr;
  protected $displayAttr;
  protected $selectManagementParameters;

  function __construct (string $label, string $description, string $ldapName, bool $required, string $objectType, string $storeAttr = 'dn', string $displayAttr = 'nameAttr', $defaultValue = '', $acl = '')
  {
    parent::__construct($label, $description, $ldapName, '', 'GenericSingleSelectManagementDialog', $defaultValue, $acl);
    $this->setRequired($required);
    $this->storeAttr    = $storeAttr;
    $this->displayAttr  = $displayAttr;
    $this->objectType   = $objectType;
  }

  function setParent (simplePlugin &$plugin)
  {
    parent::setParent($plugin);
    $infos = objects::infos($this->objectType);
    if (in_array($this->storeAttr, ['nameAttr','mainAttr'])) {
      $this->storeAttr = $infos[$this->storeAttr];
    }
    if (in_array($this->displayAttr, ['nameAttr','mainAttr'])) {
      $this->displayAttr = $infos[$this->displayAttr];
    }
    $attributes = [
      'objectClass' => '*',
      'dn'          => 'raw',
    ];
    if ($this->storeAttr != 'dn') {
      $attributes[$this->storeAttr] = '*';
    }
    if ($this->displayAttr != 'dn') {
      $attributes[$this->displayAttr] = '*';
    }
    $this->selectManagementParameters = [[$this->objectType],FALSE,$attributes];
  }

  public function getSelectManagementParameters (): array
  {
    return $this->selectManagementParameters;
  }

  function handleDialogResult ($dn, $entry)
  {
    $value = $entry[$this->storeAttr];
    if (is_array($value)) {
      $value = $value[0];
    }
    $this->setValue($value);

    $this->buttonText = $entry[$this->displayAttr];
    if (is_array($this->buttonText)) {
      $this->buttonText = $this->buttonText[0];
    }
  }

  function renderFormInput (): string
  {
    $infos = objects::infos($this->objectType);

    $id = $this->getHtmlId();
    if ($this->isTemplate()) {
      $display = $this->renderInputField(
        'text', $id,
        [
          'value' => $this->getValue()
        ]
      );
    } else {
      $display  = '<img src="'.htmlescape($infos['icon']).'" alt="'.htmlescape($infos['name']).'" class="center"/>';
      if ($this->getValue() == '') {
        $display  .= '<b>'.htmlescape(_('None')).'</b>';
      } elseif ($this->buttonText === NULL) {
        $display  .= '<b>'.htmlescape(_('Unknown')).'</b>';
      } else {
        $display  .= '<a>'.htmlescape($this->buttonText).'</a>';
      }
    }
    $display .= $this->renderInputField(
      'image', $id.'_dialog',
      [
        'class' => 'center dialog',
        'src'   => 'geticon.php?context=actions&icon=document-edit&size=16',
        'title' => _('Edit'),
        'alt'   => _('Edit')
      ]
    );
    if (!$this->isRequired()) {
      $display .= $this->renderInputField(
        'image', $id.'_remove',
        [
          'class' => 'center',
          'src'   => 'geticon.php?context=actions&icon=remove&size=16',
          'title' => _('Remove'),
          'alt'   => _('Remove')
        ]
      );
    }
    return $this->renderAcl($display);
  }

  function loadPostValue ()
  {
    $id = $this->getHtmlId();
    if (!$this->disabled && $this->isVisible()) {
      foreach (array_keys($_POST) as $name) {
        if (!$this->isRequired() && preg_match('/^'.$id.'_remove_/', $name)) {
          $this->setValue('');
          $this->buttonText = NULL;
          return;
        }
        if (preg_match('/^'.$id.'_dialog_/', $name)) {
          $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
          return;
        }
      }
      if ($this->isTemplate() && isset($_POST[$id])) {
        $this->setValue($_POST[$id]);
      }
    }
  }

  function applyPostValue ()
  {
  }

  protected function loadAttrValue (array $attrs)
  {
    parent::loadAttrValue($attrs);
    if (!$this->isTemplate()) {
      $value = $this->getValue();
      if ($value != '') {
        if ($this->storeAttr == 'dn') {
          $objects = objects::ls($this->objectType, $this->selectManagementParameters[2], $value, '', FALSE, 'base');
        } else {
          $objects = objects::ls($this->objectType, $this->selectManagementParameters[2], NULL, '('.$this->storeAttr.'='.ldap_escape_f($value).')');
        }
        if (!empty($objects)) {
          $this->handleDialogResult(key($objects), reset($objects));
        } else {
          $this->buttonText = NULL;
        }
      }
    }
  }

  public function htmlIds (): array
  {
    $id   = $this->getHtmlId();
    $ids  = [$id.'_dialog'];
    if ($this->isTemplate()) {
      $ids[] = $id;
    }
    if (!$this->isRequired()) {
      $ids[] = $id.'_remove';
    }
    return $ids;
  }
}
