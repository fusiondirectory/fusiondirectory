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

/*! \brief This class allow to handle easily a composite attribute
 *
 * That means this is only one attribute in the LDAP, but it is shown as several in the form.
 * If you need something else than scanf and printf for reading and writing the values (for instance if you want to do a addition of several int attributes),
 * you should inherit this class and write your own readValues and writeValues method
 *
 */
class CompositeAttribute extends \FusionDirectory\Core\SimplePlugin\Attribute
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
   *  \param ?string $label The label to show for this attribute. Only useful if you put this attribute inside a SetAttribute, or if you use a specific template that needs it.
   */
  function __construct (string $description, string $ldapName, array $attributes, $readFormat, $writeFormat, string $acl = '', string $label = NULL)
  {
    if ($label === NULL) {
      $label = $ldapName;
    }
    parent::__construct($label, $description, $ldapName, FALSE, '', $acl);
    $this->readFormat   = $readFormat;
    $this->writeFormat  = $writeFormat;
    $this->setAttributes($attributes);
  }

  function setAttributes (array $attributes)
  {
    $this->attributes   = $attributes;
    foreach ($this->attributes as &$attribute) {
      $attribute->setAcl($this->getAcl());
      $attribute->setParent($this->plugin);
    }
    unset($attribute);
  }

  function setIsSubAttribute (bool $bool)
  {
    parent::setIsSubAttribute($bool);
    foreach ($this->attributes as &$attribute) {
      $attribute->setIsSubAttribute($this->isSubAttribute);
    }
    unset($attribute);
  }

  function setAcl (string $acl)
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

  function setManagedAttributes (array $dontcare)
  {
    trigger_error('method setManagedAttributes is not supported for CompositeAttribute');
  }

  function setLinearRendering (bool $bool)
  {
    $this->linearRendering = $bool;
  }

  function loadValue (array $attrs)
  {
    parent::loadValue($attrs);
    foreach ($this->attributes as $attribute) {
      $attribute->setInitialValue($attribute->getValue());
    }
  }

  function readValues (string $value): array
  {
    $res = preg_match($this->readFormat, $value, $m);
    if ($res === 1) {
      $m = array_slice($m, 1);
      $values = [];
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

  function writeValues (array $values)
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
      $attribute->setDisabled($this->disabled);
      $attribute->loadPostValue();
    }
    unset($attribute);
  }

  function applyPostValue ()
  {
    foreach ($this->attributes as &$attribute) {
      $attribute->setDisabled($this->disabled);
      $attribute->applyPostValue();
    }
    unset($attribute);
  }

  function setValue ($values)
  {
    if (!is_array($values)) {
      $values = $this->inputValue($values);
    }
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

  function getArrayValue (): array
  {
    return array_map(
      function ($a)
      {
        return $a->displayValue($a->getValue());
      },
      $this->attributes
    );
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

  function renderAttribute (array &$attributes, bool $readOnly, bool $readable, bool $writable)
  {
    if ($this->visible) {
      if ($this->linearRendering) {
        parent::renderAttribute($attributes, $readOnly, $readable, $writable);
      } else {
        foreach ($this->attributes as &$attribute) {
          $attribute->setDisabled($this->disabled);
          $attribute->renderAttribute($attributes, $readOnly, $readable, $writable);
        }
        unset($attribute);
      }
    }
  }

  function getForHtmlId (): string
  {
    // Label (if any) should point to the first attribute
    if (isset($this->attributes[0])) {
      return $this->attributes[0]->getForHtmlId();
    } else {
      return '';
    }
  }

  function serializeAttribute (array &$attributes, bool $form = TRUE)
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
      $subattributes = [];
      foreach ($this->attributes as &$attribute) {
        $attribute->setDisabled($this->disabled);
        $attribute->serializeAttribute($subattributes, $form);
      }
      unset($attribute);
      $attributes[$this->getLdapName()]['attributes']       = $subattributes;
      $attributes[$this->getLdapName()]['attributes_order'] = array_keys($subattributes);
    }
  }

  function renderFormInput (): string
  {
    $display = "";
    foreach ($this->attributes as &$attribute) {
      $attribute->setDisabled($this->disabled);
      if ($attribute->isVisible()) {
        $display .= '<label for="'.$attribute->getForHtmlId().'" class="subattribute">'.$attribute->getLabel().'</label>'." ".$attribute->renderFormInput()." ";
      }
    }
    unset($attribute);
    return $display;
  }

  public function htmlIds (): array
  {
    $ret = [];
    foreach ($this->attributes as &$attribute) {
      $ret = array_merge($ret, $attribute->htmlIds());
    }
    unset($attribute);
    return $ret;
  }
}
