<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2018  FusionDirectory

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

/*! \brief Generic dialog base class
 */
class GenericDialog
{
  protected $dialogClass = "";
  protected $dialog;
  protected $attribute;

  protected $post_cancel = 'add_cancel';
  protected $post_finish = 'add_finish';

  function __construct ($simplePlugin, $attribute)
  {
    $this->attribute  = $attribute;
    $this->dialog     = new $this->dialogClass();
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

/*! \brief Generic dialog for selection using a simpleSelectManagement based class
 */
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
      if ($this->dialog instanceof selectManagement) {
        $headpage = $this->dialog->listing;
      } else {
        $headpage = $this->dialog->getHeadpage();
      }
      foreach ($result['targets'] as $dn) {
        $entry = $headpage->getEntry($dn);

        $this->attribute->addValue($dn, $entry);
      }
    }
    return FALSE;
  }
}

/*!
 * \brief System selection
 */
class systemSelect extends selectManagement
{
  /* Default columns */
  public static $columns = [
    ['ObjectTypeColumn',  []],
    ['LinkColumn',        ['attributes' => 'nameAttr',      'label' => 'Name']],
    ['IpColumn',          ['attributes' => 'ipHostNumber',  'label' => 'IP']],
    ['Column',            ['attributes' => 'macAddress',    'label' => 'Mac']],
    ['LinkColumn',        ['attributes' => 'description',   'label' => 'Description']],
  ];
}

/*! \brief System selection dialog
*/
class SystemSelectDialog extends GenericSelectManagementDialog
{
  protected $dialogClass = 'systemSelect';
}

/*! \brief This class allows to handle an attribute with a popup for selection
 *
 * It looks like a SetAttribute, but clicking "Add" will open a dialog that allow to select one or more items.
 * It stores their dn as values, but displays the cn.
 *
 */
abstract class DialogAttribute extends SetAttribute
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
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $acl = "")
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

  abstract function getFilterBlackList ();

  function getFilterWhiteList ()
  {
    return [];
  }

  function loadPostValue ()
  {
    parent::loadPostValue();
    if ($this->isVisible()) {
      $id = $this->getHtmlId();
      if (isset($_POST['add'.$id.'_dialog'])) {
        $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
      } elseif (isset($_POST['add'.$id]) && isset($_POST[$id]) && $this->isTemplate()) {
        $this->addPostValue($_POST[$id]);
      }
    }
  }

  function applyPostValue ()
  {
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons            = '';
    $dialogButtonValue  = '{msgPool type=addButton}';
    if ($this->isTemplate()) {
      $buttons  .= $this->renderInputField(
        'text', $id,
        ['value' => $this->editingValue]
      );
      $buttons  .= $this->renderInputField(
        'submit', 'add'.$id,
        [
          'value'           => '{msgPool type=addButton}',
          'formnovalidate'  => 'formnovalidate'
        ]
      );
      $dialogButtonValue = _('Add (dialog)');
    }
    $buttons  .= $this->renderInputField(
      'submit', 'add'.$id.'_dialog',
      [
        'class'           => 'dialog',
        'value'           => $dialogButtonValue,
        'formnovalidate'  => 'formnovalidate'
      ]
    );
    $buttons  .= $this->renderInputField(
      'submit', 'del'.$id,
      [
        'value'           => '{msgPool type=delButton}',
        'formnovalidate'  => 'formnovalidate'
      ]
    );
    return $buttons;
  }

  public function htmlIds (): array
  {
    $id = $this->getHtmlId();
    $ids = ['add'.$id.'_dialog','del'.$id,'row'.$id];
    if ($this->isTemplate()) {
      $ids[] = $id;
      $ids[] = 'add'.$id;
    }
    return $ids;
  }
}

/*!
 * \brief An OrderedArrayAttribute which uses a dialog to create/edit values
 */
abstract class DialogOrderedArrayAttribute extends OrderedArrayAttribute
{
  protected $dialogClass;

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $acl = "")
  {
    Attribute::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->edit_enabled = TRUE;
    $this->attribute    = FALSE;
  }

  abstract function getFilterBlackList ();

  function getFilterWhiteList ()
  {
    return [];
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

  protected function handleEdit ($key)
  {
    $this->editingValue = $this->value[$key];
    $this->delPostValue($key);
    $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this, $this->editingValue));
  }

  function applyPostValue ()
  {
  }

  function addValue ($value)
  {
    $this->value[] = $value;
    $this->reIndexValues();
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    return $this->renderInputField(
      'submit', 'add'.$id.'_dialog',
      [
        'class'           => 'dialog',
        'value'           => '{msgPool type=addButton}',
        'formnovalidate'  => 'formnovalidate'
      ]
    );
  }

  public function htmlIds (): array
  {
    $ids = parent::htmlIds();
    $ids[0] .= '_dialog';
    return $ids;
  }

  protected function getAttributeArrayValue ($key, $value)
  {
    /* Convert text value to displayable array value */
    die('Pure virtual method');
  }
}

