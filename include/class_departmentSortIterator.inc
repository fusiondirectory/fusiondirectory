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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class departmentSortIterator implements Iterator {
  private $data;

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

  function rewind() {
    return reset($this->data);
  }

  function current() {
    return current($this->data);
  }

  function key() {
    return key($this->data);
  }

  function next() {
    return next($this->data);
  }

  function valid() {
    return key($this->data) !== null;
  }
}

?>
