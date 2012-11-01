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

/*! \brief This class allow to handle easily any kind of LDAP attribute
 *
 */
class Attribute
{
  /* \brief Name of this attribute in the LDAP */
  private $ldapName;
  /* \brief Label of this attribute in the form */
  private $label;
  /* \brief Description of this attribute */
  private $description;
  /* \brief Is this attribute mandatory */
  private $required;
  /* \brief Should this attribute be saved into the LDAP */
  private $inLdap = TRUE;
  /* \brief Should this attribute be saved into the LDAP */
  private $unique = FALSE;

  /* \brief Should this attribute be saved into the LDAP */
  protected $visible = TRUE;
  /* \brief Name of the ACL to use, empty if we need our own */
  protected $acl;
  /* \brief Is this attribute disabled */
  protected $disabled = FALSE;
  /* \brief Value of this attribute */
  protected $value;
  /* \brief Value we read from POST */
  protected $postValue;
  /* \brief Default value of this attribute */
  protected $defaultValue;
  /* \brief Initial value of this attribute */
  protected $initialValue;
  /* \brief Reference to the plugin */
  protected $plugin;
  /* \brief Array of attributes to manage (prefix => value => section => attribute)
   * Prefix should be 'erase' or 'disable' */
  protected $managedAttributes = array();
  /* \brief Array of multiple values groups for managed attributes */
  protected $managedAttributesMultipleValues = array();

  /*! \brief The constructor of Attribute
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
    $this->label        = $label;
    $this->description  = $description;
    $this->ldapName     = $ldapName;
    $this->required     = $required;
    $this->defaultValue = $defaultValue;
    $this->value        = $defaultValue;
    $this->postValue    = $this->value;
    $this->acl          = $acl;
    $this->plugin       = NULL;
  }

  /*! \brief Set the parent plugin for this attribute
   *
   *  \param simplePlugin &$plugin The parent plugin
   */
  function setParent (&$plugin)
  {
    $this->plugin = $plugin;
    $this->manageAttributes($this->getValue());
  }

  function setInLdap ($inLdap)
  {
    $this->inLdap = $inLdap;
  }

  function setVisible ($visible)
  {
    $this->visible = $visible;
  }

  function isVisible ()
  {
    return $this->visible;
  }

  function setUnique ($unique)
  {
    $this->unique = $unique;
  }

  function isInLdap ()
  {
    return $this->inLdap;
  }

  function setValue ($value)
  {
    $this->value = $value;
    $this->manageAttributes($this->value);
  }

  function setPostValue ($value)
  {
    $this->postValue = $value;
    $this->manageAttributes($this->postValue);
  }

  /*! \brief Reset this attribute to its default value
   */
  function resetToDefault ()
  {
    $this->setValue($this->defaultValue);
  }

  function getValue ()
  {
    return $this->value;
  }

  function getArrayValue ()
  {
    $value = $this->getValue();
    if (is_array($value)) {
      return $value;
    } else {
      return array($value);
    }
  }

  function getLdapName ()
  {
    return $this->ldapName;
  }

  function getHtmlId ()
  {
    return preg_replace('/[\/\-]/', '_', $this->getLdapName());
  }

  function getLabel ()
  {
    return $this->label;
  }

  function getDescription ()
  {
    return $this->description;
  }

  function getAcl ()
  {
    if (empty($this->acl)) {
      return $this->getHtmlId();
    } else {
      return $this->acl;
    }
  }

  function setAcl ($acl)
  {
    $this->acl = $acl;
  }

  function isRequired ()
  {
    return $this->required;
  }

  protected function setRequired ($bool)
  {
    $this->required = $bool;
  }

  protected function setLabel ($label)
  {
    $this->label = $label;
  }

  protected function setDescription ($description)
  {
    $this->description = $description;
  }

  function setDisabled ($disabled)
  {
    $this->disabled = $disabled;
  }

  /*! \brief If in LDAP, loads this attribute value from the attrs array
   */
  function loadValue ($attrs)
  {
    if ($this->inLdap) {
      $this->loadAttrValue($attrs);
    }
    $this->initialValue = $this->getValue();
  }

  /*! \brief Loads this attribute value from the attrs array
   */
  protected function loadAttrValue ($attrs)
  {
    if (isset($attrs[$this->getLdapName()])) {
      $this->setValue($this->inputValue($attrs[$this->getLdapName()][0]));
    }
  }

  function getInitialValue ()
  {
    return $this->initialValue;
  }

  function hasChanged ()
  {
    return ($this->getValue() !== $this->initialValue);
  }

  function displayValue ($value)
  {
    return $value;
  }

  function inputValue ($value)
  {
    return $value;
  }

  function setDefaultValue ($value)
  {
    $this->defaultValue = $this->inputValue($value);
  }

  /*! \brief Set a list of attributes that are managed by this attributes.
   * See FusionDirectory wiki for detailed documentation
   */
  function setManagedAttributes ($mAttributes)
  {
    if (isset($mAttributes['multiplevalues'])) {
      $this->managedAttributesMultipleValues = $mAttributes['multiplevalues'];
      unset($mAttributes['multiplevalues']);
    } else {
      $this->managedAttributesMultipleValues = array();
    }
    $this->managedAttributes = $mAttributes;
    $this->manageAttributes($this->getValue());
  }

