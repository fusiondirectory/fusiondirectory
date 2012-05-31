<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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
 * \file class_departmentSortIterator.inc
 * Source code for class departmentSortIterator
 */

/*!
 * \brief This class contains all the function needed to sort department
 */
class departmentSortIterator implements Iterator {
  private $data;

  /*!
   * \brief departmentSortIterator constructor
   *
   * \param array $data departmentSortIterator data
   *
   * \param $direction Direction
   */ 
  public function __construct($data, $direction) {

    function depSort($ao, $bo) {
      // Override sort attribute from data if needed
      $attribute_a= $ao['sort-attribute'];
      $attribute_b= $bo['sort-attribute'];

      // Extract values from ao and bo
      $a= $b= "";
      if (isset($ao[$attribute_a])) {
        $a= $ao[$attribute_a];
        if (is_array($a)) {
          $a= $a[0];
        }
      }
      if (isset($bo[$attribute_b])) {
        $b= $bo[$attribute_b];
        if (is_array($b)) {
          $b= $b[0];
        }
      }

      // Sort for string by default
      return strnatcmp($a, $b);
    }

    // Sort for attribute
    uasort($data, "depSort");

    // Invert if direction is set
    if ($direction) {
      $this->data= array_reverse($data, true);
    } else {
      $this->data= $data;
    }
  }

  /*!
   * \brief Put the array pointer to the first element
   *
   * \return the first element of the array
   */ 
  function rewind() {
    return reset($this->data);
  }
  
  /*!
   * \brief Get the current data element
   * 
   * \return The current element pointed by array pointer
   */
  function current() {
    return current($this->data);
  }
  
  /*!
   * \brief Get the key element
   *
   * \return the key element of the array
   */
  function key() {
    return key($this->data);
  }
  
  /*!
   * \brief Get the next data element
   *
   * \return The next element pointed by array pointer
   */
  function next() {
    return next($this->data);
  }

  /*!
   * \brief Check if the data array is valid
   *
   * \return TRUE if the array is valid, return FALSE otherwise
   */
  function valid() {
    return key($this->data) !== null;
  }
}

?>
