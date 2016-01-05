<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016  FusionDirectory

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

/* Basic setup, remove eventually registered sessions */
@require_once("../include/php_setup.inc");
@require_once("functions.inc");
@require_once("variables.inc");

session_cache_limiter("private");
session::start();
session::global_set('errorsAlreadyPosted', array());

$theme = '';
if (session::global_is_set('config')) {
  $config = session::global_get('config');
  $theme  = $config->get_cfg_value('theme');
} else {
  header("cache-control: no-cache");
}
IconTheme::$extensions = array('png');
$src    = IconTheme::findThemeIcon($theme, $_GET['context'], $_GET['icon'], $_GET['size']);

header("Content-Type: image/png");
if (isset($_GET['disabled']) && $_GET['disabled']) {
  $im = imagecreatefrompng($src);
  imageAlphaBlending($im, TRUE);
  imageSaveAlpha($im, TRUE);
  imagefilter($im, IMG_FILTER_GRAYSCALE);
  imagepng($im);
  imagedestroy($im);
} else {
  readfile($src);
}
?>
