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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class acltab extends tabs
{

  function acltab($config, $data, $dn)
  {
    $data = array(array("CLASS" => "acl" , "NAME" => _("ACL")));

    /* Save dn */
    $this->dn= $dn;
    $this->config= $config;

    $baseobject= NULL;

    foreach ($data as $tab){
      $this->by_name[$tab['CLASS']]= $tab['NAME'];

      
      if ($baseobject === NULL){
        if($tab['CLASS'] == "acl"){
          $baseobject= new $tab['CLASS']($this->config, $this, $this->dn);
        }else{
          $baseobject= new $tab['CLASS']($this->config,$this->dn);
        }
        $this->by_object[$tab['CLASS']]= $baseobject;
      } else {
        if($tab['CLASS'] == "acl"){
          $this->by_object[$tab['CLASS']]= new $tab['CLASS']($this->config,$this,$this->dn, $baseobject);
        }else{
          $this->by_object[$tab['CLASS']]= new $tab['CLASS']($this->config, $this->dn, $baseobject);
        }
      }

      $this->by_object[$tab['CLASS']]->parent= &$this;
      $this->by_object[$tab['CLASS']]->set_acl_category("acl");
      $this->read_only |= $this->by_object[$tab['CLASS']]->read_only;

      /* Initialize current */
      if ($this->current == ""){
        $this->current= $tab['CLASS'];
      }
    }
  }

  function save($ignore_account= FALSE)
  {
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
    }elseif(!$this->by_object['acl']->dialog){
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
