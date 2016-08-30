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

/*! \brief This class allow to handle easily a String LDAP attribute that contains a mail address
 *
 */
class MailAttribute extends StringAttribute
{
  function validate ()
  {
    if (!tests::is_email($this->value)) {
      return msgPool::invalid($this->getLabel(), $this->value);
    }
  }
}


/*! \brief This class allow to handle easily a String LDAP attribute that needs to be validated by a test from class tests
 *
 */
class TestValidateAttribute extends StringAttribute
{
  protected $testFunc = '';

  function validate ()
  {
    $testFunc = $this->testFunc;
    if (!tests::$testFunc($this->value)) {
      return msgPool::invalid($this->getLabel(), $this->value);
    }
  }
}

/*! \brief This class allow to handle easily a String LDAP attribute that contains a host name
 *
 */
class HostNameAttribute extends TestValidateAttribute
{
  protected $testFunc = 'is_valid_hostname';
}

/*! \brief This class allow to handle easily a String LDAP attribute that contains an uid
 *
 */
class UidAttribute extends TestValidateAttribute
{
  protected $testFunc = 'is_uid';
}

/*! \brief This class allow to handle easily a String LDAP attribute that contains a path
 *
 */
class PathAttribute extends TestValidateAttribute
{
  protected $testFunc = 'is_path';
}

/*! \brief This class allow to handle easily a String LDAP attribute that contains an URL
 *
 */
class URLAttribute extends TestValidateAttribute
{
  protected $testFunc = 'is_url';
}

/*! \brief This class allow to handle easily a String LDAP attribute that contains a path
 *
 */
class PhoneNumberAttribute extends TestValidateAttribute
{
  protected $testFunc = 'is_phone_nr';
}

/*! \brief This class allow to handle easily a String LDAP attribute that contains an IP (v4 or v6)
 *
 */
class IPAttribute extends TestValidateAttribute
{
  protected $testFunc = 'is_ip';
}

/*! \brief This class allow to handle easily a String LDAP attribute that contains an IPv4
 *
 */
class IPv4Attribute extends TestValidateAttribute
{
  protected $testFunc = 'is_ipv4';
}

/*! \brief This class allow to handle easily a String LDAP attribute that contains a IPv6
 *
 */
class IPv6Attribute extends TestValidateAttribute
{
  protected $testFunc = 'is_ipv6';
}

/*! \brief This class allow to handle easily a String LDAP attribute that contains a mac address
 *
 */
class MacAddressAttribute extends StringAttribute
{
  function setValue ($value)
  {
    return parent::setValue(strtolower($value));
  }

  function validate ()
  {
    if (!tests::is_mac($this->value)) {
      return msgPool::invalid($this->getLabel(), $this->value);
    }
  }
}

class CharSeparatedCompositeAttribute extends CompositeAttribute
{
  private $sep;

  function __construct($description, $ldapName, $attributes, $sep, $acl = "", $label = "Composite attribute")
  {
    parent::__construct($description, $ldapName, $attributes, '', '', $acl, $label);
    $this->sep = $sep;
  }

  function readValues($value)
  {
    return explode($this->sep, $value);
  }

  function writeValues($values)
  {
    return join($this->sep, $values);
  }
}

class PipeSeparatedCompositeAttribute extends CharSeparatedCompositeAttribute
{
  function __construct($description, $ldapName, $attributes, $acl = "", $label = "Composite attribute")
  {
    parent::__construct($description, $ldapName, $attributes, '|', $acl, $label);
  }
}

class CommaListAttribute extends CompositeAttribute
{
  private $sep = ',';

  function __construct($ldapName, $attribute, $sep = ',')
  {
    parent::__construct(
      $attribute->getDescription(), $ldapName,
      array(
        $attribute
      ), '', ''
    );
    $this->sep = $sep;
  }

  function readValues($value)
  {
    if ($value == '') {
      return array(array());
    } else {
      return array(explode($this->sep, $value));
    }
  }

  function writeValues($values)
  {
    return join($this->sep, $values[0]);
  }
}

class UnitIntAttribute extends CompositeAttribute
{
  /* $units should contains 1 in its keys as a fallback unit */
  function __construct ($label, $description, $ldapName, $required, $units, $min = FALSE, $max = FALSE, $defaultValue = "", $acl = "")
  {
    parent::__construct($description, $ldapName,
      array(
        new IntAttribute('', '', $ldapName.'_int', $required, $min, $max, $defaultValue),
        new SelectAttribute('', '', $ldapName.'_select', TRUE, array_keys($units), '', array_values($units)),
      ), '', '',
      $acl, $label
    );
    $this->setLinearRendering(TRUE);
    $this->setRequired($required); /* As we use linear rendering we want the asterisk in the label if required */
  }

  function readValues($value)
  {
    if ($value === '') {
      return array('', 0);
    }
    $choices = $this->attributes[1]->getChoices();
    sort($choices);
    if ($value == 0) {
      return array($value, $choices[0]);
    }
    /* Find the appropriate unit */
    for (
      $i = (count($choices) - 1);
      ($value % $choices[$i] != 0) && ($i > 0);
      $i--
    );
    return array($value / $choices[$i], $choices[$i]);
  }

  function writeValues($values)
  {
    if ($values[0] === '') {
      return '';
    }
    return $values[0] * $values[1];
  }

  function displayValue($value)
  {
    $values = $this->readValues($value);
    return $values[0].$this->attributes[1]->displayValue($values[1]);
  }
}

class BytesSizeAttribute extends UnitIntAttribute
{
  function __construct ($label, $description, $ldapName, $required, $min = FALSE, $max = FALSE, $defaultValue = "", $acl = "")
  {
    $units = array(
      1             => _('B'),
      pow(1024, 1)  => _('KiB'),
      pow(1024, 2)  => _('MiB'),
      pow(1024, 3)  => _('GiB'),
      pow(1024, 4)  => _('TiB'),
    );
    parent::__construct ($label, $description, $ldapName, $required, $units, $min, $max, $defaultValue, $acl);
  }
}

class TimeAttribute extends UnitIntAttribute
{
  function __construct ($label, $description, $ldapName, $required, $min = FALSE, $max = FALSE, $defaultValue = "", $acl = "")
  {
    $units = array(
      1     => _('seconds'),
      60    => _('minutes'),
      3600  => _('hours'),
      86400 => _('days'),
    );
    parent::__construct ($label, $description, $ldapName, $required, $units, $min, $max, $defaultValue, $acl);
  }
}

?>
