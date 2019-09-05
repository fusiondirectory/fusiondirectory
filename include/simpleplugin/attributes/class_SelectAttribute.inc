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
 * \file class_SelectAttribute.inc
 * Source code for the Select attributes
 */

/*! \brief This class allow to handle easily a Select LDAP attribute with a set of choices
 *
 */
class SelectAttribute extends Attribute
{
  protected $choices;
  protected $outputs  = NULL;
  protected $size     = 1;

  /*! \brief Valid values which should be hidden from rendered input
   *
   * Used by SetAttribute to hide used values */
  protected $hiddenChoices = [];

  /*! \brief The constructor of SelectAttribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param array $choices The choices this select should offer. Pass array("") if you're gonna fill it later with setChoices
   *  \param mixed $defaultValue The default value for this attribute
   *  \param array $outputs The labels corresponding to the choices, leave to NULL if you want to display the choices themselves
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct ($label, $description, $ldapName, $required = FALSE, $choices = [], $defaultValue = "", $outputs = NULL, $acl = "")
  {
    if (!in_array($defaultValue, $choices, TRUE) && isset($choices[0])) {
      $defaultValue = $choices[0];
    }
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setChoices($choices, $outputs);
  }

  /*! \brief Set the options of the select attribute
   *
   *  \param array $choices The choices this select should offer
   *  \param array $outputs The labels corresponding to the choices, leave to NULL if you want to display the choices themselves
   */
  function setChoices (array $choices, array $outputs = NULL)
  {
    $this->outputs = NULL;
    if (!$this->isRequired() && !in_array('', $choices, TRUE)) {
      array_unshift($choices, '');
      if (is_array($outputs)) {
        array_unshift($outputs, _('None'));
      }
    }
    if ($this->isTemplate() && !in_array('%askme%', $choices)) {
      $choices[] = '%askme%';
      if (is_array($outputs)) {
        $outputs[] = '%askme%';
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

  /*! \brief Set the display options of the select attribute
   *
   *  \param array $values The labels corresponding to the choices, in the same order as the choices
   */
  function setDisplayChoices (array $values)
  {
    $this->outputs = [];
    $values = array_values($values);
    $i = 0;
    foreach ($this->choices as $choice) {
      $this->outputs[$choice] = $values[$i++];
    }
  }

  /*! \brief Get the choices
   */
  function getChoices ()
  {
    return $this->choices;
  }

  /*! \brief Get the displayed choices (keys are choices)
   */
  function getDisplayChoices ()
  {
    return $this->outputs;
  }

  function setHiddenChoices (array $choices)
  {
    $this->hiddenChoices = $choices;
  }

  function setRequired (bool $bool)
  {
    parent::setRequired($bool);
    $key = array_search("", $this->choices, TRUE);
    if ($this->isRequired() && ($key !== FALSE)) {
      unset($this->choices[$key]);
      if ($this->outputs !== NULL) {
        unset($this->outputs['']);
      }
    } elseif (!$this->isRequired() && !in_array("", $this->choices, TRUE)) {
      $this->choices[] = "";
      if (($this->outputs !== NULL) && !isset($this->output[''])) {
        $this->output[''] = _('None');
      }
    }
  }

  function displayValue ($value): string
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

  function renderFormInput (): string
  {
    $smarty         = get_smarty();
    $id             = $this->getHtmlId();
    $smartyChoices  = array_values(array_diff($this->choices, $this->hiddenChoices));
    $currentValue   = $this->getValue();
    if (in_array($currentValue, $this->hiddenChoices)) {
      $currentValue = '';
    }
    if ($this->outputs !== NULL) {
      /* Make sure choices and outputs are in the same order */
      $smartyOutputs = [];
      foreach ($smartyChoices as $choice) {
        $smartyOutputs[] = $this->outputs[$choice];
      }
    } else {
      $smartyOutputs = $this->choices;
    }

    if (!empty($currentValue) && !in_array($currentValue, $smartyChoices)) {
      $smartyChoices[] = $currentValue;
      $smartyOutputs[] = $currentValue;
    }

    $key = array_search('', $smartyOutputs, TRUE);
    if ($key !== FALSE) {
      $smartyOutputs[$key] = '&nbsp;';
    }
    $smarty->assign($id.'_choices',   $smartyChoices);
    $smarty->assign($id.'_outputs',   $smartyOutputs);
    $smarty->assign($id.'_selected',  $currentValue);
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
    if ($this->isSubAttribute) {
      $display .= 'class="subattribute" ';
    }
    $display .= '>';
    $display .= '{html_options values=$'.$id.'_choices output=$'.$id.'_outputs selected=$'.$id.'_selected}';
    $display .= '</select>';
    return $this->renderAcl($display);
  }

  function serializeAttribute (array &$attributes, bool $form = TRUE)
  {
    if (!$form || $this->visible) {
      parent::serializeAttribute($attributes, $form);

      if ($this->outputs !== NULL) {
        $tmpOutputs = array_values($this->outputs);
      } else {
        $tmpOutputs = $this->choices;
      }
      $attributes[$this->getLdapName()]['choices'] = array_combine($this->choices, $tmpOutputs);
    }
  }

  /*! \brief Set the size of the HTML input tag, useful to display several options on the screen instead of just one */
  function setSize ($size)
  {
    $this->size = $size;
  }

  function setParent (simplePlugin &$plugin)
  {
    parent::setParent($plugin);
    if ($this->isTemplate() && !in_array('%askme%', $this->choices)) {
      $this->choices[]          = '%askme%';
      if ($this->outputs !== NULL) {
        $this->outputs['%askme%'] = '%askme%';
      }
    }
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
    parent::__construct($label, $description, $ldapName, $required, [], "", NULL, $acl);
    $this->objectType   = $objectType;
    $this->objectAttrs  = $objectAttrs;
    $this->objectFilter = $objectFilter;
  }

  function setParent (simplePlugin &$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      $this->updateChoices();
    }
  }

  function updateChoices ()
  {
    $objects = objects::ls($this->objectType, $this->objectAttrs, NULL, $this->objectFilter);
    $this->setChoices(array_keys($objects), array_values($objects));
  }
}
