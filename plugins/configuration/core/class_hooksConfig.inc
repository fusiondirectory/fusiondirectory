<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2012-2019 FusionDirectory

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

class hooksConfig extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Triggers'),
      'plTitle'         => _('Triggers configuration'),
      'plPriority'      => 1,
      'plObjectType'    => ['configuration'],

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    global $config;

    $plugins = [];
    if (session::is_set('plist')) {
      $plugins = array_keys(session::get('plist')->info);
    }
    sort($plugins);

    return [
      'triggers' => [
        'name'  => _('Triggers'),
        'class' => ['fullwidth'],
        'attrs' => [
          new BooleanAttribute(
            _('Display trigger output'),
            _('When enabled successful trigger execution output is displayed to the user using a dialog.'),
            'fdDisplayHookOutput'
          ),
          new OrderedArrayAttribute(
            new CompositeAttribute(
              _('Triggers that are called when specific actions happens'),
              'fdTabHook',
              [
                new SelectAttribute(
                  _('Tab'), _('The tab that this trigger concerns'),
                  'triggerTab', TRUE,
                  $plugins
                ),
                new SelectAttribute(
                  _('Mode'), _('When to call this command'),
                  'triggerMode', TRUE,
                  ['postcreate', 'postremove', 'postmodify',
                        'precreate', 'preremove', 'premodify', 'check',
                        'prelock', 'postlock', 'preunlock', 'postunlock']
                ),
                new BooleanAttribute(
                  _('Deactivated'), _('Use this to temporarily deactivate a trigger'),
                  'triggerDecativate', TRUE,
                  FALSE, '',
                  '#', ''
                ),
                new TextAreaAttribute(
                  _('Command'), _('The command that will be called'),
                  'triggerCmd', TRUE
                )
              ],
              '/^([^\\|]+)\\|([^\\|]+)\\|(#?)(.*)$/',
              '%s|%s|%s%s',
              '', // acl
              _('Triggers')
            ),
            FALSE, // non-ordered
            [],
            TRUE // edition
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;

    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['fdTabHook']->setLinearRendering(FALSE);
    $this->attributesAccess['fdTabHook']->setHeaders([
      _('Tab'),
      _('Mode'),
      _('Deactivated'),
      _('Command'),
      '',
    ]);
  }
}
