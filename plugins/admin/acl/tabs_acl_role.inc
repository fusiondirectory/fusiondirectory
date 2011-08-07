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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class aclroletab extends tabs
{

  function aclroletab($config, $data, $dn)
  {
    $data = array(array("CLASS" => "aclrole" , "NAME" => _("ACL Templates")));
    tabs::tabs($config, $data, $dn,"acl");
  }

  function save($ignore_account= FALSE)
  {
    $baseobject= $this->by_object['aclrole'];

    /* Check for new 'dn', in order to propagate the
       'dn' to all plugins */
    $new_dn= 'cn='.$baseobject->cn.",".get_ou('aclRoleRDN').$baseobject->base;

    if ($this->dn != $new_dn){

      /* Write entry on new 'dn' */
      if ($this->dn != "new"){
        $baseobject->move($this->dn, $new_dn);
        $this->by_object['aclrole']= $baseobject;
      }

      /* Happen to use the new one */
      $this->dn= $new_dn;
    }

    $ret= tabs::save();

    return $ret;
  }

  function save_object($ignore_account= FALSE)
  {
    tabs::save_object();
  }
  
  function execute()
  {
    $display= tabs::execute();
    if($this->read_only){
      $display.= "<p style=\"text-align:right\">\n";
      $display.= "  <input type=submit name=\"edit_cancel\" value=\"".msgPool::cancelButton()."\">\n";
      $display.= "</p>";
    }elseif(!$this->by_object['aclrole']->dialog){
      $display.= "<p style=\"text-align:right\">\n";
      $display.= "  <input type=submit name=\"edit_finish\" style=\"width:80px\" value=\"".msgPool::okButton()."\">\n";
      $display.= "  &nbsp;\n";
      $display.= "  <input type=submit name=\"edit_cancel\" value=\"".msgPool::cancelButton()."\">\n";
      $display.= "</p>";
    }
    return($display);
  }

}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
