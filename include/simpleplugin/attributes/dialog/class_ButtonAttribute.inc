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
 * \brief Attribute showing a button
 */
class ButtonAttribute extends \FusionDirectory\Core\SimplePlugin\Attribute
{
  protected $buttonText = NULL;
  protected $action;

  function __construct ($label, $description, $ldapName, $buttonText, $action = NULL, $defaultValue = "", $acl = "")
  {
    parent::__construct($label, $description, $ldapName, FALSE, $defaultValue, $acl);
    $this->buttonText = $buttonText;
    if ($action === NULL) {
      $action = 'handle_'.$ldapName;
    }
    $this->action = $action;
  }

  function renderFormInput (): string
  {
    $id = $this->getHtmlId();
    $display = $this->renderInputField(
      'submit', $id,
      ['value' => $this->buttonText]
    );
    return $this->renderAcl($display);
  }

  function loadPostValue ()
  {
    $this->postValue = FALSE;
    if ($this->isVisible() && isset($_POST[$this->getHtmlId()])) {
      $this->setPostValue(TRUE);
    }
  }

  function applyPostValue ()
  {
    if (!$this->disabled && $this->isVisible() && $this->postValue) {
      $func = $this->action;
      $this->plugin->$func();
    }
  }
}
