<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2022 FusionDirectory

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

class macrosConfig extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Macro'),
      'plDescription'   => _('FusionDirectory macro configuration tool'),
      'plObjectClass'   => ['fdMacroConf'],
      'plObjectType'    => ['configuration'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'macrosConf' => [
        'name'  => _('Macros'),
        'attrs' => [
          new StringAttribute(
            _('Macros RDN'), _('Branch in which Directory Base, Macro will be stored'),
            'fdMacroRDN', TRUE,
            'ou=macroConfig'
          ),
        ]
      ],
      'Macro' => [
        'name'  => _('Mail Object'),
        'attrs' => [
         new SetAttribute(
            new CompositeAttribute(
              _('Macro definitions'),
              'fdMacroMail',
              [
                new SelectAttribute(
                  '', _('Macro type'),
                  'fdMacroTypes', TRUE
                ),
               new StringAttribute(
                  _('Macros User Definition'), _('Macro that will be used'),
                  'fdMacroOption', TRUE, ''
                ),
              ],
              '/^{(.+)}(.*)$/',
              '{%s}%s',
              '',
              _('Macro settings (wrap the macro with % sign)')
            )
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;

    parent::__construct($dn, $object, $parent, $mainTab);

    /* supannTelephonePrive */
    $MacroTypes = [
      'LOGIN'    => _('Login'),
      'UID'   => _('UID'),
      'TOKEN'     => _('Token'),
    ];
    foreach ($config->get_cfg_value('MacroTypes', []) as $line) {
      list($type,$label) = explode(':', $line, 2);
      $MacroTypes[$type] = $label;
    }
    $this->attributesAccess['fdMacroMail']->attribute->attributes[0]->setChoices(
      array_keys($MacroTypes),
      array_values($MacroTypes)
    );

  }

}