  protected function isValueManagingValue ($myvalue, $mavalue)
  {
    if (isset($this->managedAttributesMultipleValues[$mavalue])) {
      return in_array($myvalue, $this->managedAttributesMultipleValues[$mavalue]);
    } else {
      return ($myvalue == $mavalue);
    }
  }

  function manageAttributes ($myvalue)
  {
    if ($this->plugin === NULL) {
      return FALSE;
    }
    foreach ($this->managedAttributes as $prefix => $array) {
      foreach ($array as $value => $sections) {
        foreach ($sections as $section => $attributes) {
          foreach ($attributes as $attribute) {
            $disabled = $this->isValueManagingValue($myvalue, $value);
            $this->plugin->attributesInfo[$section]['attrs'][$attribute]->setDisabled($disabled);
          }
        }
      }
    }
    return TRUE;
  }

  /*! \brief Update this attributes postValue depending of the $_POST values
   */
  function loadPostValue ()
  {
    if (isset($_POST[$this->getHtmlId()])) {
      $this->setPostValue($_POST[$this->getHtmlId()]);
    }
  }

  /*! \brief Apply this attribute postValue in value if this attribute is enabled
   */
  function applyPostValue ()
  {
    if (!$this->disabled) {
      $this->setValue($this->postValue);
    }
  }

  /*! \brief Computes LDAP value
   */
  function computeLdapValue ()
  {
    return $this->getValue();
  }

  /*! \brief Fill LDAP value in the attrs array
   */
  function fillLdapValue (&$attrs)
  {
    if ($this->inLdap) {
      $value = $this->computeLdapValue();
      if ($value != "") {
        $attrs[$this->getLdapName()] = $value;
      } else {
        $attrs[$this->getLdapName()] = array();
      }
    }
  }

  /*! \brief Post-modify the attrs array if needed (used for erasing managed attributes)
   */
  function fillLdapValueHook (&$attrs)
  {
    foreach ($this->managedAttributes as $prefix => $array) {
      if ($prefix != 'erase') {
        continue;
      }
      foreach ($array as $value => $sections) {
        $myvalue = $this->getValue();
        $erase = $this->isValueManagingValue($myvalue, $value);
        if (!$erase) {
          continue;
        }
        foreach ($sections as $section => $attributes) {
          foreach ($attributes as $attribute) {
            $attrs[$attribute] = array();
          }
        }
      }
    }
  }

  /*! \brief Check the correctness of this attribute
   */
  function check ()
  {
    if ($this->isRequired() && !$this->disabled && (($this->value == "") || ($this->value == array()))) {
      return msgPool::required($this->getLabel());
    } elseif ($this->unique && !$this->disabled && ($this->value != "") && ($this->value != array())) {
      $ldap = $this->plugin->config->get_ldap_link();
      $ldap->cd(dn2base($this->plugin->dn));
      $filter = "(".$this->getLdapName()."=".$this->computeLdapValue().")";
      $pluginFilter = $this->plugin->getObjectClassFilter();
      if (!empty($pluginFilter)) {
        $filter = "(&$pluginFilter$filter)";
      }
      $ldap->search($filter, array($this->getLdapName()));
      while ($attrs = $ldap->fetch()) {
        if ($attrs['dn'] != $this->plugin->dn) {
          return msgPool::duplicated($this->getLabel());
        }
      }
    }
  }

  /*! \brief Render this attribute form input(s)
   */
  function renderAttribute(&$attributes)
  {
    if ($this->visible) {
      $attributes[$this->getLdapName()] = array(
        'htmlid'      => $this->getHtmlId(),
        'label'       => '{literal}'.$this->getLabel().'{/literal}'.($this->isRequired()?'{$must}':''),
        'description' => ($this->isRequired()?sprintf(_("%s (required)"), $this->getDescription()):$this->getDescription()),
        'input'       => $this->renderFormInput(),
      );
    }
  }

  /*! \brief Add ACL information around display
   *
   *  \param string $display the display information to pass through ACL
   */
  function renderAcl($display)
  {
    return '{render acl=$'.$this->getAcl()."ACL}\n$display\n{/render}";
  }

  /*! \brief Get ACL information about the ACL we need to create
   */
  function getAclInfo ()
  {
    if (empty($this->acl)) { // If acl is not empty, we use an acl that is not ours, we have no acl to create
      return array(
        'name' => $this->getHtmlId(),
        'desc' => $this->getDescription()
      );
    } else {
      return FALSE;
    }
  }

  protected function changeStateJS ()
  {
    return join(array_map(
      function ($id) {
        return 'changeState('.json_encode($id).');';
      },
      $this->htmlIds()
    ));
  }

  protected function htmlIds()
  {
    return array($this->getHtmlId());
  }

  protected function managedAttributesJS ()
  {
    $js = '';
    $id = $this->getHtmlId();
    foreach ($this->managedAttributes as $prefix => $array) {
      foreach ($array as $value => $sections) {
        if (isset($this->managedAttributesMultipleValues[$value])) {
          $js .= 'disableAttributes = inArray(document.getElementById('.json_encode($id).').value,'.json_encode($this->managedAttributesMultipleValues[$value]).');';
        } else {
          $js .= 'disableAttributes = (document.getElementById('.json_encode($id).').value == '.json_encode($value).');'."\n";
        }
        foreach ($sections as $section => $attributes) {
          foreach ($attributes as $attribute) {
            foreach ($this->plugin->attributesInfo[$section]['attrs'][$attribute]->htmlIds() as $htmlId) {
              $js .= 'document.getElementById('.json_encode($htmlId).').disabled = disableAttributes;'."\n";
            }
          }
        }
      }
    }
    return $js;
  }
}

