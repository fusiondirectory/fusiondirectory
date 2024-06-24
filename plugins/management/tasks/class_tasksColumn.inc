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

        case 'fdTasksGranularStatus':
          return static::filterStatus($value);

        // This case needs optimization
        case 'fdTasksMailObject':
          return "Mail Object";

        case 'fdTasksScheduleDate':
          return static::generateDateFormat($value);

        case 'fdTasksGranularSchedule':
          return static::generateDateFormat($value);

        case 'fdTasksGranularMaster':
          return static::generateMasterTaskName($value);

        default:
          return parent::renderSingleValue($entry, $value);
      }
    }
  }

  static function filterStatus (string $status = NULL): string
  {
    // A call towards a status map would be interesting here.
    switch ($status) {
      case "1" :
        return "Scheduled";

      case "2" :
        return "Completed";

      default :
        return $status;
    }
  }

  static function generateDateFormat ($value) : string
  {
    // Z is added to value to match ldap generaliseztime
    $datetime = (new LdapGeneralizedTime)->fromString($value.'Z');
    $result = $datetime->format('Y-m-d H:i:s');
    return $result;
  }

  static function generateMasterTaskName ($value) : string
  {
    // remove 'dn' keeping only 'cn'
    $rmDn = preg_replace('/(?=,).*/', '', $value);
    // only take the cn without dc
    preg_match('/cn=(.*)/', $rmDn, $matches);
    return $matches[1];
  }

}
