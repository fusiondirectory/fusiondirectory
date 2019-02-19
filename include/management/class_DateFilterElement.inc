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
class DateFilterElement extends FilterElement
{
  protected $attribute;
  protected $label;
  protected $min;
  protected $max;
  protected $types;

  public function __construct (managementFilter $parent, $attribute, $label, $types = array())
  {
    global $config;

    parent::__construct($parent);

    $this->attribute  = $attribute;
    $this->label      = $label;
    $this->types      = $types;
  }

  public function update ()
  {
    if (!empty($_POST['nt_'.$this->attribute])) {
      $this->min = $_POST['nt_'.$this->attribute];
    } else {
      $this->min = '';
    }
    if (!empty($_POST['ot_'.$this->attribute])) {
      $this->max = $_POST['ot_'.$this->attribute];
    } else {
      $this->max = '';
    }
  }

  public function render ()
  {
    $smarty = get_smarty();
    $smarty->assign('NAME',       $this->label);
    $smarty->assign('attribute',  $this->attribute);
    $smarty->assign('nt_value',   $this->min);
    $smarty->assign('ot_value',   $this->max);
    return $smarty->fetch(get_template_path('management/filter-element-date.tpl'));
  }

  public function getFilters ($type, array &$filters)
  {
    if (!empty($this->types) && !in_array_ics($type, $this->types)) {
      return FALSE;
    }
    if (!empty($this->min)) {
      $filters[] = '('.$this->attribute.'>='.LdapGeneralizedTime::toString(new DateTime($this->min)).')';
    }
    if (!empty($this->max)) {
      $filters[] = '('.$this->attribute.'<='.LdapGeneralizedTime::toString(new DateTime($this->max)).')';
    }
    return FALSE;
  }
}