/*! \brief This class allow to handle easily a Boolean LDAP attribute
 *
 */
class BooleanAttribute extends Attribute
{
  private $trueValue;
  private $falseValue;

  /*! \brief The constructor of BooleanAttribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param mixed $defaultValue The default value for this attribute
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   *  \param mixed $trueValue The value to store in LDAP when this boolean is TRUE. (For weird schemas that uses string or integer to store a boolean)
   *  \param mixed $falseValue The value to store in LDAP when this boolean is FALSE. (For weird schemas that uses string or integer to store a boolean)
   */
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = FALSE, $acl = "", $trueValue = "TRUE", $falseValue = "FALSE")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->trueValue  = $trueValue;
    $this->falseValue = $falseValue;
  }

  protected function loadAttrValue ($attrs)
  {
    if (isset($attrs[$this->getLdapName()])) {
      $this->setValue($attrs[$this->getLdapName()][0] == $this->trueValue);
    }
  }

  function loadPostValue ()
  {
    if ($this->isVisible()) {
      $this->setPostValue(isset($_POST[$this->getHtmlId()]));
    }
  }

  function computeLdapValue ()
  {
    return ($this->value?$this->trueValue:$this->falseValue);
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display  = '<input type="checkbox" name="'.$id.'" id="'.$id.'" '.
                ($this->disabled? 'disabled="disabled"':'').
                ($this->value?    'checked="checked"':'');
    if (!empty($this->managedAttributes)) {
      $js       = $this->managedAttributesJS();
      $display  .= 'onClick="javascript:'.htmlentities($js, ENT_COMPAT, 'UTF-8').'"';
    }
    $display .= '/>';
    return $this->renderAcl($display);
  }

  protected function managedAttributesJS ()
  {
    $js = '';
    $id = $this->getHtmlId();
    foreach ($this->managedAttributes as $prefix => $array) {
      foreach ($array as $value => $sections) {
        if (isset($this->managedAttributesMultipleValues[$value])) {
          trigger_error('Multiple values are not available for boolean attributes');
        } else {
          $js .= 'disableAttributes = (document.getElementById('.json_encode($id).').checked == '.($value?'true':'false').');'."\n";
        }
        foreach ($sections as $section => $attributes) {
          foreach ($attributes as $attribute) {
            foreach ($this->plugin->attributesInfo[$section]['attrs'][$attribute]->htmlIds() as $htmlId) {
              $js .= 'document.getElementById('.json_encode($htmlId).').disabled = disableAttributes;'."\n";
            }
          }
        }
      }
    }
    return $js;
  }
}

/*! \brief This class allow to handle easily a Boolean LDAP attribute that triggers a set of objectclasses
 *
 */
class ObjectClassBooleanAttribute extends BooleanAttribute
{
  private $objectclasses;


  /*! \brief The constructor of ObjectClassBooleanAttribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param mixed $objectclasses objectClass or array of objectClasses that this boolean should add/remove depending on its state
   *  \param mixed $defaultValue The default value for this attribute
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct ($label, $description, $ldapName, $required, $objectclasses, $defaultValue = FALSE, $acl = "")
  {
    if (is_array($objectclasses)) {
      $this->objectclasses = $objectclasses;
    } else {
      $this->objectclasses = array($objectclasses);
    }
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setInLdap(FALSE);
  }

  function loadValue ($attrs)
  {
    if (isset($attrs['objectClass'])) {
      $missing_oc = array_udiff($this->objectclasses, $attrs['objectClass'], 'strcasecmp');
      $this->setValue(empty($missing_oc));
    } else {
      $this->resetToDefault();
    }
    $this->initialValue = $this->value;
  }

  function fillLdapValue (&$attrs)
  {
    if ($this->getValue()) {
      $attrs['objectClass'] = array_merge_unique($this->objectclasses, $attrs['objectClass']);
    } else {
      $attrs['objectClass'] = array_remove_entries($this->objectclasses, $attrs['objectClass']);
    }
  }
}

/*! \brief This class allow to handle easily a String LDAP attribute
 *
 */
class StringAttribute extends Attribute
{
  private $pattern;
  protected $example;

  /*! \brief The constructor of StringAttribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param mixed $defaultValue The default value for this attribute
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   *  \param string $regexp A regular expression that should be matched by the value of this attribute in order for it to be considered valid. Will be used as a PHP regexp and as an html5 input pattern.
   */
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "", $regexp = "", $example = NULL)
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->pattern = $regexp;
    $this->example = ($example === NULL?$defaultValue:$example);
  }

  function setExample ($example)
  {
    $this->example = $example;
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display  = '<input type="text" name="'.$id.'" id="'.$id.'" value="{literal}'.htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8').'{/literal}" '.
                ($this->disabled? 'disabled="disabled"':'').'/>';
    return $this->renderAcl($display);
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    } else {
      if ($this->value !== "") {
        return $this->validate();
      }
    }
  }

  function validate ()
  {
    if (($this->pattern !== "") && !preg_match($this->pattern, $this->value)) {
      return msgPool::invalid($this->getLabel(), $this->value, $this->pattern, htmlentities($this->example));
    }
  }
}

