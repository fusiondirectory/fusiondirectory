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

class grouptabs extends tabs
{

  function grouptabs($config, $data, $dn, $cat = "", $copied_object = NULL)
  {
    tabs::tabs($config, $data, $dn, "groups", $copied_object);
    $this->addSpecialTabs();
  }

  function save_object($save_current = FALSE)
  {
    tabs::save_object($save_current);

    /* Update reference, transfer variables */
    $baseobject = $this->getBaseObject();
    foreach ($this->by_object as $name => &$obj) {
      /* Don't touch base object */
      if ($name != $this->baseclass) {
        $obj->parent  = &$this;
        $obj->cn      = $baseobject->cn;
      }
    }
    unset($obj);
  }

  function delete()
  {
    /* Put baseobjects 'cn' to mailobjects 'uid' */
    $baseobject = $this->getBaseObject();
    if (isset($this->by_object['mailgroup'])) {
      $this->by_object['mailgroup']->uid = $baseobject->cn;
    }
    tabs::delete();
  }

  function save()
  {
    $baseobject = $this->getBaseObject();

    /* Check for new 'dn', in order to propagate the
       'dn' to all plugins */
    $new_dn = 'cn='.$baseobject->cn.','.get_groups_ou().$baseobject->base;

    /* Transfer attributes for mailgroup account */
    if (isset($this->by_object['mailgroup'])) {
      $this->by_object['mailgroup']->uid      = $baseobject->cn;
      $this->by_object['mailgroup']->members  = $baseobject->memberUid;
    }

    /* Move group? */
    if ($this->dn != $new_dn) {

      /* Write entry on new 'dn' */
      if ($this->dn != "new") {
        $baseobject->update_acls($this->dn, $new_dn);
        $baseobject->move($this->dn, $new_dn);
        $this->by_object[$this->baseclass] = $baseobject;
      }

      /* Happen to use the new one */
      $this->dn = $new_dn;
    }

    return tabs::save();
  }
}

?>
