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

/*! \brief This class allow to handle easily a Boolean LDAP attribute
 *
 */
class BooleanAttribute extends Attribute
{
  public $trueValue;
  public $falseValue;
  protected $templatable = TRUE;

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

  function setTemplatable ($bool)
  {
    $this->templatable = $bool;
  }

  protected function isTemplatable ()
  {
    /* Allow to set to %askme% if we are not (de)activating other fields */
    return (!$this->submitForm && empty($this->managedAttributes) && $this->templatable);
  }

  function inputValue ($value)
  {
    if ($this->isTemplate() && $this->isTemplatable() && ($value === '%askme%')) {
      return $value;
    }
    return ($value == $this->trueValue);
  }

  function loadPostValue ()
  {
    if ($this->isVisible()) {
      if ($this->isTemplate() && $this->isTemplatable()) {
        if (!isset($_POST[$this->getHtmlId()])) {
          $this->setPostValue(FALSE);
        } elseif ($_POST[$this->getHtmlId()] === '%askme%') {
          $this->setPostValue('%askme%');
        } else {
          $this->setPostValue($_POST[$this->getHtmlId()] == 'TRUE');
        }
      } else {
        $this->setPostValue(isset($_POST[$this->getHtmlId()]));
      }
    }
  }

  function computeLdapValue ()
  {
    if ($this->isTemplate() && $this->isTemplatable() && ($this->value === '%askme%')) {
      return $this->value;
    } else {
      return ($this->value ? $this->trueValue : $this->falseValue);
    }
  }

  function displayValue ($value): string
  {
    if ($this->isTemplate() && $this->isTemplatable() && ($value === '%askme%')) {
      return $value;
    } else {
      return ($value ? _('yes') : _('no'));
    }
  }

  function renderFormInput (): string
  {
    $id = $this->getHtmlId();
    $attributes = ($this->value ? ['checked' => 'checked'] : []);
    if ($this->submitForm) {
      $js       = 'document.mainform.submit();';
      $attributes['onChange'] = 'javascript:'.$js;
    } elseif (!empty($this->managedAttributes)) {
      $js       = $this->managedAttributesJS();
      $attributes['onChange'] = 'javascript:'.$js;
    }
    $display  = $this->renderInputField('checkbox', $id, $attributes);
    return $this->renderAcl($display);
  }

  function renderTemplateInput (): string
  {
    if ($this->isTemplatable()) {
      $id = $this->getHtmlId();
      if ($this->getValue() === '%askme%') {
        $selected = '%askme%';
      } elseif ($this->getValue()) {
        $selected = 'TRUE';
      } else {
        $selected = 'FALSE';
      }
      $display  = '<select name="'.$id.'" id="'.$id.'" ';
      if ($this->disabled) {
        $display .= 'disabled="disabled" ';
      }
      $display .= '>';
      $display .= '{html_options values=array("FALSE","TRUE","%askme%") output=array("FALSE","TRUE","%askme%") selected="'.$selected.'"}';
      $display .= '</select>';
      return $this->renderAcl($display);
    } else {
      return $this->renderFormInput();
    }
  }

  protected function managedAttributesJS (): string
  {
    $js = '';
    $id = $this->getHtmlId();
    foreach ($this->managedAttributes as $array) {
      foreach ($array as $value => $attributes) {
        if (isset($this->managedAttributesMultipleValues[$value])) {
          trigger_error('Multiple values are not available for boolean attributes');
        } else {
          $js .= 'disableAttributes = (document.getElementById('.json_encode($id).').checked == '.($value ? 'true' : 'false').');'."\n";
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

  /*! \brief Apply value from RPC requests
   *
   *  \param mixed $value the value
   */
  function deserializeValue ($value)
  {
    if ($this->disabled) {
      return sprintf(_('Attribute %s is disabled, its value could not be set'), $this->getLdapName());
    }
    if ($value === $this->trueValue) {
      $this->setValue(TRUE);
    } elseif ($value === $this->falseValue) {
      $this->setValue(FALSE);
    } elseif (is_bool($value)) {
      $this->setValue($value);
    } elseif ($value === 1) {
      $this->setValue(TRUE);
    } elseif ($value === 0) {
      $this->setValue(FALSE);
    } else {
      return sprintf(_('"%s" is not a valid value for attribute "%s" should be "%s" or "%s"'), $value, $this->getLdapName(), $this->trueValue, $this->falseValue);
    }

    /* No error */
    return '';
  }

  function serializeAttribute (array &$attributes, bool $form = TRUE)
  {
    if (!$form || $this->visible) {
      parent::serializeAttribute($attributes, $form);
      $attributes[$this->getLdapName()]['choices'] = [
        $this->trueValue  => 'True',
        $this->falseValue => 'False',
      ];
    }
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
      $this->objectclasses = [$objectclasses];
    }
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setInLdap(FALSE);
  }

  function loadValue (array $attrs)
  {
    if (isset($attrs['objectClass'])) {
      $missing_oc = array_udiff($this->objectclasses, $attrs['objectClass'], 'strcasecmp');
      $this->setValue(empty($missing_oc));
    } else {
      $this->resetToDefault();
    }
    $this->initialValue = $this->value;
  }

  function fillLdapValue (array &$attrs)
  {
    if ($this->getValue()) {
      $attrs['objectClass'] = array_merge_unique($this->objectclasses, $attrs['objectClass']);
    } else {
      $attrs['objectClass'] = array_remove_entries($this->objectclasses, $attrs['objectClass']);
    }
  }
}
