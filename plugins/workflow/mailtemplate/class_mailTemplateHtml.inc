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

class mailTemplateHtml extends simplePlugin
{
  var $displayHeader = TRUE;

  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Mail HTML'),
      'plDescription' => _('Mail HTML'),
      'plObjectClass' => ['fdMailHtml'],
      'plFilter'      => '(objectClass=fdMailHtml)',
      'plObjectType'  => ['mailTemplate'],
      'plConflicts'   => [
        'mailTemplateText',
      ],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    global $config;
    return [
      'textHTML' => [
        'name'  => _('Mail HTML'),
        'class' => ['fullwidth'],
        'attrs' => [
          new SelectAttribute(
            _('Header HTML'), _('Header HTML file'),
            'fdMailTemplateHtmlHeader', FALSE, ["headerArriveEtudiant"]
          ),
          new SelectAttribute(
            _('Body HTML'), _('Body HTML file'),
            'fdMailTemplateHtmlBody', FALSE, ["bodyArriveEtudiant"]
          ),
          new SelectAttribute(
            _('Footer HTML'), _('Footer HTML file'),
            'fdMailTemplateHtmlFooter', FALSE, ["footerArriveEtudiant"]
          ),
        ]
      ]
    ];
  }
}