/*! \brief This class allow to handle easily a String LDAP attribute that appears as a text area
 *
 */
class TextAreaAttribute extends StringAttribute
{
  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display  = '<textarea name="'.$id.'" id="'.$id.'"'.
                ($this->disabled? 'disabled="disabled"':'').'>'.
                '{literal}'.htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8').'{/literal}</textarea>';
    return $this->renderAcl($display);
  }
}

/*! \brief This class allow to handle easily a String LDAP attribute that contains a password
 *
 */
class PasswordAttribute extends StringAttribute
{
  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display  = '<input type="password" name="'.$id.'" id="'.$id.'" value="{literal}'.htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8').'{/literal}" '.
                ($this->disabled? 'disabled="disabled"':'').'/>';
    return $this->renderAcl($display);
  }
}

/*! \brief This class allow to handle easily a Select LDAP attribute with a set of choices
 *
 */
class SelectAttribute extends Attribute
{
  protected $choices;
  protected $outputs    = NULL;
  protected $submitForm = FALSE;

  /*! \brief The constructor of SelectAttribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param array $choices The choices this select should offer. Pass array("") if you're gonna fill it later with setChoices
   *  \param mixed $defaultValue The default value for this attribute
   *  \param array $outputs The label corresponding to the choices, leave to NULL if you want to display the choices themselves
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct ($label, $description, $ldapName, $required, $choices = array(), $defaultValue = "", $outputs = NULL, $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setChoices($choices, $outputs);
  }

  function setSubmitForm ($submitForm)
  {
    $this->submitForm = $submitForm;
  }

  function setChoices ($choices, $outputs = NULL)
  {
    $this->outputs = NULL;
    if (!$this->isRequired() && !in_array("", $choices)) {
      $choices[] = "";
      if (is_array($outputs)) {
        $outputs[] = _("None");
      }
    }
    $this->choices = $choices;
    if (!in_array($this->defaultValue, $this->choices) && isset($this->choices[0])) {
      $this->defaultValue = $this->choices[0];
    }
    if (is_array($outputs)) {
      $this->setDisplayChoices($outputs);
    }
    if (!in_array($this->value, $this->choices)) {
      $this->resetToDefault();
    }
  }

  function setDisplayChoices ($values)
  {
    $this->outputs = array();
    $i = 0;
    foreach ($this->choices as $choice) {
      $this->outputs[$choice] = $values[$i++];
    }
  }

  protected function setRequired ($bool)
  {
    parent::setRequired($bool);
    $key = array_search("", $this->choices);
    if ($this->isRequired() && ($key !== FALSE)) {
      unset($this->choices[$key]);
    } elseif (!$this->isRequired() && !in_array("", $this->choices)) {
      $this->choices[] = "";
      if (!isset($this->output[""])) {
        $this->output[""] = _("None");
      }
    }
  }

  function displayValue ($value)
  {
    if ($this->outputs !== NULL) {
      return $this->outputs[$value];
    } else {
      return $value;
    }
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    } else {
      if (!$this->disabled && !in_array($this->value, $this->choices)) {
        return msgPool::invalid($this->getLabel());
      }
    }
  }

  function renderFormInput ()
  {
    $smarty = get_smarty();
    $id = $this->getHtmlId();
    $smarty->assign($id."_choices", $this->choices);
    if ($this->outputs !== NULL) {
      $smarty->assign($id."_outputs", array_values($this->outputs));
    } else {
      $smarty->assign($id."_outputs", $this->choices);
    }
    $smarty->assign($id."_selected", $this->getValue());
    $display  = '<select name="'.$id.'" id="'.$id.'" ';
    if ($this->disabled || (count($this->choices) == 0)) {
      $display .= 'disabled="disabled" ';
    }
    if ($this->submitForm) {
      $js       = 'document.mainform.submit();';
      $display  .= 'onChange="javascript:'.htmlentities($js, ENT_COMPAT, 'UTF-8').'"';
    } elseif (!empty($this->managedAttributes)) {
      $js       = $this->managedAttributesJS();
      $display  .= 'onChange="javascript:'.htmlentities($js, ENT_COMPAT, 'UTF-8').'"';
    }
    $display .= '>';
    $display .= '{html_options values=$'.$id.'_choices output=$'.$id.'_outputs selected=$'.$id.'_selected}';
    $display .= '</select>';
    return $this->renderAcl($display);
  }
}

/*! \brief This class allow to handle easily an Integer LDAP attribute
 *
 */
class IntAttribute extends Attribute
{
  protected $min;
  protected $max;
  protected $step = 1;
  protected $example;

