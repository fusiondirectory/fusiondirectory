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

class aclAssignmentDialogWindow extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('ACL Assignment Dialog'),
      'plDescription' => _('Access control roles assignment dialog'),
      'plCategory'    => ['acl'],

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'properties' => [
        'name'  => _('Properties'),
        'attrs' => [
          new SelectAttribute(
            _('Mode'), _('Is this applying on complete subtree or only the base?'),
            'aclMode', TRUE,
            ['subtree',    'base'], 'base',
            [_('Subtree'), _('Base only')]
          ),
          new SelectAttribute(
            _('Role'), _('Role to apply'),
            'aclRole', TRUE,
            []
          ),
          new BooleanAttribute(
            _('For all users'), _('Apply this ACL for all LDAP users'),
            'allUsers', FALSE
          ),
          new UsersGroupsRolesAttribute(
            _('Members'), _('Users or groups to assign this role to.'),
            'aclMembers', TRUE
          ),
        ]
      ],
      'advanced' => [
        'name'  => _('Advanced'),
        'attrs' => [
          new StringAttribute(
            _('Restrict users with filter'), _('LDAP filter which a member must match to actually get the rights'),
            'aclUserFilter', FALSE
          ),
          new StringAttribute(
            _('Restrict targets with filter'), _('LDAP filter which a dn must match to actually be concerned. May use %dn% mask for user dn. Example: (manager=%dn%).'),
            'aclTargetFilter', FALSE
          ),
        ]
      ],
    ];
  }

  function __construct ($value, $isContainer = FALSE)
  {
    parent::__construct(NULL, NULL, NULL, TRUE);
    if ($isContainer) {
      $this->attributesAccess['aclMode']->setDefaultValue('subtree');
    } else {
      $this->attributesAccess['aclMode']->setDefaultValue('base');
    }
    $this->attributesAccess['aclMode']->resetToDefault();

    $roles = objects::ls('aclRole', 'cn');
    $this->attributesAccess['aclRole']->setChoices(array_keys($roles), array_values($roles));
    $this->attributesAccess['allUsers']->setInLdap(FALSE);
    $this->attributesAccess['allUsers']->setManagedAttributes(
      [
        'disable' => [
          TRUE => ['aclMembers']
        ]
      ]
    );

    if ($value !== NULL) {
      $this->aclMode    = $value['scope'];
      $this->aclRole    = $value['role'];
      $this->aclMembers = $value['members'];
      if ($value['members'][0] == '*') {
        $this->allUsers = TRUE;
      }
      $this->aclUserFilter    = $value['userfilter'];
      $this->aclTargetFilter  = $value['targetfilter'];
    }
  }

  function execute (): string
  {
    $smarty = get_smarty();
    $display = parent::execute();
    if (!is_object($this->dialog)) {
      $display .= $smarty->fetch('string:'.
        '<p class="plugbottom">'.
        '  <input type="submit" name="add_acl_finish" value="{msgPool type=addButton}"/>'.
        '  &nbsp;'.
        '  <input type="submit" formnovalidate="formnovalidate" name="add_acl_cancel" value="{msgPool type=cancelButton}"/>'.
        '</p>');
    }
    return $display;
  }

  function getAclEntry ()
  {
    $entry = [
      'scope'         => $this->aclMode,
      'role'          => $this->aclRole,
      'members'       => $this->aclMembers,
      'userfilter'    => $this->aclUserFilter,
      'targetfilter'  => $this->aclTargetFilter,
    ];
    if ($this->allUsers) {
      $entry['members'] = ['*'];
    }
    return $entry;
  }
}
