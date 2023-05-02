<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2021  FusionDirectory

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

/**
 * This class allows to have a simplePlugin instance which sections are dynamically loaded from other classes
 *
 * It is used for configuration plugins tab
 */
class multiPlugin extends simplePlugin
{
  protected $ignore_account = TRUE;

  protected static $tabClass = '';

  static function getAttributesInfo (): array
  {
    return [];
  }

  function __construct (string $dn = NULL, $object = NULL, $parent = NULL, bool $mainTab = FALSE)
  {
    global $config;

    $attributesInfo = [];

    /* Load sections */
    foreach ($config->data['TABS'][static::$tabClass] as $plug) {
      if (!plugin_available($plug['CLASS'])) {
        continue;
      }
      $plInfos = pluglist::pluginInfos($plug['CLASS']);

      if (isset($plInfos['plObjectClass'])) {
        $this->objectclasses = array_merge_unique($this->objectclasses, $plInfos['plObjectClass']);
      }

      $attributesInfo = array_merge($attributesInfo, $plug['CLASS']::getAttributesInfo());
    }

    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);

    /* Load sections */
    foreach ($config->data['TABS'][static::$tabClass] as $plug) {
      if (!plugin_available($plug['CLASS'])) {
        continue;
      }
      $plug['CLASS']::fixAttributesOnLoad($this);
    }
  }
}
