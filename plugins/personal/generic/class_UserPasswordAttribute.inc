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
  protected $previousMethod;

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "")
  {
    $temp = passwordMethod::get_available_methods();

    /* Create password methods array */
    $pwd_methods = array();
    $this->needPassword = array();
    foreach ($temp['name'] as $id => $name) {
      $this->needPassword[$name] = $temp[$id]['object']->need_password();
      $pwd_methods[$name] = $name;
      if (!empty($temp[$id]['desc'])) {
        $pwd_methods[$name] .= " (".$temp[$id]['desc'].")";
      }
    }

    parent::__construct (
      $description, $ldapName,
      array(
        new SelectAttribute(
          _('Password method'), _('Password hash method to use'),
          $ldapName.'_pwstorage', TRUE,
          array_keys($pwd_methods), '', array_values($pwd_methods)
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
        new HiddenAttribute(
          $ldapName.'_locked', FALSE,
          FALSE
        )
      ),
      '', '', $acl, $label
    );
    $this->attributes[0]->setSubmitForm(TRUE);
  }

  public function setParent(&$plugin)
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
      $this->checkIfMethodNeedsPassword();
    }
  }

  /* We need to handle method select disabling manually */
  function renderAttribute(array &$attributes, $readOnly)
  {
    global $config;
    if ($this->visible) {
      if ($this->linearRendering) {
        parent::renderAttribute($attributes, $readOnly);
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
          $attribute->renderAttribute($attributes, $readOnly);
        }
        unset($attribute);
      }
    }
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
      $this->checkIfMethodNeedsPassword();
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

  function checkIfMethodNeedsPassword()
  {
    $method = $this->attributes[0]->getValue();
    if ($method != $this->previousMethod) {
      if (isset($this->needPassword[$method]) && $this->needPassword[$method]) {
        $hashEmpty = ($this->attributes[3]->getValue() == '');
        $this->attributes[1]->setVisible(TRUE);
        $this->attributes[1]->setRequired($hashEmpty);
        $this->attributes[2]->setVisible(TRUE);
        $this->attributes[2]->setRequired($hashEmpty);
      } else {
        $this->attributes[1]->setRequired(FALSE);
        $this->attributes[1]->setVisible(FALSE);
        $this->attributes[1]->setValue('');
        $this->attributes[2]->setRequired(FALSE);
        $this->attributes[2]->setVisible(FALSE);
        $this->attributes[2]->setValue('');
      }
    }
    $this->previousMethod = $method;
  }

  function readValues($value)
  {
    global $config;
    $pw_storage = $config->get_cfg_value('passwordDefaultHash', 'ssha');
    $locked     = FALSE;
    $password   = '';
    if ($this->plugin->is_template && !empty($value)) {
      if ($value == '%askme%') {
        return array('%askme%', '', '', $value, $locked);
      }
      list($value, $password) = explode('|', $value, 2);
    }
    if (preg_match ('/^{[^}]+}/', $value)) {
      $tmp = passwordMethod::get_method($value);
      if (is_object($tmp)) {
        $pw_storage = $tmp->get_hash();
        $locked     = $tmp->is_locked($this->plugin->dn);
        if ($this->plugin->is_template) {
          $value = $tmp->generate_hash($password);
        }
      }
    } elseif ($value != '') {
      $pw_storage = 'clear';
    } elseif ($this->plugin->initially_was_account) {
      $pw_storage = 'empty';
    }
    return array($pw_storage, $password, $password, $value, $locked);
  }

  function writeValues(array $values)
  {
    if ($this->plugin->is_template && ($values[0] == '%askme%')) {
      return '%askme%';
    }
    if (!$this->plugin->is_template && ($this->needPassword[$values[0]] || ($values[0] == 'empty')) && ($values[1] == '')) {
      return $values[3];
    }
    $temp = passwordMethod::get_available_methods();
    if (!isset($temp[$values[0]])) {
      trigger_error('Unknown password method '.$values[0]);
      return $values[3];
    }
    $test = new $temp[$values[0]]($this->plugin->dn, $this->plugin);
    $test->set_hash($values[0]);
    if ($this->plugin->is_template) {
      return $test->generate_hash($values[1]).'|'.$values[1];
    } else {
      return $test->generate_hash($values[1]);
    }
  }

  function check()
  {
    $method = $this->attributes[0]->getValue();
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    }
    if (($this->attributes[1]->getValue() != '') || ($this->attributes[2]->getValue() != '')) {
      return user::reportPasswordProblems($this->plugin->dn, $this->attributes[1]->getValue(), $this->attributes[2]->getValue());
    }
  }

  function getMethod()
  {
    return $this->attributes[0]->getValue();
  }

  function getClear()
  {
    return $this->attributes[1]->getValue();
  }

  function isLocked()
  {
    return $this->attributes[4]->getValue();
  }
}