/*!
 * \brief Base class for DialogAttribute using a GenericDialog derived dialog
 */
class GenericDialogAttribute extends DialogAttribute
{
  protected $displays     = [];
  protected $store_attr   = 'dn';
  protected $display_attr = 'cn';

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $store_attr = 'dn', $display_attr = 'cn', $acl = '')
  {
    $this->store_attr   = $store_attr;
    $this->display_attr = $display_attr;
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
  }

  function getFilterBlackList ()
  {
    return [$this->store_attr => $this->getValue()];
  }

  function addValue ($dn, $attrs)
  {
    if (
        ($this->isTemplate() && ($attrs === NULL)) ||
        ($this->store_attr == 'dn')
      ) {
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

  function setDisplayValues (array $array)
  {
    $this->displays = $array;
  }

  protected function fillDisplayValue ($i)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $value = $this->value[$i];
    if ($this->store_attr == 'dn') {
      $ldap->cat($value, $this->ldapAttributesToGet());
    } else {
      $ldap->cd($config->current['BASE']);
      $ldap->search('('.$this->store_attr.'='.ldap_escape_f($value).')', $this->ldapAttributesToGet());
    }
    $attrs = $ldap->fetch();
    if (empty($attrs) && $this->isTemplate()) {
      $this->fillDisplayValueFrom($i, NULL);
    } else {
      $this->fillDisplayValueFrom($i, $attrs);
    }
  }

  protected function ldapAttributesToGet ()
  {
    return [$this->display_attr];
  }

  protected function fillDisplayValueFrom ($i, $attrs)
  {
    if ($this->isTemplate() && ($attrs === NULL)) {
      $this->displays[$i] = $this->value[$i];
    } elseif (!isset($attrs[$this->display_attr])) {
      unset($this->value[$i]);
    } else {
      if ($this->display_attr == 'dn') {
        $this->displays[$i] = $attrs['dn'];
      } else {
        $this->displays[$i] = $attrs[$this->display_attr][0];
      }
    }
  }

  function foreignKeyUpdate ($oldvalue, $newvalue, array $source)
  {
    foreach ($this->value as $key => &$value) {
      if (($source['FIELD'] == 'dn') && ($source['MODE'] == 'move')) {
        if ($newvalue === NULL) {
          if (preg_match('/'.preg_quote($oldvalue, '/').'$/', $value)) {
            $this->removeValue($key);
          }
        } else {
          $value = preg_replace('/'.preg_quote($oldvalue, '/').'$/', $newvalue, $value, -1, $count);
          if ($count > 0) {
            /* Update display */
            $this->fillDisplayValue($key);
          }
        }
      } elseif ($value == $oldvalue) {
        if ($newvalue === NULL) {
          $this->removeValue($key);
        } elseif ($source['MODE'] == 'copy') {
          $this->value[] = $newvalue;
        } elseif ($source['MODE'] == 'move') {
          $value = $newvalue;
          /* Update display */
          $this->fillDisplayValue($key);
        }
      }
    }
    unset($value);
  }
}

/*! \brief Generic dialog for selection using a selectManagement class
 */
class GenericSelectManagementDialog extends GenericSelectDialog
{
  protected $dialogClass = 'selectManagement';

  function __construct ($simplePlugin, $attribute)
  {
    $this->attribute  = $attribute;
    $this->dialog     = new $this->dialogClass(...$this->attribute->getSelectManagementParameters());
  }
}

/*! \brief Object selection dialog allowing only one object to get selected
 */
class GenericSingleSelectManagementDialog extends GenericSelectManagementDialog
{
  function handle_finish ()
  {
    $result = $this->dialog->detectPostActions();
    if (isset($result['targets']) && count($result['targets'])) {
      $dn     = $result['targets'][0];
      $entry  = $this->dialog->listing->getEntry($dn);
      $this->attribute->handleDialogResult($dn, $entry);
    }
    return FALSE;
  }
}

/*! \brief This class allows to handle an attribute for selecting objects
 *
 * It looks like a SetAttribute, but clicking "Add" will open a dialog that allow to select one or more objects.
 * It stores their dn as values, but displays the cn.
 *
 */
class ObjectsAttribute extends GenericDialogAttribute
{
  protected $dialogClass  = 'GenericSelectManagementDialog';

  protected $selectManagementParameters;

