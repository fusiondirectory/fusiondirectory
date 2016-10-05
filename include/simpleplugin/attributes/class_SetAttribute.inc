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

/*! \brief This class allow to handle easily a multi-valuated attribute
 *
 */
class SetAttribute extends Attribute
{
  public $attribute;
  protected $valueUnicity     = TRUE;
  protected $editingValue     = FALSE;
  protected $linearRendering  = TRUE;
  protected $size = 4;

  /*! \brief The constructor of SetAttribute
   *
   *  \param Attribute $attribute The attribute you want to see multi-valuated
   *  \param array $values The default values
   *  \param boolean $valueUnicity Should the value unicity be checked
   */
  function __construct ($attribute, $values = array(), $valueUnicity = TRUE)
  {
    parent::__construct(
      $attribute->getLabel(),     $attribute->getDescription(),
      $attribute->getLdapName(),  $attribute->isRequired(),
      $values
    );
    $this->attribute = $attribute;
    $this->attribute->setRequired(TRUE);
    $this->valueUnicity = $valueUnicity;
  }

  function setManagedAttributes ($dontcare)
  {
    trigger_error('method setManagedAttributes is not supported for SetAttributes');
  }

  function setLinearRendering ($bool)
  {
    $this->linearRendering = $bool;
  }

  protected function loadAttrValue ($attrs)
  {
    if (isset($attrs[$this->getLdapName()]["count"])) {
      $this->value = array();
      for ($i = 0; $i < $attrs[$this->getLdapName()]["count"]; $i++) {
        $this->value[] = $attrs[$this->getLdapName()][$i];
      }
    } else {
      $this->resetToDefault();
    }
  }

  function getAcl ()
  {
    if ($this->attribute === FALSE) {
      return parent::getAcl();
    }
    return $this->attribute->getAcl();
  }

  function setAcl ($acl)
  {
    if ($this->attribute === FALSE) {
      return parent::setAcl($acl);
    }
    $this->attribute->setAcl($acl);
  }

  function addPostValue ($value)
  {
    if ($value === '') {
      return FALSE;
    }
    if ($this->valueUnicity && in_array($value, $this->postValue, TRUE)) {
      return FALSE;
    }
    $this->postValue[] = $value;
    return TRUE;
  }

  function delPostValue ($key)
  {
    unset($this->postValue[$key]);
  }

