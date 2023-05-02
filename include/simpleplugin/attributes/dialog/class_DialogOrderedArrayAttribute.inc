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
 * \brief An OrderedArrayAttribute which uses a dialog to create/edit values
 */
abstract class DialogOrderedArrayAttribute extends OrderedArrayAttribute
{
  protected $dialogClass;

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $acl = "")
  {
    \FusionDirectory\Core\SimplePlugin\Attribute::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->edit_enabled = TRUE;
    $this->attribute    = FALSE;
  }

  function getFilterWhiteList ()
  {
    return [];
  }

  function loadPostValue ()
  {
    if ($this->isVisible()) {
      parent::loadPostValue();
      parent::applyPostValue();
      if (isset($_POST['add'.$this->getHtmlId().'_dialog'])) {
        $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
      }
    }
  }

  protected function handleEdit ($key)
  {
    $this->editingValue = $this->value[$key];
    $this->delPostValue($key);
    $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this, $this->editingValue));
  }

  function applyPostValue ()
  {
  }

  function addValue (string $dn, $attrs = NULL)
  {
    if ($attrs !== NULL) {
      $this->value[] = $attrs;
      $this->reIndexValues();
    }
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    return $this->renderInputField(
      'submit', 'add'.$id.'_dialog',
      [
        'class'           => 'dialog subattribute',
        'value'           => msgPool::addButton(FALSE),
        'formnovalidate'  => 'formnovalidate'
      ]
    );
  }

  public function htmlIds (): array
  {
    $ids = parent::htmlIds();
    $ids[0] .= '_dialog';
    return $ids;
  }

  protected function getAttributeArrayValue ($key, $value)
  {
    /* Convert text value to displayable array value */
    die('Pure virtual method');
  }
}