  /*! \brief The constructor of IntAttribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param int $min The minimum value it can take
   *  \param int $max The maximum value it can take
   *  \param mixed $defaultValue The default value for this attribute
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct ($label, $description, $ldapName, $required, $min, $max, $defaultValue = "", $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->min      = ($min === FALSE ? FALSE : $this->inputValue($min));
    $this->max      = ($max === FALSE ? FALSE : $this->inputValue($max));
    $this->example  = "";

    if (($min !== FALSE) && ($max !== FALSE)) {
      $this->example = sprintf(_("An integer between %d and %d"), $min, $max);
    } elseif ($min !== FALSE) {
      $this->example = sprintf(_("An integer larger than %d"),    $min);
    } elseif ($max !== FALSE) {
      $this->example = sprintf(_("An integer smaller than %d"),   $max);
    }
  }

  function setStep ($step)
  {
    $this->step = $step;
  }

  function inputValue ($value)
  {
    if (!$this->isRequired() && empty($value) && !is_numeric($value)) {
      // value is "" or array()
      return "";
    }
    return intval($value);
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    } else {
      if (!is_numeric($this->value) && (!empty($this->value) || $this->isRequired())) {
        return msgPool::invalid($this->getLabel(), $this->value, "/./", $this->example);
      }
      if ((($this->min !== FALSE) && ($this->value < $this->min))
      || (($this->max !== FALSE) && ($this->value > $this->max))) {
        return msgPool::invalid($this->getLabel(), $this->value, "/./", $this->example);
      }
    }
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display = '<input type="number" name="'.$id.'" id="'.$id.'" value="{literal}'.htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8').'{/literal}"'.
    ($this->min !== FALSE?' min="'.$this->min.'"':'').
    ($this->max !== FALSE?' max="'.$this->max.'"':'').
    ($this->disabled? 'disabled="disabled"':'').
    ($this->step !== FALSE?' step="'.$this->step.'"':'').
    '/>';
    return $this->renderAcl($display);
  }
}

/*! \brief This class allow to handle easily an Float LDAP attribute
 *
 */
class FloatAttribute extends IntAttribute
{

  /*! \brief The constructor of FloatAttribute
   *
   * By default a FloatAttribute will have a step of 0.1, use setStep in order to change it.
   * You can use setStep(FALSE) to disable it.
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param float $min The minimum value it can take
   *  \param float $max The maximum value it can take
   *  \param mixed $defaultValue The default value for this attribute
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct ($label, $description, $ldapName, $required, $min, $max, $defaultValue = 0.0, $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $min, $max, $defaultValue, $acl);

    $this->step = 0.01;

    $this->example  = "";
    if (($min !== FALSE) && ($max !== FALSE)) {
      $this->example = sprintf(_("A float between %f and %f"), $min, $max);
    } elseif ($min !== FALSE) {
      $this->example = sprintf(_("A float larger than %f"),    $min);
    } elseif ($max !== FALSE) {
      $this->example = sprintf(_("A float smaller than %f"),   $max);
    }
  }

  function inputValue ($value)
  {
    return floatval($value);
  }
}

/*! \brief This class allow to handle easily an Date LDAP attribute
 *
 */
class DateAttribute extends Attribute
{
  protected $format;

  /*! \brief The constructor of DateAttribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param string $format The date format. It can be any format recognized by DateTime::format. see http://www.php.net/manual/fr/function.date.php
   *  \param mixed $defaultValue The default value for this attribute
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct ($label, $description, $ldapName, $required, $format, $defaultValue = 'now', $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->format = $format;
  }

  function inputValue ($value)
  {
    if ($value === "" && !$this->isRequired()) {
      return $value;
    } elseif ($value instanceof DateTime) {
      return $value;
    } else {
      try {
        $date = DateTime::createFromFormat($this->format, $value);
        if ($date !== FALSE) {
          return $date;
        }
      } catch (Exception $e) {
      }
      return new DateTime($value);
    }
  }

  function getValue ()
  {
    return $this->inputValue($this->value);
  }

  function computeLdapValue ()
  {
    if ($this->value === "" && !$this->isRequired()) {
      return $this->value;
    } elseif (!($this->value instanceof DateTime)) {
      $this->setValue($this->inputValue($this->value));
    }
    return $this->value->format($this->format);
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    } else {
      if ($this->value instanceof DateTime) {
        return;
      } else {
        try {
          $this->inputValue($this->value);
        } catch (Exception $e) {
          return _("Error, incorrect date: ").$e->getMessage();
        }
      }
    }
  }

  private function getDisplayValue ()
  {
    if ($this->value === "" && !$this->isRequired()) {
      return $this->value;
    } else {
      return $this->getValue()->format('d.m.Y');
    }
  }

  function renderFormInput ()
  {
    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    $id = $this->getHtmlId();
    $display  = '<input type="text" class="date" name="'.$id.'" id="'.$id.'" value="{literal}'.$this->getDisplayValue().'{/literal}" '.
                ($this->disabled? 'disabled="disabled"':'').'/>';
    $display  .= '{if $'.$this->getAcl().'ACL|regex_replace:"/[cdmr]/":"" == "w"}'.
        '<script type="text/javascript">
          {literal}
          var datepicker  = new DatePicker({ relative : \''.$id.'\', language : \'{/literal}{$lang}{literal}\', keepFieldEmpty : true,
                                             enableCloseEffect : false, enableShowEffect : false });
          {/literal}
        </script>
        {/if}';
    return $this->renderAcl($display);
  }
}

/*! \brief This class allow to handle easily an File LDAP attribute
 *
 */
class FileAttribute extends Attribute
{
  function loadPostValue()
  {
    if (!empty($_FILES[$this->getHtmlId()]['name'])) {
      if ($_FILES[$this->getHtmlId()]['size'] <= 0) {
        msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file is empty")), ERROR_DIALOG);
      } elseif (!file_exists($_FILES[$this->getHtmlId()]['tmp_name'])) {
        // Is there a tmp file, which we can use ?
        msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file not found")), ERROR_DIALOG);
      } elseif (!$handle = @fopen($_FILES[$this->getHtmlId()]['tmp_name'], "r")) {
        // Can we open the tmp file, for reading
        msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file not readable")), ERROR_DIALOG);
      } else {
        // Everything just fine :)

        // Reading content
        $this->readFile($handle);
      }
      $_FILES[$this->getHtmlId()]['name'] = ""; // so that we only handle the file once
    }
  }

