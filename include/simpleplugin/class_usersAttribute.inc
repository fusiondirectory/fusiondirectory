<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012  FusionDirectory

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


class UserSelectDialog
{
  private $userSelect;
  private $usersAttribute;

  function __construct ($simplePlugin, $usersAttribute)
  {
    $this->usersAttribute = $usersAttribute;
    $this->userSelect = new userSelect($simplePlugin->config, get_userinfo());
  }

  function execute ()
  {
    if (isset($_POST['add_users_cancel'])) {
      return FALSE;
    }
    if (isset($_POST['add_users_finish'])) {
      $users = $this->userSelect->detectPostActions();
      if (isset($users['targets'])) {
        $headpage = $this->userSelect->getHeadpage();
        foreach ($users['targets'] as $dn) {
          $attrs = $headpage->getEntry($dn);

          $this->usersAttribute->addValue($dn, $attrs);
        }
      }
      return FALSE;
    }
    session::set('filterBlacklist', $this->usersAttribute->getFilterBlackList());
    return $this->userSelect->execute();
  }
}

class UserGroupSelectDialog
{
  private $userGroupSelect;
  private $dialogAttribute;

  function __construct ($simplePlugin, $dialogAttribute)
  {
    $this->dialogAttribute = $dialogAttribute;
    $this->userGroupSelect = new userGroupSelect($simplePlugin->config, get_userinfo());
  }

  function execute ()
  {
    if (isset($_POST['userGroupSelect_cancel'])) {
      return FALSE;
    }
    if (isset($_POST['userGroupSelect_save'])) {
      $users = $this->userGroupSelect->save();
      foreach ($users as $user) {
        $this->dialogAttribute->addValue($user['dn'], $user);
      }
      return FALSE;
    }
    session::set('filterBlacklist', $this->dialogAttribute->getFilterBlackList());
    return $this->userGroupSelect->execute();
  }
}

/*! \brief This class allows to handle an attribute with a popup for selection
 *
 * It looks like a SetAttribute, but clicking "Add" will open a dialog that allow to select one or more items.
 * It stores their dn as values, but displays the cn.
 *
 */
class DialogAttribute extends SetAttribute
{
  protected $dialogClass = NULL;

  /*! \brief The constructor of DialogAttribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param array $defaultValue The default value for this attribute
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = array(), $acl = "")
  {
    Attribute::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
  }

  function addValue ($value, $attrs)
  {
    trigger_error("abstract method");
  }

  function getFilterBlackList ()
  {
    trigger_error("abstract method");
  }

  function getDisplayValues ()
  {
    return $this->value;
  }

  function loadPostValue ()
  {
    if (isset($_POST["add".$this->getLdapName()])) {
      $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
    } elseif (isset($_POST["del".$this->getLdapName()]) && isset($_POST["row".$this->getLdapName()])) {
      foreach ($_POST["row".$this->getLdapName()] as $row) {
        $this->removeValue($row);
      }
    }
  }

  protected function removeValue ($row)
  {
    unset($this->value[$row]);
  }

  function applyPostValue ()
  {
  }

  function getAcl ()
  {
    return Attribute::getAcl();
  }

  function renderAttributeInput ()
  {
  }

  function check ()
  {
    return Attribute::check();
  }
}

/*! \brief This class allows to handle an attribute for selecting user
 *
 * It looks like a SetAttribute, but clicking "Add" will open a dialog that allow to select one or more users.
 * It stores their dn as values, but displays the cn.
 *
 */
class UsersAttribute extends DialogAttribute
{
  protected $displays;
  protected $dialogClass = "UserSelectDialog";

  function getFilterBlackList ()
  {
    return array('dn' => $this->getValue());
  }

  function addValue ($value, $attrs)
  {
    if (!in_array($value, $this->value)) {
      $this->value[]    = $value;
      $this->displays[] = $attrs['cn'][0];
    }
  }

  protected function removeValue ($row)
  {
    unset($this->displays[$row]);
    unset($this->value[$row]);
  }

  function getDisplayValues ()
  {
    foreach ($this->value as $i => $dn) {
      if (!isset($this->displays[$dn])) {
        $this->fillDisplayValue($i);
      }
    }
    return $this->displays;
  }

  protected function fillDisplayValue ($i)
  {
    $dn = $this->value[$i];
    $ldap = $this->plugin->config->get_ldap_link();
    $ldap->cat($dn, array('cn'));
    if (!$ldap->count()) {
      unset($this->value[$i]);
    } else {
      $attrs = $ldap->fetch();
      if (!isset($attrs['cn'])) {
        unset($this->value[$i]);
      } else {
        $this->displays[$i] = $attrs['cn'][0];
      }
    }
  }
}
?>
