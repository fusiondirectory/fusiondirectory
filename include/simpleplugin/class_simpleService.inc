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

/*! \brief This class allow to handle easily the status flag attribute of services
 *
 */
class StatusFlagAttribute extends Attribute
{
  /*! \brief The constructor of StatusFlagAttribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param mixed defaultValue The default value for this attribute
   *  \param string acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "")
  {
    parent::__construct ($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setVisible(FALSE);
  }

  /*! \brief Set the parent plugin for this attribute
   *
   *  \param simplePlugin &$plugin The parent plugin
   */
  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    $name = $this->getLdapName();
    if (is_object($this->plugin)) {
      $this->plugin->StatusFlag = $name;
      $this->plugin->$name =& $this->value;
    }
  }

  function setManagedAttributes ($dontcare)
  {
    trigger_error('method setManagedAttributes is not supported for StatusFlagAttribute');
  }
}

/*! \brief This class is made for easy service creation for editing LDAP attributes
 *
 */
class simpleService extends simplePlugin {

  var $StatusFlag       = "";

  var $conflicts            = array();
  var $dn                   = NULL;
  var $cn                   = "";
  var $DisplayName          = "";
  var $view_logged          = FALSE;

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
    $plInfos = $this->plInfo();
    $this->DisplayName = $plInfos['plShortName'];
  }

  /*! \brief This function display the service and return the html code
   */
  function execute()
  {
    if ($this->is_account && !$this->view_logged) {
      $this->view_logged = TRUE;
      new log("view", "server/".get_class($this), $this->dn);
    }

    $str = "<div style='width:100%; text-align:right;'>".
           "  <input type='submit' name='SaveService' value='".msgPool::saveButton()."'>&nbsp;".
           "  <input type='submit' name='CancelService' value='".msgPool::cancelButton()."'>".
           "</div>";

    return parent::execute().$str;
  }

  protected function acl_skip_write()
  {
    return FALSE;
  }

  /*! \brief Get service information for serverService plugin */
  function getListEntry()
  {
    $this->updateStatusState();

    /* Assign status flag */
    if (!empty($this->StatusFlag)) {
      $flag                 = $this->StatusFlag;
      $fields['Status']     = $this->$flag;
    } else {
      $fields['Status']     = "";
    }

    /* Name displayed in service overview */
    $fields['Message']      = $this->DisplayName;

    /* Allow/disallow some functions */
    $sf = !empty($this->StatusFlag);
    $fields['AllowStart']   = $sf && $this->acl_is_writeable("start");
    $fields['AllowStop']    = $sf && $this->acl_is_writeable("stop");
    $fields['AllowRestart'] = $sf && $this->acl_is_writeable("restart");
    $fields['AllowRemove']  = $this->acl_is_removeable();
    $fields['AllowEdit']    = $this->acl_is_readable("");

    return $fields;
  }

  /*! \brief This function removes the service object from LDAP
   */
  function remove_from_parent()
  {
    if (!$this->initially_was_account || !$this->acl_is_removeable()) {
      return;
    }

    parent::remove_from_parent();

    /* Remove status flag, it is not a member of
        this->attributes, so ensure that it is deleted too */
    if (!empty($this->StatusFlag)) {
      $this->attrs[$this->StatusFlag] = array();
    }

    /* Check if this is a new entry ... add/modify */
    $ldap = $this->config->get_ldap_link();
    $ldap->cat($this->dn, array("objectClass"));
    if ($ldap->count()) {
      $ldap->cd($this->dn);
      $ldap->modify($this->attrs);
    }

    new log("remove", "server/".get_class($this), $this->dn, array_keys($this->attrs), $ldap->get_error());

    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }
    $this->handle_post_events("remove");
  }

  /*! \brief This function save new status flag */
  function setStatus($value)
  {
    if ($value == "none") {
      return;
    }

    /* Can't set status flag for new services (Object doesn't exists in ldap tree) */
    if (!$this->initially_was_account) {
      return;
    }

    /* Can't set status flag, if no flag is specified  */
    if (empty($this->StatusFlag)) {
      return;
    }

    /* Get object (server), update status flag and save changes */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->dn);
    $ldap->cat($this->dn, array("objectClass"));
    if ($ldap->count()) {
      $tmp = $ldap->fetch();
      for ($i = 0; $i < $tmp['objectClass']['count']; $i++) {
        $attrs['objectClass'][] = $tmp['objectClass'][$i];
      }
      $flag         = $this->StatusFlag;
      $attrs[$flag] = $value;
      $this->$flag  = $value;
      $ldap->modify($attrs);
      if (!$ldap->success()) {
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));
      }
      $this->action_hook();
    }
  }

  function action_hook($add_attrs = array())
  {
    /* Find postcreate entries for this class */
    $command = $this->config->search(get_class($this), "SERVICEACTIONHOOK", array('menu', 'tabs'));
    if ($command != "") {

      /* Walk through attribute list */
      foreach ($this->attributes as $attr) { // TODO : use attribuesInfo instead
        if (!is_array($this->$attr)) {
          $command = preg_replace("/%$attr/", $this->$attr, $command);
        }
      }
      $command = preg_replace("/%dn/", $this->dn, $command);

      /* Additional attributes */
      foreach ($add_attrs as $name => $value) {
        $command = preg_replace("/%$name/", $value, $command);
      }

      /* If there are still some %.. in our command, try to fill these with some other class vars */
      if (preg_match("/%/", $command)) {
        $attrs = get_object_vars($this);
        foreach ($attrs as $name => $value) {
          if (!is_string($value)) {
            continue;
          }
          $command = preg_replace("/%$name/", escapeshellarg($value), $command);
        }
      }

      if (check_command($command)) {
        @DEBUG (DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, "Execute");

        exec($command);
      } else {
        msg_dialog::display(_("Configuration error"), msgPool::cmdnotfound("SERVICEACTIONHOOK", get_class($this)), ERROR_DIALOG);
      }
    }
  }

  /*! \brief This function updates the status flag from LDAP */
  function updateStatusState()
  {
    if (empty($this->StatusFlag)) {
      return;
    }

    $flag = $this->StatusFlag;
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->cn);
    $ldap->cat($this->dn, array($flag));
    if ($ldap->count()) {
      $attrs = $ldap->fetch();
    }
    if (isset($attrs[$flag][0])) {
      $this->$flag = $attrs[$flag][0];
    }
  }
}
?>
