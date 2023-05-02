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

/*! \brief This class allows to handle an attribute with a popup for selection
 *
 * It looks like a SetAttribute, but clicking "Add" will open a dialog that allow to select one or more items.
 * It stores their dn as values, but displays the cn.
 *
 */
abstract class DialogAttribute extends SetAttribute
{
  protected $dialogClass = NULL;

  /*! \brief The constructor of DialogAttribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param array $defaultValue The default value for this attribute
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $acl = "")
  {
    \FusionDirectory\Core\SimplePlugin\Attribute::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->attribute = FALSE;
  }

  function addPostValue ($value)
  {
    $this->addValue($value, NULL);
  }

  function delPostValue ($key)
  {
    $this->removeValue($key);
  }

  abstract function addValue (string $dn, $attrs = NULL);

  function searchAndRemove ($value)
  {
    $row = array_search($value, $this->value);
    if ($row !== FALSE) {
      $this->removeValue($row);
    }
  }

  protected function removeValue ($row)
  {
    unset($this->value[$row]);
  }

  abstract function getFilterBlackList ();

  function getFilterWhiteList ()
  {
    return [];
  }

  function loadPostValue ()
  {
    parent::loadPostValue();
    if ($this->isVisible()) {
      $id = $this->getHtmlId();
      if (isset($_POST['add'.$id.'_dialog'])) {
        $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
      } elseif (isset($_POST['add'.$id]) && isset($_POST[$id]) && $this->isTemplate()) {
        $this->addPostValue($_POST[$id]);
      }
    }
  }

  function applyPostValue ()
  {
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons            = '';
    $dialogButtonValue  = msgPool::addButton(FALSE);
    if ($this->isTemplate()) {
      $buttons  .= $this->renderInputField(
        'text', $id,
        ['value' => $this->editingValue, 'class' => 'subattribute']
      );
      $buttons  .= $this->renderInputField(
        'submit', 'add'.$id,
        [
          'value'           => msgPool::addButton(FALSE),
          'formnovalidate'  => 'formnovalidate',
          'class'           => 'subattribute',
        ]
      );
      $dialogButtonValue = _('Add (dialog)');
    }
    $buttons  .= $this->renderInputField(
      'submit', 'add'.$id.'_dialog',
      [
        'class'           => 'dialog subattribute',
        'value'           => $dialogButtonValue,
        'formnovalidate'  => 'formnovalidate',
      ]
    );
    $buttons  .= $this->renderInputField(
      'submit', 'del'.$id,
      [
        'value'           => msgPool::delButton(FALSE),
        'formnovalidate'  => 'formnovalidate',
        'class'           => 'subattribute',
      ]
    );
    return $buttons;
  }

  public function htmlIds (): array
  {
    $id = $this->getHtmlId();
    $ids = ['add'.$id.'_dialog','del'.$id,'row'.$id];
    if ($this->isTemplate()) {
      $ids[] = $id;
      $ids[] = 'add'.$id;
    }
    return $ids;
  }
}
