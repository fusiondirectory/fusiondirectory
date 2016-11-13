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

/*! \brief This class allow to handle easily a String LDAP attribute
 *
 */
class StringAttribute extends Attribute
{
  protected $pattern;
  protected $example;
  protected $autocomplete = NULL;

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
    if ($this->autocomplete !== NULL) {
      $attributes['autocomplete'] = ($this->autocomplete ? 'on' : 'off' );
    }
    $display  = $this->renderInputField('text', $id, $attributes);
    return $this->renderAcl($display);
  }

  function fixPostValue ($value)
  {
    /* Replace CRLF by LF, to avoid non-ASCII chars in multiline values (mainly useful for textarea) */
    return str_replace(array("\r\n", "\r"), "\n", $value);
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

  function setAutocomplete ($bool)
  {
    $this->autocomplete = $bool;
  }

  function getAutocomplete ()
  {
    return $this->autocomplete;
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
    if ($this->autocomplete === FALSE) {
      $display = '{literal}<input autocomplete="off" value="foolautocompleteworkaround" type="text" style="display:none;"/>{/literal}'.$display;
    }
    return $this->renderAcl($display);
  }

  function renderTemplateInput ()
  {
    return parent::renderFormInput();
  }
}

