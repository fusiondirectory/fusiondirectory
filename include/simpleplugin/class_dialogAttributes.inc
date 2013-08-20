<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2013  FusionDirectory

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

class GenericDialog
{
  protected $dialogClass = "";
  protected $dialog;
  protected $attribute;

  protected $post_cancel = 'add_cancel';
  protected $post_finish = 'add_finish';

  function __construct($simplePlugin, &$attribute)
  {
    $this->attribute  = $attribute;
    $this->dialog     = new $this->dialogClass($simplePlugin->config, get_userinfo());
  }

  function execute ()
  {
    if (isset($_POST[$this->post_cancel])) {
      return FALSE;
    }
    if (isset($_POST[$this->post_finish]) || isset($_GET[$this->post_finish])) {
      return $this->handle_finish();
    }
    return $this->dialog_execute();
  }

  function dialog_execute ()
  {
    return $this->dialog->execute();
  }

  function handle_finish ()
  {
    trigger_error('empty function');
    return FALSE;
  }
}

class GenericSelectDialog extends GenericDialog
{
  function dialog_execute ()
  {
    session::set('filterBlacklist', $this->attribute->getFilterBlackList());
    return parent::dialog_execute();
  }

  function handle_finish ()
  {
    $result = $this->dialog->detectPostActions();
    if (isset($result['targets'])) {
      $headpage = $this->dialog->getHeadpage();
      foreach ($result['targets'] as $dn) {
        $attrs = $headpage->getEntry($dn);

        $this->attribute->addValue($dn, $attrs);
      }
    }
    return FALSE;
  }
}

class UserSelectDialog extends GenericSelectDialog
{
  protected $dialogClass = 'userSelect';
}

class SingleUserSelectDialog extends GenericDialog
{
  protected $dialogClass = 'singleUserSelect';

  function handle_finish ()
  {
    $result = $this->dialog->detectPostActions();
    if (isset($result['targets']) && count($result['targets'])) {
      $headpage = $this->dialog->getHeadpage();
      $dn       = $result['targets'][0];
      $attrs    = $headpage->getEntry($dn);
      $this->attribute->handleDialogResult($dn, $attrs);
    }
    return FALSE;
  }
}

class SystemSelectDialog extends GenericSelectDialog
{
  protected $dialogClass = 'systemSelect';
  protected $post_cancel = 'add_ws_cancel';
  protected $post_finish = 'add_ws_finish';
}

class UserGroupSelectDialog extends GenericSelectDialog
{
  protected $dialogClass = 'userGroupSelect';
}

class GroupSelectDialog extends GenericSelectDialog
{
  protected $dialogClass = 'groupSelect';
  protected $post_cancel = 'add_groups_cancel';
  protected $post_finish = 'add_groups_finish';
}

class MailSelectDialog extends GenericSelectDialog
{
  protected $dialogClass = 'mailAddressSelect';
  protected $post_cancel = 'mailAddressSelect_cancel';
  protected $post_finish = 'mailAddressSelect_save';

