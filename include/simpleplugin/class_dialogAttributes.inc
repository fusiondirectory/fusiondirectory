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

class GroupSelectDialog
{
  private $groupSelect;
  private $dialogAttribute;

  function __construct ($simplePlugin, $dialogAttribute)
  {
    $this->dialogAttribute = $dialogAttribute;
    $this->groupSelect = new groupSelect($simplePlugin->config, get_userinfo());
  }

  function execute ()
  {
    if (isset($_POST['add_groups_cancel'])) {
      return FALSE;
    }
    if (isset($_POST['add_groups_finish'])) {
      $groups = $this->groupSelect->detectPostActions();
      if (isset($groups['targets'])) {
        $headpage = $this->groupSelect->getHeadpage();
        foreach ($groups['targets'] as $dn) {
          $attrs = $headpage->getEntry($dn);

          $this->dialogAttribute->addValue($dn, $attrs);
        }
      }
      return FALSE;
    }
    session::set('filterBlacklist', $this->dialogAttribute->getFilterBlackList());
    return $this->groupSelect->execute();
  }
}

class MailSelectDialog
{
  private $mailSelect;
  private $mailsAttribute;

  function __construct ($simplePlugin, $mailsAttribute)
  {
    $this->mailsAttribute = $mailsAttribute;
    $this->mailSelect = new mailAddressSelect($simplePlugin->config, get_userinfo());
  }

