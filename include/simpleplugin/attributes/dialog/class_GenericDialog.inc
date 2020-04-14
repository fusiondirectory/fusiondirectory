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

/*! \brief Generic dialog base class
 */
class GenericDialog
{
  protected $dialogClass = "";
  protected $dialog;
  protected $attribute;

  protected $post_cancel = 'add_cancel';
  protected $post_finish = 'add_finish';

  function __construct ($simplePlugin, $attribute)
  {
    $this->attribute  = $attribute;
    $this->dialog     = new $this->dialogClass();
  }

  function execute ()
  {
    if (isset($_POST[$this->post_cancel])) {
      return $this->handle_cancel();
    }
    if (isset($_POST[$this->post_finish]) || isset($_GET[$this->post_finish])) {
      return $this->handle_finish();
    }
    return $this->dialog_execute();
  }

  function dialog_execute ()
  {
    return $this->dialog->execute();
  }

  function handle_finish ()
  {
    trigger_error('empty function');
    return FALSE;
  }

  function handle_cancel ()
  {
    return FALSE;
  }
}