  function __construct (string $label, string $description, string $ldapName, bool $required, array $objectTypes, array $defaultValue = [], string $store_attr = 'dn', string $display_attr = 'cn', string $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $store_attr, $display_attr, $acl);
    $this->selectManagementParameters = [
      $objectTypes,
      TRUE,
      [
        'objectClass' => '*',
        $store_attr   => (($store_attr == 'dn') ? 'raw' : '*'),
        $display_attr => (($display_attr == 'dn') ? 'raw' : '*'),
      ]
    ];
  }

  public function getSelectManagementParameters (): array
  {
    return array_merge(
      $this->selectManagementParameters,
      [
        $this->getFilterBlackList(),
        $this->getFilterWhiteList(),
      ]
    );
  }
}

/*! \brief This class allows to handle an attribute for selecting users
 *
 * It looks like a SetAttribute, but clicking "Add" will open a dialog that allow to select one or more users.
 * It stores their dn as values, but displays the cn.
 *
 */
class UsersAttribute extends ObjectsAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $store_attr = 'dn', $display_attr = 'cn', $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, ['user'], $defaultValue, $store_attr, $display_attr, $acl);
  }
}

/*! \brief This class allows to handle an attribute for selecting user or groups
 *
 * It looks like a SetAttribute, but clicking "Add" will open a dialog that allow to select one or more users or groups.
 * It stores their dn as values, but displays the cn.
 */
class UsersGroupsRolesAttribute extends ObjectsAttribute
{
  function __construct (string $label, string $description, string $ldapName, bool $required = FALSE, array $defaultValue = [], string $store_attr = 'dn', string $display_attr = 'cn', string $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, ['user', 'group', 'role', 'ogroup'], $defaultValue, $store_attr, $display_attr, $acl);
    $this->selectManagementParameters[2]['cn']          = '*';
    $this->selectManagementParameters[2]['uid']         = '*';
    $this->selectManagementParameters[2]['objectClass'] = '*';
  }

  protected function ldapAttributesToGet ()
  {
    return array_values(array_unique([$this->store_attr, $this->display_attr, 'cn', 'uid', 'objectClass']));
  }

  protected function fillDisplayValueFrom ($i, $attrs)
  {
    if (!isset($attrs[$this->display_attr])) {
      unset($this->value[$i]);
    } else {
      $cn = $attrs['cn'][0];
      if ($this->display_attr == 'dn') {
        $this->displays[$i] = $attrs['dn'];
      } elseif (in_array('posixGroup', $attrs['objectClass'])) {
        $this->displays[$i] = sprintf(_('POSIX group %s'), trim($cn));
      } elseif (in_array('organizationalRole', $attrs['objectClass'])) {
        $this->displays[$i] = sprintf(_('Role %s'), trim($cn));
      } elseif (in_array('groupOfNames', $attrs['objectClass'])) {
        $this->displays[$i] = sprintf(_('Group %s'), trim($cn));
      } elseif (isset($attrs['uid'])) {
        $this->displays[$i] = trim($attrs['uid'][0]);
      } else {
        $this->displays[$i] = $attrs[$this->display_attr][0];
      }
    }
  }
}

/*!
 * \brief Attribute storing a system list
 */
class SystemsAttribute extends ObjectsAttribute
{
  protected $dialogClass  = 'SystemSelectDialog';

  function __construct (string $label, string $description, string $ldapName, bool $required, array $objectTypes = ['terminal', 'workstation', 'server'], array $defaultValue = [], string $store_attr = 'cn', string $display_attr = 'cn', string $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, $objectTypes, $defaultValue, $store_attr, $display_attr, $acl);
  }
}