  function loadPostValue ()
  {
    $this->editingValue = FALSE;
    $id = $this->getHtmlId();
    if ($this->isVisible()) {
      $this->postValue = $this->value;
      if (isset($_POST["add".$id])) {
        if ($this->attribute !== FALSE) {
          $this->attribute->loadPostValue();
          $this->attribute->applyPostValue();
          $this->addPostValue($this->attribute->getValue());
        }
      } elseif (isset($_POST["del".$id]) && isset($_POST["row".$id])) {
        foreach ($_POST["row".$id] as $key) {
          $this->delPostValue($key);
        }
      } elseif ($this->attribute !== FALSE) {
        $this->attribute->loadPostValue();
        $this->attribute->applyPostValue();
        $this->editingValue = $this->attribute->getValue();
      }
    }
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error) || ($this->attribute === FALSE)) {
      return $error;
    } else {
      foreach ($this->value as $value) {
        $this->attribute->setValue($value);
        $error = $this->attribute->check();
        if (!empty($error)) {
          return $error;
        }
      }
    }
  }

  function renderFormInput ()
  {
    $display      = $this->renderOnlyFormInput();
    $attr_display = $this->renderAttributeInput(FALSE);
    $buttons      = $this->renderButtons();
    return $this->renderAcl($display).$attr_display.$this->renderAcl($buttons);
  }

  function renderTemplateInput ()
  {
    $display      = $this->renderOnlyFormInput();
    $attr_display = $this->renderAttributeInput(TRUE);
    $buttons      = $this->renderButtons();
    return $this->renderAcl($display).$attr_display.$this->renderAcl($buttons);
  }

  function renderOnlyFormInput()
  {
    if (($this->size < 15) && ($this->size < count($this->value))) {
      $this->size = min(15, count($this->value));
    }
    $id = $this->getHtmlId();
    $smarty = get_smarty();
    $smarty->assign($id.'_values', $this->getDisplayValues());
    $display = '<select multiple="multiple"  name="row'.$id.'[]" id="row'.$id.'" size="'.$this->size.'"'.
                ($this->disabled? ' disabled="disabled"':'').
                ' >'."\n";
    $display .= '{html_options options=$'.$id.'_values}';
    $display .= '</select><br/>'."\n";
    return $display;
  }

  function getDisplayValues ()
  {
    if ($this->attribute === FALSE) {
      return $this->value;
    }
    $attribute = $this->attribute;
    return array_map(
      function ($value) use($attribute)
      {
        return $attribute->displayValue($value);
      },
      $this->value
    );
  }

  function handleEditingValue()
  {
    if ($this->editingValue === FALSE) {
      $this->attribute->resetToDefault();
    } else {
      $this->attribute->setValue($this->editingValue);
    }
  }

  function renderAttributeInput ($template = FALSE)
  {
    if ($this->attribute === FALSE) {
      return;
    }
    $this->handleEditingValue();
    if ($template) {
      return $this->attribute->renderTemplateInput();
    } else {
      return $this->attribute->renderFormInput();
    }
  }

  function renderAttribute(&$attributes, $readOnly)
  {
    if ($this->attribute === FALSE) {
      return parent::renderAttribute($attributes, $readOnly);
    }
    if ($this->visible) {
      $this->attribute->setDisabled($this->disabled);
      if ($this->linearRendering || $readOnly) {
        parent::renderAttribute($attributes, $readOnly);
      } else {
        $attributes[$this->getLdapName()] = array(
          'htmlid'      => $this->getForHtmlId(),
          'label'       => '{literal}'.$this->getLabel().'{/literal}'.($this->isRequired()?'{$must}':''),
          'description' => ($this->isRequired()?sprintf(_("%s (required)"), $this->getDescription()):$this->getDescription()),
          'input'       => $this->renderOnlyFormInput(),
        );
        $this->handleEditingValue();
        $this->attribute->renderAttribute($attributes, $readOnly);
        $attributes[$this->getLdapName().'_buttons'] = array(
          'htmlid'      => 'add'.$this->getHtmlId(),
          'label'       => '{literal}'.$this->getLabel().'{/literal}'.($this->isRequired()?'{$must}':''),
          'description' => '',
          'input'       => $this->renderButtons(),
        );
      }
    }
  }

  function serializeAttribute(&$attributes, $form = TRUE)
  {
    parent::serializeAttribute($attributes, $form);
    if ($this->attribute === FALSE) {
      return;
    }
    if ($form) {
      return;
    } else {
      $subattributes = array();
      $this->attribute->setDisabled($this->disabled);
      $this->attribute->serializeAttribute($subattributes, $form);
      $attributes[$this->getLdapName()]['attributes']       = $subattributes;
      $attributes[$this->getLdapName()]['attributes_order'] = array_keys($subattributes);
    }
  }

  function getForHtmlId()
  {
    // Label should point to the attribute
    if (is_object($this->attribute)) {
      return $this->attribute->getForHtmlId();
    } else {
      return '';
    }
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons  = $this->renderInputField('submit', 'add'.$id, array('value' => '{msgPool type=addButton}'));
    $buttons .= $this->renderInputField('submit', 'del'.$id, array('value' => '{msgPool type=delButton}'));
    return $buttons;
  }

  function computeLdapValue ()
  {
    return array_values($this->value);
  }

  public function htmlIds()
  {
    $id = $this->getHtmlId();
    return array_merge(array('add'.$id,'del'.$id,'row'.$id), $this->attribute->htmlIds());
  }

  /*! \brief Set the parent plugin for this attribute
   *
   *  \param simplePlugin &$plugin The parent plugin
   */
  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if ($this->attribute !== FALSE) {
      $this->attribute->setParent($plugin);
    }
  }

  function getArrayValues()
  {
    $result = array();
    foreach ($this->value as $value) {
      $this->attribute->setValue($value);
      $row = array();
      foreach ($this->attribute->getArrayValue() as $val) {
        $row[] = $val;
      }
      $result[] = $row;
    }
    return $result;
  }

  function foreignKeyUpdate($oldvalue, $newvalue, $source)
  {
    foreach ($this->value as $key => &$value) {
      if ($value == $oldvalue) {
        if ($newvalue === NULL) {
          unset($this->value[$key]);
        } elseif ($source['MODE'] == 'copy') {
          $this->value[] = $newvalue;
        } elseif ($source['MODE'] == 'move') {
          $value = $newvalue;
        }
      }
    }
    unset($value);
  }

  function foreignKeyCheck($value, $source)
  {
    return in_array($value, $this->value);
  }

  function setSize($size)
  {
    $this->size = $size;
  }
}

