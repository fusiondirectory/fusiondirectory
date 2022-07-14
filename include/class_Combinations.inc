<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2018  FusionDirectory

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

/*!brief Iterator that returns all combinations of $size element from $input array
 */
class Combinations implements Iterator
{
  protected $current  = NULL;
  protected $input    = NULL;
  protected $n        = 0;
  protected $size     = 0;
  protected $pos      = 0;

  function __construct(array $input, $size)
  {
    $this->input = array_values($input);
    $this->n = count($this->input);
    $this->size = $size;
    $this->rewind();
  }

  function key()
  {
    return $this->pos;
  }

  function current()
  {
    $r = array();
    for ($i = 0; $i < $this->size; $i++) {
      $r[] = $this->input[$this->current[$i]];
    }
    return $r;
  }

  function next()
  {
    if ($this->_next()) {
      $this->pos++;
    } else {
      $this->pos = -1;
    }
  }

  function rewind()
  {
    $this->current  = range(0, $this->size);
    $this->pos      = 0;
  }

  function valid()
  {
    return ($this->pos >= 0);
  }

  protected function _next()
  {
    $i = $this->size - 1;
    while (($i >= 0) && ($this->current[$i] == $this->n - $this->size + $i)) {
      $i--;
    }
    if ($i < 0) {
      return FALSE;
    }
    $this->current[$i]++;
    while ($i++ < $this->size - 1) {
      $this->current[$i] = $this->current[$i - 1] + 1;
    }
    return TRUE;
  }
}
