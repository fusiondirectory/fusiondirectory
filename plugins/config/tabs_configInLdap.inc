<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2014 FusionDirectory

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

class tabs_configInLdap extends tabs
{
  function save ()
  {
    global $config;
    parent::save();
    /* backup parts which are read from pluglist */
    $saved_datas = array('OBJECTS', 'SECTIONS', 'CATEGORIES', 'TABS', 'MENU');
    $save = array();
    foreach ($saved_datas as $saved_data) {
      $save[$saved_data] = $config->data[$saved_data];
    }
    /* check_and_reload */
    $config->check_and_reload(TRUE);
    /* restore backup */
    foreach ($saved_datas as $saved_data) {
      $config->data[$saved_data] = $save[$saved_data];
    }
  }
}
?>
