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

/*! \brief This class allow to handle an attribute that stores flags based on other BooleanAttributes
 *
 */
class FlagsAttribute extends StringAttribute
{
  protected $flags;

  /*! \brief The constructor of IntAttribute
   *
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param array  $flags The ids of the BooleanAttributes to use as flags
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct ($ldapName, $flags, $acl = "")
  {
    parent::__construct('', '', $ldapName, FALSE, '', $acl);
    $this->setVisible(FALSE);
    $this->flags = $flags;
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      foreach ($this->flags as $attr) {
        $this->plugin->attributesAccess[$attr]->setInLdap(FALSE);
      }
    }
  }

  function setValue ($value)
  {
    parent::setValue($value);
    if (is_object($this->plugin)) {
      foreach ($this->flags as $attr) {
        $trueValue = $this->plugin->attributesAccess[$attr]->trueValue;
        $this->plugin->attributesAccess[$attr]->setValue(preg_match("/$trueValue/", $this->value));
      }
    }
  }

  function getValue()
  {
    $value = '[';
    if (is_object($this->plugin)) {
      foreach ($this->flags as $attr) {
        $value .= $this->plugin->attributesAccess[$attr]->computeLdapValue();
      }
    }
    $value .= ']';
    return $value;
  }
}
