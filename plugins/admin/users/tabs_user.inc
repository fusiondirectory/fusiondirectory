<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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

class usertabs extends simpleTabs
{
  function save_object($save_current = FALSE)
  {
    parent::save_object($save_current);

    if (isset($this->by_object['gofaxAccount']) && isset($this->by_object['mailAccount']) && $this->by_object['mailAccount']->is_account) {
      $this->by_object['gofaxAccount']->mail = $this->by_object['mailAccount']->mail;
    }

    /* Move facsimile / phone number if nessecary */
    if ($this->last == 'user' && isset($this->by_object['gofaxAccount'])) {
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
}
?>
