<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2017-2019  FusionDirectory

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
 * \brief Column showing the types of the member objects
 */
class GroupContentColumn extends Column
{
  function renderCell (ListingEntry $entry): string
  {
    global $config;

    if (strtolower($entry->getTemplatedType()) == 'ogroup') {
      $types = preg_replace('/[^a-z]/i', '', implode('', $this->getAttributeValues($entry)));
    } else {
      $types = 'U';
    }

    $result = '';
    for ($i = 0; $i < strlen($types); $i++) {
      if ($types[$i] == 'I') {
        continue;
      }
      try {
        $infos  = objects::infos(ogroup::$objectTypes[$types[$i]]);
        $result .= '<img src="'.htmlescape($infos['icon']).'"'.
                    ' alt="'.htmlescape($infos['name']).'"'.
                    ' title="'.htmlescape($infos['name']).'"/>';
      } catch (NonExistingObjectTypeException $e) {
        continue;
      }
    }
    if ($result == '') {
      return '&nbsp;';
    }
    return $result;
  }
}
