<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2017-2019  FusionDirectory

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
 * \brief Management dialog
 */
class ManagementDialog extends simplePlugin
{
  protected $post_cancel = 'edit_cancel';
  protected $post_finish = 'edit_finish';

  function __construct (string $dn = NULL, $object = NULL, management $parent = NULL, bool $mainTab = FALSE, array $attributesInfo = NULL)
  {
    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);
  }

  function save_object ()
  {
    parent::save_object();
    if (isset($_POST[$this->post_cancel])) {
      $this->handle_cancel();
    } elseif (isset($_POST[$this->post_finish])) {
      $this->handle_finish();
    }
  }

  protected function handle_finish ()
  {
    $msgs = $this->check();
    if (count($msgs)) {
      msg_dialog::displayChecks($msgs);
      return;
    } else {
      $errors = $this->save();
      if (count($errors)) {
        msg_dialog::displayChecks($errors);
        return;
      } else {
        $this->parent->remove_lock();
        $this->parent->closeDialogs();
      }
    }
  }

  protected function handle_cancel ()
  {
    $this->parent->remove_lock();
    $this->parent->closeDialogs();
  }

  function execute (): string
  {
    return parent::execute().$this->getFooter();
  }

  /*! \brief  Returns the plugin footer (save cancel), displayed in the template.
    @return String  HTML footer part.
   */
  protected function getFooter (): string
  {
    if (!$this->displayPlugin) {
      return '';
    }
    return  '<p class="plugbottom">'.
            ' <input type="submit" name="'.$this->post_finish.'" value="'.msgPool::saveButton().'"/>'.
            ' <input type="submit" formnovalidate="formnovalidate" name="'.$this->post_cancel.'" value="'.msgPool::cancelButton().'"/>'.
            '</p>';
  }
}
