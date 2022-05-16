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
 */
class tasksColumn extends Column


{
  protected function renderSingleValue (ListingEntry $entry, string $value): string
  {
    if ($value == '') {
      return '&nbsp;';
    } else {
      switch ($this->attributes[0]) {
        case 'TYPE':
          return static::filterTypes($value, $entry['PROGRESS']);
        case 'TIMESTAMP':
          return static::filterSchedule($value);
        case 'STATUS':
          return static::filterStatus($entry->row, $value, $entry['SUBSTATUS']);
        default:
          return parent::renderSingleValue($entry, $value);
      }
    }
  }

  static function filterTypes (string $name, string $targetdn, string $targettype): string
  {
    $text = '';
    if (!empty($name) && ($name != 'none')) {
      $text = $name;
    }
    if (!empty($targetdn) && !empty($targettype)) {
      return objects::link($targetdn, $targettype, '', $text, TRUE);
    } else {
      return htmlescape($text);
    }
  }

  static function filterSchedule (string $stamp): string
  {
    if ($stamp == '19700101000000') {
      return htmlescape(_('immediately'));
    } else {
      return htmlescape(date('d.m.Y H:i:s', strtotime($stamp)));
    }
  }

  static function filterStatus (int $row, string $status, string $substatus): string
  {
    if ($status == 'waiting') {
      $status = '<img class="center" src="geticon.php?context=status&amp;icon=task-waiting&amp;size=16" alt="clock"/>&nbsp;'.htmlescape(_('Waiting'));
    }
    if ($status == 'error') {
      $status = '<input class="center" type="image" src="geticon.php?context=status&amp;icon=task-failure&amp;size=16" title="'.htmlescape(_('Show error')).'" '.
                'name="listing_showError_'.$row.'" style="padding:1px"/>'.htmlescape(_('Error'));
    }
    if ($status == 'processed') {
      $status = '<img class="center" src="geticon.php?context=status&amp;icon=task-complete&amp;size=16" alt=""/>&nbsp;'.htmlescape(_('Processed'));
    }

    /* Special handling for all entries that have
       STATUS == "processing" && PROGRESS == NUMERIC
     */
    if ($status == 'processing' && $substatus) {
      $status = $substatus;
    } elseif ($status == 'processing') {
      $status = preg_replace('/ /', '&nbsp;', htmlescape(_('in progress')));
    }

    return $status;
  }
}