  /*! \brief This function read the file from the given handle and then closes it
   *
   *  \param filehandle $handle The handle on the opened uploaded file
   */
  function readFile($handle)
  {
    $this->postValue = fread($handle, 1024);
    while (!feof($handle)) {
      $postValue .= fread($handle, 1024);
    }
    @fclose($handle);
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display  = '<input type="file" name="'.$id.'" id="'.$id.'" '.
                ($this->disabled? 'disabled="disabled"':'').'/>';
    return $this->renderAcl($display);
  }
}

/*! \brief This class allow to handle easily an Base selector attribute
 *
 */
class BaseSelectorAttribute extends Attribute
{
  private $baseSelector = NULL;
  private $orig_dn      = NULL;
  private $ou           = NULL;

  /*! \brief The constructor of BaseSelectorAttribute
   *
   *  \param string $ou The ou your objects are into. It will be used in order to detect the base they are in.
   */
  function __construct ($ou)
  {
    parent::__construct(_('Base'), _('Object base'), 'base', FALSE, '');
    $this->setInLdap(FALSE);
    $this->ou = $ou;
  }

  function setManagedAttributes ($dontcare)
  {
    trigger_error('method setManagedAttributes is not supported for BaseSelectorAttribute');
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      $this->plugin->base =& $this->value;
      /* Do base conversation */
      if ($this->plugin->dn == "new") {
        $ui = get_userinfo();
        $this->setValue(dn2base(session::global_is_set("CurrentMainBase")?"cn=dummy,".session::global_get("CurrentMainBase"):$ui->dn));
      } else {
        $this->setValue(dn2base($this->plugin->dn, $this->ou));
        echo "Setting value to ".dn2base($this->plugin->dn, $this->ou)." for ".$this->plugin->dn." and ".$this->ou."<br/>\n";
      }
      $this->orig_dn = $this->plugin->dn;
      /* Instanciate base selector */
      $this->initialValue = $this->value;
      $this->baseSelector = new baseSelector($this->plugin->get_allowed_bases(), $this->value);
      $this->baseSelector->setSubmitButton(FALSE);
      $this->baseSelector->setHeight(300);
      $this->baseSelector->update(TRUE);
    }
  }

  function loadPostValue ()
  {
  }

  function applyPostValue ()
  {
    if (!$this->disabled) {
      /* Refresh base */
      if  ($this->plugin->acl_is_moveable($this->value) ||
          ($this->plugin->dn == "new" && $this->plugin->acl_is_createable($this->value))) {
        if (!$this->baseSelector->update()) {
          msg_dialog::display(_("Error"), msgPool::permMove(), ERROR_DIALOG);
        }
        if ($this->value != $this->baseSelector->getBase()) {
          $this->setValue($this->baseSelector->getBase());
          $this->plugin->is_modified = TRUE;
        }
      }
    }
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    } else {
      /* Check if we are allowed to create/move this user */
      if ($this->orig_dn == "new" && !$this->plugin->acl_is_createable($this->value)) {
        return msgPool::permCreate();
      } elseif ($this->orig_dn != "new" && $this->plugin->dn != $this->orig_dn && !$this->plugin->acl_is_moveable($this->value)) {
        return msgPool::permMove();
      }
      // Check if a wrong base was supplied
      if (!$this->baseSelector->checkLastBaseUpdate()) {
        return msgPool::check_base();
      }
    }
  }

  function setValue ($value)
  {
    parent::setValue($value);
    if (is_object($this->plugin)) {
      /* Set the new acl base */
      if ($this->plugin->dn == "new") {
        $this->plugin->set_acl_base($this->value);
      }
    }
  }

  function getValue ()
  {
    return $this->value;
  }

  function renderFormInput ()
  {
    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    return $this->renderAcl('{literal}'.$this->baseSelector->render().'{/literal}');
  }
}

/*! \brief This class allow to handle easily a multi-valuated attribute
 *
 */
class ArrayAttribute extends Attribute
{
  public $attribute;

  /*! \brief The constructor of ArrayAttribute
   *
   *  \param Attribute $attribute The attribute you want to see multi-valuated
   *  \param array $values The default values
   */
  function __construct ($attribute, $values = array())
  {
    parent::__construct(
      $attribute->getLabel(),     $attribute->getDescription(),
      $attribute->getLdapName(),  $attribute->isRequired(),
      $values
    );
    $this->attribute = $attribute;
    $this->attribute->setRequired(TRUE);
  }

  function setManagedAttributes ($dontcare)
  {
    trigger_error('method setManagedAttributes is not supported for ArrayAttributes');
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
    return $this->attribute->getAcl();
  }

  function setAcl ($acl)
  {
    $this->attribute->setAcl($acl);
  }

  function addPostValue ($value)
  {
    $this->postValue[] = $value;
  }

  function delPostValue ($key)
  {
    unset($this->postValue[$key]);
  }

