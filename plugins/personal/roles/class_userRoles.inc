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
  var $objectclasses = [];

  protected $savedGroupsMembership  = [];
  protected $savedRolesMembership   = [];
  protected $templateGroups         = [];
  protected $templateRoles          = [];

  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Groups and roles'),
      'plDescription'   => _('Edit user\'s groups and roles'),
      'plIcon'          => 'geticon.php?context=types&icon=role&size=48',
      'plSmallIcon'     => 'geticon.php?context=types&icon=role&size=16',
      'plSelfModify'    => FALSE,
      'plObjectType'    => ['user'],
      'plPriority'      => 5,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'groups' => [
        'name'  => _('Groups membership'),
        'attrs' => [
          new SetAttribute(
            new SelectAttribute(
              '', _('Groups membership'),
              'groupsMembership', FALSE
            )
          )
        ]
      ],
      'roles' => [
        'name'  => _('Roles membership'),
        'attrs' => [
          new SetAttribute(
            new SelectAttribute(
              '', _('Roles membership'),
              'rolesMembership', FALSE
            )
          )
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->ignore_account = FALSE;

    /* Groups handling */
    $groups      = [];
    $groupsattrs = objects::ls('ogroup', ['cn' => 1,  'description' => 1], NULL, '', TRUE);
    foreach ($groupsattrs as $dn => $groupattr) {
      $groupDisplay = $groupattr['cn'];
      if (isset($groupattr['description'])) {
        if (mb_strlen($groupattr['description']) > 50) {
          $groupattr['description'] = mb_substr($groupattr['description'], 0, 50).'…';
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
        $myGroups = [];
      }
    } else {
      if (($this->dn != '') && ($this->dn != 'new')) {
        $myGroups  = objects::ls(
          'ogroup', NULL, NULL,
          '(member='.$this->dn.')'
        );
      } else {
        $myGroups = [];
      }
      $this->savedGroupsMembership = array_keys($myGroups);
    }
    /* We might be in some groups we have no read ACL on, so merge them in the choices */
    $groups = array_merge($myGroups, $groups);
    asort($groups);
    asort($myGroups);
    $this->attributesAccess['groupsMembership']->attribute->setChoices(array_keys($groups), array_values($groups));
    $this->attributesAccess['groupsMembership']->setValue(array_keys($myGroups));

    /* Roles handling */
    $roles        = [];
    $rolesattrs  = objects::ls('role', ['cn' => 1,  'description' => 1], NULL, '', TRUE);
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
        $myRoles = [];
      }
    } else {
      if (($this->dn != '') && ($this->dn != 'new')) {
        $myRoles  = objects::ls(
          'role', NULL, NULL,
          '(roleOccupant='.$this->dn.')'
        );
      } else {
        $myRoles = [];
      }
      $this->savedRolesMembership = array_keys($myRoles);
    }
    /* We might be in some roles we have no read ACL on, so merge them in the choices */
    $roles = array_merge($myRoles, $roles);
    asort($roles);
    asort($myRoles);
    $this->attributesAccess['rolesMembership']->attribute->setChoices(array_keys($roles), array_values($roles));
    $this->attributesAccess['rolesMembership']->setValue(array_keys($myRoles));

    $this->is_account = ((count($this->rolesMembership) > 0) || (count($this->groupsMembership) > 0));
    $this->initially_was_account = $this->is_account;
  }

  function is_this_account ($attrs)
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

  /*! \brief This function removes the object from LDAP
   */
  function remove (bool $fulldelete = FALSE): array
  {
    if (!$fulldelete) {
      /* We are not deleting the object it's just that there are no groups left
       * Make sure memberships are empty to avoid surprises and call save. */
      $this->rolesMembership  = [];
      $this->groupsMembership = [];
      return $this->save();
    } else {
      return parent::remove($fulldelete);
    }
  }

  protected function ldap_remove (): array
  {
    if ($this->is_template) {
      return parent::ldap_remove();
    } elseif (($this->dn != '') && ($this->dn != 'new')) {
      /* Remove all groups */
      foreach ($this->savedGroupsMembership as $key => $ogroupdn) {
        try {
          $g = objects::open($ogroupdn, 'ogroup');
          $g->getBaseObject()->attributesAccess['member']->searchAndRemove($this->dn);
          $msg = $g->save();
          if (empty($msg)) {
            unset($this->savedGroupsMembership[$key]);
          } else {
            /* We do not prevent user deletion on error, but still warn the user */
            foreach ($msg as $error) {
              $warning = new FusionDirectoryWarning(
                htmlescape(sprintf(
                  _('Could not remove membership to group %s: %s'),
                  $ogroupdn,
                  $error
                )),
                0,
                ($error instanceof Throwable ? $error : NULL)
              );
              $warning->display();
            }
          }
        } catch (NonExistingLdapNodeException $e) {
          /* Ignore deleted groups */
        }
      }
      /* Remove all roles */
      foreach ($this->savedRolesMembership as $key => $roledn) {
        try {
          $r = objects::open($roledn, 'role');
          $r->getBaseObject()->attributesAccess['roleOccupant']->searchAndRemove($this->dn);
          $msg = $r->save();
          if (empty($msg)) {
            unset($this->savedRolesMembership[$key]);
          } else {
            /* We do not prevent user deletion on error, but still warn the user */
            foreach ($msg as $error) {
              $warning = new FusionDirectoryWarning(
                htmlescape(sprintf(
                  _('Could not remove membership to role %s: %s'),
                  $roledn,
                  $error
                )),
                0,
                ($error instanceof Throwable ? $error : NULL)
              );
              $warning->display();
            }
          }
        } catch (NonExistingLdapNodeException $e) {
          /* Ignore deleted roles */
        }
      }
    }
    return [];
  }

  public function update (): bool
  {
    $res = parent::update();

    $this->is_account = ((count($this->rolesMembership) > 0) || (count($this->groupsMembership) > 0));

    return $res;
  }

  protected function shouldSave (): bool
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

  function prepare_save (): array
  {
    $errors = parent::prepare_save();
    if ($this->is_template) {
      $this->attrs['userGroups']  = $this->groupsMembership;
      $this->attrs['userRoles']   = $this->rolesMembership;
    }
    return $errors;
  }

  function prepare_remove ()
  {
    parent::prepare_remove();
    if ($this->is_template) {
      $this->attrs['userGroups']  = [];
      $this->attrs['userRoles']   = [];
    }
  }

  function ldap_save (): array
  {
    $errors = parent::ldap_save();

    if (!empty($errors)) {
      return $errors;
    }

    if (!$this->is_template) {
      /* We need to give an array which looks like an ldap fetch and match user filter */
      $fake_attrs = [
        'objectClass' => ['inetOrgPerson','organizationalPerson','person'],
        'cn'          => [$this->dn],
        'uid'         => [$this->dn],
        'dn'          => $this->dn
      ];

      /* Take care about groupsMembership values: add to groups */
      $groupsMembership = $this->attributesAccess['groupsMembership']->getValue();
      foreach ($groupsMembership as $ogroupdn) {
        if (!in_array($ogroupdn, $this->savedGroupsMembership)) {
          try {
            $g = objects::open($ogroupdn, 'ogroup');
            if (!in_array($ogroupdn, $this->templateGroups) && !$g->getBaseObject()->attrIsWriteable('member')) {
              $errors[] = new SimplePluginPermissionError($this, msgPool::permModify($ogroupdn, 'member'));
              continue;
            }
            $g->getBaseObject()->attributesAccess['member']->addValue($this->dn, $fake_attrs);
            $msg = $g->save();
            if (empty($msg)) {
              $this->savedGroupsMembership[] = $ogroupdn;
            } else {
              foreach ($msg as $error) {
                $errors[] = new SimplePluginError($this, htmlescape(sprintf(_('Could not add membership to group %s: %s'), $ogroupdn, $error)));
              }
            }
          } catch (NonExistingLdapNodeException $e) {
            $errors[] = new SimplePluginError($this, htmlescape($e->getMessage()), 0, $e);
          }
        }
      }

      /* Remove groups not listed in groupsMembership */
      foreach ($this->savedGroupsMembership as $key => $ogroupdn) {
        if (!in_array($ogroupdn, $groupsMembership)) {
          try {
            $g = objects::open($ogroupdn, 'ogroup');
            if (!$g->getBaseObject()->attrIsWriteable('member')) {
              $errors[] = new SimplePluginPermissionError($this, msgPool::permModify($ogroupdn, 'member'));
              continue;
            }
            $g->getBaseObject()->attributesAccess['member']->searchAndRemove($this->dn);
            $msg = $g->save();
            if (empty($msg)) {
              unset($this->savedGroupsMembership[$key]);
            } else {
              foreach ($msg as $error) {
                $errors[] = new SimplePluginError($this, htmlescape(sprintf(_('Could not remove membership to group %s: %s'), $ogroupdn, $error)));
              }
            }
          } catch (NonExistingLdapNodeException $e) {
            $errors[] = new SimplePluginError($this, htmlescape($e->getMessage()), 0, $e);
          }
        }
      }

      /* Take care about rolesMembership values: add to roles */
      $rolesMembership = $this->attributesAccess['rolesMembership']->getValue();
      foreach ($rolesMembership as $roledn) {
        if (!in_array($roledn, $this->savedRolesMembership)) {
          try {
            $r = objects::open($roledn, 'role');
            if (!in_array($roledn, $this->templateRoles) && !$r->getBaseObject()->attrIsWriteable('roleOccupant')) {
              $errors[] = new SimplePluginPermissionError($this, msgPool::permModify($roledn, 'roleOccupant'));
              continue;
            }
            $r->getBaseObject()->attributesAccess['roleOccupant']->addValue($this->dn, $fake_attrs);
            $msg = $r->save();
            if (empty($msg)) {
              $this->savedRolesMembership[] = $roledn;
            } else {
              foreach ($msg as $error) {
                $errors[] = new SimplePluginError($this, htmlescape(sprintf(_('Could not add membership to role %s: %s'), $roledn, $error)));
              }
            }
          } catch (NonExistingLdapNodeException $e) {
            $errors[] = new SimplePluginError($this, htmlescape($e->getMessage()), 0, $e);
          }
        }
      }

      /* Remove roles not listed in rolesMembership */
      foreach ($this->savedRolesMembership as $key => $roledn) {
        if (!in_array($roledn, $rolesMembership)) {
          try {
            $r = objects::open($roledn, 'role');
            if (!$r->getBaseObject()->attrIsWriteable('roleOccupant')) {
              $errors[] = new SimplePluginPermissionError($this, msgPool::permModify($roledn, 'roleOccupant'));
              continue;
            }
            $r->getBaseObject()->attributesAccess['roleOccupant']->searchAndRemove($this->dn);
            $msg = $r->save();
            if (empty($msg)) {
              unset($this->savedRolesMembership[$key]);
            } else {
              foreach ($msg as $error) {
                $errors[] = new SimplePluginError($this, htmlescape(sprintf(_('Could not remove membership to role %s: %s'), $roledn, $error)));
              }
            }
          } catch (NonExistingLdapNodeException $e) {
            $errors[] = new SimplePluginError($this, htmlescape($e->getMessage()), 0, $e);
          }
        }
      }
    }

    return $errors;
  }

  function adapt_from_template (array $attrs, array $skip = [])
  {
    parent::adapt_from_template($attrs, $skip);

    $this->savedGroupsMembership = $this->groupsMembership;
    if (isset($this->attrs['userGroups'])) {
      unset($this->attrs['userGroups']['count']);
      $myGroups = array_combine($this->attrs['userGroups'], $this->attrs['userGroups']);
      $groups   = $this->attributesAccess['groupsMembership']->attribute->getDisplayChoices();
      $groups   = array_merge($myGroups, $groups);
      asort($groups);
      asort($myGroups);
      $this->attributesAccess['groupsMembership']->attribute->setChoices(array_keys($groups), array_values($groups));
      $this->attributesAccess['groupsMembership']->setValue(array_keys($myGroups));
      $this->templateGroups = array_keys($myGroups);
    } else {
      $this->templateGroups = [];
    }

    $this->savedRolesMembership = $this->rolesMembership;
    if (isset($this->attrs['userRoles'])) {
      unset($this->attrs['userRoles']['count']);
      $myRoles = array_combine($this->attrs['userRoles'], $this->attrs['userRoles']);
      $roles   = $this->attributesAccess['rolesMembership']->attribute->getDisplayChoices();
      $roles   = array_merge($myRoles, $roles);
      asort($roles);
      asort($myRoles);
      $this->attributesAccess['rolesMembership']->attribute->setChoices(array_keys($roles), array_values($roles));
      $this->attributesAccess['rolesMembership']->setValue(array_keys($myRoles));
      $this->templateRoles = array_keys($myRoles);
    } else {
      $this->templateRoles = [];
    }

    $this->is_account = ((count($this->rolesMembership) > 0) || (count($this->groupsMembership) > 0));
  }
}
