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

/*! \brief This class allow to display an attribute.
 *
 * It can be used to display an attribute value the user is never allowed to modify.
 * (But FD might edit it)
 */
class DisplayLDAPAttribute extends \FusionDirectory\Core\SimplePlugin\Attribute
{
  protected $allowHTML    = FALSE;
  protected $allowSmarty  = FALSE;

  function renderFormInput (): string
  {
    if ($this->allowHTML) {
      $value = $this->getValue();
    } else {
      $value = htmlescape($this->getValue());
    }
    if ($this->allowSmarty) {
      return $value;
    } else {
      return '{literal}'.$value.'{/literal}';
    }
  }

  function setAllowHTML (bool $allowHTML)
  {
    $this->allowHTML = $allowHTML;
  }

  function setAllowSmarty (bool $allowSmarty)
  {
    $this->allowSmarty = $allowSmarty;
  }
}

/*! \brief This class allow to display an attribute.
 *
 * It can be used to display an attribute value the user and FD are never allowed to modify.
 */
class ReadOnlyLDAPAttribute extends DisplayLDAPAttribute
{
  function fillLdapValue (array &$attrs)
  {
  }
}

/*! \brief This class allow to display an attribute.
 *
 * It can be used to display an attribute value the user is never allowed to modify.
 */
class DisplayLDAPArrayAttribute extends \FusionDirectory\Core\SimplePlugin\Attribute
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

  function renderFormInput (): string
  {
    $value = $this->getValue();
    if (is_array($value)) {
      $value = join(', ', $value);
    }
    return '{literal}'.htmlescape($value).'{/literal}';
  }
}

/*! \brief This class allow to display a text in front of an attribute.
 *
 * For instance, it can be used to display a link.
 */
class DisplayAttribute extends DisplayLDAPAttribute
{
  function __construct (string $label, string $description, string $ldapName, bool $required = FALSE, $defaultValue = '', string $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setInLdap(FALSE);
  }
}

/*! \brief This class allow to display a link to an object which dn is stored in the attribute
 */
class ObjectLinkAttribute extends DisplayLDAPAttribute
{
  protected $type;

  function __construct (string $label, string $description, string $ldapName, bool $required, string $type, $defaultValue = '', string $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->type = $type;
  }

  function renderFormInput (): string
  {
    if (empty($this->value)) {
      return '';
    }

    try {
      return objects::link($this->value, $this->type);
    } catch (NonExistingLdapNodeException $e) {
      return '<a><img src="geticon.php?context=status&amp;icon=dialog-warning&amp;size=16" alt="warning" class="center"/>&nbsp;'.sprintf(_('Invalid: %s'), $this->value).'</a>';
    }
  }
}
