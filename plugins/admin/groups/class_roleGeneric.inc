<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2016  FusionDirectory

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

class RoleMembersAttribute extends UsersAttribute
{
  protected $whitelistDns = NULL;

  function getFilterWhiteList()
  {
    global $config;
    if ($config->get_cfg_value('RestrictRoleMembers') == 'TRUE') {
      if ($this->whitelistDns === NULL) {
        /* Computes a list of members of all groups within our branch */
        $groups = objects::ls('ogroup', array('member' => '*'), $this->plugin->base);
        if (empty($groups)) {
          $this->whitelistDns = array();
        } else {
          $this->whitelistDns = call_user_func_array('array_merge_recursive', $groups)['member'];
        }
      }
      return array(
        'branches'  => array($this->plugin->base),
        'dn'        => $this->whitelistDns,
      );
    } else {
      return array();
    }
  }
}

class roleGeneric extends simplePlugin
{
  var $objectclasses = array('organizationalRole');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Role'),
      'plDescription' => _('Role information'),
      'plObjectType'  => array('role' =>
        array(
          'name'        => _('Role'),
          'description' => _('Organizational role'),
          'ou'          => get_ou('roleRDN'),
          'filter'      => '(&(objectClass=organizationalRole)(!(objectClass=simpleSecurityObject)))',
          'icon'        => 'geticon.php?context=types&icon=role&size=16',
          'mainAttr'    => 'cn',
        )
      ),
      'plForeignKeys'  => array(
        'roleOccupant' => 'user'
      ),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

   /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Information'),
        'attrs' => array(
          new BaseSelectorAttribute (get_ou('roleRDN')),
          new StringAttribute(
            _('Name'), _('Name of this group'),
            'cn', TRUE,
            '', '', (strict_uid_mode() ? '/^[a-z0-9_-]+$/i' : '/^[a-z0-9_.-]+$/i')
          ),
          new StringAttribute (
            _('Description'), _('Description of the role'),
            'description'
          ),
          new StringAttribute (
            _('Phone number'), _('Phone number'),
            'telephoneNumber'
          ),
          new StringAttribute (
            _('Fax number'), _('Fax number'),
            'facsimileTelephoneNumber'
          )
        )
      ),
      'members' => array(
        'name'  => _('Role members'),
        'attrs' => array(
          new RoleMembersAttribute (
            '', _('Add users for the role'),
            'roleOccupant', FALSE
          )
        )
      )
    );
  }
}
?>
