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

class country extends department
{
  /* attribute list for save action */
  var $attributes     = array( "c", "ou", "description","gosaUnitTag", "manager");
  var $objectclasses  = array("top", "gosaDepartment");
  var $structuralOC   = array("country");
  var $type   ="country";
  var $c      ="";
  var $orgi_c ="";

  var $namingAttr = "c";
  var $manager_enabled = FALSE;
  var $manager_name ="";
  var $manager ="";


  function check()
  {
    $message = plugin::check();

    /* Check for presence of this department */
    $ldap= $this->config->get_ldap_link();
    $ldap->ls ("(&(c=".$this->c.")(objectClass=country))", $this->base, array('dn'));
    if ($this->orig_c == "new" && $ldap->count()){
      $message[]= msgPool::duplicated(_("Name"));
    } elseif ($this->orig_dn != $this->dn && $ldap->count()){
      $message[]= msgPool::duplicated(_("Name"));
    }

    /* All required fields are set? */
    if ($this->c == ""){
      $message[]= msgPool::required(_("Name"));
    }elseif(tests::is_department_name_reserved($this->c,$this->base)){
      $message[]= msgPool::reserved(_("Name"));
    }elseif(preg_match ('/[#+:=>\\\\\/]/', $this->c)){
      $message[]= msgPool::invalid(_("Name"), $this->c, "/[^#+:=>\\\\\/]/");
    }

    /* Check description */
    if ($this->description == ""){
      $message[]= msgPool::required(_("Description"));
    }

    /* Check if we are allowed to create or move this object
     */
    if($this->orig_dn == "new" && !$this->acl_is_createable($this->base)){
      $message[] = msgPool::permCreate();
    }elseif($this->orig_dn != "new" && $this->base != $this->orig_base && !$this->acl_is_moveable($this->base)){
      $message[] = msgPool::permMove();
    }

    return($message);
  }


  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Country"),
      "plDescription" => _("Country"),
      "plSelfModify"  => FALSE,
      "plCategory"    => array("department"),
      "plObjectType"  => array("country" => array("name" => _("Country"), "filter" => "objectClass=country")),

      "plProvidedAcls" => array(
        "c"                 => _("Country name"),
        "description"       => _("Description"),
        "manager"           => _("Manager"),
        "base"              => _("Base"),
        "gosaUnitTag"       => _("Administrative settings")
      )
    );
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