/*!
 * \brief Attribute showing a button
 */
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

  function renderFormInput (): string
  {
    $id = $this->getHtmlId();
    $display = $this->renderInputField(
      'submit', $id,
      ['value' => '{literal}'.htmlentities($this->buttonText, ENT_COMPAT, 'UTF-8').'{/literal}']
    );
    return $this->renderAcl($display);
  }

  function loadPostValue ()
  {
    $this->postValue = FALSE;
    if ($this->isVisible() && isset($_POST[$this->getHtmlId()])) {
      $this->setPostValue(TRUE);
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

/*!
 * \brief Attribute showing a button which triggers a dialog
 */
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

  function getFilterBlackList ()
  {
    return [];
  }

  function getFilterWhiteList ()
  {
    return [];
  }
}

/*!
 * \brief Attribute storing the dn of an object which can be selected through a dialog
 */
class ObjectAttribute extends DialogButtonAttribute
{
  protected $objectType;
  protected $displayAttr;

  function __construct ($label, $description, $ldapName, $required, $objectType, $defaultValue = '', $acl = '')
  {
    parent::__construct($label, $description, $ldapName, '', 'GenericSingleSelectManagementDialog', $defaultValue, $acl);
    $this->setRequired($required);
    $this->objectType                 = $objectType;
  }

  public function getSelectManagementParameters (): array
  {
    return [[$this->objectType], FALSE];
  }

  function setParent (simplePlugin &$plugin)
  {
    parent::setParent($plugin);
    $infos = objects::infos($this->objectType);
    $this->displayAttr = $infos['nameAttr'];
  }

  function handleDialogResult ($dn, $entry)
  {
    $this->setValue($dn);

    $this->buttonText = $entry[$this->displayAttr];
    if (is_array($this->buttonText)) {
      $this->buttonText = $this->buttonText[0];
    }
  }

  function renderFormInput (): string
  {
    $infos = objects::infos($this->objectType);

    $id = $this->getHtmlId();
    if ($this->isTemplate()) {
      $display = $this->renderInputField(
        'text', $id,
        [
          'value' => '{literal}'.htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8').'{/literal}'
        ]
      );
    } else {
      $display  = '<img src="'.htmlentities($infos['icon'], ENT_COMPAT, 'UTF-8').'" alt="'.$infos['name'].'" class="center"/>';
      if ($this->getValue() == '') {
        $display  .= '<b>'._('None').'</b>';
      } elseif ($this->buttonText === NULL) {
        $display  .= '<b>'._('Unknown').'</b>';
      } else {
        $display  .= '<a>'.$this->buttonText.'</a>';
      }
    }
    $display .= $this->renderInputField(
      'image', $id.'_dialog',
      [
        'class' => 'center dialog',
        'src'   => 'geticon.php?context=actions&amp;icon=document-edit&amp;size=16',
        'title' => _('Edit'),
        'alt'   => _('Edit')
      ]
    );
    if (!$this->isRequired()) {
      $display .= $this->renderInputField(
        'image', $id.'_remove',
        [
          'class' => 'center',
          'src'   => 'geticon.php?context=actions&amp;icon=remove&amp;size=16',
          'title' => _('Remove'),
          'alt'   => _('Remove')
        ]
      );
    }
    return $this->renderAcl($display);
  }

  function loadPostValue ()
  {
    $id = $this->getHtmlId();
    if (!$this->disabled && $this->isVisible()) {
      foreach (array_keys($_POST) as $name) {
        if (!$this->isRequired() && preg_match('/^'.$id.'_remove_/', $name)) {
          $this->setValue('');
          $this->buttonText = NULL;
          return;
        }
        if (preg_match('/^'.$id.'_dialog_/', $name)) {
          $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
          return;
        }
      }
      if ($this->isTemplate() && isset($_POST[$id])) {
        $this->setValue($_POST[$id]);
      }
    }
  }

  function applyPostValue ()
  {
  }

  protected function loadAttrValue (array $attrs)
  {
    global $config;
    parent::loadAttrValue($attrs);
    if (!$this->isTemplate()) {
      $dn = $this->getValue();
      if ($dn != '') {
        $ldap = $config->get_ldap_link();
        $ldap->cat($dn, [$this->displayAttr]);
        if ($attrs = $ldap->fetch()) {
          $this->handleDialogResult($dn, $attrs);
        } else {
          $this->buttonText = NULL;
        }
      }
    }
  }

  public function htmlIds (): array
  {
    $id   = $this->getHtmlId();
    $ids  = [$id.'_dialog'];
    if ($this->isTemplate()) {
      $ids[] = $id;
    }
    if (!$this->isRequired()) {
      $ids[] = $id.'_remove';
    }
    return $ids;
  }
}

/*!
 * \brief Attribute storing the dn of a user which can be selected through a dialog
 */
class UserAttribute extends ObjectAttribute
{
  function __construct ($label, $description, $ldapName, $required, $defaultValue = '', $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, 'user', $defaultValue = '', $acl = '');
  }
}

/*!
 * \brief GenericDialog using a simplePlugin based class as dialog
 */
class GenericSimplePluginDialog extends GenericDialog
{
  protected $initialDialogValue = NULL;

  function __construct ($simplePlugin, $attribute, $value = [])
  {
    $this->attribute  = $attribute;
    $this->dialog     = new $this->dialogClass('new');

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
           '  <input type="submit" formnovalidate="formnovalidate" name="'.$this->post_cancel.'" value="'.msgPool::cancelButton().'"/>'.
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
    $msgs = $this->dialog->check();
    if (count($msgs)) {
      msg_dialog::displayChecks($msgs);
      return $this->dialog->execute().$this->buttons();
    }

    $value = [];
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
