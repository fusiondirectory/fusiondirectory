<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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
 * \file class_listingSortIterator.inc
 * Source code for class listingSortIterator
 */

/*!
 * \brief This class contains all the function needed to sort list
 * go up, go down , back , next. etc...
 */
class listingSortIterator implements Iterator {
  private $data;

  /*!
   * \brief listingSortIterator constructor
   *
   * \param array $data listingSortIterator data
   *
   * \param $direction Direction
   *
   * \param $attribute Attribute
   *
   * \param string $type
   */
  public function __construct($data, $direction, $attribute, $type = "string")
  {
    global $_sortAttribute;
    global $_sortType;
    $_sortAttribute = $attribute;
    $_sortType      = $type;

    // Sort for attribute
    if ($attribute != "") {
      uasort(
        $data,
        function ($ao, $bo)
        {
          global $_sortAttribute;
          global $_sortType;

          // Extract values from ao and bo
          $a = $b = "";
          if (isset($ao[$_sortAttribute])) {
            $a = $ao[$_sortAttribute];
            if (is_array($a)) {
              $a = $a[0];
            }
          }
          if (isset($bo[$_sortAttribute])) {
            $b = $bo[$_sortAttribute];
            if (is_array($b)) {
              $b = $b[0];
            }
          }

          // Take a look at the several types
          switch ($_sortType) {
            case 'string':
              return strcoll($a, $b);

            case 'integer':
              return $b - $a;

            case 'date':
              if ($a == "") {
                $a = "31.12.0000";
              }
              if ($b == "") {
                $b = "31.12.0000";
              }
              list($d, $m, $y) = explode('.', $a);
              $a = (int)sprintf("%04d%02d%02d", $y, $m, $d);
              list($d, $m, $y) = explode('.', $b);
              $b = (int)sprintf("%04d%02d%02d", $y, $m, $d);
              return $b - $a;

            // Sort for string by default
            default:
              return strcoll($a, $b);
          }
        }
      );
    }

    // Invert if direction is set
    if ($direction) {
      $this->data = array_reverse($data, TRUE);
    } else {
      $this->data = $data;
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
?>
