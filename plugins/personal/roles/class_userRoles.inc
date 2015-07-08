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
      'plShortName'     => _('Groups and roles'),
      'plDescription'   => _('Edit user\'s groups and roles'),
      'plIcon'          => 'geticon.php?context=types&icon=role&size=48',
      'plSmallIcon'     => 'geticon.php?context=types&icon=role&size=16',
      'plSelfModify'    => FALSE,
      'plObjectType'    => array('user'),
      'plPriority'      => 5,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'groups' => array(
        'name'  => _('Groups membership'),
        'attrs' => array(
          new SetAttribute(
            new SelectAttribute(
              '', _('Groups membership'),
              'groupsMembership', FALSE
            )
          )
        )
      ),
      'roles' => array(
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

    /* Groups handling */
    $groups    = objects::ls('ogroup');
    $this->attributesAccess['groupsMembership']->setInLdap(FALSE);
    $this->attributesAccess['groupsMembership']->attribute->setChoices(array_keys($groups), array_values($groups));
    if ($this->is_template) {
      if (isset($this->attrs['userGroups'])) {
        unset($this->attrs['userGroups']['count']);
        $this->groupsMembership = $this->attrs['userGroups'];
      }
    } else {
      $myGroups  = objects::ls(
        'ogroup', NULL, NULL,
        '(member='.$this->dn.')'
      );
      $this->attributesAccess['groupsMembership']->setValue(array_keys($myGroups));
      $this->savedGroupsMembership = array_keys($myGroups);
    }

    /* Roles handling */
    $roles    = objects::ls('role');
    $this->attributesAccess['rolesMembership']->setInLdap(FALSE);
    $this->attributesAccess['rolesMembership']->attribute->setChoices(array_keys($roles), array_values($roles));
    if ($this->is_template) {
      if (isset($this->attrs['userRoles'])) {
        unset($this->attrs['userRoles']['count']);
        $this->rolesMembership = $this->attrs['userRoles'];
      }
    } else {
      $myRoles  = objects::ls(
        'role', NULL, NULL,
        '(roleOccupant='.$this->dn.')'
      );
      $this->attributesAccess['rolesMembership']->setValue(array_keys($myRoles));
      $this->savedRolesMembership = array_keys($myRoles);
    }

    $this->updateAttributesValues();
  }

  function is_this_account($attrs)
  {
    if (!isset($attrs['dn'])) {
      if (isset($attrs['userGroups'])) { /* Template groups field */
        return TRUE;
      }
      if (isset($attrs['userRoles'])) { /* Template roles field */
        return TRUE;
      }
      return FALSE;
    }
    return ((objects::count('role', NULL, '(roleOccupant='.$attrs['dn'].')') > 0) ||
            (objects::count('ogroup', NULL, '(member='.$attrs['dn'].')') > 0));
  }

  function remove_from_parent()
  {
    /* Remove all groups */
    foreach ($this->savedGroupsMembership as $ogroupdn) {
      $g = objects::open($ogroupdn, 'ogroup');
      $g->getBaseObject()->attributesAccess['member']->searchAndRemove($this->dn);
      $g->save();
    }
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
    $this->is_account = ((count($this->rolesMembership) > 0) || (count($this->groupsMembership) > 0));
  }

  function prepare_save()
  {
    parent::prepare_save();
    if ($this->is_template) {
      $this->attrs['userGroups'] = $this->groupsMembership;
      $this->attrs['userRoles'] = $this->rolesMembership;
    }
  }

  function ldap_save($cleanup = TRUE)
  {
    parent::ldap_save($cleanup);

    if (!$this->is_template) {
      /* Take care about groupsMembership values: add to groups */
      $groupsMembership = $this->attributesAccess['groupsMembership']->getValue();
      foreach ($groupsMembership as $ogroupdn) {
        if (!in_array($ogroupdn, $this->savedGroupsMembership)) {
          $g = objects::open($ogroupdn, 'ogroup');
          $g->getBaseObject()->attributesAccess['member']->addValue($this->dn, array('cn' => 'user'));
          $g->save();
        }
      }

      /* Remove groups not listed in groupsMembership */
      foreach ($this->savedGroupsMembership as $ogroupdn) {
        if (!in_array($ogroupdn, $groupsMembership)) {
          $g = objects::open($ogroupdn, 'ogroup');
          $g->getBaseObject()->attributesAccess['member']->searchAndRemove($this->dn);
          $g->save();
        }
      }

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

  function adapt_from_template($attrs, $skip = array())
  {
    parent::adapt_from_template($attrs, $skip);

    $this->savedGroupsMembership = $this->groupsMembership;
    if (isset($this->attrs['userGroups'])) {
      unset($this->attrs['userGroups']['count']);
      $this->groupsMembership = $this->attrs['userGroups'];
    } else {
      $this->groupsMembership = array();
    }

    $this->savedRolesMembership = $this->rolesMembership;
    if (isset($this->attrs['userRoles'])) {
      unset($this->attrs['userRoles']['count']);
      $this->rolesMembership = $this->attrs['userRoles'];
    } else {
      $this->rolesMembership = array();
    }
  }
}
