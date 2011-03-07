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

class usertabs extends tabs
{
  var $base;

  /* Template adaption variables
   */
  var $uid  ="";
  var $givenName = "";
  var $sn;

  function usertabs($config, $data, $dn, $cat = "", $hide_refs = FALSE, $hide_acls = FALSE)
  {
    tabs::tabs($config, $data, $dn, 'users',$hide_refs, $hide_acls);
    $this->base= $this->by_object['user']->base;

    $this->uid = &$this->by_object['user']->uid;
    $this->sn = &$this->by_object['user']->sn;
    $this->givenName = &$this->by_object['user']->givenName;

    /* Add references/acls/snapshots */
    $this->addSpecialTabs();

  }

  function save_object($save_current= FALSE)
  {
    tabs::save_object($save_current);

    /* Update reference, transfer variables */
    $baseobject= $this->by_object['user'];
    foreach ($this->by_object as $name => $obj){

      /* Adding uid to sub plugins of connectivity */
      if($name == "connectivity"){
        foreach ($obj->plugin_name as $plg_name){
          if(isset($obj->plugin[$plg_name]->uid)){
            $obj->plugin[$plg_name]->uid = $baseobject->uid;
          }
        }
      }

      /* Don't touch base object */
      if ($name != 'user'){
        $obj->parent= &$this;
        $obj->uid= $baseobject->uid;
        $obj->sn= $baseobject->uid;
        $obj->givenName= $baseobject->uid;
      }

      /* Copy mail if needed */
      if ($name == "gofaxAccount"){
        if (isset($this->by_object['mailAccount']) && $this->by_object['mailAccount']->is_account){
          $obj->mail= $this->by_object['mailAccount']->mail;
        }
      }

      $this->by_object[$name]= $obj;

      /* Update parent in base object */
      $this->by_object['user']->parent= &$this;
    }

    /* Move facsimile / phone number if nessecary */
    if ($this->last == "user" && isset($this->by_object['gofaxAccount'])){

      /* Move number to fax plugin */
      $this->by_object['gofaxAccount']->facsimileTelephoneNumber=
        $this->by_object['user']->facsimileTelephoneNumber;

      /* Move phone number if plugin exists */
      if (isset($this->by_object['phoneAccount']) && 
          !$this->by_object['phoneAccount']->is_account){
        $this->by_object['phoneAccount']->phoneNumbers= array();
        if ($this->by_object['user']->telephoneNumber != ""){
          $this->by_object['phoneAccount']->phoneNumbers[$this->by_object['user']->telephoneNumber]= $this->by_object['user']->telephoneNumber;
        }
      }
    }

    /* Move number from fax plugin */
    if ($this->last == "gofaxAccount"){

      $this->by_object['user']->facsimileTelephoneNumber=
        $this->by_object['gofaxAccount']->facsimileTelephoneNumber;
    }

    /* Move number from fax plugin */
    if ($this->last == "phoneAccount" && $this->by_object['phoneAccount']->is_account){
      reset($this->by_object['phoneAccount']->phoneNumbers);
      $number= key($this->by_object['phoneAccount']->phoneNumbers);

      /* Only the first phoneAccount number, if it is not empty */
      if(!empty($number)){
        $this->by_object['user']->telephoneNumber= $number;
      }
    }

    /* Possibly change acl base */
    $this->set_acl_base();
  }

  function save($ignore_account= FALSE)
  {
    /* Check for new 'dn', in order to propagate the
       'dn' to all plugins */
    $baseobject= $this->by_object['user'];
    $baseobject->update_new_dn();
    if ($this->dn != 'new'){

      $new_dn= $baseobject->new_dn;

      if ($this->dn != $new_dn){

        /* Udpate acls */
        $baseobject->update_acls($this->dn,$new_dn);
        $baseobject->move($this->dn, $new_dn);
        $this->by_object['user']= $baseobject;

        /* Did we change ourselves? Update ui object. */
        change_ui_dn($this->dn, $new_dn);
      }
    }

    $this->dn= $baseobject->new_dn;

    return tabs::save();
  }


  function set_template_mode()
  {
    foreach ($this->by_object as $key => $obj){
      $this->by_object[$key]->is_template= TRUE;
    }
  }
  
  function saveCopyDialog()
  {
    tabs::saveCopyDialog();
    $baseobject= $this->by_object['user'];
    $uid = $baseobject->uid;
    foreach($this->by_object as  $name => $obj){
      $this->by_object[$name]->uid = $uid;
    }
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
