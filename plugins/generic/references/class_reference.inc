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

class reference extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('References'),
      'plDescription' => _('References'),
      'plCategory'    => ['acl'],
      'plObjectType'  => [
        'special' => [
          'name' => 'special'
        ]
      ],
      'plPriority'    => 10,

      'plProvidedAcls'  => []
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'      => _('References'),
        'class'     => ['fullwidth'],
        'attrs'     => [
          new FakeAttribute('refs')
        ],
        'template'  => get_template_path('contents.tpl', TRUE, dirname(__FILE__))
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->refs = NULL;
  }

  function aclGetPermissions ($attribute = '0', string $base = NULL, bool $skipWrite = FALSE): string
  {
    /* Everyone can see this tab and its attribute */
    return 'r';
  }

  public function render (): string
  {
    if ($this->refs === NULL) {
      $this->fillRefs();
    }
    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    return parent::render();
  }

  function fillRefs ()
  {
    if (!isset($this->parent)) {
      $this->refs = NULL;
      return;
    }
    $this->refs = $this->browseForeignKeys('references');
  }

  function check (): array
  {
    return [];
  }

  function save (): array
  {
    return [];
  }

  function remove (bool $fulldelete = FALSE): array
  {
    return [];
  }
}
