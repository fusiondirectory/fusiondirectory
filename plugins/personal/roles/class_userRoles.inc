<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2014-2016  FusionDirectory

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

  protected $savedGroupsMembership  = array();
  protected $savedRolesMembership   = array();

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

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
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

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->ignore_account = FALSE;

    /* Groups handling */
    $groups      = array();
    $groupsattrs = objects::ls('ogroup', array('cn' => 1,  'description' => 1), NULL, '', TRUE);
    foreach ($groupsattrs as $dn => $groupattr) {
      $groupDisplay = $groupattr['cn'];
      if (isset($groupattr['description'])) {
        if (strlen($groupattr['description']) > 50) {
          $groupattr['description'] = substr($groupattr['description'], 0, 50).'…';
        }
        $groupDisplay .= ' ['.$groupattr['description'].']';
      }
      $groups[$dn] = $groupDisplay;
    }
    $this->attributesAccess['groupsMembership']->setInLdap(FALSE);
    if ($this->is_template) {
      if (isset($this->attrs['userGroups'])) {
        unset($this->attrs['userGroups']['count']);
        $myGroups = array_combine($this->attrs['userGroups'], $this->attrs['userGroups']);
      } else {
        $myGroups = array();
      }
    } else {
      if (($this->dn != '') && ($this->dn != 'new')) {
        $myGroups  = objects::ls(
          'ogroup', NULL, NULL,
          '(member='.$this->dn.')'
        );
      } else {
        $myGroups = array();
      }
      $this->savedGroupsMembership = array_keys($myGroups);
    }
    /* We might be in some groups we have no read ACL on, so merge them in the choices */
    $groups = array_merge($myGroups, $groups);
    $this->attributesAccess['groupsMembership']->attribute->setChoices(array_keys($groups), array_values($groups));
    $this->attributesAccess['groupsMembership']->setValue(array_keys($myGroups));

    /* Roles handling */
    $roles        = array();
    $rolesattrs  = objects::ls('role', array('cn' => 1,  'description' => 1), NULL, '', TRUE);
    foreach ($rolesattrs as $dn => $roleattr) {
      $roleDisplay = $roleattr['cn'];
      if (isset($roleattr['description'])) {
        if (strlen($roleattr['description']) > 50) {
          $roleattr['description'] = substr($roleattr['description'], 0, 50).'…';
        }
        $roleDisplay .= ' ['.$roleattr['description'].']';
      }
      $roles[$dn] = $roleDisplay;
    }
    $this->attributesAccess['rolesMembership']->setInLdap(FALSE);
    if ($this->is_template) {
      if (isset($this->attrs['userRoles'])) {
        unset($this->attrs['userRoles']['count']);
        $myRoles = array_combine($this->attrs['userRoles'], $this->attrs['userRoles']);
      } else {
        $myRoles = array();
      }
    } else {
      if (($this->dn != '') && ($this->dn != 'new')) {
        $myRoles  = objects::ls(
          'role', NULL, NULL,
          '(roleOccupant='.$this->dn.')'
        );
      } else {
        $myRoles = array();
      }
      $this->savedRolesMembership = array_keys($myRoles);
    }
    /* We might be in some roles we have no read ACL on, so merge them in the choices */
    $roles = array_merge($myRoles, $roles);
    $this->attributesAccess['rolesMembership']->attribute->setChoices(array_keys($roles), array_values($roles));
    $this->attributesAccess['rolesMembership']->setValue(array_keys($myRoles));

    $this->is_account = ((count($this->rolesMembership) > 0) || (count($this->groupsMembership) > 0));
    $this->initially_was_account = $this->is_account;
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
    return FALSE;
  }

  function remove_from_parent()
  {
    if (!$this->is_template && ($this->dn != '') && ($this->dn != 'new')) {
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
  }

  function save_object()
  {
    parent::save_object();
    $this->is_account = ((count($this->rolesMembership) > 0) || (count($this->groupsMembership) > 0));
  }

  protected function shouldSave()
  {
    if (parent::shouldSave()) {
      return TRUE;
    }
    if (array_differs($this->groupsMembership, $this->savedGroupsMembership)) {
      return TRUE;
    }
    if (array_differs($this->rolesMembership, $this->savedRolesMembership)) {
      return TRUE;
    }
    return FALSE;
  }

  function prepare_save()
  {
    parent::prepare_save();
    if ($this->is_template) {
      $this->attrs['userGroups']  = $this->groupsMembership;
      $this->attrs['userRoles']   = $this->rolesMembership;
    }
  }

  function ldap_save()
  {
    $errors = parent::ldap_save();

    if (!empty($errors)) {
      return $errors;
    }

    if (!$this->is_template) {
      /* We need to give an array which looks like an ldap fetch and match user filter */
      $fake_attrs = array(
        'objectClass' => array('inetOrgPerson','organizationalPerson','person'),
        'cn'          => array($this->dn),
        'uid'         => array($this->dn),
        'dn'          => $this->dn
      );

      /* Take care about groupsMembership values: add to groups */
      $groupsMembership = $this->attributesAccess['groupsMembership']->getValue();
      foreach ($groupsMembership as $ogroupdn) {
        if (!in_array($ogroupdn, $this->savedGroupsMembership)) {
          $g = objects::open($ogroupdn, 'ogroup');
          if (!$g->getBaseObject()->attrIsWriteable('member')) {
            $errors[] = msgPool::permModify($ogroupdn, 'member');
            continue;
          }
          $g->getBaseObject()->attributesAccess['member']->addValue($this->dn, $fake_attrs);
          $msg = $g->save();
          if (empty($msg)) {
            $this->savedGroupsMembership[] = $ogroupdn;
          } else {
            $errors = array_merge($errors, $msg);
          }
        }
      }

      /* Remove groups not listed in groupsMembership */
      foreach ($this->savedGroupsMembership as $key => $ogroupdn) {
        if (!in_array($ogroupdn, $groupsMembership)) {
          $g = objects::open($ogroupdn, 'ogroup');
          if (!$g->getBaseObject()->attrIsWriteable('member')) {
            $errors[] = msgPool::permModify($ogroupdn, 'member');
            continue;
          }
          $g->getBaseObject()->attributesAccess['member']->searchAndRemove($this->dn);
          $msg = $g->save();
          if (empty($msg)) {
            unset($this->savedGroupsMembership[$key]);
          } else {
            $errors = array_merge($errors, $msg);
          }
        }
      }

      /* Take care about rolesMembership values: add to roles */
      $rolesMembership = $this->attributesAccess['rolesMembership']->getValue();
      foreach ($rolesMembership as $roledn) {
        if (!in_array($roledn, $this->savedRolesMembership)) {
          $r = objects::open($roledn, 'role');
          if (!$r->getBaseObject()->attrIsWriteable('roleOccupant')) {
            $errors[] = msgPool::permModify($roledn, 'roleOccupant');
            continue;
          }
          $r->getBaseObject()->attributesAccess['roleOccupant']->addValue($this->dn, $fake_attrs);
          $msg = $r->save();
          if (empty($msg)) {
            $this->savedRolesMembership[] = $roledn;
          } else {
            $errors = array_merge($errors, $msg);
          }
        }
      }

      /* Remove roles not listed in rolesMembership */
      foreach ($this->savedRolesMembership as $key => $roledn) {
        if (!in_array($roledn, $rolesMembership)) {
          $r = objects::open($roledn, 'role');
          if (!$r->getBaseObject()->attrIsWriteable('roleOccupant')) {
            $errors[] = msgPool::permModify($roledn, 'roleOccupant');
            continue;
          }
          $r->getBaseObject()->attributesAccess['roleOccupant']->searchAndRemove($this->dn);
          $msg = $r->save();
          if (empty($msg)) {
            unset($this->savedRolesMembership[$key]);
          } else {
            $errors = array_merge($errors, $msg);
          }
        }
      }
    }

    return $errors;
  }

  function adapt_from_template($attrs, $skip = array())
  {
    parent::adapt_from_template($attrs, $skip);

    $this->savedGroupsMembership = $this->groupsMembership;
    if (isset($this->attrs['userGroups'])) {
      unset($this->attrs['userGroups']['count']);
      $this->groupsMembership = $this->attrs['userGroups'];
    }

    $this->savedRolesMembership = $this->rolesMembership;
    if (isset($this->attrs['userRoles'])) {
      unset($this->attrs['userRoles']['count']);
      $this->rolesMembership = $this->attrs['userRoles'];
    }
  }
}
