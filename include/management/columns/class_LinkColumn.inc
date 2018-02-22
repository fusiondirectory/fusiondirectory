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
 * \brief Column rendering a link to edit the object
 */
class LinkColumn extends Column
{
  function renderCell(ListingEntry $entry)
  {
    $attribute = $this->attribute;
    if (isset($this->templateAttribute) && $entry->isTemplate()) {
      $attribute = $this->templateAttribute;
    }
    if (isset($entry[$attribute])) {
      return '<a href="?plug='.$_GET['plug'].'&amp;PID='.$entry->getPid().'&amp;act=listing_edit_'.$entry->row.'" title="'.$entry->dn.'">'.htmlentities($entry[$attribute], ENT_COMPAT, 'UTF-8').'</a>';
    } else {
      return '&nbsp;';
    }
  }
}
