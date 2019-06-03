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

/* Debug level is an OR combination of several values */
class DebugLevelAttribute extends SetAttribute
{
  protected function loadAttrValue (array $attrs)
  {
    if (isset($attrs[$this->getLdapName()])) {
      $value = $attrs[$this->getLdapName()][0];
      $this->value = [];
      foreach ($this->attribute->getChoices() as $choice) {
        if ($value & $choice) {
          $this->value[] = $choice;
        }
      }
    } else {
      $this->resetToDefault();
    }
  }

  function computeLdapValue ()
  {
    $value = 0;
    foreach ($this->value as $v) {
      $value |= $v;
    }
    return $value;
  }
}
