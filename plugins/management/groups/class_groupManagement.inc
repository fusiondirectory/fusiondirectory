<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2019  FusionDirectory

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

class groupManagement extends management
{
  public static $skipTemplates = FALSE;

  public static $columns = [
    ['ObjectTypeColumn',   []],
    ['LinkColumn',         ['attributes' => 'cn',                'label' => 'Name']],
    ['LinkColumn',         ['attributes' => 'description',       'label' => 'Description']],
    ['GroupContentColumn', ['attributes' => 'gosaGroupObjects',  'label' => 'Members']],
    ['PropertiesColumn',   ['label' => 'Properties']],
    ['ActionsColumn',      ['label' => 'Actions']],
  ];

  protected $siActive = FALSE;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Groups and Roles'),
      'plDescription' => _('Manage object groups, posix groups and roles'),
      'plIcon'        => 'geticon.php?context=types&icon=resource-group&size=48',
      'plSection'     => ['accounts' => ['name' => _('Users and groups'), 'priority' => 0]],
      'plManages'     => ['ogroup', 'group', 'role', 'dyngroup'],
      'plPriority'    => 20,

      'plProvidedAcls' => []
    ];
  }

  function __construct ()
  {
    parent::__construct(
      FALSE,
      [
        ['TabFilterElement',          []],
        ['GroupContentFilterElement', []],
      ]
    );
  }
}
