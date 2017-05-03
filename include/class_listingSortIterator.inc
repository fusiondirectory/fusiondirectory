<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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
  protected $data;

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
  public function __construct($data, $direction, $attribute, $type = 'string')
  {
    // Sort for attribute
    if ($attribute != '') {
      uasort(
        $data,
        function ($ao, $bo) use ($attribute, $type)
        {
          if ($attribute == 'sort-attribute') {
            // Override sort attribute from data if needed
            $attribute_a = $ao['sort-attribute'];
            $attribute_b = $bo['sort-attribute'];
          } else {
            $attribute_a = $attribute;
            $attribute_b = $attribute;
          }

          // Extract values from ao and bo
          $a = $b = '';
          if (isset($ao[$attribute_a])) {
            $a = $ao[$attribute_a];
            if (is_array($a)) {
              $a = $a[0];
            }
          }
          if (isset($bo[$attribute_b])) {
            $b = $bo[$attribute_b];
            if (is_array($b)) {
              $b = $b[0];
            }
          }

          // Take a look at the several types
          switch ($type) {
            // Sort for string by default
            default:
            case 'string':
              return strcoll($a, $b);

            case 'department':
              return strnatcmp($a, $b);

            case 'integer':
              return $b - $a;

            case 'date':
              if ($a == '') {
                $a = '31.12.0000';
              }
              if ($b == '') {
                $b = '31.12.0000';
              }
              list($d, $m, $y) = explode('.', $a);
              $a = (int)sprintf('%04d%02d%02d', $y, $m, $d);
              list($d, $m, $y) = explode('.', $b);
              $b = (int)sprintf('%04d%02d%02d', $y, $m, $d);
              return $b - $a;

            case 'ip':
              $parts_a = explode('.', $a, 4);
              $parts_b = explode('.', $b, 4);
              for ($i = 0; $i < 4; $i++) {
                if ((int)($parts_a[$i]) != (int)($parts_b[$i])) {
                  return (int)($parts_a[$i]) - (int)($parts_b[$i]);
                }
              }
              return 0;
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
