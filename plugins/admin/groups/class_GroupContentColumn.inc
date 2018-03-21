<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018  FusionDirectory

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
  function isSortable()
  {
    return FALSE;
  }

  function fillNeededAttributes(array &$attrs)
  {
    $attrs['gosaGroupObjects'] = '1';
  }

  function renderCell(ListingEntry $entry)
  {
    global $config;

    if (strtolower($entry->getTemplatedType()) == 'ogroup') {
      $types = preg_replace('/[^a-z]/i', '', $entry['gosaGroupObjects']);
    } else {
      $types = 'U';
    }

    $result = '';
    for ($i = 0; $i < strlen($types); $i++) {
      if ($types[$i] == 'I') {
        continue;
      }
      $infos  = objects::infos(ogroup::$objectTypes[$types[$i]]);
      $result .= '<img src="'.htmlentities($infos['icon'], ENT_COMPAT, 'UTF-8').'"'.
                  ' alt="'.htmlentities($infos['name'], ENT_COMPAT, 'UTF-8').'"'.
                  ' title="'.htmlentities($infos['name'], ENT_COMPAT, 'UTF-8').'"/>';
    }
    if ($result == '') {
      return '&nbsp;';
    }
    return $result;
  }
}
