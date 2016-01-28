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

class roleGeneric extends simplePlugin
{
  var $mainTab = TRUE;

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

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

   /*!
   *  \brief The main function : information about attributes
   */
  static function getAttributesInfo ()
  {
    return array(
      'section1' => array(
        'name'  => _('Informations'),
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
          ),
          new UsersAttribute(
            _('Users'), _('Add users for the role'),
            'roleOccupant', FALSE
          )
        )
      )
    );
  }

  /*!
   * \brief Constructor
   */
  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
    $this->attributesAccess['cn']->setUnique(TRUE);
  }
}
?>
