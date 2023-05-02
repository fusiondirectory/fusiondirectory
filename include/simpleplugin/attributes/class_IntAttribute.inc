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

/*! \brief This class allow to handle easily an Integer LDAP attribute
 *
 */
class IntAttribute extends \FusionDirectory\Core\SimplePlugin\Attribute
{
  protected $min;
  protected $max;
  protected $step = 1;

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
  }

  public function getExample ()
  {
    if (($this->min !== FALSE) && ($this->max !== FALSE)) {
      return sprintf(_('An integer between %d and %d'), $this->min, $this->max);
    } elseif ($this->min !== FALSE) {
      return sprintf(_('An integer larger than %d'),    $this->min);
    } elseif ($this->max !== FALSE) {
      return sprintf(_('An integer smaller than %d'),   $this->max);
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
    if ($this->isTemplate()) {
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
    } elseif ($this->value !== '') {
      if (!is_numeric($this->value)) {
        return new SimplePluginCheckError(
          $this,
          SimplePluginCheckError::invalidValue(sprintf(_('"%s" is not an number'), $this->getValue()))
        );
      }
      if (($this->min !== FALSE) && ($this->value < $this->min)) {
        return new SimplePluginCheckError(
          $this,
          SimplePluginCheckError::invalidValue(sprintf(_('%s is smaller than %s'), $this->getValue(), $this->min))
        );
      }
      if (($this->max !== FALSE) && ($this->value > $this->max)) {
        return new SimplePluginCheckError(
          $this,
          SimplePluginCheckError::invalidValue(sprintf(_('%s is larger than %s'), $this->getValue(), $this->max))
        );
      }
    }
  }

  function renderFormInput (): string
  {
    $id = $this->getHtmlId();
    $attributes = [
      'value' => $this->getValue()
    ];
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
      $attributes['onChange'] = 'javascript:'.$js;
    }
    if ($this->isSubAttribute) {
      $attributes['class'] = 'subattribute';
    } elseif ($this->isRequired()) {
      $attributes['required'] = 'required';
    }
    $display = $this->renderInputField('number', $id, $attributes);
    return $this->renderAcl($display);
  }

  function renderTemplateInput (): string
  {
    $id = $this->getHtmlId();
    $attributes = [
      'value' => $this->getValue()
    ];
    if ($this->isSubAttribute) {
      $attributes['class'] = 'subattribute';
    }
    $display = $this->renderInputField('text', $id, $attributes);
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
  }

  public function getExample ()
  {
    if (($this->min !== FALSE) && ($this->max !== FALSE)) {
      return sprintf(_('A float between %f and %f'),  $this->min, $this->max);
    } elseif ($this->min !== FALSE) {
      return sprintf(_('A float larger than %f'),     $this->min);
    } elseif ($this->max !== FALSE) {
      return sprintf(_('A float smaller than %f'),    $this->max);
    }
  }

  function inputValue ($value)
  {
    if (!$this->isRequired() && empty($value) && !is_numeric($value)) {
      // value is "" or array()
      return "";
    }
    if ($this->isTemplate()) {
      return $value;
    } else {
      return floatval($value);
    }
  }
}
