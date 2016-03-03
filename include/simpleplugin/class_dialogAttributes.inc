<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2016  FusionDirectory

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
      return $this->handle_cancel();
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

  function handle_cancel ()
  {
    return FALSE;
  }
}

class GenericSelectDialog extends GenericDialog
{
  function dialog_execute ()
  {
    session::set('filterBlacklist', $this->attribute->getFilterBlackList());
    session::set('filterWhitelist', $this->attribute->getFilterWhiteList());
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
}

class UserGroupSelectDialog extends GenericSelectDialog
{
  protected $dialogClass = 'userGroupSelect';
}

class UserGroupRoleSelectDialog extends GenericSelectDialog
{
  protected $dialogClass = 'userGroupRoleSelect';
}

class GroupSelectDialog extends GenericSelectDialog
{
  protected $dialogClass = 'groupSelect';
}

class MailSelectDialog extends GenericSelectDialog
{
  protected $dialogClass = 'mailAddressSelect';
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
    $this->attribute = FALSE;
  }

  function addPostValue ($value)
  {
    $this->addValue($value, NULL);
  }

  function delPostValue ($key)
  {
    $this->removeValue($key);
  }

  function addValue ($value, $attrs)
  {
    trigger_error("abstract method");
  }

  function searchAndRemove ($value)
  {
    $row = array_search($value, $this->value);
    if ($row !== FALSE) {
      $this->removeValue($row);
    }
  }

  protected function removeValue ($row)
  {
    unset($this->value[$row]);
  }

  function getFilterBlackList ()
  {
    trigger_error("abstract method");
  }

  function getFilterWhiteList ()
  {
    return array();
  }

  function loadPostValue ()
  {
    parent::loadPostValue();
    if ($this->isVisible()) {
      if (isset($_POST['add'.$this->getHtmlId().'_dialog'])) {
        $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
      }
    }
  }

  function applyPostValue ()
  {
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons  = $this->renderInputField(
      'submit', 'add'.$id.'_dialog',
      array('value' => '{msgPool type=addButton}')
    );
    $buttons  .= $this->renderInputField(
      'submit', 'del'.$id,
      array('value' => '{msgPool type=delButton}')
    );
    return $buttons;
  }

  protected function htmlIds()
  {
    $id = $this->getHtmlId();
    return array('add'.$id.'_dialog','del'.$id,'row'.$id);
  }
}

class DialogOrderedArrayAttribute extends OrderedArrayAttribute
{
  protected $dialogClass;

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = array(), $acl = "")
  {
    Attribute::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->edit_enabled = TRUE;
    $this->attribute    = FALSE;
  }

  function loadPostValue ()
  {
    if ($this->isVisible()) {
      parent::loadPostValue();
      parent::applyPostValue();
      if (isset($_POST['add'.$this->getHtmlId().'_dialog'])) {
        $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
      }
    }
  }

  protected function handleEdit($key)
  {
    $this->editingValue = $this->value[$key];
    $this->delPostValue($key);
    $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this, $this->editingValue));
  }

  function applyPostValue ()
  {
  }

  function addValue($value)
  {
    $this->value[] = $value;
    $this->reIndexValues();
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    return $this->renderInputField(
      'submit', 'add'.$id.'_dialog',
      array('value' => '{msgPool type=addButton}')
    );
  }

  protected function htmlIds()
  {
    $ids = parent::htmlIds();
    $ids[0] .= '_dialog';
    return $ids;
  }

  protected function getAttributeArrayValue($key, $value)
  {
    /* Convert text value to displayable array value */
    die('Pure virtual method');
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
      end($this->value);
      $new_key = key($this->value) + 1;
      $this->value[$new_key] = $value;
      $this->fillDisplayValueFrom($new_key, $attrs);
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
      $ldap->cd($this->plugin->config->current['BASE']);
      $ldap->search('('.$this->store_attr.'='.ldap_escape_f($value).')', $this->ldapAttributesToGet());
    }
    $this->fillDisplayValueFrom($i, $ldap->fetch());
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
 */
class UsersGroupsAttribute extends GenericDialogAttribute
{
  protected $dialogClass = 'UserGroupSelectDialog';

  protected function ldapAttributesToGet ()
  {
    return array('cn', 'uid', 'objectClass');
  }

  protected function fillDisplayValueFrom ($i, $attrs)
  {
    if (!isset($attrs[$this->display_attr])) {
      unset($this->value[$i]);
    } else {
      if ($this->display_attr == 'dn') {
        $this->displays[$i] = $attrs['dn'];
      } elseif (in_array('posixGroup', $attrs['objectClass'])) {
        $this->displays[$i] = sprintf(_('Group %s'), trim($attrs['cn'][0]));
      } elseif (in_array('organizationalRole', $attrs['objectClass'])) {
        $this->displays[$i] = sprintf(_('Role %s'), trim($attrs['cn'][0]));
      } elseif (isset($attrs['uid'][0])) {
        $this->displays[$i] = trim($attrs['uid'][0]);
      } else {
        $this->displays[$i] = $attrs[$this->display_attr][0];
      }
    }
  }
}

