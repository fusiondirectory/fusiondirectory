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
  /* \brief Should this attribute be unique
   * FALSE  -> no unicity check
   * one    -> unicity check in the same base -> broken right now because of object ous
   * sub    -> unicity check in the same subtree
   *  \__> this should not be used as it’s breaking reciprocity
   * whole  -> unicity check in the whole LDAP
   */
  private $unique = FALSE;

  /* \brief Prefix for the html id */
  protected $htmlid_prefix = '';
  /* \brief Should this attribute be shown */
  protected $visible = TRUE;
  /* \brief Name of the ACL to use, empty if we need our own */
  protected $acl;
  /* \brief Is this attribute disabled */
  protected $disabled = FALSE;
  /* \brief Should this attribute submit formular when changing value
   * If this is not a boolean it is a string containing a method name to call on the plugin when changing value */
  protected $submitForm = FALSE;
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
  /* \brief Array of attributes to manage (prefix => value => attribute)
   * Prefix should be 'erase' or 'disable' */
  protected $managedAttributes = array();
  /* \brief Array of multiple values groups for managed attributes */
  protected $managedAttributesMultipleValues = array();

  /* \bried Array of booleans telling for each managing attributes if he's disabling us */
  protected $managingAttributesOrders = array();

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
    if ($unique === TRUE) {
      $this->unique = 'one';
    } else {
      $this->unique = $unique;
    }
  }

  function getUnique ()
  {
    return $this->unique;
  }

  function isInLdap ()
  {
    return $this->inLdap;
  }

  function setValue ($value)
  {
    $old_value    = $this->value;
    $this->value  = $value;
    if (($this->submitForm != FALSE) && ($this->submitForm !== TRUE) && ($old_value != $value) && is_object($this->plugin)) {
      $func = $this->submitForm;
      $this->plugin->$func();
    }
    $this->manageAttributes($this->value);
  }

  function setPostValue ($value)
  {
    if ($this->isVisible()) {
      $this->postValue = $value;
      $this->manageAttributes($this->postValue);
    }
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

  /* Return the value as an array of values to be displayed in a table columns */
  function getArrayValue ()
  {
    return array($this->displayValue($this->getValue()));
  }

  function getLdapName ()
  {
    return $this->ldapName;
  }

  function getHtmlId ()
  {
    return $this->htmlid_prefix.preg_replace('/[\/\-,.#:;]/', '_', $this->getLdapName());
  }

  /* html id to put in the "for" attribute of our "label" tag */
  function getForHtmlId ()
  {
    return $this->getHtmlId();
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

  function setRequired ($bool)
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

  function isDisabled ()
  {
    return $this->disabled;
  }

  function setManagingDisabled ($sender, $value)
  {
    $this->managingAttributesOrders[$sender] = $value;
    $this->setDisabled(array_reduce($this->managingAttributesOrders,
      function ($a, $b)
      {
        return $a || $b;
      }
    ));
  }

  function setSubmitForm ($submitForm)
  {
    $this->submitForm = $submitForm;
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

  function setInitialValue ($value)
  {
    $this->initialValue = $value;
  }

  function hasChanged ()
  {
    return ($this->getValue() !== $this->initialValue);
  }

  function displayValue ($value)
  {
    return $value;
  }

  /*! \brief Return the ldap value in the correct intern format value
   *
   *  \param $ldapValue The value as found in the LDAP
   */
  function inputValue ($ldapValue)
  {
    return $ldapValue;
  }

  function setDefaultValue ($value)
  {
    $this->defaultValue = $value;
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
    foreach ($this->managedAttributes as $array) {
      foreach ($array as $value => $attributes) {
        foreach ($attributes as $attribute) {
          $disabled = $this->isValueManagingValue($myvalue, $value);
          $this->plugin->attributesAccess[$attribute]->setManagingDisabled($this->getLdapName(), $disabled);
        }
      }
    }
    return TRUE;
  }

  /*! \brief Update this attributes postValue depending of the $_POST values
   */
  function loadPostValue ()
  {
    if ($this->isVisible()) {
      $this->postValue = $this->value;
      if (isset($_POST[$this->getHtmlId()])) {
        $this->setPostValue($_POST[$this->getHtmlId()]);
      }
    }
  }

  /*! \brief Apply this attribute postValue in value if this attribute is enabled
   */
  function applyPostValue ()
  {
    if (!$this->disabled && $this->isVisible()) {
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
      if ($value !== '') {
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
      foreach ($array as $value => $attributes) {
        $myvalue = $this->getValue();
        $erase = $this->isValueManagingValue($myvalue, $value);
        if (!$erase) {
          continue;
        }
        foreach ($attributes as $attribute) {
          $attrs[$attribute] = array();
        }
      }
    }
  }

  /*! \brief Check the correctness of this attribute
   */
  function check ()
  {
    global $config;
    $value = $this->getValue();
    if ($this->isRequired() && !$this->disabled && (($value === "") || ($value === array()))) {
      return msgPool::required($this->getLabel());
    } elseif (($this->unique !== FALSE) && !$this->disabled) {
      $value = $this->computeLdapValue();
      if (($value === "") || ($value === array())) {
        return;
      }
      $ldap = $config->get_ldap_link();
      $base = $config->current['BASE'];
      if ($this->unique !== 'whole') {
        if (isset($this->plugin->base) && !empty($this->plugin->base)) {
          $base = $this->plugin->base;
        } elseif (isset($this->plugin->dn) && !empty($this->plugin->dn) && ($this->plugin->dn != 'new')) {
          $base = dn2base($this->plugin->dn);
        }
      }
      $ldap->cd($base);
      if (is_array($value)) {
        $filter = '(|('.$this->getLdapName().'='.join(')('.$this->getLdapName().'=', array_map('ldap_escape_f', $value)).'))';
      } else {
        $filter = '('.$this->getLdapName().'='.ldap_escape_f($value).')';
      }
      $infos = pluglist::pluginInfos(get_class($this->plugin));
      $filters = array_map(
        function ($key, $ot)
        {
          if (!is_numeric($key)) {
            $ot = $key;
          }
          try {
            $oinfos = objects::infos($ot);
            return $oinfos['filter'];
          } catch (NonExistingObjectTypeException $e) {
            return '';
          }
        },
        array_keys($infos['plObjectType']),
        array_values($infos['plObjectType'])
      );
      $pluginFilter = $this->plugin->getObjectClassFilter();
      if (!empty($pluginFilter)) {
        $filters[] = $pluginFilter;
      }
      $filter = '(&'.$filter.implode($filters).')';
      $ldap->search($filter, array($this->getLdapName()));
      $branches = array_filter(
        array_map(
          function ($key, $ot)
          {
            if (!is_numeric($key)) {
              $ot = $key;
            }
            try {
              $oinfos = objects::infos($ot);
              return $oinfos['ou'];
            } catch (NonExistingObjectTypeException $e) {
              return FALSE;
            }
          },
          array_keys($infos['plObjectType']),
          array_values($infos['plObjectType'])
        )
      );
      while ($attrs = $ldap->fetch()) {
        if ($attrs['dn'] != $this->plugin->dn) {
          $dn_base  = preg_replace('/^[^,]+,/', '', $attrs['dn']);
          $found    = FALSE;
          if ($this->unique === 'one') {
            /* Check that this entry is in a concerned branch */
            foreach ($branches as $branch) {
              if ($branch.$base == $dn_base) {
                $dn_base  = preg_replace('/^'.preg_quote($branch, '/').'/', '', $dn_base);
                $found    = TRUE;
                break;
              }
            }
          } else { /* whole (or sub) */
            /* Check that this entry is in a concerned branch */
            foreach ($branches as $branch) {
              if (preg_match('/^'.preg_quote($branch, '/').'/', $dn_base)) {
                $dn_base  = preg_replace('/^'.preg_quote($branch, '/').'/', '', $dn_base);
                $found    = TRUE;
                break;
              }
            }
            if (!in_array($dn_base, $config->departments)) {
              continue;
            }
          }
          if (!$found) {
            continue;
          }

          return msgPool::duplicated($this->getLabel(), $attrs['dn']);
        }
      }
    }
  }

  /*! \brief Render this attribute form input(s)
   *
   *  \param array& attributes the attributes array
   *
   *  \param bool readOnly should we show text or input
   */
  function renderAttribute(&$attributes, $readOnly)
  {
    if ($this->visible) {
      if ($readOnly) {
        $input = '{literal}'.htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8').'{/literal}';
      } elseif (is_object($this->plugin) && $this->plugin->is_template) {
        $input = $this->renderTemplateInput();
      } else {
        $input = $this->renderFormInput();
      }
      $attributes[$this->getLdapName()] = array(
        'htmlid'      => $this->getForHtmlId(),
        'label'       => '{literal}'.$this->getLabel().'{/literal}'.($this->isRequired()?'{$must}':''),
        'description' => ($this->isRequired()?sprintf(_("%s (required)"), $this->getDescription()):$this->getDescription()),
        'input'       => $input,
      );
    }
  }

  /*! \brief Serialize this attribute for RPC requests
   *
   *  \param array& attributes the attributes array
   */
  function serializeAttribute(&$attributes, $form = TRUE)
  {
    if (!$form || $this->visible) {
      $class = get_class($this);
      while ($class != 'Attribute') {
        $type[] = $class;
        $class  = get_parent_class($class);
      }
      $type[] = 'Attribute'; // To avoid empty array
      $infos = array(
        'htmlid'      => $this->getHtmlId(),
        'label'       => $this->getLabel(),
        'required'    => $this->isRequired(),
        'disabled'    => $this->disabled,
        'description' => $this->getDescription(),
        'value'       => $this->getValue(),
        'default'     => $this->defaultValue,
        'type'        => $type,
      );
      if (!$form) {
        $infos['inldap']   = $this->isInLdap();
        $infos['visible']  = $this->visible;
        $infos['htmlids']  = $this->htmlIds();
      }
      $attributes[$this->getLdapName()] = $infos;
    }
  }

  /*! \brief Apply value from RPC requests
   *
   *  \param array values the values array
   */
  function deserializeValue($values)
  {
    if (isset($values[$this->getLdapName()])) {
      $this->setValue($values[$this->getLdapName()]);
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
    foreach ($this->managedAttributes as $array) {
      foreach ($array as $value => $attributes) {
        if (isset($this->managedAttributesMultipleValues[$value])) {
          $js .= 'disableAttributes = inArray(document.getElementById('.json_encode($id).').value,'.json_encode($this->managedAttributesMultipleValues[$value]).');';
        } else {
          $js .= 'disableAttributes = (document.getElementById('.json_encode($id).').value == '.json_encode($value).');'."\n";
        }
        foreach ($attributes as $attribute) {
          foreach ($this->plugin->attributesAccess[$attribute]->htmlIds() as $htmlId) {
            $js .= 'if (document.getElementById('.json_encode($htmlId).')) { document.getElementById('.json_encode($htmlId).').disabled = disableAttributes; }'."\n";
          }
        }
      }
    }
    return $js;
  }

  function renderTemplateInput ()
  {
    return $this->renderFormInput();
  }

  function foreignKeyUpdate($oldvalue, $newvalue, $source)
  {
    if ($source['MODE'] == 'move') {
      if ($this->getValue() == $oldvalue) {
        $this->setValue($newvalue);
      }
    }
  }

  function foreignKeyCheck($value, $source)
  {
    return ($this->getValue() == $value);
  }

  protected function renderInputField($type, $name, $attributes = array())
  {
    $input  = '<input type="'.$type.'" '.
              'name="'.$name.'" id="'.$name.'"'.
              ($this->disabled? ' disabled="disabled"':'');
    foreach ($attributes as $label => $value) {
      $input .= ' '.$label.'="'.$value.'"';
    }
    $input .= '/>';
    return $input;
  }
}

/*! \brief This class allow to handle easily a Boolean LDAP attribute
 *
 */
class BooleanAttribute extends Attribute
{
  public $trueValue;
  public $falseValue;

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

  function inputValue ($value)
  {
    return ($value == $this->trueValue);
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
    $attributes = ($this->value?array('checked' => 'checked'): array());
    if ($this->submitForm) {
      $js       = 'document.mainform.submit();';
      $attributes['onChange'] = 'javascript:'.htmlentities($js, ENT_COMPAT, 'UTF-8');
    } elseif (!empty($this->managedAttributes)) {
      $js       = $this->managedAttributesJS();
      $attributes['onChange'] = 'javascript:'.htmlentities($js, ENT_COMPAT, 'UTF-8');
    }
    $display  = $this->renderInputField('checkbox', $id, $attributes);
    return $this->renderAcl($display);
  }

  protected function managedAttributesJS ()
  {
    $js = '';
    $id = $this->getHtmlId();
    foreach ($this->managedAttributes as $array) {
      foreach ($array as $value => $attributes) {
        if (isset($this->managedAttributesMultipleValues[$value])) {
          trigger_error('Multiple values are not available for boolean attributes');
        } else {
          $js .= 'disableAttributes = (document.getElementById('.json_encode($id).').checked == '.($value?'true':'false').');'."\n";
        }
        foreach ($attributes as $attribute) {
          foreach ($this->plugin->attributesAccess[$attribute]->htmlIds() as $htmlId) {
            $js .= 'if (document.getElementById('.json_encode($htmlId).')) { document.getElementById('.json_encode($htmlId).').disabled = disableAttributes; }'."\n";
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
  protected $pattern;
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

  function setPattern ($pattern)
  {
    $this->pattern = $pattern;
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $attributes = array(
      'value' => '{literal}'.htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8').'{/literal}'
    );
    if (!empty($this->managedAttributes)) {
      $js       = $this->managedAttributesJS();
      $attributes['onChange'] = 'javascript:'.htmlentities($js, ENT_COMPAT, 'UTF-8');
    }
    $display  = $this->renderInputField('text', $id, $attributes);
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

class HiddenAttribute extends Attribute
{
  /*! \brief The constructor of HiddenAttribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param mixed $defaultValue The default value for this attribute
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct ($ldapName, $required = FALSE, $defaultValue = "", $acl = "", $label = NULL, $description = "hidden")
  {
    if ($label === NULL) {
      $label = $ldapName;
    }
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setVisible(FALSE);
  }
}

class HiddenArrayAttribute extends HiddenAttribute
{
  protected function loadAttrValue ($attrs)
  {
    if (isset($attrs[$this->getLdapName()]['count'])) {
      $this->value = array();
      for ($i = 0; $i < $attrs[$this->getLdapName()]['count']; $i++) {
        $this->value[] = $attrs[$this->getLdapName()][$i];
      }
    } else {
      $this->resetToDefault();
    }
  }
}

/* Dummy attribute class in order to give stats information to the template */
class FakeAttribute extends Attribute
{
  function __construct ($ldapName)
  {
    parent::__construct("Fake one", "", $ldapName, FALSE, "", "noacl");
    $this->setInLdap(FALSE);
  }

  function renderAttribute(&$attributes, $readOnly)
  {
    $attributes[$this->getLdapName()] = $this->getValue();
  }
}

/*! \brief This class allow to display an attribute.
 *
 * It can be used to display an attribute value the user is never allowed to modify.
 * (But FD might edit it)
 */
class DisplayLDAPAttribute extends Attribute
{
  protected $allowHTML = FALSE;

  function renderFormInput ()
  {
    if ($this->allowHTML) {
      return $this->getValue();
    } else {
      return htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8');
    }
  }

  function setAllowHTML ($allowHTML)
  {
    $this->allowHTML = $allowHTML;
  }
}

/*! \brief This class allow to display an attribute.
 *
 * It can be used to display an attribute value the user and FD are never allowed to modify.
 */
class ReadOnlyLDAPAttribute extends DisplayLDAPAttribute
{
  function fillLdapValue (&$attrs)
  {
  }
}

/*! \brief This class allow to display an attribute.
 *
 * It can be used to display an attribute value the user is never allowed to modify.
 */
class DisplayLDAPArrayAttribute extends Attribute
{
  protected function loadAttrValue ($attrs)
  {
    if (isset($attrs[$this->getLdapName()]['count'])) {
      $this->value = array();
      for ($i = 0; $i < $attrs[$this->getLdapName()]['count']; $i++) {
        $this->value[] = $attrs[$this->getLdapName()][$i];
      }
    } else {
      $this->resetToDefault();
    }
  }

  function renderFormInput ()
  {
    $value = $this->getValue();
    if (is_array($value)) {
      $value = join(', ', $value);
    }
    return htmlentities($value, ENT_COMPAT, 'UTF-8');
  }
}

/*! \brief This class allow to display a text in front of an attribute.
 *
 * For instance, it can be used to display a link.
 */
class DisplayAttribute extends DisplayLDAPAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "")
  {
    parent::__construct ($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setInLdap(FALSE);
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
  protected $autocomplete = FALSE;

  function setAutocomplete ($bool)
  {
    $this->autocomplete = $bool;
  }

  function getAutocomplete ()
  {
    return $this->autocomplete;
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display  = $this->renderInputField(
      'password', $id,
      array(
        'value' => '{literal}'.htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8').'{/literal}',
        'autocomplete' => ($this->autocomplete?'on':'off'),
      )
    );
    return $this->renderAcl($display);
  }

  function renderTemplateInput ()
  {
    return parent::renderFormInput();
  }
}

/*! \brief This class allow to handle easily a Select LDAP attribute with a set of choices
 *
 */
class SelectAttribute extends Attribute
{
  protected $choices;
  protected $outputs  = NULL;
  protected $size     = 1;

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
  function __construct ($label, $description, $ldapName, $required = FALSE, $choices = array(), $defaultValue = "", $outputs = NULL, $acl = "")
  {
    if (!in_array($defaultValue, $choices, TRUE) && isset($choices[0])) {
      $defaultValue = $choices[0];
    }
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setChoices($choices, $outputs);
  }

  function setChoices ($choices, $outputs = NULL)
  {
    $this->outputs = NULL;
    if (!$this->isRequired() && !in_array("", $choices, TRUE)) {
      array_unshift($choices, "");
      if (is_array($outputs)) {
        array_unshift($outputs, _("None"));
      }
    }
    $this->choices = $choices;
    if (!in_array($this->defaultValue, $this->choices, TRUE) && isset($this->choices[0])) {
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

  function setRequired ($bool)
  {
    parent::setRequired($bool);
    $key = array_search("", $this->choices, TRUE);
    if ($this->isRequired() && ($key !== FALSE)) {
      unset($this->choices[$key]);
      unset($this->outputs[""]);
    } elseif (!$this->isRequired() && !in_array("", $this->choices, TRUE)) {
      $this->choices[] = "";
      if (!isset($this->output[""])) {
        $this->output[""] = _("None");
      }
    }
  }

  function displayValue ($value)
  {
    if ($this->outputs !== NULL) {
      if (isset($this->outputs[$value])) {
        return $this->outputs[$value];
      } else {
        trigger_error("No display value set for '$value' in ".$this->getLabel());
        return $value;
      }
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
        return sprintf(_('The value "%s" for field "%s" is not in the list of possible choices'), $this->value, $this->getLabel());
      }
    }
  }

  function renderFormInput ()
  {
    $smarty = get_smarty();
    $id = $this->getHtmlId();
    $smarty->assign($id."_choices", $this->choices);
    if ($this->outputs !== NULL) {
      $outputs = array_values($this->outputs);
    } else {
      $outputs = $this->choices;
    }
    $key = array_search('', $outputs, TRUE);
    if ($key !== FALSE) {
      $outputs[$key] = '&nbsp;';
    }
    $smarty->assign($id."_outputs", $outputs);
    $smarty->assign($id."_selected", $this->getValue());
    $display  = '<select name="'.$id.'" id="'.$id.'" ';
    if ($this->disabled || (count($this->choices) == 0)) {
      $display .= 'disabled="disabled" ';
    }
    if ($this->size > 1) {
      $display .= 'size="'.$this->size.'" ';
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

  function getChoices ()
  {
    return $this->choices;
  }

  function serializeAttribute(&$attributes, $form = TRUE)
  {
    if (!$form || $this->visible) {
      parent::serializeAttribute($attributes, $form);

      if ($this->outputs !== NULL) {
        $outputs = array_values($this->outputs);
      } else {
        $outputs = $this->choices;
      }
      $attributes[$this->getLdapName()]['choices'] = array_combine($this->choices, $outputs);
    }
  }

  function setSize($size)
  {
    $this->size = $size;
  }
}

/*! \brief This class allows to handle a select attribute which allow to choose an object
 *
 */
class ObjectSelectAttribute extends SelectAttribute
{
  protected $objectType;
  protected $objectAttrs  = NULL;
  protected $objectFilter = '';

  function __construct ($label, $description, $ldapName, $required, $objectType, $objectAttrs = NULL, $objectFilter = '', $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, array(), "", NULL, $acl);
    $this->objectType   = $objectType;
    $this->objectAttrs  = $objectAttrs;
    $this->objectFilter = $objectFilter;
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      $this->updateChoices();
    }
  }

  function updateChoices()
  {
    $objects = objects::ls($this->objectType, $this->objectAttrs, NULL, $this->objectFilter);
    $this->setChoices(array_keys($objects), array_values($objects));
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
    if (is_object($this->plugin) && $this->plugin->is_template) {
      return $value;
    } else {
      return intval($value);
    }
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
    $attributes = array(
      'value' => '{literal}'.htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8').'{/literal}'
    );
    if ($this->min !== FALSE) {
      $attributes['min'] = $this->min;
    }
    if ($this->max !== FALSE) {
      $attributes['max'] = $this->max;
    }
    if ($this->step !== FALSE) {
      $attributes['step'] = $this->step;
    }
    if (!empty($this->managedAttributes)) {
      $js       = $this->managedAttributesJS();
      $attributes['onChange'] = 'javascript:'.htmlentities($js, ENT_COMPAT, 'UTF-8');
    }
    $display = $this->renderInputField('number', $id, $attributes);
    return $this->renderAcl($display);
  }

  function renderTemplateInput ()
  {
    $id = $this->getHtmlId();
    $display = $this->renderInputField(
      'text', $id,
      array(
        'value' => '{literal}'.htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8').'{/literal}'
      )
    );
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
    if (!$this->isRequired() && empty($value) && !is_numeric($value)) {
      // value is "" or array()
      return "";
    }
    if (is_object($this->plugin) && $this->plugin->is_template) {
      return $value;
    } else {
      return floatval($value);
    }
  }
}

/*! \brief This class allow to handle easily an Date LDAP attribute
 *
 * We are using UTC timezone because we don't care about time, we just want date.
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
    } elseif (is_object($this->plugin) && $this->plugin->is_template && (preg_match('/%/', $value))) {
      return $value;
    } else {
      return $this->ldapToDate($value);
    }
  }

  function getValue ()
  {
    if ($this->value === "" && !$this->isRequired()) {
      return $this->value;
    } else {
      try {
        return $this->getDateValue()->format('d.m.Y');
      } catch (Exception $e) {
        return "";
      }
    }
  }

  protected function ldapToDate($ldapValue)
  {
    $date = DateTime::createFromFormat($this->format, $ldapValue, new DateTimeZone('UTC'));
    if ($date !== FALSE) {
      return $date;
    } else {
      trigger_error('LDAP value for '.$this->getLdapName().' was not in the right date format.');
      return new DateTime($ldapValue, new DateTimeZone('UTC'));
    }
  }

  protected function dateToLdap($dateValue)
  {
    return $dateValue->format($this->format);
  }

  function getDateValue()
  {
    $value = $this->value;
    if (!($value instanceof DateTime)) {
      $value = new DateTime($value, new DateTimeZone('UTC'));
    }
    return $value;
  }

  function computeLdapValue ()
  {
    if ($this->value === "" && !$this->isRequired()) {
      return $this->value;
    } elseif (!($this->value instanceof DateTime)) {
      $this->setValue($this->getDateValue());
    }
    return $this->dateToLdap($this->value);
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
          $this->getDateValue();
        } catch (Exception $e) {
          return _("Error, incorrect date: ").$e->getMessage();
        }
      }
    }
  }

  function renderFormInput ()
  {
    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    $id = $this->getHtmlId();
    $display = $this->renderInputField(
      'text', $id,
      array(
        'value' => '{literal}'.$this->getValue().'{/literal}',
        'class' => 'date'
      )
    );
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

class GeneralizedTimeDateAttribute extends DateAttribute
{
  function __construct ($label, $description, $ldapName, $required, $defaultValue = 'now', $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, '', $defaultValue, $acl);
  }

  protected function ldapToDate($ldapValue)
  {
    try {
      return LdapGeneralizedTime::fromString($ldapValue);
    } catch (LdapGeneralizedTimeBadFormatException $e) {
      trigger_error('LDAP value "'.$ldapValue.'" for '.$this->getLdapName().' is not in the right date format.');
      return new DateTime($ldapValue, timezone::utc());
    }
  }

  protected function dateToLdap($dateValue)
  {
    return LdapGeneralizedTime::toString($dateValue);
  }
}

/*! \brief This class allow to handle easily an File LDAP attribute
 *
 */
class FileAttribute extends Attribute
{
  function loadPostValue()
  {
    $this->postValue = $this->value;
    if (!empty($_FILES[$this->getHtmlId()]['name']) && $this->isVisible()) {
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
      $this->postValue .= fread($handle, 1024);
    }
    @fclose($handle);
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display = $this->renderInputField('file', $id);
    return $this->renderAcl($display);
  }

  function displayValue($value)
  {
    return sprintf(_('%s (%d bytes)'), $this->getLabel(), mb_strlen($value, '8bit'));
  }
}

/* FileAttribue with download button */
class FileDownloadAttribute extends FileAttribute
{
  protected $extension;
  protected $upload;

  function __construct ($label, $description, $ldapName, $required = FALSE, $extension = '', $upload = FALSE, $defaultValue = "", $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->extension  = $extension;
    $this->upload     = $upload;
  }

  function computeFilename()
  {
    return $this->getLdapName().$this->extension;
  }

  function loadPostValue ()
  {
    if ($this->isVisible()) {
      $this->postValue = $this->value;
      foreach (array_keys($_POST) as $name) {
        if (preg_match('/^download'.$this->getHtmlId().'/', $name)) {
          session::set('binary', $this->value);
          session::set('binarytype', 'octet-stream');
          session::set('binaryfile', $this->computeFilename());
          header('location: getbin.php');
          exit();
        }
      }
      if ($this->upload && isset($_POST['upload'.$this->getHtmlId()])) {
        parent::loadPostValue();
      }
    }
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display = '';
    if ($this->upload) {
      $display  .= $this->renderInputField('file', $id);
      $display  .= $this->renderInputField('submit', 'upload'.$id, array('value' => _('Upload')));
    }
    $display  .= $this->renderInputField(
      'image', 'download'.$id,
      array(
        'title' => _('Download'),
        'alt'   => _('Download'),
        'class' => 'center',
        'src'   => 'geticon.php?context=actions&amp;icon=document-save&amp;size=16',
      )
    );
    return $this->renderAcl($display);
  }

  protected function htmlIds()
  {
    $id   = $this->getHtmlId();
    $ids  = array('download'.$id);
    if ($this->upload) {
      $ids[] =  $id;
      $ids[] =  'upload'.$id;
    }
    return $ids;
  }

  function renderAttribute(&$attributes, $readOnly)
  {
    if ($this->upload == FALSE) {
      parent::renderAttribute($attributes, FALSE);
    } else {
      parent::renderAttribute($attributes, $readOnly);
    }
  }
}

class FileTextAreaAttribute extends FileDownloadAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $extension = '.txt', $upload = TRUE, $defaultValue = "", $acl = "")
  {
    parent::__construct(
      $label, $description, $ldapName, $required,
      $extension, $upload, $defaultValue, $acl
    );
  }
  /*! \brief Update this attributes postValue depending of the $_POST values
   */
  function loadPostValue ()
  {
    if ($this->isVisible()) {
      $this->postValue = $this->value;
      foreach (array_keys($_POST) as $name) {
        if (preg_match('/^download'.$this->getHtmlId().'/', $name)) {
          session::set('binary', $this->value);
          session::set('binarytype', 'octet-stream');
          session::set('binaryfile', $this->computeFilename());
          header('location: getbin.php');
          exit();
        }
      }
      if ($this->upload) {
        if (isset($_POST['upload'.$this->getHtmlId()])) {
          parent::loadPostValue();
        } else {
          $id = $this->getHtmlId().'_text';
          if (isset($_POST[$id])) {
            $this->setPostValue($_POST[$id]);
          }
        }
      }
    }
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display  = '<textarea name="'.$id.'_text" id="'.$id.'_text"'.
                ($this->disabled? 'disabled="disabled"':'').'>'.
                '{literal}'.htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8').'{/literal}</textarea><br/>';
    return $this->renderAcl($display).parent::renderFormInput();
  }

  protected function htmlIds()
  {
    $ids    = parent::htmlIds();
    $ids[]  = $id.'_text';
    return $ids;
  }
}

class ImageAttribute extends FileAttribute
{
  protected $width;
  protected $height;
  protected $format;
  protected $forceSize;
  protected $placeholder;

  function __construct ($label, $description, $ldapName, $required = FALSE, $width = 48, $height = 48, $format = 'png', $forceSize = FALSE, $defaultValue = "", $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->width      = $width;
    $this->height     = $height;
    $this->format     = $format;
    $this->forceSize  = $forceSize;
  }

  function setPlaceholder($placeholder)
  {
    $this->placeholder = $placeholder;
  }

  /*! \brief Update this attributes postValue depending of the $_POST values
   */
  function loadPostValue ()
  {
    $this->postValue = $this->value;
    if (isset($_POST['upload'.$this->getHtmlId()])) {
      parent::loadPostValue();
    }
  }

  function setValue ($value)
  {
    if ($value == "") {
      $this->value = "";
      return;
    }
    if (class_exists('Imagick')) {
      $im     = new Imagick();
      $modify = FALSE;
      $im->readImageBlob($value);

      $size = $im->getImageGeometry();

      if (
          ($size['width'] > 0 && $size['height'] > 0) &&
          (
            ($size['width'] < $this->width && $size['height'] < $this->height) ||
            ($size['width'] > $this->width) ||
            ($size['height'] > $this->height)
          )
        ) {
        $modify = TRUE;
        $im->resizeImage($this->width, $this->height, Imagick::FILTER_GAUSSIAN, 1, !$this->forceSize);
      }

      if ($modify || !preg_match('/^'.$this->format.'$/i', $im->getImageFormat())) {
        if ($this->format == 'jpeg') {
          $im->setImageCompression(Imagick::COMPRESSION_JPEG);
          $im->setImageCompressionQuality(90);
        }
        $im->setImageFormat($this->format);

        /* Save attribute */
        $this->value = $im->getImageBlob();
      } else {
        $this->value = $value;
      }
    } else {
      msg_dialog::display(
        _('Error'),
        _('Cannot save user picture, FusionDirectory requires the PHP module "imagick" to be installed!'),
        ERROR_DIALOG
      );
    }
  }

  function renderFormInput ()
  {
    $this->setValue($this->inputValue($this->getValue()));
    $id = $this->getHtmlId();
    srand((double)microtime() * 1000000); // Just to be sure the image is not cached
    $display  = '<img id="'.$id.'_img"'.
                ($this->disabled? 'disabled="disabled"':'').
                ' src="getbin.php?rand='.rand(0, 10000).'"'.
                ' style="border:1px solid black;"'.
                ' alt="'.$this->getDescription().'"'.
                ' title="'.$this->getDescription().'"'.
                ' /><br/>';
    $display  .= $this->renderInputField('file', $id);
    $display  .= $this->renderInputField('submit', 'upload'.$id, array('value' => _('Upload')));
    if (($this->getValue() == '') && ($this->placeholder != '')) {
      session::set('binary', $this->placeholder);
    } else {
      session::set('binary', $this->getValue());
    }
    session::set('binarytype', 'image/'.$this->format);
    return $this->renderAcl($display);
  }

  protected function htmlIds()
  {
    $id = $this->getHtmlId();
    return array($id.'_img',$id,'upload'.$id);
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
  function __construct ($ou, $label = NULL, $desc = NULL)
  {
    if ($label === NULL) {
      $label = _('Base');
    }
    if ($desc === NULL) {
      $desc = _('Object base');
    }
    parent::__construct($label, $desc, 'base', FALSE, '', 'base');
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
      /* Do base conversation */
      if ($this->plugin->is_template) {
        $this->ou = 'ou=templates,'.$this->ou;
      }
      if ($this->plugin->dn == "new") {
        $ui = get_userinfo();
        $this->setValue(dn2base(session::global_is_set("CurrentMainBase")?"cn=dummy,".session::global_get("CurrentMainBase"):$ui->dn));
      } else {
        $this->setValue(dn2base($this->plugin->dn, $this->ou));
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
    if (!$this->disabled && $this->isVisible()) {
      /* Refresh base */
      if  ($this->plugin->acl_is_moveable($this->value) || ($this->plugin->dn == 'new')) {
        if (!$this->baseSelector->update()) {
          if ($this->plugin->dn == 'new') {
            msg_dialog::display(_('Error'), msgPool::permCreate(), ERROR_DIALOG);
          } else {
            msg_dialog::display(_('Error'), msgPool::permMove($this->plugin->dn), ERROR_DIALOG);
          }
        }
        if ($this->value != $this->baseSelector->getBase()) {
          $this->setValue($this->baseSelector->getBase());
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
      if (($this->orig_dn == 'new') && !$this->plugin->acl_is_createable($this->value)) {
        return msgPool::permCreate();
      } elseif (
        ($this->orig_dn != 'new') &&
        ($this->plugin->dn != $this->orig_dn) &&
        !$this->plugin->acl_is_moveable($this->value)) {
        return msgPool::permMove($this->plugin->dn);
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
      if ($this->plugin->dn == 'new') {
        $this->plugin->set_acl_base($this->value);
        $this->plugin->parent->set_acl_base();
      }

      if (($this->baseSelector !== NULL) && ($this->baseSelector->getBase() !== $this->value)) {
        $this->baseSelector->setBase($this->value);
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
    if ($this->disabled) {
      $display = $this->renderInputField(
        'text', '',
        array(
          'value' => '{literal}'.htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8').'{/literal}'
        )
      );
    } else {
      $display = '{literal}'.$this->baseSelector->render().'{/literal}';
    }
    return $this->renderAcl($display);
  }

  function getHtmlId()
  {
    if (isset($this->baseSelector)) {
      return $this->baseSelector->getInputHtmlId();
    } else {
      return '';
    }
  }

  function serializeAttribute(&$attributes, $form = TRUE)
  {
    if (!$form || $this->visible) {
      parent::serializeAttribute($attributes, $form);

      $attributes[$this->getLdapName()]['choices'] = $this->baseSelector->getBases();
    }
  }
}

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

  protected function htmlIds()
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
  protected $readFormat;
  protected $writeFormat;
  protected $linearRendering = FALSE;

  /*! \brief The constructor of CompositeAttribute
   *
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param array $attributes The attributes that are parts of this composite attribute
   *  \param string $readFormat the preg_match format that's gonna be used in order to read values from LDAP
   *  \param string $writeFormat the printf format that's gonna be used in order to write values into LDAP
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   *  \param string $label The label to show for this attribute. Only useful if you put this attribute inside a SetAttribute, or if you use a specific template that needs it.
   */
  function __construct ($description, $ldapName, $attributes, $readFormat, $writeFormat, $acl = "", $label = NULL)
  {
    if ($label === NULL) {
      $label = $ldapName;
    }
    parent::__construct($label, $description, $ldapName, FALSE, "", $acl);
    $this->readFormat   = $readFormat;
    $this->writeFormat  = $writeFormat;
    $this->setAttributes($attributes);
  }

  function setAttributes ($attributes)
  {
    $this->attributes   = $attributes;
    foreach ($this->attributes as &$attribute) {
      $attribute->setAcl($this->getAcl());
    }
    unset($attribute);
  }

  function setAcl ($acl)
  {
    parent::setAcl($acl);
    foreach ($this->attributes as &$attribute) {
      $attribute->setAcl($this->getAcl());
    }
    unset($attribute);
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    foreach ($this->attributes as &$attribute) {
      $attribute->setParent($plugin);
    }
    unset($attribute);
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
    $res = preg_match($this->readFormat, $value, $m);
    if ($res === 1) {
      $m = array_slice($m, 1);
      $values = array();
      foreach (array_keys($this->attributes) as $name) {
        if (isset($m[$name])) {
          $values[] = $m[$name];
        } else {
          $values[] = '';
        }
      }
      return $values;
    } elseif ($res === FALSE) {
      trigger_error('Error in preg_match : '.preg_last_error());
    } elseif ($value !== "") { /* If an empty value does not match, we don't trigger an error */
      trigger_error('String passed "'.$value.'"to Composite did not match format "'.$this->readFormat.'"');
    }
    return array_fill(0, count($this->attributes), '');
  }

  function writeValues($values)
  {
    if ($this->writeFormat === FALSE) {
      return $values;
    } else {
      return vsprintf($this->writeFormat, $values);
    }
  }

  function resetToDefault ()
  {
    foreach ($this->attributes as &$attribute) {
      $attribute->resetToDefault();
    }
    unset($attribute);
  }

  function inputValue ($value)
  {
    $values = $this->readValues($value);
    $i = 0;
    foreach ($this->attributes as &$attribute) {
      $values[$i] = $attribute->inputValue($values[$i]);
      $i++;
    }
    unset($attribute);
    return $values;
  }

  function loadPostValue ()
  {
    foreach ($this->attributes as &$attribute) {
      $attribute->loadPostValue();
    }
    unset($attribute);
  }

  function applyPostValue ()
  {
    foreach ($this->attributes as &$attribute) {
      $attribute->applyPostValue();
    }
    unset($attribute);
  }

  function setValue ($values)
  {
    if (!is_array($values)) {
      $values = $this->inputValue($values);
    }
    $i = 0;
    reset($values);
    foreach ($this->attributes as &$attribute) {
      $attribute->setValue(current($values));
      next($values);
    }
    unset($attribute);
    reset($values);
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
        return $a->displayValue($a->getValue());
      },
      $this->attributes
    );
    return $values;
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    }
    foreach ($this->attributes as &$attribute) {
      $error = $attribute->check();
      if (!empty($error)) {
        return $error;
      }
    }
    unset($attribute);
  }

  function renderAttribute(&$attributes, $readOnly)
  {
    if ($this->visible) {
      if ($this->linearRendering) {
        parent::renderAttribute($attributes, $readOnly);
      } else {
        foreach ($this->attributes as &$attribute) {
          $attribute->setDisabled($this->disabled);
          $attribute->renderAttribute($attributes, $readOnly);
        }
        unset($attribute);
      }
    }
  }

  function getForHtmlId()
  {
    // Label (if any) should point to the first attribute
    if (isset($this->attributes[0])) {
      return $this->attributes[0]->getForHtmlId();
    } else {
      return '';
    }
  }

  function serializeAttribute(&$attributes, $form = TRUE)
  {
    if ($form) {
      if ($this->visible) {
        foreach ($this->attributes as &$attribute) {
          $attribute->setDisabled($this->disabled);
          $attribute->serializeAttribute($attributes, $form);
        }
        unset($attribute);
      }
    } else {
      parent::serializeAttribute($attributes, $form);
      $subattributes = array();
      foreach ($this->attributes as &$attribute) {
        $attribute->setDisabled($this->disabled);
        $attribute->serializeAttribute($subattributes, $form);
      }
      unset($attribute);
      $attributes[$this->getLdapName()]['attributes'] = $subattributes;
    }
  }

  function deserializeValue($values)
  {
    if ($this->visible) {
      foreach ($this->attributes as &$attribute) {
        $attribute->setDisabled($this->disabled);
        $attribute->deserializeValue($values);
      }
      unset($attribute);
    }
  }

  function renderFormInput()
  {
    $display = "";
    foreach ($this->attributes as &$attribute) {
      $attribute->setDisabled($this->disabled);
      if ($attribute->isVisible()) {
        $display .= '<label for="'.$attribute->getForHtmlId().'">'.$attribute->getLabel().'</label>'." ".$attribute->renderFormInput()." ";
      }
    }
    unset($attribute);
    return $display;
  }

  protected function htmlIds()
  {
    $ret = array();
    foreach ($this->attributes as &$attribute) {
      $ret = array_merge($ret, $attribute->htmlIds());
    }
    unset($attribute);
    return $ret;
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
        msg_dialog::display(sprintf(_('Invalid value for %s'), $this->getLabel()), $error);
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

  protected function htmlIds()
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


/*! \brief This class allow to handle an attribute that stores flags based on other BooleanAttributes
 *
 */
class FlagsAttribute extends StringAttribute
{
  protected $flags;

  /*! \brief The constructor of IntAttribute
   *
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param array  $flags The ids of the BooleanAttributes to use as flags
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct ($ldapName, $flags, $acl = "")
  {
    parent::__construct('', '', $ldapName, FALSE, '', $acl);
    $this->setVisible(FALSE);
    $this->flags = $flags;
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      foreach ($this->flags as $attr) {
        $this->plugin->attributesAccess[$attr]->setInLdap(FALSE);
      }
    }
  }

  function setValue ($value)
  {
    parent::setValue($value);
    if (is_object($this->plugin)) {
      foreach ($this->flags as $attr) {
        $trueValue = $this->plugin->attributesAccess[$attr]->trueValue;
        $this->plugin->attributesAccess[$attr]->setValue(preg_match("/$trueValue/", $this->value));
      }
    }
  }

  function getValue()
  {
    $value = '[';
    if (is_object($this->plugin)) {
      foreach ($this->flags as $attr) {
        $value .= $this->plugin->attributesAccess[$attr]->computeLdapValue();
      }
    }
    $value .= ']';
    return $value;
  }
}

?>
