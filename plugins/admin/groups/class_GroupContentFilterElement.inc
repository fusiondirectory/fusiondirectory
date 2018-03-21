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
 * \brief Filter on member types
 */
class GroupContentFilterElement extends FilterElement
{
  protected $types;

  public function __construct(managementFilter $parent)
  {
    global $config;

    parent::__construct($parent);

    $this->types = array();
    foreach (ogroup::$objectTypes as $key => $type) {
      $infos = objects::infos($type);
      $this->types[$type] = array(
        'filter'  => '(gosaGroupObjects=*'.$key.'*)',
        'infos'   => $infos,
        'checked' => FALSE,
      );
    }
  }

  public function update()
  {
    foreach ($this->types as $type => &$infos) {
      $infos['checked'] = isset($_POST['filter_type_'.$type]);
    }
    unset($infos);
  }

  public function render()
  {
    $inputs = array();
    foreach ($this->types as $type => $infos) {
      $inputs['filter_type_'.$type] = array(
        'name'    => $infos['infos']['name'],
        'desc'    => (isset($infos['infos']['description']) ? $infos['infos']['description'] : $infos['infos']['name']).' '.$infos['filter'],
        'icon'    => (isset($infos['infos']['icon']) ? $infos['infos']['icon'] : NULL),
        'checked' => $infos['checked'],
      );
    }
    $smarty = get_smarty();
    $smarty->assign('NAME',   _('Members'));
    $smarty->assign('INPUTS', $inputs);
    return $smarty->fetch(get_template_path('management/filter-element.tpl'));
  }

  public function getFilters($type, &$filters)
  {
    if (strtoupper($type) == 'OGROUP') {
      foreach ($this->types as $type => $infos) {
        if ($infos['checked']) {
          $filters[] = $infos['filter'];
        }
      }
    } elseif (!$this->types['user']['checked']) {
      /* Skip POSIX groups and roles if users are unchecked and at least one type is checked */
      foreach ($this->types as $type => $infos) {
        if ($infos['checked']) {
          return TRUE;
        }
      }
    }
    return FALSE;
  }
}