class OrderedArrayAttribute extends SetAttribute
{
  protected $order;
  protected $edit_enabled;
  protected $height   = 90;
  protected $headers  = FALSE;

  /*! \brief The constructor of OrderedArrayAttribute
   *
   *  \param Attribute $attribute The composite attribute you want to see multi-valuated
   *  \param array $values The default values
   */
  function __construct ($attribute, $order = TRUE, $values = array(), $edit_enabled = FALSE)
  {
    parent::__construct($attribute, $values);
    $this->order        = $order;
    $this->edit_enabled = $edit_enabled;
  }

  function setHeight($h)
  {
    $this->height = $h;
  }

  function setHeaders($h)
  {
    $this->headers = $h;
  }

  function readValue($value)
  {
    if ($this->order) {
      return preg_split('/:/', $value, 2);
    } else {
      return $value;
    }
  }

  function writeValue($key, $value)
  {
    if ($this->order) {
      return $key.":".$value;
    } else {
      return $value;
    }
  }

  function computeLdapValue ()
  {
    $ldapValue = array();
    foreach ($this->value as $key => $value) {
      $ldapValue[] = $this->writeValue($key, $value);
    }
    return $ldapValue;
  }

  protected function loadAttrValue ($attrs)
  {
    if (isset($attrs[$this->getLdapName()]["count"])) {
      $this->value = array();
      for ($i = 0; $i < $attrs[$this->getLdapName()]["count"]; $i++) {
        $value = $this->readValue($attrs[$this->getLdapName()][$i]);
        if (is_array($value)) {
          $this->value[$value[0]] = $value[1];
        } else {
          $this->value[] = $value;
        }
      }
    } else {
      $this->resetToDefault();
    }
    if ($this->order) {
      $this->reIndexValues();
    }
  }

  function renderOnlyFormInput ()
  {
    $id = $this->getHtmlId();
    $div = new divSelectBox('rows'.$id);
    $smarty = get_smarty();
    $div->SetHeight($this->height);
    $div->SetHeaders($this->headers);
    foreach ($this->value as $key => $value) {
      $fields = array();
      foreach ($this->getAttributeArrayValue($key, $value) as $field) {
        if (is_array($field)) {
          $fields[] = $field;
        } else {
          $fields[] = array('string' => $field);
        }
      }
      if (empty($fields)) {
        continue;
      }

      list ($img, $width) = $this->genRowIcons($key, $value);

      $fields[] = array("html" => $img, "attach" => 'style="border:0px;width:'.$width.'px;"');
      $div->AddEntry($fields);
    }
    $smarty->assign("div_$id", $div->DrawList());
    return '{$div_'.$id.'}'."\n";
  }

