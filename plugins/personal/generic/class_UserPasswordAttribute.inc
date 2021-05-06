<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2018  FusionDirectory

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

/* Handle a password and its hash method */
class UserPasswordAttribute extends CompositeAttribute
{
  protected $needPassword;
  protected $previousMethodInfo = NULL;

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "")
  {
    global $config;

    $temp = passwordMethod::get_available_methods();

    /* Create password methods array */
    $pwd_methods        = $config->get_cfg_value('passwordAllowedHashes', $temp['name']);
    $this->needPassword = [];
    foreach ($temp['name'] as $id => $name) {
      $this->needPassword[$name] = $temp[$id]['object']->need_password();
    }

    parent::__construct(
      $description, $ldapName,
      [
        new SelectAttribute(
          _('Password method'), _('Password hash method to use'),
          $ldapName.'_pwstorage', TRUE,
          $pwd_methods
        ),
        new PasswordAttribute(
          _('Password'), _('Password (Leave empty if you do not wish to change it)'),
          $ldapName.'_password', $required
        ),
        new PasswordAttribute(
          _('Password again'), _('Same password as above, to avoid errors'),
          $ldapName.'_password2', $required
        ),
        new HiddenAttribute(
          $ldapName.'_hash'
        ),
        new BooleanAttribute(
          /* Label/Description will only be visible for templates */
          _('Locked'), _('Whether accounts created with this template will be locked'),
          $ldapName.'_locked', FALSE,
          FALSE
        )
      ],
      '', '', $acl, $label
    );
    $this->attributes[0]->setSubmitForm(TRUE);
    $this->attributes[4]->setTemplatable(FALSE);
  }

  public function setParent (&$plugin)
  {
    global $config;
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      $hash = $config->get_cfg_value('passwordDefaultHash', 'ssha');
      $this->attributes[0]->setDefaultValue($hash);
      if ($config->get_cfg_value('forcePasswordDefaultHash', 'FALSE') == 'TRUE') {
        $this->attributes[0]->setValue($hash);
        $this->attributes[0]->setDisabled(TRUE);
      }
      if (!$this->plugin->is_template) {
        $this->attributes[4]->setVisible(FALSE);
      }
      $this->checkIfMethodNeedsPassword();
    }
  }

  /* We need to handle method select disabling manually */
  function renderAttribute (array &$attributes, bool $readOnly, bool $readable, bool $writable)
  {
    global $config;
    if ($this->visible) {
      if ($this->linearRendering) {
        parent::renderAttribute($attributes, $readOnly, $readable, $writable);
      } else {
        foreach ($this->attributes as $key => &$attribute) {
          if (is_object($this->plugin) && $this->plugin->is_template && ($key == 2)) {
            /* Do not display confirmation field in template mode */
            continue;
          }
          if (($key == 0) && ($config->get_cfg_value('forcePasswordDefaultHash', 'FALSE') == 'TRUE')) {
            $attribute->setDisabled(TRUE);
          } else {
            $attribute->setDisabled($this->disabled);
          }
          $attribute->renderAttribute($attributes, $readOnly, $readable, $writable);
        }
        unset($attribute);
      }
    }
  }

  function loadValue (array $attrs)
  {
    parent::loadValue($attrs);
    /* Needs to reset previousMethodInfo to force check, because initialValue of attribute 0 changed */
    $this->previousMethodInfo = NULL;
    $this->checkIfMethodNeedsPassword();
  }

  /*! \brief Loads this attribute value from the attrs array
   */
  protected function loadAttrValue (array $attrs)
  {
    if (isset($attrs[$this->getLdapName()])) {
      $this->setValue($this->inputValue($attrs[$this->getLdapName()][0]));
    } elseif ($this->plugin->initially_was_account) {
      $this->setValue($this->inputValue(''));
    } else {
      $this->attributes[0]->resetToDefault();
    }
  }

  function setValue ($value)
  {
    if (!is_array($value)) {
      $value = $this->inputValue($value);
    }
    reset($value);
    $key = key($value);
    if ($this->attributes[0]->isDisabled() || ($value[$key] == '')) {
      $value[$key] = $this->attributes[0]->getValue();
    }
    parent::setValue($value);
    $this->checkIfMethodNeedsPassword();
  }

  function applyPostValue ()
  {
    parent::applyPostValue();
    $this->checkIfMethodNeedsPassword();
  }

  function checkIfMethodNeedsPassword ()
  {
    $method     = $this->attributes[0]->getValue();
    $methodInfo = $method.$this->attributes[3]->getValue();
    if ($methodInfo != $this->previousMethodInfo) {
      if (isset($this->needPassword[$method]) && $this->needPassword[$method]) {
        $hashEmptyOrMethodChange = (($this->attributes[3]->getValue() == '') || $this->attributes[0]->hasChanged());
        $this->attributes[1]->setVisible(TRUE);
        $this->attributes[1]->setRequired($hashEmptyOrMethodChange);
        $this->attributes[2]->setVisible(TRUE);
        $this->attributes[2]->setRequired($hashEmptyOrMethodChange);
      } else {
        $this->attributes[1]->setRequired(FALSE);
        $this->attributes[1]->setVisible(FALSE);
        $this->attributes[1]->setValue('');
        $this->attributes[2]->setRequired(FALSE);
        $this->attributes[2]->setVisible(FALSE);
        $this->attributes[2]->setValue('');
      }
    }
    $this->previousMethodInfo = $methodInfo;
  }

  function readValues (string $value): array
  {
    return $this->readUserPasswordValues($value, $this->plugin->is_template);
  }

  function readUserPasswordValues ($value, $istemplate)
  {
    global $config;
    $pw_storage = $config->get_cfg_value('passwordDefaultHash', 'ssha');
    $locked     = FALSE;
    $password   = '';
    if (!empty($value) || ($this->plugin->initially_was_account && !$istemplate)) {
      if ($istemplate) {
        if ($value == '%askme%') {
          return ['%askme%', '', '', $value, 'FALSE'];
        }
        $parts = explode('|', $value, 3);
        if ((count($parts) < 3) || !in_array($parts[1], ['TRUE','FALSE'])) {
          /* Old format from FD<1.4 */
          list($value, $password) = $parts;
        } else {
          list($pw_storage, $locked, $password) = $parts;
          return [$pw_storage, $password, $password, $this->attributes[3]->getValue(), $locked];
        }
      }
      $tmp = passwordMethod::get_method($value);
      if (is_object($tmp)) {
        $pw_storage = $tmp->get_hash();
        $locked     = $tmp->is_locked('', $value);
      }
      if ($istemplate && empty($password)) {
        /* Do not store hash for templates,
         * we have the password anyway, and this avoids problems with empty passwords */
        $value = $this->attributes[3]->getValue();
      }
    }
    return [$pw_storage, $password, $password, $value, ($locked ? 'TRUE' : 'FALSE')];
  }

  function writeValues (array $values)
  {
    if ($this->plugin->is_template && ($values[0] == '%askme%')) {
      return '%askme%';
    }
    if (!$this->plugin->is_template && (!$this->attributes[0]->hasChanged()) && ($this->needPassword[$values[0]] || ($values[0] == 'empty')) && ($values[1] == '')) {
      return $values[3];
    }
    $temp = passwordMethod::get_available_methods();
    if (!isset($temp[$values[0]])) {
      trigger_error('Unknown password method '.$values[0]);
      return $values[3];
    }
    if ($this->plugin->is_template) {
      return $values[0].'|'.$values[4].'|'.$values[1];
    } else {
      $test = new $temp[$values[0]]($this->plugin->dn, $this->plugin);
      $test->set_hash($values[0]);
      return $test->generate_hash($values[1]);
    }
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    }
    if (($this->attributes[1]->getValue() != '') || ($this->attributes[2]->getValue() != '')) {
      $error = user::reportPasswordProblems($this->plugin->dn, $this->attributes[1]->getValue(), $this->attributes[2]->getValue());
      if ($error !== FALSE) {
        return new SimplePluginCheckError(
          $this,
          htmlescape($error)
        );
      }
    }
  }

  function getMethod ()
  {
    return $this->attributes[0]->getValue();
  }

  function getClear ()
  {
    return $this->attributes[1]->getValue();
  }

  function isLocked ()
  {
    return $this->attributes[4]->getValue();
  }

  /*! \brief Apply value from RPC requests
   *
   *  \param mixed $value the value
   */
  function deserializeValue ($value)
  {
    if ($this->disabled) {
      return parent::deserializeValue($value);
    }
    if (is_array($value)) {
      if (count($value) > 5) {
        return new SimplePluginError(
          $this,
          htmlescape(sprintf(_('Too many elements in array value: %d instead of %d'), $this->getLdapName(), count($value), 5))
        );
      } elseif (count($value) < 5) {
        return new SimplePluginError(
          $this,
          htmlescape(sprintf(_('Not enough elements in array value: %d instead of %d'), $this->getLdapName(), count($value), 5))
        );
      } elseif (!isset($value[0])) {
        return new SimplePluginError(
          $this,
          htmlescape(_('Array value for password field must have numeric keys'))
        );
      }
      $this->setValue($value);
    } elseif (is_string($value)) {
      $this->setValue(['', $value, $value, '', FALSE]);
    } else {
      return new SimplePluginError(
        $this,
        htmlescape(_('Invalid value type for password field, must be array or string'))
      );
    }
  }
}
