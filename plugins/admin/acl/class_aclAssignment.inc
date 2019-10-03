<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2019  FusionDirectory

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

class aclAssignment extends simplePlugin
{
  static function plInfo (): array
  {
    global $config;
    $oc = [
      'aclAssignment' => [
        'aclCategory' => 'acl',
        'name'        => _('ACL assignment'),
        'ou'          => '',
        'icon'        => 'geticon.php?context=categories&icon=acl&size=16',
        'tabClass'    => 'simpleTabs_noSpecial',
        'mainAttr'    => FALSE
      ],
    ];
    if ($config->get_cfg_value('aclTabOnObjects') == 'TRUE') {
      $oc[] = 'special';
    } else {
      $oc = array_merge($oc, departmentManagement::getDepartmentTypes());
    }
    return [
      'plShortName'   => _('ACL Assignment'),
      'plDescription' => _('Access control roles assignment'),
      'plSmallIcon'   => 'geticon.php?context=categories&icon=acl&size=16',
      'plObjectClass' => ['gosaAcl'],
      'plObjectType'  => $oc,
      'plForeignKeys'  => [
        'gosaAclEntry' => [
          ['aclRole',      'dn', 'gosaAclEntry=*:*:%b|oldvalue%:*',    'gosaAclEntry=*'],
          ['user',         'dn', 'gosaAclEntry=*:*:*:*%b|oldvalue%*',  'gosaAclEntry=*'],
          ['posixGroup',   'dn', 'gosaAclEntry=*:*:*:*%b|oldvalue%*',  'gosaAclEntry=*'],
          ['roleGeneric',  'dn', 'gosaAclEntry=*:*:*:*%b|oldvalue%*',  'gosaAclEntry=*'],
        ]
      ],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Assignments'),
        'class' => ['fullwidth'],
        'attrs' => [
          new ACLsAssignmentAttribute(
            '', _('ACL role assignments for this base'),
            'gosaAclEntry', FALSE
          )
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, FALSE);
    $this->attributesInfo['main']['name'] = sprintf(_('Assignments on object or subtree %s'), $this->dn);
  }

  function compute_dn (): string
  {
    return $this->dn;
  }

  function save (): array
  {
    $this->ignore_account = FALSE;
    $this->is_account = (count($this->gosaAclEntry) != 0);
    if ($this->is_account) {
      return parent::save();
    } else {
      return $this->remove(FALSE);
    }
  }

  function post_remove ()
  {
    parent::post_remove();

    /* Refresh users ACLs */
    $ui = get_userinfo();
    $ui->loadACL();
    session::set('ui', $ui);
  }

  function post_save ()
  {
    parent::post_save();

    /* Refresh users ACLs */
    $ui = get_userinfo();
    $ui->loadACL();
    session::set('ui', $ui);
  }

  function foreignKeyUpdate (string $field, $oldvalue, $newvalue, array $source)
  {
    /* Handle foreign keys only as the main tab so that they are not handled twice */
    if ($this->parent->getBaseObject() !== $this) {
      return;
    }
    parent::foreignKeyUpdate($field, $oldvalue, $newvalue, $source);
  }
}
