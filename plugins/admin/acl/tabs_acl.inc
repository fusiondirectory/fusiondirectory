<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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

class acltab extends tabs
{

  function acltab($config, $data, $dn, $cat = "", $copied_object = NULL)
  {
    $data = array(array("CLASS" => "acl" , "NAME" => _("ACL")));
    parent::__construct($config, $data, $dn, "", $copied_object);
  }

  function save($ignore_account = FALSE)
  {
    return parent::save();
  }

  function save_object($ignore_account = FALSE)
  {
    return parent::save_object();
  }

  function execute()
  {
    $display = parent::execute();
    if ($this->read_only) {
      $display .= "<p style=\"text-align:right\">\n";
      $display .= "  <input type=submit name=\"edit_cancel\" value=\"".msgPool::cancelButton()."\">\n";
      $display .= "</p>";
    } elseif (!$this->by_object['acl']->dialog) {
      $display .= "<p style=\"text-align:right\">\n";
      $display .= "  <input type=submit name=\"edit_finish\" style=\"width:80px\" value=\"".msgPool::okButton()."\">\n";
      $display .= "  &nbsp;\n";
      $display .= "  <input type=submit name=\"edit_cancel\" value=\"".msgPool::cancelButton()."\">\n";
      $display .= "</p>";
    }
    return $display;
  }
}
?>