  function handle_finish ()
  {
    $list = $this->mailSelect->save();
    foreach ($list as $entry) {
      $this->mailsAttribute->addValue($entry['dn'], $entry);
    }
    return FALSE;
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
      if (isset($_POST['add'.$this->getHtmlId().'_dialog'])) {
        $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
      } elseif (isset($_POST['del'.$this->getHtmlId()]) && isset($_POST['row'.$this->getHtmlId()])) {
        foreach ($_POST['row'.$this->getHtmlId()] as $row) {
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

  function renderAttributeInput ($template = FALSE)
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

class GenericDialogAttribute extends DialogAttribute
{
  protected $displays     = array();
  protected $store_attr   = 'dn';
  protected $display_attr = 'cn';

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = array(), $store_attr = 'dn', $display_attr = 'cn', $acl = '')
  {
    $this->store_attr   = $store_attr;
    $this->display_attr = $display_attr;
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
  }

  function getFilterBlackList ()
  {
    return array($this->store_attr => $this->getValue());
  }

  function addValue ($dn, $attrs)
  {
    if ($this->store_attr == 'dn') {
      $value = $dn;
    } else {
      $value = $attrs[$this->store_attr][0];
    }
    if (!in_array($value, $this->value)) {
      $size = array_push($this->value, $value);
      $this->fillDisplayValueFrom($size - 1, $attrs);
    }
  }

  protected function removeValue ($row)
  {
    unset($this->displays[$row]);
    unset($this->value[$row]);
  }

  function getDisplayValues ()
  {
    foreach (array_keys($this->value) as $i) {
      if (!isset($this->displays[$i])) {
        $this->fillDisplayValue($i);
      }
    }
    return $this->displays;
  }

  function setDisplayValues ($array)
  {
    $this->displays = $array;
  }

  protected function fillDisplayValue ($i)
  {
    $ldap = $this->plugin->config->get_ldap_link();

    $value = $this->value[$i];
    if ($this->store_attr == 'dn') {
      $ldap->cat($value, $this->ldapAttributesToGet());
    } else {
      $ldap->search('('.$this->store_attr.'='.$value.')', $this->ldapAttributesToGet());
    }
    if (!$ldap->count()) {
      unset($this->value[$i]);
    } else {
      $attrs = $ldap->fetch();
      $this->fillDisplayValueFrom($i, $attrs);
    }
  }

  protected function ldapAttributesToGet ()
  {
    return array($this->display_attr);
  }

  protected function fillDisplayValueFrom ($i, $attrs)
  {
    if (!isset($attrs[$this->display_attr])) {
      unset($this->value[$i]);
    } else {
      if ($this->display_attr == 'dn') {
        $this->displays[$i] = $attrs['dn'];
      } else {
        $this->displays[$i] = $attrs[$this->display_attr][0];
      }
    }
  }
}

/*! \brief This class allows to handle an attribute for selecting user
 *
 * It looks like a SetAttribute, but clicking "Add" will open a dialog that allow to select one or more users.
 * It stores their dn as values, but displays the cn.
 *
 */
class UsersAttribute extends GenericDialogAttribute
{
  protected $dialogClass  = 'UserSelectDialog';
}

/*! \brief This class allows to handle an attribute for selecting user or groups
 *
 * It looks like a SetAttribute, but clicking "Add" will open a dialog that allow to select one or more users or groups.
 * It stores their dn as values, but displays the cn.
 *"
 */
class UsersGroupsAttribute extends DialogAttribute
{
  protected $dialogClass = 'UserGroupSelectDialog';

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
class GroupsAttribute extends GenericDialogAttribute
{
  protected $dialogClass = 'GroupSelectDialog';

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = array(), $acl = '')
  {
    parent::__construct ($label, $description, $ldapName, $required, $defaultValue, 'dn', 'cn', $acl);
  }

  function addValue ($dn, $entry)
  {
    if (!in_array($dn, $this->value)) {
      $display = $entry['cn'][0];
      if (isset($entry['description'][0]) && !empty($entry['description'][0])) {
        $display .= ' ['.preg_replace ('/^Group of user/', _('Group of user'), $entry['description'][0]).']';
      }

      $this->value[]    = $dn;
      $this->displays[] = $display;
    }
  }
}

/*! \brief This class allows to handle an attribute for selecting an mail address
 *
 * It looks like a SetAttribute, but clicking "Add" will open a dialog that allow to select one or more users.
 *
 */
class MailsAttribute extends DialogAttribute
{
  protected $dialogClass = 'MailSelectDialog';

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

  function renderAttributeInput ($template = FALSE)
  {
    return SetAttribute::renderAttributeInput($template);
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

class SystemsAttribute extends DialogAttribute
{
  protected $dialogClass = 'SystemSelectDialog';

  function addValue ($dn, $entry)
  {
    $value = $entry['cn'][0];
    if (!in_array($value, $this->value)) {
      $this->value[]    = $value;
    }
  }

  function getFilterBlackList ()
  {
    return array('cn' => $this->getValue());
  }
}

class ButtonAttribute extends Attribute
{
  protected $buttonText = NULL;

  function __construct ($label, $description, $ldapName, $required, $buttonText, $defaultValue = "", $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->buttonText   = $buttonText;
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

class DialogButtonAttribute extends ButtonAttribute
{
  protected $dialogClass  = NULL;

  function __construct ($label, $description, $ldapName, $required, $dialogClass, $buttonText, $defaultValue = "", $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $buttonText, $defaultValue, $acl);
    $this->dialogClass  = $dialogClass;
  }

  function loadPostValue ()
  {
    if (!$this->disabled && $this->isVisible()) {
      if (isset($_POST[$this->getHtmlId()])) {
        $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
      }
    }
  }
}

class UserAttribute extends DialogButtonAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, 'SingleUserSelectDialog', '', $defaultValue, $acl);
  }

  function handleDialogResult ($dn, $attrs)
  {
    $this->setValue($dn);
    $this->buttonText = $attrs['cn'][0];
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display  = '<img src="plugins/users/images/iconMini.png" alt="user icon" class="center"/>';
    if ($this->getValue() == '') {
      $display  .= '<b>'._('None').'</b>';
    } elseif ($this->buttonText === NULL) {
      $display  .= '<b>'._('Unknown').'</b>';
    } else {
      $display  .= '<a>'.$this->buttonText.'</a>';
    }
    $display .= '<input class="center" type="image" src="images/lists/edit.png" '.
                  'alt="'._('Edit').'" title="'._('Edit').'" name="'.$id.'" id="'.$id.'" '.
                  ($this->disabled? 'disabled="disabled"':'').'/>';
    if (!$this->isRequired()) {
      $display .= '<input class="center" type="image" src="images/lists/trash.png" '.
                    'alt="'._('Remove').'" title="'._('Remove').'" name="'.$id.'_remove" id="'.$id.'_remove" '.
                    ($this->disabled? 'disabled="disabled"':'').'/>';
    }
    return $this->renderAcl($display);
  }

  function loadPostValue ()
  {
    $id = $this->getHtmlId();
    if (!$this->disabled && $this->isVisible()) {
      foreach (array_keys($_POST) as $name) {
        if (!$this->isRequired()) {
          if (preg_match('/^'.$id.'_remove_/', $name)) {
            $this->setValue('');
            $this->buttonText = NULL;
            break;
          }
        }
        if (preg_match('/^'.$id.'_/', $name)) {
          $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
          break;
        }
      }
    }
  }

  protected function loadAttrValue ($attrs)
  {
    parent::loadAttrValue($attrs);
    $dn = $this->getValue();
    if ($dn != '') {
      $ldap = $this->plugin->config->get_ldap_link();
      $ldap->cat($dn);
      if ($attrs = $ldap->fetch()) {
        $this->handleDialogResult($dn, $attrs);
      } else {
        $this->buttonText = NULL;
      }
    }
  }

  protected function htmlIds()
  {
    $ids = array($this->getHtmlId());
    if (!$this->isRequired()) {
      $ids[] = $ids[0].'_remove';
    }
    return $ids;
  }
}
?>
