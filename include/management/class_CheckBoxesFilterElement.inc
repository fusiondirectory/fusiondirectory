<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2017-2019  FusionDirectory

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

class CheckBoxesFilterElement extends FilterElement
{
  protected $items;
  protected $name;
  protected $operator;

  /*!
   * \brief __construct
   *
   * \param managementFilter $parent
   * \param string $name Name of the filter element
   * \param array $items Associative array of items with keys: name, ?desc, filter, ?icon
   */
  public function __construct (managementFilter $parent, string $name, array $items, string $operator = '&')
  {
    global $config;

    parent::__construct($parent);

    $this->name     = $name;
    $this->items    = $items;
    $this->operator = $operator;

    foreach ($this->items as &$item) {
      $item['checked'] = FALSE;
    }
    unset($item);
  }

  public function update ()
  {
    foreach ($this->items as $key => &$item) {
      $item['checked'] = isset($_POST['filter_item_'.$key]);
    }
    unset($item);
  }

  public function render (): string
  {
    if (empty($this->items)) {
      return '';
    }
    $inputs = [];
    foreach ($this->items as $key => $item) {
      $inputs['filter_item_'.$key] = [
        'name'    => ($item['name'] ?? $key),
        'desc'    => ($item['desc'] ?? $item['name'] ?? $key).' '.$item['filter'],
        'icon'    => ($item['icon'] ?? NULL),
        'checked' => $item['checked'],
      ];
    }
    $smarty = get_smarty();
    $smarty->assign('NAME',   $this->name);
    $smarty->assign('INPUTS', $inputs);
    return $smarty->fetch(get_template_path('management/filter-element.tpl'));
  }

  public function getFilters (string $type, array &$filters): bool
  {
    if ($this->operator == '&') {
      foreach ($this->items as $item) {
        if ($item['checked']) {
          $filters[] = $item['filter'];
        }
      }
    } else {
      $subfilters = [];
      foreach ($this->items as $item) {
        if ($item['checked']) {
          $subfilters[] = $item['filter'];
        }
      }
      if (!empty($subfilters)) {
        $filter = new ldapFilter($this->operator, $subfilters);
        $filters[] = (string)$filter;
      }
    }
    return FALSE;
  }
}