  protected function genRowIcons($key, $value)
  {
    $id = $this->getHtmlId();

    $img = '';
    $width = 25;

    if ($this->order) {
      $width += 20;
      if ($key != 0) {
        $img .= $this->renderInputField(
          'image', $id.'_up_'.$key,
          array(
            'src'   => 'geticon.php?context=actions&amp;icon=view-sort-descending&amp;size=16',
            'title' => _('Sort up'),
            'alt'   => _('Sort up'),
            'class' => 'center'
          )
        ).'&nbsp;';
      } else {
        $img .= '<img src="images/empty.png" alt="" style="width:10px;"/>';
      }
      if (($key + 1) < count($this->value)) {
        $img .= $this->renderInputField(
          'image', $id.'_down_'.$key,
          array(
            'src'   => 'geticon.php?context=actions&amp;icon=view-sort-ascending&amp;size=16',
            'title' => _('Sort down'),
            'alt'   => _('Sort down'),
            'class' => 'center'
          )
        ).'&nbsp;';
      } else {
        $img .= '<img src="images/empty.png" alt="" style="width:10px;"/>';
      }
    }
    if ($this->edit_enabled) {
      $width += 15;
      $img .= $this->renderInputField(
        'image', $id.'_edit_'.$key,
        array(
          'src'   => 'geticon.php?context=actions&amp;icon=document-edit&amp;size=16',
          'title' => _('Edit'),
          'alt'   => _('Edit'),
          'class' => 'center'
        )
      ).'&nbsp;';
    }
    $img .= $this->renderInputField(
      'image', $id.'_del_'.$key,
      array(
        'src'   => 'geticon.php?context=actions&amp;icon=edit-delete&amp;size=16',
        'title' => _('Delete'),
        'alt'   => _('Delete'),
        'class' => 'center'
      )
    ).'&nbsp;';

    return array ($img, $width);
  }

  protected function getAttributeArrayValue($key, $value)
  {
    $this->attribute->setValue($value);
    return $this->attribute->getArrayValue();
  }

  protected function reIndexValues ()
  {
    $this->value = array_values($this->value);
  }

  function loadPostValue ()
  {
    $this->editingValue = FALSE;
    if ($this->isVisible()) {
      $this->postValue = $this->value;
      $id = $this->getHtmlId();
      foreach (array_keys($_POST) as $name) {
        if ($this->handlePostValueActions($id, $name)) {
          break;
        }
      }
      $this->handleAddAndEditValue();
    }
  }

  protected function handlePostValueActions($id, $postValue)
  {
    if ($this->order) {
      if (preg_match('/^'.$id.'_up_/', $postValue)) {
        $key = preg_replace('/^'.$id.'_up_/', '', $postValue);
        $key = preg_replace('/_[xy]$/', '', $key);

        $tmp                        = $this->postValue[$key];
        $this->postValue[$key]      = $this->postValue[$key - 1];
        $this->postValue[$key - 1]  = $tmp;
        return TRUE;
      }
      if (preg_match('/^'.$id.'_down_/', $postValue)) {
        $key = preg_replace('/^'.$id.'_down_/', '', $postValue);
        $key = preg_replace('/_[xy]$/', '', $key);

        $tmp                        = $this->postValue[$key];
        $this->postValue[$key]      = $this->postValue[$key + 1];
        $this->postValue[$key + 1]  = $tmp;
        return TRUE;
      }
    }
    if ($this->edit_enabled) {
      if (preg_match('/^'.$id.'_edit_/', $postValue)) {
        $key = preg_replace('/^'.$id.'_edit_/', '', $postValue);
        $key = preg_replace('/_[xy]$/', '', $key);
        $this->handleEdit($key);
        return TRUE;
      }
    }
    if (preg_match('/^'.$id.'_del_/', $postValue)) {
      $key = preg_replace('/^'.$id.'_del_/', '', $postValue);
      $key = preg_replace('/_[xy]$/', '', $key);
      $this->delPostValue($key);
      return TRUE;
    }
    return FALSE;
  }

  protected function handleAddAndEditValue()
  {
    $id = $this->getHtmlId();
    if ($this->attribute === FALSE) {
      return;
    }
    if (isset($_POST["add$id"])) {
      $this->attribute->loadPostValue();
      $this->attribute->applyPostValue();
      if ($error = $this->attribute->check()) {
        $label = $this->getLabel();
        if (empty($label)) {
          $label = '<em>'.$this->getLdapName().'</em>';
        } else {
          $label = sprintf(_('"%s"'), $label);
        }
        msg_dialog::display(sprintf(_('Invalid value for %s'), $label), $error);
      } else {
        $this->addPostValue($this->attribute->getValue());
      }
    } elseif ($this->editingValue === FALSE) {
      $this->attribute->loadPostValue();
      $this->attribute->applyPostValue();
      $this->editingValue = $this->attribute->getValue();
    }
  }

  protected function handleEdit($key)
  {
    $this->editingValue = $this->value[$key];
    $this->delPostValue($key);
    $this->plugin->focusedField = $this->getHtmlId();
  }

