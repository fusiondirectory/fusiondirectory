<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007 Fabian Hickert
  Copyright (C) 2011-2016  FusionDirectory

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

class setupStep extends simplePlugin
{
  protected $s_short_name   = 'Still undefined';
  protected $s_title        = 'Still undefined';
  protected $s_description  = 'Still undefined';
  var $is_active    = FALSE;
  var $is_enabled   = FALSE;
  var $is_completed = FALSE;

  var $header_image = '';

  static function getAttributesInfo()
  {
    return array();
  }

  function __construct()
  {
    global $config;
    parent::__construct($config);
    $this->update_strings();
  }

  function update_strings()
  {
    die('abstract method');
  }

  function get_short_name()
  {
    return $this->s_short_name;
  }

  function get_title()
  {
    return $this->s_title;
  }

  function get_description()
  {
    return $this->s_description;
  }

  function is_active()
  {
    return $this->is_active;
  }

  function is_enabled()
  {
    return $this->is_enabled;
  }

  function is_completed()
  {
    return $this->is_completed;
  }

  function set_active($value = TRUE)
  {
    $this->is_active = ($value == TRUE);
  }

  function set_enabled($value = TRUE)
  {
    $this->is_enabled = ($value == TRUE);
  }

  function set_completed($value = TRUE)
  {
    $this->is_completed = ($value == TRUE);
  }

  /* Return attributes handled by this setup step */
  function get_attributes()
  {
    $tmp = array();
    foreach (array_keys($this->attributesAccess) as $attr) {
      $tmp[$attr] = $this->$attr;
    }
    return $tmp;
  }

  /* bypass LDAP loading */
  protected function loadAttributes()
  {
    foreach ($this->attributesInfo as &$sectionInfo) {
      foreach ($sectionInfo['attrs'] as $name => &$attr) {
        $attr->setParent($this);
      }
      unset($attr);
    }
    unset($sectionInfo);

    $this->updateAttributesValues();
  }
}
?>
