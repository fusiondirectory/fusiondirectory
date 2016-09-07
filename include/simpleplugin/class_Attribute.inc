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
      return msgPool::required($this->getLabel(), $this->getLdapName(), $this->getDescription());
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

          return msgPool::duplicated($this->getLabel(), $attrs['dn'], $this->getLdapName(), $this->getDescription());
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
      /* Avoid empty array */
      $type[] = 'Attribute';
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
    if (empty($this->acl)) {
      return array(
        'name' => $this->getHtmlId(),
        'desc' => $this->getDescription()
      );
    } else {
      /* If acl is not empty, we use an acl that is not ours, we have no acl to create */
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

  public function htmlIds()
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
?>
