<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2020  FusionDirectory

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

class ogroup extends simplePlugin
{
  var $used_workstations = [];

  protected $savedTypedMembers;

  static $objectTypes = [
    'U' => 'user',
    'G' => 'ogroup',
    'A' => 'application',
    'S' => 'server',
    'W' => 'workstation',
    'T' => 'terminal',
    'F' => 'phone',
    'P' => 'printer',
    'D' => 'simpleSecurityObject',
  ];

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Object group'),
      'plDescription' => _('Object group information'),
      'plFilter'      => '(objectClass=groupOfNames)',
      'plPriority'    => 1,
      'plObjectClass' => ['groupOfNames', 'gosaGroupOfNames'],
      'plFilter'      => '(objectClass=groupOfNames)',
      'plObjectType'  => ['ogroup' => [
        'name'        => _('Group'),
        'description' => _('Group'),
        'ou'          => get_ou('ogroupRDN'),
        'icon'        => 'geticon.php?context=types&icon=resource-group&size=16',
        'tabClass'    => 'ogrouptabs',
      ]],
      'plForeignKeys'  => [
        'member' => [
          ['user',               'dn','member=%oldvalue%','*'],
          ['ogroup',             'dn','member=%oldvalue%','*'],
          ['application',        'dn','member=%oldvalue%','*'],
          ['serverGeneric',      'dn','member=%oldvalue%','*'],
          ['workstationGeneric', 'dn','member=%oldvalue%','*'],
          ['terminalGeneric',    'dn','member=%oldvalue%','*'],
          ['phoneGeneric',       'dn','member=%oldvalue%','*'],
          ['printGeneric',       'dn','member=%oldvalue%','*'],
        ],
        'owner' => [
          ['user','dn','owner=%oldvalue%','*']
        ]
      ],
      'plSearchAttrs' => ['description'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Properties'),
        'attrs' => [
          new BaseSelectorAttribute(get_ou('ogroupRDN')),
          new StringAttribute(
            _('Name'), _('Name of this group'),
            'cn', TRUE,
            '', '', (strict_uid_mode() ? '/^[a-z0-9_-]+$/' : '/^[a-z0-9_.-]+$/i')
          ),
          new TextAreaAttribute(
            _('Description'), _('Short description of this group'),
            'description', FALSE
          ),
          new HiddenAttribute('gosaGroupObjects'),
          new UserAttribute(
            _('Owner'), _('Owner'),
            'owner', FALSE
          ),
        ]
      ],
      'members' => [
        'name'  => _('Member objects'),
        'attrs' => [
          new GroupMembersAttribute(
            '', _('Objects member of this group'),
            'member', TRUE,
            [], 'dn'
          )
        ]
      ],
      'system_trust' => [
        'name'  => _('System trust'),
        'icon'  => 'geticon.php?context=status&icon=locked&size=16',
        'attrs' => [
          new SelectAttribute(
            _('Trust mode'), _('Type of authorization for those hosts'),
            'trustMode', FALSE,
            ['', 'fullaccess', 'byhost'],
            '',
            [_('disabled'), _('full access'), _('allow access to these hosts')]
          ),
          new SystemsAttribute(
            '', _('Only allow this group to connect to this list of hosts'),
            'host', FALSE
          )
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['trustMode']->setInLdap(FALSE);
    $this->attributesAccess['trustMode']->setManagedAttributes(
      [
        'multiplevalues' => ['notbyhost' => ['','fullaccess']],
        'erase' => [
          'notbyhost' => ['host']
        ]
      ]
    );
    if ((count($this->host) == 1) && ($this->host[0] == '*')) {
      $this->trustMode = 'fullaccess';
    } elseif (count($this->host) > 0) {
      $this->trustMode = 'byhost';
    }

    /* Detect all workstations, which are already assigned to an object group
        - Those objects will be hidden in the add object dialog.
        - Check() will complain if such a system is assigned to this object group.*/
    $this->used_workstations = [];
    try {
      $ws_dns = array_keys(objects::ls(['workstation','terminal']));
      $res = objects::ls('ogroup', ['member' => '*'], NULL, '(&(member=*)(|(gosaGroupObjects=[W])(gosaGroupObjects=[T])))');
    } catch (NonExistingObjectTypeException $e) {
      /* If workstation/terminal objectType is not existing, systems plugin is missing so there are no systems */
      $ws_dns = [];
      $res    = [];
    }
    foreach ($res as $odn => $og) {
      if ($odn == $this->dn) {
        continue;
      }
      $this->used_workstations = array_merge($this->used_workstations, array_intersect($ws_dns, $og['member']));
    }

    $this->reload();
    $this->savedTypedMembers = $this->attributesAccess['member']->getTypedValues();
  }

  function prepare_save (): array
  {
    $this->reload();
    $errors = parent::prepare_save();

    if ($this->trustMode == 'fullaccess') {
      $this->attrs['host'] = ['*'];
    }

    /* Trust accounts */
    if (($this->trustMode != "") && !in_array('hostObject', $this->attrs['objectClass'])) {
      $this->attrs['objectClass'][] = 'hostObject';
    } elseif (($this->trustMode == "") && (($key = array_search('hostObject', $this->attrs['objectClass'])) !== FALSE)) {
      unset($this->attrs['objectClass'][$key]);
    }

    return $errors;
  }

  function reload ()
  {
    $this->gosaGroupObjects = $this->attributesAccess['member']->listObjectTypes();
  }

  function check (): array
  {
    $errors = parent::check();
    $this->reload();
    if (preg_match('/W/', $this->gosaGroupObjects) && preg_match('/T/', $this->gosaGroupObjects)) {
      $errors[] = new SimplePluginCheckError(
        $this,
        htmlescape(_('Putting both workstations and terminals in the same group is not allowed'))
      );
    }

    return $errors;
  }

  function ldap_save (): array
  {
    global $ui;

    $errors = [];

    if (isset($this->attrs['member'])) {
      $userMembers  = [];
      $savedMembers = [];
      $members      = $this->attributesAccess['member']->getTypedValues();
      if (isset($members['U'])) {
        $userMembers = $members['U'];
      }
      if (isset($this->savedTypedMembers['U'])) {
        $savedMembers = $this->savedTypedMembers['U'];
      }
      $addingMembers    = array_diff($userMembers, $savedMembers);
      $removingMembers  = array_diff($savedMembers, $userMembers);
      foreach ($addingMembers as $dn) {
        if (strpos($ui->get_permissions($dn, 'user/userRoles', 'groupsMembership', $this->acl_skip_write()), 'w') === FALSE) {
          $errors[] = new SimplePluginPermissionError($this, msgPool::permModify($dn, 'groupsMembership'));
        }
      }
      foreach ($removingMembers as $dn) {
        if (strpos($ui->get_permissions($dn, 'user/userRoles', 'groupsMembership', $this->acl_skip_write()), 'w') === FALSE) {
          $errors[] = new SimplePluginPermissionError($this, msgPool::permModify($dn, 'groupsMembership'));
        }
      }
    }

    if (!empty($errors)) {
      return $errors;
    }

    return parent::ldap_save();
  }

  function handleForeignKeys (string $olddn = NULL, string $newdn = NULL, string $mode = 'move')
  {
    if (($olddn !== NULL) && ($olddn == $newdn)) {
      return;
    }
    if ($this->is_template) {
      return;
    }
    parent::handleForeignKeys($olddn, $newdn, $mode);
    if ($this->attributeHaveChanged('gosaGroupObjects')) {
      /* Propagate member type changes to parent groups */
      $parents = objects::ls('ogroup', ['dn' => 'raw'], NULL, '(member='.ldap_escape_f($this->dn).')');
      foreach (array_keys($parents) as $dn) {
        $tabobject = objects::open($dn, 'ogroup');
        $errors = $tabobject->save();
        msg_dialog::displayChecks($errors);
      }
    }
  }

  function getGroupObjectTypes ()
  {
    $this->reload();
    return $this->gosaGroupObjects;
  }
}
