<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2018  FusionDirectory

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
 * \brief This class contains all the function needed to sort list
 * go up, go down , back , next. etc...
 */
class EntrySortIterator implements Iterator
{
  protected $data;

  /*!
   * \brief EntrySortIterator constructor
   *
   * \param array $entries entries array
   *
   * \param Column $column Column to sort by
   *
   * \param bool $direction Direction
   */
  public function __construct(array $entries, Column $column, $direction)
  {
    // Sort for attribute
    if (is_object($column)) {
      uasort(
        $entries,
        function ($ao, $bo) use ($column)
        {
          return $column->compare($ao, $bo);
        }
      );
    }

    // Invert if direction is set
    if ($direction) {
      $this->data = array_reverse($entries, TRUE);
    } else {
      $this->data = $entries;
    }
  }

  /*!
   * \brief Put the array pointer to the first element
   *
   * \return the first element of the array
   */
  function rewind()
  {
    return reset($this->data);
  }

  /*!
   * \brief Get the current data element
   *
   * \return The current element pointed by array pointer
   */
  function current()
  {
    return current($this->data);
  }

  /*!
   * \brief Get the key element
   *
   * \return the key element of the array
   */
  function key()
  {
    return key($this->data);
  }

  /*!
   * \brief Get the next data element
   *
   * \return The next element pointed by array pointer
   */
  function next()
  {
    return next($this->data);
  }

  /*!
   * \brief Check if the data array is valid
   *
   * \return TRUE if the array is valid, return FALSE otherwise
   */
  function valid()
  {
    return (key($this->data) !== NULL);
  }
}
