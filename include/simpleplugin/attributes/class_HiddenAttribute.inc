<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2012-2021  FusionDirectory

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
 * \brief Attribute hidden from the user
 */
class HiddenAttribute extends \FusionDirectory\Core\SimplePlugin\Attribute
{
  /*! \brief The constructor of HiddenAttribute
   *
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param mixed $defaultValue The default value for this attribute
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   */
  function __construct (string $ldapName, bool $required = FALSE, $defaultValue = '', string $acl = '', string $label = NULL, string $description = 'hidden')
  {
    if ($label === NULL) {
      $label = $ldapName;
    }
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setVisible(FALSE);
  }
}

/*!
 * \brief HiddenAttribute with several values
 */
class HiddenArrayAttribute extends HiddenAttribute
{
  protected function loadAttrValue (array $attrs)
  {
    if (isset($attrs[$this->getLdapName()]['count'])) {
      $this->value = [];
      for ($i = 0; $i < $attrs[$this->getLdapName()]['count']; $i++) {
        $this->value[] = $attrs[$this->getLdapName()][$i];
      }
    }
  }

  public function computeLdapValue ()
  {
    if (is_array($this->value)) {
      return array_values($this->value);
    } else {
      return parent::computeLdapValue();
    }
  }
}