  function execute ()
  {
    if (isset($_POST['mailAddressSelect_cancel'])) {
      return FALSE;
    }
    if (isset($_POST['mailAddressSelect_save'])) {
      $list = $this->mailSelect->save();
      foreach ($list as $entry) {
        $this->mailsAttribute->addValue($entry['dn'], $entry);
      }
      return FALSE;
    }
    session::set('filterBlacklist', $this->mailsAttribute->getFilterBlackList());
    return $this->mailSelect->execute();
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
    if ($this->isVisible()) {
      if (isset($_POST['add'.$this->getLdapName().'_dialog'])) {
        $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
      } elseif (isset($_POST['del'.$this->getLdapName()]) && isset($_POST['row'.$this->getLdapName()])) {
        foreach ($_POST['row'.$this->getLdapName()] as $row) {
          $this->removeValue($row);
        }
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

  function setAcl ($acl)
  {
    return Attribute::setAcl($acl);
  }

  function renderAttributeInput ()
  {
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons  = '<input type="submit" name="add'.$id.'_dialog" id="add'.$id.'_dialog"'.
                ($this->disabled? ' disabled="disabled"':'').
                ' value="{msgPool type=addButton}"/>';
    $buttons .= '<input type="submit" name="del'.$id.'" id="del'.$id.'"'.
                ($this->disabled? ' disabled="disabled"':'').
                ' value="{msgPool type=delButton}"/>';
    return $buttons;
  }

  function renderAttribute(&$attributes, $readOnly)
  {
    Attribute::renderAttribute($attributes, $readOnly);
  }

  function check ()
  {
    return Attribute::check();
  }

  protected function htmlIds()
  {
    $id = $this->getHtmlId();
    return array('add'.$id.'_dialog','del'.$id,'row'.$id);
  }

  function setParent(&$plugin)
  {
    Attribute::setParent($plugin);
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
  protected $attr = 'cn';

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = array(), $attr = 'cn', $acl = "")
  {
    $this->attr = $attr;
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
  }

  function getFilterBlackList ()
  {
    return array('dn' => $this->getValue());
  }

  function addValue ($value, $attrs)
  {
    if (!in_array($value, $this->value)) {
      $this->value[]    = $value;
      $this->displays[] = $attrs[$this->attr][0];
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
      if (!isset($this->displays[$i])) {
        $this->fillDisplayValue($i);
      }
    }
    return $this->displays;
  }

  protected function fillDisplayValue ($i)
  {
    $dn = $this->value[$i];
    $ldap = $this->plugin->config->get_ldap_link();
    $ldap->cat($dn, array($this->attr));
    if (!$ldap->count()) {
      unset($this->value[$i]);
    } else {
      $attrs = $ldap->fetch();
      if (!isset($attrs[$this->attr])) {
        unset($this->value[$i]);
      } else {
        $this->displays[$i] = $attrs[$this->attr][0];
      }
    }
  }
}

/*! \brief This class allows to handle an attribute for selecting user or groups
 *
 * It looks like a SetAttribute, but clicking "Add" will open a dialog that allow to select one or more users or groups.
 * It stores their dn as values, but displays the cn.
 *
 */
class UsersGroupsAttribute extends DialogAttribute
{
  protected $dialogClass = "UserGroupSelectDialog";

  function addValue ($dn, $entry)
  {
    if (in_array("posixGroup", $entry['objectClass'])) {
      $name = trim("%".$entry['cn'][0]);
    } elseif (isset($entry['uid'][0])) {
      $name = trim($entry['uid'][0]);
    }
    if (!in_array($name, $this->value) && !in_array("!".$name, $this->value)) {
      $this->value[] = $name;
    }
  }

  function getFilterBlackList ()
  {
    $used = array();
    foreach ($this->value as $name) {
      $str = preg_replace("/^!/", "", $name);
      if (preg_match("/^%/", $str)) {
        $used['cn'][] = preg_replace("/^%/", "", $str);
      } else {
        $used['uid'][] = $str;
      }
    }
    return $used;
  }
}

/*! \brief This class allows to handle an attribute for selecting groups
 *
 * It looks like a SetAttribute, but clicking "Add" will open a dialog that allow to select one or more groups.
 * It stores their dn as values, but displays the cn.
 *
 */
class GroupsAttribute extends DialogAttribute
{
  protected $displays;
  protected $dialogClass = "GroupSelectDialog";

  function addValue ($dn, $entry)
  {
    if (!in_array($dn, $this->value)) {
      $display = $entry['cn'][0];
      if (isset($entry['description'][0])) {
        $display .= ' ['.preg_replace ('/^Group of user/', _('Group of user'), $entry['description'][0]).']';
      }

      $this->value[]    = $dn;
      $this->displays[] = $display;
    }
  }

  function setDisplayValues ($array)
  {
    $this->displays = $array;
  }

  function getDisplayValues ()
  {
    return $this->displays;
  }

  protected function removeValue ($row)
  {
    unset($this->displays[$row]);
    unset($this->value[$row]);
  }

  function getFilterBlackList ()
  {
    return array('dn' => $this->getValue());
  }
}

/*! \brief This class allows to handle an attribute for selecting user
 *
 * It looks like a SetAttribute, but clicking "Add" will open a dialog that allow to select one or more users.
 *
 */
class MailsAttribute extends DialogAttribute
{
  protected $dialogClass = "MailSelectDialog";

  /*! \brief The constructor of MailsAttribute
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
    SetAttribute::__construct(new MailAttribute($label, $description, $ldapName, $required, "", $acl), $defaultValue);
  }

  function getFilterBlackList ()
  {
    return array('mail' => $this->getValue());
  }

  function addValue ($dn, $attrs)
  {
    $value = ($attrs != NULL?$attrs['mail'][0]:$dn);
    if (!in_array($value, $this->value)) {
      $this->value[]    = $value;
    }
  }

  function loadPostValue ()
  {
    parent::loadPostValue();
    if ($this->isVisible()) {
      if (isset($_POST['add'.$this->getHtmlId()])) {
        $this->attribute->loadPostValue();
        $this->attribute->applyPostValue();
        $this->addValue($this->attribute->getValue(), NULL);
      }
    }
  }

  function renderAttributeInput ()
  {
    return SetAttribute::renderAttributeInput();
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons  = '<input type="submit" name="add'.$id.'" id="add'.$id.'"'.
                ($this->disabled? ' disabled="disabled"':'').
                ' value="{msgPool type=addButton}"/>';
    $buttons  .= '<input type="submit" name="add'.$id.'_dialog" id="add'.$id.'"'.
                ($this->disabled? ' disabled="disabled"':'').
                ' value="{msgPool type=addButton} (from list)"/>';
    $buttons .= '<input type="submit" name="del'.$id.'" id="del'.$id.'"'.
                ($this->disabled? ' disabled="disabled"':'').
                ' value="{msgPool type=delButton}"/>';
    return $buttons;
  }

  function check ()
  {
    return SetAttribute::check();
  }

  protected function htmlIds()
  {
    $id = $this->getHtmlId();
    return array('add'.$id,'add'.$id.'_dialog','del'.$id,'row'.$id);
  }
}

class ButtonAttribute extends Attribute
{
  protected $dialogClass  = NULL;
  protected $buttonText   = NULL;

  function __construct ($label, $description, $ldapName, $dialogClass, $buttonText, $defaultValue = "", $acl = "")
  {
    parent::__construct($label, $description, $ldapName, FALSE, $defaultValue, $acl);
    $this->dialogClass  = $dialogClass;
    $this->buttonText   = $buttonText;
  }

  function loadPostValue ()
  {
    if ($this->isVisible()) {
      if (isset($_POST[$this->getLdapName()])) {
        $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
      }
    }
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display  = '<input type="submit" name="'.$id.'" id="'.$id.'" value="{literal}'.$this->buttonText.'{/literal}" '.
                ($this->disabled? 'disabled="disabled"':'').'/>';
    return $this->renderAcl($display);
  }

  function applyPostValue ()
  {
  }

}
?>