  function loadPostValue ()
  {
    $this->postValue = $this->value;
    if (isset($_POST["add".$this->getHtmlId()])) {
      $this->attribute->loadPostValue();
      $this->attribute->applyPostValue();
      $this->addPostValue($this->attribute->getValue());
    } elseif (isset($_POST["del".$this->getHtmlId()]) && isset($_POST["row".$this->getHtmlId()])) {
      foreach ($_POST["row".$this->getHtmlId()] as $key) {
        $this->delPostValue($key);
      }
    }
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
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
    $id = $this->getHtmlId();
    $smarty = get_smarty();
    $smarty->assign($id.'_values', $this->getDisplayValues());
    $display = '<select multiple="multiple"  name="row'.$id.'[]" id="row'.$id.'" >'."\n";
    $display .= '{html_options options=$'.$id.'_values}';
    $display .= '</select><br/>'."\n";
    $attr_display = $this->renderAttributeInput();
    $buttons = $this->renderButtons();
    return $this->renderAcl($display).$attr_display.$this->renderAcl($buttons);
  }

  function getDisplayValues ()
  {
    $attribute = $this->attribute;
    return array_map(
      function ($value) use($attribute)
      {
        return $attribute->displayValue($value);
      },
      $this->value
    );
  }

  function renderAttributeInput ()
  {
    $this->attribute->resetToDefault();
    $this->attribute->setDisabled($this->disabled);
    return $this->attribute->renderFormInput();
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons  = '<input type="submit" name="add'.$id.'" id="add'.$id.'"'.
                ($this->disabled? ' disabled="disabled"':'').
                ' value="{msgPool type=addButton}"/>';
    $buttons .= '<input type="submit" name="del'.$id.'" id="del'.$id.'"'.
                ($this->disabled? ' disabled="disabled"':'').
                ' value="{msgPool type=delButton}"/>';
    return $buttons;
  }

  function computeLdapValue ()
  {
    return array_values($this->value);
  }

  protected function htmlIds()
  {
    $id = $this->getHtmlId();
    return array_merge(array('add'.$id,'del'.$id,'row'.$id), $this->attribute->htmlIds());
  }
}

/*! \brief This class allow to handle easily a multi-valuated attribute with no duplicate values
 *
 */
class SetAttribute extends ArrayAttribute
{
  /*! \brief The constructor of SetAttribute
   *
   *  \param Attribute $attribute The attribute you want to see multi-valuated
   *  \param array $values The default values
   */
  function __construct ($attribute, $values = array())
  {
    parent::__construct($attribute, $values);
  }

  function addPostValue ($value)
  {
    if (!in_array($value, $this->postValue)) {
      $this->postValue[] = $value;
      return TRUE;
    }
    return FALSE;
  }
}

/*! \brief This class allow to handle easily a composite attribute
 *
 * That means this is only one attribute in the LDAP, but it is shown as several in the form.
 * If you need something else than scanf and printf for reading and writing the values (for instance if you want to do a addition of several int attributes),
 * you should inherit this class and write your own readValues and writeValues method
 *
 */
class CompositeAttribute extends Attribute
{
  public $attributes;
  private $readFormat;
  private $writeFormat;
  private $linearRendering = FALSE;

  /*! \brief The constructor of CompositeAttribute
   *
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param array $attributes The attributes that are parts of this composite attribute
   *  \param string $readFormat the scanf format that's gonna be used in order to read values from LDAP
   *  \param string $writeFormat the printf format that's gonna be used in order to write values into LDAP
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   *  \param string $label The label to show for this attribute. Only useful if you put this attribute inside a Set or ArrayAttribute, or if you use a specific template that needs it.
   */
  function __construct ($description, $ldapName, $attributes, $readFormat, $writeFormat, $acl = "", $label = "Composite attribute")
  {
    parent::__construct($label, $description, $ldapName, FALSE, "", $acl);
    $this->attributes   = $attributes;
    $this->readFormat   = $readFormat;
    $this->writeFormat  = $writeFormat;
    foreach ($this->attributes as &$attribute) {
      $attribute->setAcl($this->getAcl());
    }
  }

  function setAcl ($acl)
  {
    parent::setAcl($acl);
    foreach ($this->attributes as &$attribute) {
      $attribute->setAcl($this->getAcl());
    }
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    foreach ($this->attributes as &$attribute) {
      $attribute->setParent($plugin);
    }
  }

  function setManagedAttributes ($dontcare)
  {
    trigger_error('method setManagedAttributes is not supported for CompositeAttribute');
  }

  function setLinearRendering ($bool)
  {
    $this->linearRendering = $bool;
  }

  function readValues($value)
  {
    return sscanf($value, $this->readFormat);
  }

  function writeValues($values)
  {
    return vsprintf($this->writeFormat, $values);
  }

  function resetToDefault ()
  {
    foreach ($this->attributes as &$attribute) {
      $attribute->resetToDefault();
    }
  }

  function inputValue ($value)
  {
    if (is_array($value)) {
      return $value;
    } else {
      return $this->readValues($value);
    }
  }

  function loadPostValue ()
  {
    foreach ($this->attributes as &$attribute) {
      $attribute->loadPostValue();
    }
  }

  function applyPostValue ()
  {
    foreach ($this->attributes as &$attribute) {
      $attribute->applyPostValue();
    }
  }

  function setValue ($values)
  {
    $values = $this->inputValue($values);
    $i = 0;
    foreach ($this->attributes as &$attribute) {
      $attribute->setValue($values[$i++]);
    }
  }

