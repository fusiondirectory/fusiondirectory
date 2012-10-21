<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012  FusionDirectory

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
  private $template;
  /*! \brief The constructor of MailAttribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param mixed $defaultValue The default value for this attribute
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   *  \param boolean $template Should this email field accept template character '%' ?
   */
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "", $template = FALSE)
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->template = $template;
  }
  function validate ()
  {
    if (!tests::is_email($this->value, $this->template)) {
      return msgPool::invalid($this->getLabel(), $this->value);
    }
  }
}

/*! \brief This class allow to handle easily a String LDAP attribute that contains a host name
 *
 */
class HostNameAttribute extends StringAttribute
{
  function validate ()
  {
    if (!tests::is_valid_hostname($this->value)) {
      return msgPool::invalid($this->getLabel(), $this->value);
    }
  }
}

/*! \brief This class allow to handle easily a String LDAP attribute that contains an IPv4
 *
 */
class IPv4Attribute extends StringAttribute
{
  function validate ()
  {
    if (!tests::is_ip($this->value)) {
      return msgPool::invalid($this->getLabel(), $this->value);
    }
  }
}

/*! \brief This class allow to handle easily a String LDAP attribute that contains a IPv6
 *
 */
class IPv6Attribute extends StringAttribute
{
  function validate ()
  {
    if (!tests::is_ipv6($this->value)) {
      return msgPool::invalid($this->getLabel(), $this->value);
    }
  }
}

/*! \brief This class allow to handle easily a String LDAP attribute that contains a mac address
 *
 */
class MacAddressAttribute extends StringAttribute
{
  function validate ()
  {
    if (!tests::is_mac($this->value)) {
      return msgPool::invalid($this->getLabel(), $this->value);
    }
  }
}

/*! \brief This class allow to handle easily a String LDAP attribute that contains an IP (v4 or v6)
 *
 */
class IPAttribute extends StringAttribute
{
  function validate ()
  {
    if (!tests::is_ip($this->value) && !tests::is_ipv6($this->value)) {
      return msgPool::invalid($this->getLabel(), $this->value);
    }
  }
}

?>
