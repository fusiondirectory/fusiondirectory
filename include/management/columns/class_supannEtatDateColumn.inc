<?php
/*
This code is part of FusionDirectory (http://www.fusiondirectory.org/)

Copyright (C) 2025 FusionDirectory

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
 * Note : Class used to parse supannRessourceEtatDate in a readable human format.
 */
class SupannEtatDateColumn extends LinkColumn
{
  /**
   * @param ListingEntry $entry
   * @param string $value
   * @return string
   * Note : Allows to receive an attribute value and return it to readable format.
   */
  protected function renderSingleValue (ListingEntry $entry, string $value): string
  {
    if (!empty($value)) {
      $parts        = explode(':', $value);
      $startDateStr = $parts[2] ?? '';
      $endDateStr   = $parts[3] ?? '';

      $startDate = $this->parseDate($startDateStr);
      $endDate   = $this->parseDate($endDateStr);

      $text = $parts[0] . ' : ' . $this->buildDateText($startDate, $endDate);

      return $this->renderLink($entry, $text);
    }
    return '&nbsp;';
  }

  /**
   * @param string $dateStr
   * @return DateTimeImmutable|null
   * Note :
   */
  private function parseDate (string $dateStr): ?DateTimeImmutable
  {
    // Veirfy if the date received is of proper stored format.
    if (strlen($dateStr) === 8) {
      $year  = (int)substr($dateStr, 0, 4);
      $month = (int)substr($dateStr, 4, 2);
      $day   = (int)substr($dateStr, 6, 2);

      try {
        return (new DateTimeImmutable())->setDate($year, $month, $day);
      } catch (\Exception $e) {
      }
    }
    return NULL;
  }

  /**
   * @param DateTimeImmutable|null $startDate
   * @param DateTimeImmutable|null $endDate
   * @return string
   * Note : Once date have been formated, we simply add small 'from' and to 'text' string before returning.
   */
  private function buildDateText (?DateTimeImmutable $startDate, ?DateTimeImmutable $endDate): string
  {
    if ($startDate !== NULL && $endDate !== NULL) {
      return 'From ' . $startDate->format('d/m/Y') . ' to ' . $endDate->format('d/m/Y');
    }
    if ($startDate !== NULL) {
      return 'From ' . $startDate->format('d/m/Y');
    }
    if ($endDate !== NULL) {
      return 'Until ' . $endDate->format('d/m/Y');
    }
    return 'No start or end dates set';
  }
}