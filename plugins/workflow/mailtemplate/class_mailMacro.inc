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

class mailMacro extends simplePlugin
{
  var $displayHeader = TRUE;

  public static function plInfo (): array
  {
    return [
      'plShortName'    => _('Mail Macro'),
      'plDescription'  => _('Mail Macro'),
      'plObjectClass'  => ['fdMailTemplateMacroObject'],
      'plFilter'       => '(objectClass=fdMailTemplateMacroObject)',
      'plObjectType'   => ['mailTemplate'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    global $config;
    return [
      // Attributes are grouped by section
      'mailMacro' => [
        'name'  => _('Mail Macro'),
        'class' => ['fullwidth'],
        'attrs' => [
          new OrderedArrayAttribute(
            new PipeSeparatedCompositeAttribute(
              _('Definition of the attributes to be asked'),
              'fdMailTemplateMacro',
              [
                new StringAttribute(
                  _('Mailtemplate macro name'),
                  _('Name of the macro that will be between percent in the Mailtemplate'),
                  'fdMailTemplateMacroName',
                  TRUE,
                ),
                new StringAttribute(
                  _('Mailtemplate macro value'),
                  _('Attribute that will be taken from the LDAP information of the user'),
                  'fdMailtemplateMacroValue',
                  TRUE,
                ),
              ],
              '',
              'List of Mailtemplate macros',
            ),
            $order = FALSE,
            $values = [],
            $edit_enabled = TRUE
          )
        ]
      ],
    ];
  }
}
