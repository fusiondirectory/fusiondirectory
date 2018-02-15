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
 * \brief Column base class
 */
class StringColumn
{
  protected $attribute;
  protected $label;
  protected $type = 'string';

  function __construct($attribute, $label)
  {
    $this->attribute  = $attribute;
    $this->label      = $label;
  }

  function isSortable()
  {
    return TRUE;
  }

  function getRowClasses()
  {
    return array();
  }

  function getHtmlProps()
  {
    return '';
  }

  function getHtmlCellProps()
  {
    return '';
  }

  function getLabel()
  {
    return _($this->label);
    //~ return $this->label;
  }

  function fillNeededAttributes(array &$attrs)
  {
    $attrs[$this->attribute] = '1';
  }

  function renderCell(string $pid, int $row, string $dn, array $entry)
  {
    if (isset($entry[$this->attribute])) {
      return htmlentities($entry[$this->attribute], ENT_COMPAT, 'UTF-8');
    } else {
      return '&nbsp;';
    }
  }

  function compare($ao, $bo)
  {
    if ($this->attribute == 'sort-attribute') {
      // Override sort attribute from data if needed
      $attribute_a = $ao['sort-attribute'];
      $attribute_b = $bo['sort-attribute'];
    } else {
      $attribute_a = $this->attribute;
      $attribute_b = $this->attribute;
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
    switch ($this->type) {
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
}

class LinkColumn extends StringColumn
{
  function renderCell(string $pid, int $row, string $dn, array $entry)
  {
    if (isset($entry[$this->attribute])) {
      return '<a href="?plug='.$_GET['plug'].'&amp;PID='.$pid.'&amp;act=listing_edit_'.$row.'" title="'.$dn.'">'.htmlentities($entry[$this->attribute], ENT_COMPAT, 'UTF-8').'</a>';
    } else {
      return '&nbsp;';
    }
  }
}
