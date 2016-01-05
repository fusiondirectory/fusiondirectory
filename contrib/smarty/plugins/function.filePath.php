<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2016  FusionDirectory

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
function smarty_function_filePath($params, &$smarty)
{
  $config = session::global_get('config');

  /* Set theme */
  if (isset ($config)) {
    $theme = $config->get_cfg_value("theme", "default");
  } else {
    $theme = "default";
  }

  $filename = $params['file'];

  if (file_exists("themes/$theme/$filename")) {
    return "themes/$theme/$filename";
  }
  $dir = $smarty->template_dir;
  if (is_array($dir)) {
    $dir = $dir[0];
  }
  if (file_exists("$dir/themes/$theme/$filename")) {
    return "$dir/themes/$theme/$filename";
  }
  if (file_exists("themes/default/$filename")) {
    return "themes/default/$filename";
  }
  if (file_exists("$dir/themes/default/$filename")) {
    return "$dir/themes/default/$filename";
  }
  return $filename;
}
?>
