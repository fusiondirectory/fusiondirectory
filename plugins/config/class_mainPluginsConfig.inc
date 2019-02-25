<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2017 FusionDirectory

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

class mainPluginsConfig extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Plugins configuration'),
      'plDescription'   => _('FusionDirectory plugins configuration'),
      'plCategory'      => ['configuration'],
      'plObjectClass'   => ['fusionDirectoryPluginsConf'],
      'plObjectType'    => ['smallConfig'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'ogroups' => [
        'name'  => _('Object groups'),
        'attrs' => [
          new StringAttribute(
            _('Groups RDN'), _('Branch in which object groups will be stored'),
            'fdOGroupRDN', TRUE,
            'ou=groups'
          ),
        ]
      ],
      'sasl' => [
        'name'  => _('SASL'),
        'attrs' => [
          new BooleanAttribute(
            _('Force to ask for password'), _('Useful if you add a hook using password value when SASL user passwords are edited'),
            'fdForceSaslPasswordAsk', TRUE,
            FALSE
          ),
        ]
      ],
    ];
  }
}