class UsersGroupsRolesAttribute extends UsersGroupsAttribute
{
  protected $dialogClass = 'UserGroupRoleSelectDialog';
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

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons  = $this->renderInputField(
      'submit', 'add'.$id,
      array('value' => '{msgPool type=addButton}')
    );
    $buttons  .= $this->renderInputField(
      'submit', 'add'.$id.'_dialog',
      array('value' => '{msgPool type=addButton} (from list)')
    );
    $buttons  .= $this->renderInputField(
      'submit', 'del'.$id,
      array('value' => '{msgPool type=delButton}')
    );
    return $buttons;
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
  protected $action;

  function __construct ($label, $description, $ldapName, $buttonText, $action = NULL, $defaultValue = "", $acl = "")
  {
    parent::__construct($label, $description, $ldapName, FALSE, $defaultValue, $acl);
    $this->buttonText = $buttonText;
    if ($action === NULL) {
      $action = 'handle_'.$ldapName;
    }
    $this->action = $action;
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display = $this->renderInputField(
      'submit', $id,
      array('value' => '{literal}'.htmlentities($this->buttonText, ENT_COMPAT, 'UTF-8').'{/literal}')
    );
    return $this->renderAcl($display);
  }

  function loadPostValue ()
  {
    $this->postValue = FALSE;
    if ($this->isVisible()) {
      if (isset($_POST[$this->getHtmlId()])) {
        $this->setPostValue(TRUE);
      }
    }
  }

  function applyPostValue ()
  {
    if (!$this->disabled && $this->isVisible() && $this->postValue) {
      $func = $this->action;
      $this->plugin->$func();
    }
  }
}

class DialogButtonAttribute extends ButtonAttribute
{
  protected $dialogClass  = NULL;

  function __construct ($label, $description, $ldapName, $buttonText, $dialogClass, $defaultValue = '', $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $buttonText, NULL, $defaultValue, $acl);
    $this->dialogClass  = $dialogClass;
  }

  function applyPostValue ()
  {
    if (!$this->disabled && $this->isVisible() && $this->postValue) {
      $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
    }
  }
}

class UserAttribute extends DialogButtonAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "")
  {
    parent::__construct($label, $description, $ldapName, '', 'SingleUserSelectDialog', $defaultValue, $acl);
    $this->setRequired($required);
  }

  function handleDialogResult ($dn, $attrs)
  {
    $this->setValue($dn);
    $this->buttonText = $attrs['cn'][0];
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display  = '<img src="geticon.php?context=types&amp;icon=user&amp;size=16" alt="user icon" class="center"/>';
    if ($this->getValue() == '') {
      $display  .= '<b>'._('None').'</b>';
    } elseif ($this->buttonText === NULL) {
      $display  .= '<b>'._('Unknown').'</b>';
    } else {
      $display  .= '<a>'.$this->buttonText.'</a>';
    }
    $display .= $this->renderInputField(
      'image', $id,
      array(
        'class' => 'center',
        'src'   => 'geticon.php?context=actions&amp;icon=document-edit&amp;size=16',
        'title' => _('Edit'),
        'alt'   => _('Edit')
      )
    );
    if (!$this->isRequired()) {
      $display .= $this->renderInputField(
        'image', $id.'_remove',
        array(
          'class' => 'center',
          'src'   => 'geticon.php?context=actions&amp;icon=edit-delete&amp;size=16',
          'title' => _('Remove'),
          'alt'   => _('Remove')
        )
      );
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

class GenericSimplePluginDialog extends GenericDialog
{
  protected $initialDialogValue = NULL;

  function __construct($simplePlugin, &$attribute, $value = array())
  {
    $this->attribute  = $attribute;
    $this->dialog     = new $this->dialogClass($simplePlugin->config, 'new');

    $base = $simplePlugin->dn;
    if ($base == 'new') {
      $base = $simplePlugin->base;
    }
    $this->dialog->set_acl_base($base);
    $this->dialog->set_acl_category($simplePlugin->acl_category);
    if (!empty($value)) {
      $this->initialDialogValue = $value;
      foreach ($this->dialog->attributes as $attr) {
        if (isset($value[$attr])) {
          $this->dialog->$attr = $value[$attr];
        }
      }
    }
  }

  function buttons ()
  {
    return '<div style="width:100%; text-align:right; clear:both; float:none;">'.
           '  <input type="submit" name="'.$this->post_finish.'" value="'.msgPool::saveButton().'"/>&nbsp;'.
           '  <input type="submit" name="'.$this->post_cancel.'" value="'.msgPool::cancelButton().'"/>'.
           '</div>';
  }

  function dialog_execute ()
  {
    $this->dialog->save_object();
    return $this->dialog->execute().$this->buttons();
  }

  function handle_finish ()
  {
    $this->dialog->save_object();
    if (count($this->dialog->check())) {
      foreach ($this->dialog->check() as $msg) {
        msg_dialog::display(_('Error'), $msg, ERROR_DIALOG);
      }
      return $this->dialog->execute().$this->buttons();
    }

    $value = array();
    foreach ($this->dialog->attributes as $attr) {
      $value[$attr] = $this->dialog->$attr;
    }

    $this->attribute->addValue($value);
    return FALSE;
  }

  function handle_cancel ()
  {
    if ($this->initialDialogValue !== NULL) {
      $this->attribute->addValue($this->initialDialogValue);
    }
    return FALSE;
  }
}
?>
