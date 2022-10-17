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
  // Keep in mind this method is being called for each value.
  protected function renderSingleValue (ListingEntry $entry, string $value): string
  {
    if ($value == '') {
      return '&nbsp;';
    } else {
      switch ($this->attributes[0]) {
        case 'fdTasksStatus':
          return static::filterStatus($value);
        
        case 'fdTasksMailObject':
          return "Mail Object";

        default:
          return parent::renderSingleValue($entry, $value);
      }
    }
  }

  static function filterStatus (int $status = NULL): string
  {

    // A call towards a status map would be interesting here.
    switch ($status) {
      case 1 :
        return "Created";

      case 2 :
        return "Processed";

      default :
        return "Error Encountered";

    }
  }

  // Not used for now
  // Find a key value in multi dimensionnal array (multi unknown levels).
  static function in_array_r($needle, $haystack, $strict = FALSE) 
  {
    foreach ($haystack as $item) {
      if (($strict ? $item === $needle : $item == $needle) || (is_array($item) && in_array_r($needle, $item, $strict))) {
        return true;
      }
    }

    return false;
  }

}
