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
 * \brief GenericDialog using a simplePlugin based class as dialog
 */
class GenericSimplePluginDialog extends GenericDialog
{
  protected $initialDialogValue = NULL;

  function __construct ($simplePlugin, $attribute, $value = [])
  {
    $this->attribute  = $attribute;
    $this->dialog     = new $this->dialogClass('new');

    $base = $simplePlugin->dn;
    if ($base == 'new') {
      $base = $simplePlugin->base;
    }
    $this->dialog->base = $base;
    $this->dialog->set_acl_category($simplePlugin->acl_category);
    if (!empty($value)) {
      $this->initialDialogValue = $value;
      foreach ($this->dialog->attributes as $attr) {
        if (isset($value[$attr])) {
          $this->dialog->$attr = $value[$attr];
        }
      }
    }
  }

  function buttons ()
  {
    return '<div style="width:100%; text-align:right; clear:both; float:none;">'.
           '  <input type="submit" name="'.$this->post_finish.'" value="'.msgPool::saveButton().'"/>&nbsp;'.
           '  <input type="submit" formnovalidate="formnovalidate" name="'.$this->post_cancel.'" value="'.msgPool::cancelButton().'"/>'.
           '</div>';
  }

  function dialog_execute ()
  {
    $this->dialog->save_object();
    return $this->dialog->execute().$this->buttons();
  }

  function handle_finish ()
  {
    $this->dialog->save_object();
    $msgs = $this->dialog->check();
    if (count($msgs)) {
      msg_dialog::displayChecks($msgs);
      return $this->dialog->execute().$this->buttons();
    }

    $value = [];
    foreach ($this->dialog->attributes as $attr) {
      $value[$attr] = $this->dialog->$attr;
    }

    $this->attribute->addValue('', $value);
    return FALSE;
  }

  function handle_cancel ()
  {
    if ($this->initialDialogValue !== NULL) {
      $this->attribute->addValue('', $this->initialDialogValue);
    }
    return FALSE;
  }
}
