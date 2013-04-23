<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

class organization extends department
{
  /* attribute list for save action */
  var $attributes     = array( "o", "ou", "description","userPassword","searchGuide",
      "seeAlso","businessCategory","x121Address","registeredAddress","manager",
      "destinationIndicator","preferredDeliveryMethod","telexNumber",
      "telephoneNumber","internationaliSDNNumber","facsimileTelephoneNumber","street",
      "postOfficeBox","postalCode", "postalAddress","physicalDeliveryOfficeName",
      "st","l","teletexTerminalIdentifier","gosaUnitTag");

  var $objectclasses  = array("top", "gosaDepartment");
  var $structuralOC   = array("organization");

  /* Attributes
   */
  var $st                       ="";
  var $l                        ="";
  var $description              ="";
  var $userPassword             ="";
  var $searchGuide              ="";
  var $seeAlso                  ="";
  var $businessCategory         ="";
  var $x121Address              ="";
  var $registeredAddress        ="";
  var $destinationIndicator     ="";
  var $preferredDeliveryMethod  ="";
  var $telexNumber              ="";
  var $telephoneNumber          ="";
  var $internationaliSDNNumber  ="";
  var $facsimileTelephoneNumber ="";
  var $street                   ="";
  var $postOfficeBox            ="";
  var $postalCode               ="";
  var $postalAddress            ="";
  var $teletexTerminalIdentifier="";
  var $physicalDeliveryOfficeName="";

  /* Naming attributes
   */
  var $o            ="";
  var $type         = "organization";
  var $orgi_o       = "";
  var $namingAttr   = "o";

  var $manager_enabled = FALSE;
  var $manager_name ="";
  var $manager ="";

  function check()
  {
    $message = plugin::check();

    /* Check for presence of this department */
    $ldap= $this->config->get_ldap_link();
    $ldap->ls ("(&(o=".$this->o.")(objectClass=organization))", $this->base, array('dn'));
    if ($this->orig_o == "new" && $ldap->count()){
      $message[]= msgPool::duplicated(_("Name"));
    } elseif ($this->orig_dn != $this->dn && $ldap->count()){
      $message[]= msgPool::duplicated(_("Name"));
    }

    /* All required fields are set? */
    if ($this->o == ""){
      $message[]= msgPool::required(_("Name"));
    }elseif(tests::is_department_name_reserved($this->o,$this->base)){
      $message[]= msgPool::reserved(_("Name"));
    }elseif(preg_match ('/[#+:=>\\\\\/]/', $this->o)){
      $message[]= msgPool::invalid(_("Name"), $this->o, "/[^#+:=>\\\\\/]/");
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
      "plShortName"   => _("Organization"),
      "plDescription" => _("Organization"),
      "plSelfModify"  => FALSE,
      "plCategory"    => array("department"),
      "plObjectType"  => array("organization" => array("name" => _("Organization"), "filter" => "objectClass=organization")),

      "plProvidedAcls" => array(
        "o"                       => _("Organization name"),
        "description"             => _("Description"),
        "businessCategory"        => _("Category"),
        "base"                    => _("Base"),

        "manager"                 => _("Manager"),

        "st"                        => _("State"),
        "l"                         => _("Location"),
        "postalAddress"             => _("Postal address"),
        "telephoneNumber"           => _("Phone number"),
        "facsimileTelephoneNumber"  => _("Fax"),

        "gosaUnitTag"             => _("Administrative settings")
      )
    );
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
