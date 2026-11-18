<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2026 FusionDirectory

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

class mailTemplateText extends simplePlugin
{
  var $displayHeader = TRUE;

  public static function plInfo (): array
  {
    return [
      'plShortName' => _('Text'),
      'plDescription' => _('Text'),
      'plObjectClass' => ['fdMailTemplateText'],
      'plFilter' => '(objectClass=fdMailTemplateText)',
      'plObjectType' => ['mailTemplate'],
      'plConflicts' => [ 'mailTemplateHtml' ],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    global $config;
    return [
      // Attributes are grouped by section
      'text' => [
        'name' => _('Text'),
        'attrs' => [
          new TextAreaAttribute(
            _('Body'), _('Text to be sent to recipient'),
            'fdMailTemplateBody', TRUE
          ),
          new TextAreaAttribute(
            _('Signature'), _('Signature'),
            'fdMailTemplateSignature', FALSE
          )
        ]
      ],
    ];
  }
}
