<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2016  FusionDirectory

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

/*! \brief This class is made for easy service creation for editing LDAP attributes
 *
 */
class simpleService extends simplePlugin {

  var $status = "";

  var $conflicts            = array();
  var $dn                   = NULL;
  var $cn                   = "";
  var $DisplayName          = "";
  var $view_logged          = FALSE;
  var $showActions          = TRUE;

  /*! \brief constructor
   *
   *  \param mixed $config The config object
   *  \param string $dn The dn of this instance
   *  \param Object $object An object to copy values from
   *  \param array $attributesInfo An attributesInfo array, if NULL, getAttributesInfo will be used.
   *
   */
  function __construct(&$config, $dn, $object = NULL, $attributesInfo = NULL)
  {
    parent::__construct($config, $dn, $object, $attributesInfo);
    $plInfos = pluglist::pluginInfos(get_class($this));
    $this->DisplayName  = $plInfos['plShortName'];
    $this->parent       = $object; /* $object is the instance of serverService in this case */
  }

  /*! \brief This function display the service and return the html code
   */
  function execute()
  {
    if ($this->is_account && !$this->view_logged) {
      $this->view_logged = TRUE;
      new log("view", "server/".get_class($this), $this->dn);
    }

    $str = parent::execute();

    if (!$this->dialog) {
      $str .= '<div class="plugbottom servicebottom">'.
             '  <input type="submit" name="SaveService" value="'.msgPool::saveButton().'"/>&nbsp;'.
             '  <input type="submit" name="CancelService" value="'.msgPool::cancelButton().'"/>'.
             '</div>';
    }

    return $str;
  }

  protected function acl_skip_write()
  {
    return FALSE;
  }

  /*! \brief Get service information for serverService plugin */
  function getListEntry()
  {
    /* Assign status flag */
    $fields['Status']       = $this->status;

    /* Name displayed in service overview */
    $fields['Message']      = $this->DisplayName;

    if ($this->showActions && is_object($this->parent->parent) && isset($this->parent->parent->by_object['argonautClient']) && $this->parent->parent->by_object['argonautClient']->is_account) {
      /* Allow/disallow some functions */
      $fields['AllowStatus']  = ($this->status == "") && $this->acl_is_writeable("start");
      $fields['AllowStart']   = ($this->status == "stopped") && $this->acl_is_writeable("start");
      $fields['AllowStop']    = ($this->status == "running") && $this->acl_is_writeable("stop");
      $fields['AllowRestart'] = ($this->status == "running") && $this->acl_is_writeable("restart");
    } else {
      /* Disable some functions */
      $fields['AllowStatus']  = FALSE;
      $fields['AllowStart']   = FALSE;
      $fields['AllowStop']    = FALSE;
      $fields['AllowRestart'] = FALSE;
    }

    $fields['AllowRemove']  = $this->acl_is_removeable();
    $fields['AllowEdit']    = $this->acl_is_readable("");

    return $fields;
  }

  /*! \brief This function save new status flag */
  function setStatus($value)
  {
    /* Can't set status flag for new services (Object doesn't exists in ldap tree) */
    if (!$this->initially_was_account) {
      return;
    }

    $this->status = $value;
  }
}
?>