  function applyPostValue ()
  {
    parent::applyPostValue();
    if ($this->order) {
      $this->reIndexValues();
    }
  }

  public function htmlIds()
  {
    $id   = $this->getHtmlId();
    $ids  = array('add'.$id);
    if ($this->attribute !== FALSE) {
      $ids  = array_merge($ids, $this->attribute->htmlIds());
    }
    $nb_values = count($this->value);
    for ($i = 0; $i < $nb_values; ++$i) {
      if ($this->order) {
        if ($i > 0) {
          $ids[] = $id.'_up_'.$i;
        }
        if (($i + 1) < $nb_values) {
          $ids[] = $id.'_down_'.$i;
        }
      }
      $ids[] = $id.'_del_'.$i;
    }
    return $ids;
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons = $this->renderInputField('submit', 'add'.$id, array('value' => '{msgPool type=addButton}'));
    return $buttons;
  }
}

class SubNodesAttribute extends OrderedArrayAttribute
{
  protected $objectClass;
  protected $objectClasses;

  function __construct ($label, $description, $ldapName, $objectClass, $attributes, $order = FALSE, $values = array(), $edit_enabled = FALSE, $acl = "")
  {
    $attributes_keys = array();
    foreach ($attributes as $attribute) {
      $attributes_keys[$attribute->getLdapName()] = $attribute;
      $attributes_keys[$attribute->getLdapName()]->htmlid_prefix = $ldapName.'_';
    }
    $composite = new CompositeAttribute(
      $description, $ldapName,
      $attributes_keys,
      FALSE, FALSE,
      $acl, $label
    );
    parent::__construct($composite, $order, $values, $edit_enabled);
    if (is_array($objectClass)) {
      $this->objectClass    = $objectClass[0];
      $this->objectClasses  = $objectClass;
    } else {
      $this->objectClass    = $objectClass;
      $this->objectClasses  = array($objectClass);
    }
  }

  protected function loadAttrValue ($attrs)
  {
    global $config;
    /* Should we take dn from attrs or plugin? */
    if (isset($attrs['dn'])) {
      $ldap = $config->get_ldap_link();
      $ldap->ls('objectClass='.$this->objectClass, $attrs['dn']);
      $this->value = array();
      while ($subattrs = $ldap->fetch()) {
        $this->attribute->resetToDefault();
        foreach ($this->attribute->attributes as &$attribute) {
          $attribute->loadAttrValue($subattrs);
        }
        unset($attribute);
        $this->value[] = $this->attribute->getValue();
      }
    } else {
      $this->resetToDefault();
    }
  }

  /* Not saving anything into base node */
  function fillLdapValue (&$attrs)
  {
  }

  /* Special LDAP treatment that this attribute does after plugin ldap save */
  function postLdapSave ($ldap)
  {
    /* First delete all old nodes */
    $ldap->ls('objectClass='.$this->objectClass, $this->plugin->dn, array('dn'));
    $delete = array();
    while ($attrs = $ldap->fetch()) {
      $delete[] = $attrs['dn'];
    }
    foreach ($delete as $dn) {
      $ldap->rmdir($dn);
    }
    /* Then add our values */
    foreach ($this->value as $val) {
      $attrs = array('objectClass' => $this->objectClasses);
      $this->attribute->setValue($val);
      foreach ($this->attribute->attributes as &$attribute) {
        $attribute->fillLdapValue($attrs);
      }
      unset($attribute);
      $dn = $this->compute_attribute_dn();
      $ldap->cd($dn);
      foreach (array_keys($attrs) as $index) {
        if (is_array($attrs[$index]) && (count($attrs[$index]) == 0)) {
          unset($attrs[$index]);
        }
      }
      $ldap->add($attrs);
      if (!$ldap->success()) {
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_ADD, get_class()), LDAP_ERROR);
      }
    }
  }

  function compute_attribute_dn ()
  {
    /* Later we might want to be able to choose which attribute to use in the dn */
    reset($this->attribute->attributes);
    $attribute = key($this->attribute->attributes);
    return $attribute.'='.$this->attribute->attributes[$attribute]->computeLdapValue().','.$this->plugin->dn;
  }
}
