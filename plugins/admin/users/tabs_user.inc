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

class usertabs extends tabs
{
  var $base;

  /* Template adaption variables
   */
  var $uid        = '';
  var $givenName  = '';
  var $sn         = '';

  function __construct($config, $data, $dn, $cat = '', $copied_object = NULL)
  {
    parent::__construct($config, $data, $dn, 'user', $copied_object);
    $this->base = $this->getBaseObject()->base;

    $this->uid        = &$this->by_object['user']->uid;
    $this->givenName  = &$this->by_object['user']->givenName;
    $this->sn         = &$this->by_object['user']->sn;

    /* Add references/acls/snapshots */
    $this->addSpecialTabs();
  }

  function save_object($save_current = FALSE)
  {
    parent::save_object($save_current);

    /* Update reference, transfer variables */
    $baseobject = $this->getBaseObject();
    foreach ($this->by_object as $name => &$obj) {
      /* Don't touch base object */
      if ($name != 'user') {
        $obj->parent    = &$this;
        $obj->uid       = $baseobject->uid;
        $obj->givenName = $baseobject->givenName;
        $obj->sn        = $baseobject->sn;
      }

      /* Copy mail if needed */
      if ($name == 'gofaxAccount') {
        if (isset($this->by_object['mailAccount']) && $this->by_object['mailAccount']->is_account) {
          $obj->mail = $this->by_object['mailAccount']->mail;
        }
      }
    }
    unset($obj);

    /* Update parent in base object */
    $this->by_object['user']->parent = &$this;

    /* Move facsimile / phone number if nessecary */
    if (($this->last == 'user') && isset($this->by_object['gofaxAccount'])) {

      /* Move number to fax plugin */
      $this->by_object['gofaxAccount']->facsimileTelephoneNumber =
        $this->by_object['user']->facsimileTelephoneNumber;

      /* Move phone number if plugin exists */
      if (isset($this->by_object['phoneAccount']) &&
          !$this->by_object['phoneAccount']->is_account) {
        $this->by_object['phoneAccount']->phoneNumbers = array();
        if ($this->by_object['user']->telephoneNumber != '') {
          $this->by_object['phoneAccount']->phoneNumbers[$this->by_object['user']->telephoneNumber] = $this->by_object['user']->telephoneNumber;
        }
      }
    }

    /* Move number from fax plugin */
    if ($this->last == 'gofaxAccount') {
      $this->by_object['user']->facsimileTelephoneNumber =
        $this->by_object['gofaxAccount']->facsimileTelephoneNumber;
    }

    /* Move number from fax plugin */
    if ($this->last == 'phoneAccount' && $this->by_object['phoneAccount']->is_account) {
      reset($this->by_object['phoneAccount']->phoneNumbers);
      $number = key($this->by_object['phoneAccount']->phoneNumbers);

      /* Only the first phoneAccount number, if it is not empty */
      if (!empty($number)) {
        $this->by_object['user']->telephoneNumber = $number;
      }
    }

    /* Possibly change acl base */
    $this->set_acl_base();
  }

  function save()
  {
    /* Check for new 'dn', in order to propagate the
       'dn' to all plugins */
    $baseobject = $this->getBaseObject();
    $baseobject->update_new_dn();
    if ($this->dn != 'new') {

      $new_dn = $baseobject->new_dn;

      if ($this->dn != $new_dn) {

        /* Udpate acls */
        $baseobject->update_acls($this->dn, $new_dn);
        $baseobject->move($this->dn, $new_dn);
      }
    }

    $this->dn = $baseobject->new_dn;

    return parent::save();
  }


  function set_template_mode()
  {
    foreach ($this->by_object as $key => $obj) {
      $this->by_object[$key]->is_template = TRUE;
    }
  }
}
?>
