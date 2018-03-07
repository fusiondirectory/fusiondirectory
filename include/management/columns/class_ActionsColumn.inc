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
 * \brief Column showing the actions available on the object
 */
class ActionsColumn extends Column
{
  function isSortable()
  {
    return FALSE;
  }

  function fillRowClasses(&$classes, ListingEntry $entry)
  {
    $this->parent->parent->fillActionRowClasses($classes, $entry);
  }

  function renderCell(ListingEntry $entry)
  {
    return $this->parent->parent->renderActionColumn($entry);
  }
}
