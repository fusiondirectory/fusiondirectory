<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2011-2022  FusionDirectory

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

/*!
 * \brief Column rendering Tasks columns
 */
class TasksColumn extends Column
{
  protected function renderSingleValue (ListingEntry $entry, string $value): string
  {
    if ($value == '') {
      return '&nbsp;';
    } else {
      print_r($this->attributes);
      switch ($this->attributes[0]) {
        case 'fdTasksStatus':
          return static::filterStatus();
        
        case 'fdTasksMailObject':
          return static::filterType();
        
        default:
          return parent::renderSingleValue($entry, $value);
      }
    }
  }

  static function filterStatus (): string
  {

    $status = "test_status";
    return $status;
  }

  static function filtertype (): string
  {
    $type ="test_type";
    return $type;
  }

  static function in_array_r($needle, $haystack, $strict = false) 
  {
    foreach ($haystack as $item) {
      if (($strict ? $item === $needle : $item == $needle) || (is_array($item) && in_array_r($needle, $item, $strict))) {
        return true;
      }
    }

    return false;
  }

}
