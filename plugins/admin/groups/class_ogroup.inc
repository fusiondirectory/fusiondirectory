<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2018  FusionDirectory

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

class ObjectsAttribute extends GenericDialogAttribute
{
  protected $dialogClass  = 'ObjectSelectDialog';
  protected $types        = [];

  function getFilterBlackList ()
  {
    return [
      'dn' => array_merge($this->getValue(), $this->plugin->used_workstations, [$this->plugin->dn])
    ];
  }

  protected function fillDisplayValueFrom ($i, $attrs)
  {
    if ($attrs) {
      foreach (ogroup::$objectTypes as $code => $objectType) {
        try {
          if (objects::isOfType($attrs, $objectType)) {
            $infos = objects::infos($objectType);
            $this->types[$i]    = $code;
            $this->displays[$i] = $attrs[$infos['nameAttr']][0];
            if (strtolower($objectType) == 'user') {
              if (isset($attrs['description'][0])) {
                $this->displays[$i] .= ' ['.$attrs['uid'][0].' - '.$attrs['description'][0].']';
              } else {
                $this->displays[$i] .= ' ['.$attrs['uid'][0].']';
              }
            } elseif (isset($attrs['description'][0])) {
              $this->displays[$i] .= ' ['.$attrs['description'][0].']';
            }
            break;
          }
        } catch (NonExistingObjectTypeException $e) {
          @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $e->getMessage(), "Objecttype");
          unset(ogroup::$objectTypes[$code]);
        }
      }
      if (!isset($this->displays[$i])) {
        trigger_error('Unkown type for "'.$this->value[$i].'"');
        $this->displays[$i] = sprintf(_('Unknown type : %s'), $this->value[$i]);
        $this->types[$i]    = 'I';
      }
    } else {
      if (($attrs === NULL) && $this->isTemplate()) {
        $this->displays[$i] = $this->value[$i];
        $this->types[$i]    = 'I';
      } else {
        $this->displays[$i] = sprintf(_('Non existing dn: %s'), $this->value[$i]);
        $this->types[$i]    = 'I';
      }
    }
  }

  function renderOnlyFormInput ()
  {
    if (($this->size < 15) && ($this->size < count($this->value))) {
      $this->size = min(15, count($this->value));
    }
    $id       = $this->getHtmlId();
    $display  = '<select multiple="multiple"  name="row'.$id.'[]" id="row'.$id.'" size="'.$this->size.'"'.
                ($this->disabled ? ' disabled="disabled"' : '').
                ' >'."\n";
    foreach ($this->getDisplayValues() as $key => $value) {
      try {
        $infos = [];
        if ($this->types[$key] !== 'I') {
          $infos = objects::infos(ogroup::$objectTypes[$this->types[$key]]);
        }
      } catch (NonExistingObjectTypeException $e) {
        die('No type for '.$key.' '.$value);
      }
      if (isset($infos['icon'])) {
        $img = $infos['icon'];
      } else {
        $img = 'images/empty.png';
      }
      $display .= '<option value="'.$key.'" class="select"'.
                  ' style="background-image:url(\''.get_template_path($img).'\');"'.
                  '>'.$value."</option>\n";
    }
    $display .= '</select><br/>'."\n";
    return $display;
  }

  function ldapAttributesToGet ()
  {
    return [
      'objectClass',
      'cn',
      'sn',
      'ou',
      'uid',
      'givenName',
      'description',
    ];
  }

  function listObjectTypes ()
  {
    /* Refresh types and displays */
    $this->getDisplayValues();
    return '['.join(array_unique($this->types)).']';
  }

  function getTypedValues ()
  {
    $values = $this->getValue();
    $ret    = [];
    foreach ($values as $i => $v) {
      if (!isset($ret[$this->types[$i]])) {
        $ret[$this->types[$i]] = [];
      }
      $ret[$this->types[$i]][] = $v;
    }

    return $ret;
  }

  protected function removeValue ($row)
  {
    parent::removeValue($row);
    unset($this->types[$row]);
  }
}

class ObjectSelectDialog extends GenericSelectDialog
{
  protected $dialogClass = 'objectSelect';
}

class ogroup extends simplePlugin
{
  var $objectclasses  = ['groupOfNames', 'gosaGroupOfNames'];

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

  static function plInfo ()
  {
    return [
      'plShortName'   => _('Object group'),
      'plDescription' => _('Object group information'),
      'plSelfModify'  => FALSE,
      'plPriority'    => 1,
      'plObjectType'  => ['ogroup' => [
        'name'        => _('Group'),
        'description' => _('Group'),
        'ou'          => get_ou('ogroupRDN'),
        'filter'      => 'objectClass=groupOfNames',
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

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo ()
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
          new ObjectsAttribute(
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

  /*! \brief This function returns an LDAP filter for this plugin object classes
   */
  function getObjectClassFilter ()
  {
    return '(objectClass=groupOfNames)';
  }

  function is_this_account ($attrs)
  {
    $this->objectclasses  = ['groupOfNames'];
    $found = parent::is_this_account($attrs);
    $this->objectclasses  = ['groupOfNames', 'gosaGroupOfNames'];
    return $found;
  }

  function prepare_save ()
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

  function check ()
  {
    $message = parent::check();
    $this->reload();
    if (preg_match('/W/', $this->gosaGroupObjects) && preg_match('/T/', $this->gosaGroupObjects)) {
      $message[] = _('Putting both workstations and terminals in the same group is not allowed');
    }

    return $message;
  }

  function ldap_save ()
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
          $errors[] = msgPool::permModify($dn, 'groupsMembership');
        }
      }
      foreach ($removingMembers as $dn) {
        if (strpos($ui->get_permissions($dn, 'user/userRoles', 'groupsMembership', $this->acl_skip_write()), 'w') === FALSE) {
          $errors[] = msgPool::permModify($dn, 'groupsMembership');
        }
      }
    }

    if (!empty($errors)) {
      return $errors;
    }

    return parent::ldap_save();
  }

  function getGroupObjectTypes ()
  {
    $this->reload();
    return $this->gosaGroupObjects;
  }
}
?>
