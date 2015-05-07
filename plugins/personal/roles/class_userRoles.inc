<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014-2015  FusionDirectory

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

class userRoles extends simplePlugin
{
  var $objectclasses = array();

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Roles'),
      'plDescription'   => _('Edit user\'s roles'),
      'plIcon'          => 'geticon.php?context=types&icon=role&size=48',
      'plSmallIcon'     => 'geticon.php?context=types&icon=role&size=16',
      'plSelfModify'    => FALSE,
      'plObjectType'    => array('user'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Roles membership'),
        'attrs' => array(
          new SetAttribute(
            new SelectAttribute(
              '', _('Roles membership'),
              'rolesMembership', FALSE
            )
          )
        )
      ),
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->ignore_account = FALSE;

    /* Roles handling */
    $roles    = objects::ls('role');
    $myRoles  = objects::ls(
      'role', NULL, NULL,
      '(roleOccupant='.$this->dn.')'
    );
    $this->attributesAccess['rolesMembership']->setInLdap(FALSE);
    $this->attributesAccess['rolesMembership']->attribute->setChoices(array_keys($roles), array_values($roles));
    $this->attributesAccess['rolesMembership']->setValue(array_keys($myRoles));
    $this->savedRolesMembership = array_keys($myRoles);

    $this->updateAttributesValues();
  }

  function is_this_account($attrs)
  {
    return (objects::count('role', NULL, '(roleOccupant='.$attrs['dn'].')') > 0);
  }

  function remove_from_parent()
  {
    /* Remove all roles */
    foreach ($this->savedRolesMembership as $roledn) {
      $r = objects::open($roledn, 'role');
      $r->getBaseObject()->attributesAccess['roleOccupant']->searchAndRemove($this->dn);
      $r->save();
    }
  }

  function save_object()
  {
    parent::save_object();
    $this->is_account = (count($this->rolesMembership) > 0);
  }

  function ldap_save($cleanup = TRUE)
  {
    parent::ldap_save($cleanup);

    /* Take care about rolesMembership values: add to roles */
    $rolesMembership = $this->attributesAccess['rolesMembership']->getValue();
    foreach ($rolesMembership as $roledn) {
      if (!in_array($roledn, $this->savedRolesMembership)) {
        $r = objects::open($roledn, 'role');
        $r->getBaseObject()->attributesAccess['roleOccupant']->addValue($this->dn, array('cn' => 'user'));
        $r->save();
      }
    }

    /* Remove roles not listed in rolesMembership */
    foreach ($this->savedRolesMembership as $roledn) {
      if (!in_array($roledn, $rolesMembership)) {
        $r = objects::open($roledn, 'role');
        $r->getBaseObject()->attributesAccess['roleOccupant']->searchAndRemove($this->dn);
        $r->save();
      }
    }
  }
}
