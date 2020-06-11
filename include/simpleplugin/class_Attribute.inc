<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2012-2019  FusionDirectory

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

/*!
 * \file class_Attribute.inc
 * Source code for the main Attribute class
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

  /* \brief Filter to use when checking unicity
   * Most of the time this is NULL and filter is computed from plugin objectTypes and objectClasses */
  private $uniqueFilter = NULL;

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
  protected $managedAttributes = [];
  /* \brief Array of multiple values groups for managed attributes */
  protected $managedAttributesMultipleValues = [];

  /* \bried Array of booleans telling for each managing attributes if he's disabling us */
  protected $managingAttributesOrders = [];

  /* \bried If this is TRUE it means this attribute is not directly submitted with the form
   * but is part of a multivalue attribute.
   * It means it should not be set as required in the HTML form for instance.
   */
  protected $isSubAttribute = FALSE;

  /*! \brief The constructor of Attribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param mixed $defaultValue The default value for this attribute
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct (string $label, string $description, string $ldapName, bool $required = FALSE, $defaultValue = '', string $acl = '')
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
  function setParent (simplePlugin &$plugin)
  {
    $this->plugin = $plugin;
    $this->manageAttributes($this->getValue());
  }

  /*! \brief Get parent plugin instance, if any
   *
   * \return simplePlugin|null
   */
  function getParent ()
  {
    return $this->plugin;
  }

  function setIsSubAttribute (bool $bool)
  {
    $this->isSubAttribute = $bool;
  }

  function setInLdap (bool $inLdap)
  {
    $this->inLdap = $inLdap;
  }

  function setVisible (bool $visible)
  {
    $this->visible = $visible;
  }

  function isVisible (): bool
  {
    return $this->visible;
  }

  function isTemplate (): bool
  {
    return (is_object($this->plugin) && $this->plugin->is_template);
  }

  function setUnique ($unique, string $filter = NULL)
  {
    if ($unique === TRUE) {
      trigger_error('Deprecated fallback');
      $this->unique = 'one';
    } else {
      $this->unique = $unique;
    }
    $this->uniqueFilter = $filter;
  }

  function getUnique (): string
  {
    return $this->unique;
  }

  function isInLdap (): bool
  {
    return $this->inLdap;
  }

  /*! \brief Get an example value
   *
   * \return string|null
   */
  public function getExample ()
  {
    return NULL;
  }

  function checkValue ($value)
  {
    /* Should throw InvalidValueException if needed */
  }

  function setValue ($value)
  {
    $this->checkValue($value);
    $old_value    = $this->value;
    $this->value  = $value;
    if (($this->submitForm !== FALSE) && ($this->submitForm !== TRUE) && ($old_value != $value) && is_object($this->plugin)) {
      $func = $this->submitForm;
      $this->plugin->$func();
    }
    $this->manageAttributes($this->value);
  }

  /*! \brief Set the postValue */
  function setPostValue ($value)
  {
    if ($this->isVisible()) {
      $this->postValue = $this->fixPostValue($value);
      $this->manageAttributes($this->postValue);
    }
  }

  /*! \brief In case a treatment is needed on POST content */
  function fixPostValue ($value)
  {
    return $value;
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
  function getArrayValue (): array
  {
    return [$this->displayValue($this->getValue())];
  }

  function getLdapName (): string
  {
    return $this->ldapName;
  }

  function getHtmlId (): string
  {
    return $this->htmlid_prefix.userinfo::sanitizeAttributeName($this->getLdapName());
  }

  /* html id to put in the "for" attribute of our "label" tag */
  function getForHtmlId (): string
  {
    return $this->getHtmlId();
  }

  function getLabel (): string
  {
    return $this->label;
  }

  function getDescription (): string
  {
    return $this->description;
  }

  function getAcl (): string
  {
    if (empty($this->acl)) {
      return $this->getHtmlId();
    } else {
      return $this->acl;
    }
  }

  function setAcl (string $acl)
  {
    $this->acl = $acl;
  }

  function isRequired (): bool
  {
    return $this->required;
  }

  function setRequired (bool $bool)
  {
    $this->required = $bool;
  }

  protected function setLabel (string $label)
  {
    $this->label = $label;
  }

  protected function setDescription (string $description)
  {
    $this->description = $description;
  }

  function setDisabled (bool $disabled)
  {
    $this->disabled = $disabled;
  }

  function isDisabled (): bool
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
  function loadValue (array $attrs)
  {
    if ($this->inLdap) {
      $this->loadAttrValue($attrs);
    }
    $this->initialValue = $this->getValue();
  }

  /*! \brief Loads this attribute value from the attrs array if present
   *
   * If the attribute is not present in $attrs, does not change value
   *
   * \param array $attrs The attributes array as fetch from the LDAP
   */
  protected function loadAttrValue (array $attrs)
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

  function hasChanged (): bool
  {
    return ($this->getValue() !== $this->initialValue);
  }

  function displayValue ($value): string
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
  function setManagedAttributes (array $mAttributes)
  {
    if (isset($mAttributes['multiplevalues'])) {
      $this->managedAttributesMultipleValues = $mAttributes['multiplevalues'];
      unset($mAttributes['multiplevalues']);
    } else {
      $this->managedAttributesMultipleValues = [];
    }
    $this->managedAttributes = $mAttributes;
    $this->manageAttributes($this->getValue());
  }

  protected function isValueManagingValue ($myvalue, $mavalue): bool
  {
    if (isset($this->managedAttributesMultipleValues[$mavalue])) {
      return in_array($myvalue, $this->managedAttributesMultipleValues[$mavalue]);
    } else {
      return ($myvalue == $mavalue);
    }
  }

  function manageAttributes ($myvalue): bool
  {
    if ($this->plugin === NULL) {
      return FALSE;
    }
    foreach ($this->managedAttributes as $array) {
      foreach ($array as $value => $attributes) {
        foreach ($attributes as $attribute) {
          $disable = $this->isValueManagingValue($myvalue, $value);
          $this->plugin->attributesAccess[$attribute]->setManagingDisabled($this->getLdapName(), $disable);
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
  function fillLdapValue (array &$attrs)
  {
    if ($this->inLdap) {
      $ldapValue = $this->computeLdapValue();
      if ($ldapValue !== '') {
        $attrs[$this->getLdapName()] = $ldapValue;
      } else {
        $attrs[$this->getLdapName()] = [];
      }
    }
  }

  /*! \brief Post-modify the attrs array if needed (used for erasing managed attributes)
   */
  function fillLdapValueHook (array &$attrs)
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
          $attrs[$attribute] = [];
        }
      }
    }
  }

  /*! \brief Check the correctness of this attribute
   */
  function check ()
  {
    global $config;
    $currentValue = $this->getValue();
    if ($this->isRequired() && !$this->disabled && (($currentValue === "") || ($currentValue === []))) {
      return msgPool::required($this->getLabel());
    } elseif (($this->unique !== FALSE) && !$this->disabled) {
      $ldapValue = $this->computeLdapValue();
      if (($ldapValue === "") || ($ldapValue === [])) {
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
      if (is_array($ldapValue)) {
        $filter = '(|('.$this->getLdapName().'='.join(')('.$this->getLdapName().'=', array_map('ldap_escape_f', $ldapValue)).'))';
      } else {
        $filter = '('.$this->getLdapName().'='.ldap_escape_f($ldapValue).')';
      }
      $infos = pluglist::pluginInfos(get_class($this->plugin));
      if ($this->uniqueFilter === NULL) {
        $objectTypeFilters = array_map(
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
        $filters = [];
        if (!empty($objectTypeFilters)) {
          $filters[] = '(|'.implode($objectTypeFilters).')';
        }
        $pluginFilter = call_user_func([get_class($this->plugin), 'getLdapFilter']);
        if (!empty($pluginFilter)) {
          $filters[] = $pluginFilter;
        }
      } else {
        $filters = [$this->uniqueFilter];
      }
      $filter = '(&'.$filter.implode($filters).')';
      $ldap->search($filter, [$this->getLdapName()]);
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
        ),
        function ($ou)
        {
          return ($ou !== FALSE);
        }
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
          } elseif ($this->uniqueFilter === NULL) { /* whole (or sub) */
            /* Check that this entry is in a concerned branch */
            foreach ($branches as $branch) {
              if (preg_match('/^'.preg_quote($branch, '/').'/', $dn_base)) {
                $dn_base  = preg_replace('/^'.preg_quote($branch, '/').'/', '', $dn_base);
                $found    = TRUE;
                break;
              }
            }
            if (!in_array($dn_base, $config->getDepartmentList())) {
              continue;
            }
          } else {
            $found = TRUE;
          }
          if (!$found) {
            continue;
          }

          return new SimplePluginCheckError(
            $this,
            htmlescape(msgPool::duplicated($this->getLabel(), $attrs['dn']))
          );
        }
      }
    }
  }

  /*! \brief Render this attribute form input(s)
   *
   *  \param array &$attributes the attributes array
   *
   *  \param bool $readOnly should we show text or input
   *
   *  \param bool $readable ACL read
   *
   *  \param bool $writable ACL write
   */
  function renderAttribute (array &$attributes, bool $readOnly, bool $readable, bool $writable)
  {
    if ($this->visible) {
      if ($readOnly) {
        $currentValue = $this->getValue();
        if (is_array($currentValue)) {
          $input = '{literal}'.implode('<br/>', array_map('htmlescape', $currentValue)).'{/literal}';
        } else {
          $input = '{literal}'.htmlescape($currentValue).'{/literal}';
        }
      } elseif ($this->isTemplate()) {
        $input = $this->renderTemplateInput();
      } else {
        $input = $this->renderFormInput();
      }
      $attributes[$this->getLdapName()] = [
        'htmlid'        => $this->getForHtmlId(),
        'label'         => '{literal}'.htmlescape($this->getLabel()).'{/literal}',
        'description'   => ($this->isRequired() ? sprintf(_("%s (required)"), $this->getDescription()) : $this->getDescription()),
        'input'         => $input,
        'subattribute'  => $this->isSubAttribute,
        'required'      => $this->isRequired(),
        'readable'      => $readable,
        'writable'      => $writable,
      ];
    }
  }

  /*! \brief Serialize this attribute for RPC requests
   *
   * \param array &$attributes the attributes array
   * \param boolean $form
   */
  function serializeAttribute (array &$attributes, bool $form = TRUE)
  {
    if (!$form || $this->visible) {
      $class = get_class($this);
      while ($class != 'Attribute') {
        $type[] = $class;
        $class  = get_parent_class($class);
      }
      /* Avoid empty array */
      $type[] = 'Attribute';
      $infos = [
        'htmlid'      => $this->getHtmlId(),
        'label'       => $this->getLabel(),
        'required'    => $this->isRequired(),
        'disabled'    => $this->disabled,
        'description' => $this->getDescription(),
        'value'       => $this->serializeValue(),
        'default'     => $this->serializeValue($this->defaultValue),
        'type'        => $type,
      ];
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
   *  \param mixed $value the value
   */
  function deserializeValue ($value)
  {
    if ($this->disabled) {
      return sprintf(_('Attribute %s is disabled, its value could not be set'), $this->getLdapName());
    }
    $this->setValue($value);
  }

  /*! \brief Serialize value for RPC requests
   *
   *  \param mixed $value the value
   */
  function serializeValue ($value = NULL)
  {
    if ($value === NULL) {
      $value = $this->getValue();
    }
    return $value;
  }

  /*! \brief Add ACL information around display
   *
   *  \param string $display the display information to pass through ACL
   */
  function renderAcl (string $display): string
  {
    return '{render aclName="'.$this->getAcl().'" acl=$'.$this->getAcl()."ACL}\n$display\n{/render}";
  }

  /*! \brief Get ACL information about the ACL we need to create
   */
  function getAclInfo ()
  {
    if (empty($this->acl)) {
      return [
        'name' => $this->getHtmlId(),
        'desc' => $this->getDescription()
      ];
    } else {
      /* If acl is not empty, we use an acl that is not ours, we have no acl to create */
      return FALSE;
    }
  }

  protected function changeStateJS (): string
  {
    return implode('', array_map(
      function ($id)
      {
        return 'changeState('.json_encode($id).');';
      },
      $this->htmlIds()
    ));
  }

  public function htmlIds (): array
  {
    return [$this->getHtmlId()];
  }

  protected function managedAttributesJS (): string
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

  function renderTemplateInput (): string
  {
    return $this->renderFormInput();
  }

  function foreignKeyUpdate ($oldvalue, $newvalue, array $source)
  {
    if ($source['MODE'] == 'move') {
      if ($newvalue === NULL) {
        $this->resetToDefault();
      } elseif ($source['FIELD'] == 'dn') {
        $initialValue = $this->getInitialValue();
        $initialValue = preg_replace('/'.preg_quote($oldvalue, '/').'$/', $newvalue, $initialValue, -1, $count);
        if ($count > 0) {
          $this->setValue($initialValue);
        }
      } elseif ($this->getInitialValue() == $oldvalue) {
        $this->setValue($newvalue);
      }
    }
  }

  function foreignKeyCheck ($value, array $source): bool
  {
    return ($this->getValue() == $value);
  }

  protected function renderInputField (string $type, string $name, array $attributes = []): string
  {
    $input  = '{literal}<input type="'.htmlescape($type).'" '.
              'name="'.htmlescape($name).'" id="'.htmlescape($name).'"'.
              ($this->disabled ? ' disabled="disabled"' : '');
    foreach ($attributes as $label => $value) {
      $input .= ' '.$label.'="'.htmlescape($value).'"';
    }
    $input .= '/>{/literal}';
    return $input;
  }
}

/*!
 * \brief Attribute hidden from the user
 */
class HiddenAttribute extends Attribute
{
  /*! \brief The constructor of HiddenAttribute
   *
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param mixed $defaultValue The default value for this attribute
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   */
  function __construct (string $ldapName, bool $required = FALSE, $defaultValue = '', string $acl = '', string $label = NULL, string $description = 'hidden')
  {
    if ($label === NULL) {
      $label = $ldapName;
    }
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setVisible(FALSE);
  }
}

/*!
 * \brief HiddenAttribute with several values
 */
class HiddenArrayAttribute extends HiddenAttribute
{
  protected function loadAttrValue (array $attrs)
  {
    if (isset($attrs[$this->getLdapName()]['count'])) {
      $this->value = [];
      for ($i = 0; $i < $attrs[$this->getLdapName()]['count']; $i++) {
        $this->value[] = $attrs[$this->getLdapName()][$i];
      }
    }
  }

  public function computeLdapValue ()
  {
    if (is_array($this->value)) {
      return array_values($this->value);
    } else {
      return parent::computeLdapValue();
    }
  }
}

/*!
 * \brief Dummy attribute class in order to give stats information to the template
 */
class FakeAttribute extends Attribute
{
  function __construct (string $ldapName)
  {
    parent::__construct('Fake one', '', $ldapName, FALSE, '', 'noacl');
    $this->setInLdap(FALSE);
  }

  function renderAttribute (array &$attributes, bool $readOnly, bool $readable, bool $writable)
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
  protected $allowHTML    = FALSE;
  protected $allowSmarty  = FALSE;

  function renderFormInput (): string
  {
    if ($this->allowHTML) {
      $value = $this->getValue();
    } else {
      $value = htmlescape($this->getValue());
    }
    if ($this->allowSmarty) {
      return $value;
    } else {
      return '{literal}'.$value.'{/literal}';
    }
  }

  function setAllowHTML (bool $allowHTML)
  {
    $this->allowHTML = $allowHTML;
  }

  function setAllowSmarty (bool $allowSmarty)
  {
    $this->allowSmarty = $allowSmarty;
  }
}

/*! \brief This class allow to display an attribute.
 *
 * It can be used to display an attribute value the user and FD are never allowed to modify.
 */
class ReadOnlyLDAPAttribute extends DisplayLDAPAttribute
{
  function fillLdapValue (array &$attrs)
  {
  }
}

/*! \brief This class allow to display an attribute.
 *
 * It can be used to display an attribute value the user is never allowed to modify.
 */
class DisplayLDAPArrayAttribute extends Attribute
{
  protected function loadAttrValue (array $attrs)
  {
    if (isset($attrs[$this->getLdapName()]['count'])) {
      $this->value = [];
      for ($i = 0; $i < $attrs[$this->getLdapName()]['count']; $i++) {
        $this->value[] = $attrs[$this->getLdapName()][$i];
      }
    }
  }

  function renderFormInput (): string
  {
    $value = $this->getValue();
    if (is_array($value)) {
      $value = join(', ', $value);
    }
    return '{literal}'.htmlescape($value).'{/literal}';
  }
}

/*! \brief This class allow to display a text in front of an attribute.
 *
 * For instance, it can be used to display a link.
 */
class DisplayAttribute extends DisplayLDAPAttribute
{
  function __construct (string $label, string $description, string $ldapName, bool $required = FALSE, $defaultValue = '', string $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setInLdap(FALSE);
  }
}

/*! \brief This class allow to display a link to an object which dn is stored in the attribute
 */
class ObjectLinkAttribute extends DisplayLDAPAttribute
{
  protected $type;

  function __construct (string $label, string $description, string $ldapName, bool $required, string $type, $defaultValue = '', string $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->type = $type;
  }

  function renderFormInput (): string
  {
    if (empty($this->value)) {
      return '';
    }

    try {
      return objects::link($this->value, $this->type);
    } catch (NonExistingLdapNodeException $e) {
      return '<a><img src="geticon.php?context=status&amp;icon=dialog-warning&amp;size=16" alt="warning" class="center"/>&nbsp;'.sprintf(_('Invalid: %s'), $this->value).'</a>';
    }
  }
}