  /* We always return the LDAP value as the composite attribute has nothing else */
  function getValue ()
  {
    $values = array_map(
      function ($a)
      {
        return $a->computeLdapValue();
      },
      $this->attributes
    );
    return $this->writeValues($values);
  }

  function getArrayValue ()
  {
    $values = array_map(
      function ($a)
      {
        return $a->getValue();
      },
      $this->attributes
    );
    return $values;
  }

  function check ()
  {
    foreach ($this->attributes as $attribute) {
      $error = $attribute->check();
      if (!empty($error)) {
        return $error;
      }
    }
  }

  function renderAttribute(&$attributes)
  {
    if ($this->visible) {
      if ($this->linearRendering) {
        parent::renderAttribute($attributes);
      } else {
        foreach ($this->attributes as $attribute) {
          $attribute->setDisabled($this->disabled);
          $attribute->renderAttribute($attributes);
        }
      }
    }
  }

  function renderFormInput()
  {
    $display = "";
    foreach ($this->attributes as $attribute) {
      $attribute->setDisabled($this->disabled);
      $display .= '<label for="'.$attribute->getHtmlId().'">'.$attribute->getLabel().'</label>'." ".$attribute->renderFormInput()." ";
    }
    return $display;
  }

  protected function htmlIds()
  {
    $ret = array();
    foreach ($this->attributes as $attribute) {
      $ret = array_merge($ret, $attribute->htmlIds());
    }
    return $ret;
  }
}

class OrderedArrayAttribute extends SetAttribute
{
  protected $order;

  /*! \brief The constructor of OrderedArrayAttribute
   *
   *  \param Attribute $attribute The composite attribute you want to see multi-valuated
   *  \param array $values The default values
   */
  function __construct ($attribute, $order = TRUE, $values = array())
  {
    parent::__construct($attribute);
    $this->order = $order;
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

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $div = new divSelectBox($id);
    $smarty = get_smarty();
    $div->SetHeight(90);
    foreach ($this->value as $key => $value) {
      $img = '';

      if ($this->order) {
        if ($key != 0) {
          $img .= '<input type="image" src="images/lists/sort-up.png" name="'.$id.'_up_'.$key.'" class="center"/>&nbsp;';
        } else {
          $img .= '<img src="images/empty.png" style="width:10px;"/>';
        }
        if (($key + 1) < count($this->value)) {
          $img .= '<input type="image" src="images/lists/sort-down.png" name="'.$id.'_down_'.$key.'" class="center"/>&nbsp;';
        } else {
          $img .= '<img src="images/empty.png" style="width:10px;"/>';
        }
      }
      $img .= '<input type="image" src="images/lists/trash.png" name="'.$id.'_del_'.$key.'" class="center">&nbsp;';

      $fields = array();
      $this->attribute->setValue($value);
      foreach ($this->attribute->getArrayValue() as $field) {
        $fields[] = array("string" => $field);
      }
      $fields[] = array("string" => $img, "attach" => "style='border-right:0px;width:40px;'");
      $div->AddEntry($fields);
    }
    $smarty->assign("div_$id", $div->DrawList());
    $display = '{$div_'.$id.'}'."\n";
    $attr_display = $this->renderAttributeInput();
    $buttons = $this->renderButtons();
    return $this->renderAcl($display).$attr_display.$this->renderAcl($buttons);
  }

  protected function reIndexValues ()
  {
    $this->value = array_values($this->value);
  }

  function loadPostValue ()
  {
    $this->postValue = $this->value;
    $id = $this->getHtmlId();
    foreach ($_POST as $name => $value) {
      if ($this->order) {
        if (preg_match('/^'.$id.'_up_/', $name)) {
          $key = preg_replace('/^'.$id.'_up_/', '', $name);
          $key = preg_replace('/_[xy]$/', '', $key);

          $tmp                        = $this->postValue[$key];
          $this->postValue[$key]      = $this->postValue[$key - 1];
          $this->postValue[$key - 1]  = $tmp;
          break;
        }
        if (preg_match('/^'.$id.'_down_/', $name)) {
          $key = preg_replace('/^'.$id.'_down_/', '', $name);
          $key = preg_replace('/_[xy]$/', '', $key);

          $tmp                        = $this->postValue[$key];
          $this->postValue[$key]      = $this->postValue[$key + 1];
          $this->postValue[$key + 1]  = $tmp;
          break;
        }
      }
      if (preg_match('/^'.$id.'_del_/', $name)) {
        $key = preg_replace('/^'.$id.'_del_/', '', $name);
        $key = preg_replace('/_[xy]$/', '', $key);
        $this->delPostValue($key);
        break;
      }
    }
    if (isset($_POST["add$id"])) {
      $this->attribute->loadPostValue();
      $this->attribute->applyPostValue();
      $this->addPostValue($this->attribute->getValue());
    }
  }

  function applyPostValue ()
  {
    parent::applyPostValue();
    if ($this->order) {
      $this->reIndexValues();
    }
  }

  protected function htmlIds()
  {
    $id = $this->getHtmlId();
    $ids = array_merge(array('add'.$id), $this->attribute->htmlIds());
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
    $buttons  = '<input type="submit" name="add'.$id.'" id="add'.$id.'"'.
                ($this->disabled? ' disabled="disabled"':'').
                ' value="{msgPool type=addButton}"/>';
    return $buttons;
  }
}

?>
