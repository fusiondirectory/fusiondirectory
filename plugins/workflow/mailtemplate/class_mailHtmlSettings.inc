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

class mailHtmlSettings extends simplePlugin
{
  var $displayHeader = FALSE;

  public static function plInfo (): array
  {
    return [
      'plShortName' => _('HTML Settings'),
      'plDescription' => _('HTML Settings'),
      'plObjectClass' => ['fdMailTemplateHtmlSettings'],
      'plFilter' => '(objectClass=fdMailTemplateHtmlSettings)',
      'plObjectType' => ['mailHtmlSettings' => [
        'name' => _('Html Settings'),
        'ou' => get_ou('mailTemplateRDN'),
        'icon' => 'geticon.php?context=applications&icon=html&size=16',
      ]],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    global $config;
    return [
      // Attributes are grouped by section
      'htmlSettings' => [
        'name' => _('Template'),
        'class' => ['fullwidth'],
        'attrs' => [
          new StringAttribute(
            _('Name'), _('Name'),
            'cn', TRUE
          ),
          new StringAttribute(
            _('Description'), _('Description'),
            'description', TRUE
          ),
          new StringAttribute(
            _('Header Path'), _('Path for html template for the header'),
            'fdMailTemplateHtmlHeaderPath', TRUE
          ),
          new StringAttribute(
            _('Body Path'), _('Path for html template for the body'),
            'fdMailTemplateHtmlBodyPath', TRUE
          ),
          new StringAttribute(
            _('Footer Path'), _('Path for html template for the footer'),
            'fdMailTemplateHtmlFooterPath', TRUE
          )
        ]
      ]
    ];
  }
}
