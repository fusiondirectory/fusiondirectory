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
 * \brief Column rendering Tasks
 * Important that a "query method" must be defined in order to have the listing
 */
class tasksColumn extends Column
{

  function __construct (managementListing $parent, array $attributes = NULL, string $label = NULL)
  {
    parent::__construct($parent, ['supannRessourceEtatDate', 'supannRessourceEtat'], $label);
  }

  protected function renderSingleValue (ListingEntry $entry, string $value): string
  {
    if ($value == '') {
      return '&nbsp;';
    } else {
      switch ($this->attributes[0]) {
        case 'TYPE':
          return static::filterTypes();
        case 'TIMESTAMP':
          return static::filterSchedule();
        case 'STATUS':
          return static::filterStatus();
        default:
          return parent::renderSingleValue();
      }
    }
  }

  static function filterTypes (): string
  {
    //A method retrieving the type of the tasks.
    return '';
  }

  static function filterSchedule (): string
  {
    //A method retrieving the time set by the task
     return '';
  }

  static function filterStatus (): string
  {
    //A method retrieving the status of the task
     return '';
  }
}

