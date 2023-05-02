<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2018-2023  FusionDirectory

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
* \brief Column rendering EpochDays attributes
*/
class EpochDaysColumn extends LinkColumn
{
  protected $type = 'integer';

  protected function renderSingleValue (ListingEntry $entry, string $value): string
  {
    if (!empty($value)) {
      // Multiply epoch day received as value by numbers of seconds in a day.
      $epochInSeconds = (int) $value * 86400;
      // casting is required for DateTime; It expects string.
      $value = (string) $epochInSeconds;
      $dateObject = DateTime::createFromFormat('U', $value, new DateTimeZone('UTC'));
      if (is_object($dateObject)) {
        return $this->renderLink($entry, $dateObject->format('d.m.Y'));
      }
    }
    return '&nbsp;';
  }
}
