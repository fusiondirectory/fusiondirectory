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
   * */
  protected $attributes;
  /*! \brief Same thing for templates, if it differs */
  protected $templateAttributes = NULL;
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
  static function build (managementListing $parent, $type, array $data): Column
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

  function __construct (managementListing $parent, array $attributes = NULL, string $label = NULL)
  {
    $this->parent = $parent;
    $this->label  = $label;
    $this->setAttributesVar('attributes', $attributes);
  }

  protected function setAttributesVar (string $var, array $attributes = NULL)
  {
    $this->$var = $attributes;
  }

  function setTemplateAttributes (array $attributes = NULL)
  {
    $this->setAttributesVar('templateAttributes', $attributes);
  }

  function isSortable (): bool
  {
    return TRUE;
  }

  function isExportable (): bool
  {
    return !empty($this->attributes);
  }

  function getHtmlProps (): string
  {
    return '';
  }

  function getHtmlCellProps (): string
  {
    return '';
  }

  function getLabel (): string
  {
    if (isset($this->label)) {
      return _($this->label);
    } else {
      return ' ';
    }
  }

  function fillNeededAttributes (array &$attrs)
  {
    if (isset($this->attributes)) {
      foreach ($this->attributes as $attr) {
        if (($attr == 'mainAttr') || ($attr == 'nameAttr')) {
          /* nameAttr and mainAttr as always set as needed in managementFilter */
          continue;
        } elseif ($attr == 'dn') {
          /* Handle special case of dn */
          $attrs[$attr] = 'raw';
        } else {
          /* Get all values from other attributes */
          $attrs[$attr] = '*';
        }
      }
    }
  }

  function fillSearchedAttributes (array &$attrs)
  {
    if (isset($this->attributes)) {
      foreach ($this->attributes as $attr) {
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

  function fillRowClasses (&$classes, ListingEntry $entry)
  {
  }

  protected function getAttributeValues (ListingEntry $entry): array
  {
    $attrs = $this->attributes;
    if (isset($this->templateAttributes) && $entry->isTemplate()) {
      $attrs = $this->templateAttributes;
    }
    if (isset($attrs)) {
      foreach ($attrs as $attr) {
        if (($attr == 'mainAttr') || ($attr == 'nameAttr')) {
          $infos  = objects::infos($entry->getTemplatedType());
          $attr   = $infos[$attr];
        }
        if (isset($entry[$attr])) {
          if (is_array($entry[$attr])) {
            return $entry[$attr];
          } else {
            /* Should only happen for dn */
            return [$entry[$attr]];
          }
        }
      }
    }
    return [];
  }

  function renderCell (ListingEntry $entry): string
  {
    $values = $this->getAttributeValues($entry);
    if (empty($values)) {
      return '&nbsp;';
    } else {
      return implode("<br/>\n",
        array_map(
          function ($value) use ($entry)
          {
            return $this->renderSingleValue($entry, $value);
          },
          $values
        )
      );
    }
  }

  protected function renderSingleValue (ListingEntry $entry, string $value): string
  {
    return htmlentities($value, ENT_COMPAT, 'UTF-8');
  }

  function getRawExportValues (ListingEntry $entry): array
  {
    return $this->getAttributeValues($entry);
  }

  function compare (ListingEntry $ao, ListingEntry $bo): int
  {
    $a = $this->getAttributeValues($ao)[0] ?? '';
    $b = $this->getAttributeValues($bo)[0] ?? '';

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
