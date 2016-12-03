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

class organization extends department
{
  static $namingAttr  = "o";
  var $objectclasses  = array("top", "gosaDepartment");
  var $structuralOC   = array("organization");

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Organization'),
      'plDescription' => _('Organization'),
      'plObjectType'  => array('organization' => array(
        'aclCategory' => 'department',
        'name'        => _('Organization'),
        'filter'      => '(&(objectClass=organization)(objectClass=gosaDepartment))',
        'ou'          => '',
        'icon'        => 'geticon.php?context=types&icon=organization&size=16',
        'tabClass'    => 'deptabs',
        'mainAttr'    => static::$namingAttr,
      )),
      'plForeignKeys'  => array(
        'manager' => 'user'
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return parent::getDepartmentAttributesInfo(_('organization'));
  }
}
?>
