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
class Column
{
  /*! \brief Array of attributes to look for, ordered by priority
   * The first non-empty attribute will be displayed
   * The array is organized as the one passed to objects::ls
   * */
  private $attributes;
  /*! \brief Same thing for templates, if it differs */
  private $templateAttributes = NULL;
  protected $label;
  protected $type = 'string';


  /* management class instance */
  protected $parent = NULL;

  /*!
   * \brief Builds a column object from given data
   *
   * \param string $type a column class
   * \param array $data an associative array with "attributes" and "label"
   * */
  static function build(managementListing $parent, $type, array $data)
  {
    $attributes = NULL;
    $label      = NULL;
    if (isset($data['attributes'])) {
      $attributes = $data['attributes'];
      if (!is_array($attributes)) {
        $attributes = array_map('trim', explode(',', $attributes));
      }
    }
    if (isset($data['label'])) {
      $label = $data['label'];
    }

    return new $type($parent, $attributes, $label);
  }

  function __construct(managementListing $parent, array $attributes = NULL, $label = NULL)
  {
    $this->parent     = $parent;
    $this->setAttributesVar('attributes', $attributes);
    $this->label      = $label;
  }

  protected function setAttributesVar($var, array $attributes = NULL)
  {
    if (is_array($attributes) && is_numeric(key($attributes))) {
      $val = array();
      foreach ($attributes as $attribute) {
        if ($attribute == 'dn') {
          $val[$attribute] = 'raw';
        } else {
          $val[$attribute] = 1;
        }
      }
      $this->$var = $val;
    } else {
      $this->$var = $attributes;
    }
  }

  function setTemplateAttributes(array $attributes = NULL)
  {
    $this->setAttributesVar('templateAttributes', $attributes);
  }

  function isSortable()
  {
    return TRUE;
  }

  function isExportable()
  {
    return !empty($this->attributes);
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
    if (isset($this->label)) {
      return _($this->label);
    } else {
      return ' ';
    }
  }

  function fillNeededAttributes(array &$attrs)
  {
    if (isset($this->attributes)) {
      foreach ($this->attributes as $attr => $how) {
        if (($attr == 'mainAttr') || ($attr == 'nameAttr')) {
          /* nameAttr and mainAttr as always set as needed in managementFilter */
          continue;
        }
        $attrs[$attr] = $how;
      }
    }
  }

  function fillSearchedAttributes(array &$attrs)
  {
    if (isset($this->attributes)) {
      foreach (array_keys($this->attributes) as $attr) {
        if (($attr == 'mainAttr') || ($attr == 'nameAttr')) {
          /* nameAttr and mainAttr as always searched for */
          continue;
        }
        if ($attr != 'dn') {
          $attrs[] = $attr;
        }
      }
    }
  }

  function fillRowClasses(&$classes, ListingEntry $entry)
  {
  }

  protected function getAttributeValue(ListingEntry $entry)
  {
    $attrs = $this->attributes;
    if (isset($this->templateAttributes) && $entry->isTemplate()) {
      $attrs = $this->templateAttributes;
    }
    if (isset($attrs)) {
      foreach (array_keys($attrs) as $attr) {
        if (($attr == 'mainAttr') || ($attr == 'nameAttr')) {
          $infos  = objects::infos($entry->getTemplatedType());
          $attr   = $infos[$attr];
        }
        if (isset($entry[$attr])) {
          return $entry[$attr];
        }
      }
    }
    return '';
  }

  function renderCell(ListingEntry $entry)
  {
    $value = $this->getAttributeValue($entry);
    if ($value == '') {
      return '&nbsp;';
    } else {
      return htmlentities($value, ENT_COMPAT, 'UTF-8');
    }
  }

  function getRawExportValue(ListingEntry $entry)
  {
    return $this->getAttributeValue($entry);
  }

  function compare(ListingEntry $ao, ListingEntry $bo)
  {
    $a = $this->getAttributeValue($ao);
    $b = $this->getAttributeValue($bo);
    if (is_array($a)) {
      $a = $a[0];
    }
    if (is_array($b)) {
      $b = $b[0];
    }

    // Take a look at the several types
    switch ($this->type) {
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

      // Sort for string by default
      case 'string':
      default:
        return strcoll($a, $b);
    }
  }
}
