<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016  FusionDirectory

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

class country extends department
{
  static protected $namingAttr  = 'c';
  protected $structuralOC       = array('country');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Country'),
      'plDescription' => _('Country'),
      'plObjectClass' => array('gosaDepartment'),
      'plFilter'      => '(&(objectClass=country)(objectClass=gosaDepartment))',
      'plObjectType'  => array('country' => array(
        'aclCategory' => 'department',
        'name'        => _('Country'),
        'ou'          => '',
        'icon'        => 'geticon.php?context=types&icon=country&size=16',
        'tabClass'    => 'deptabs',
        'mainAttr'    => static::$namingAttr,
      )),
      'plForeignKeys'  => array(
        'manager' => array('user','dn','manager=%oldvalue%','*')
      ),
      'plSearchAttrs' => array('description'),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    $attributesInfo = parent::getDepartmentAttributesInfo(_('country'));
    unset($attributesInfo['location']);
    unset($attributesInfo['properties']['attrs'][3]); // category
    /* Replace Name field by country code */
    $attributesInfo['properties']['attrs'][1] =
      new StringAttribute(
        _('Country code'), _('A two-letter country code from ISO 3166'),
        static::$namingAttr, TRUE,
        '', '',
        '/^[a-z0-9\.-]{2}$/i', 'IT'
      );
    return $attributesInfo;
  }
}
?>
