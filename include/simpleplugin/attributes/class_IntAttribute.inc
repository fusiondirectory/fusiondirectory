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
