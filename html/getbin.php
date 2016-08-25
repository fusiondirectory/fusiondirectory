<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

/* Basic setup, remove eventually registered sessions */
@require_once('../include/php_setup.inc');
@require_once('functions.inc');
@require_once('variables.inc');

error_reporting (0);
session_cache_limiter('private');
session::start();
session::global_set('errorsAlreadyPosted', array());

/* Logged in? Simple security check */
if (!session::global_is_set('ui')) {
  logging::log('security', 'unknown', '', array(), 'Error: getbin.php called without session');
  header ('Location: index.php');
  exit;
}

header('Expires: Mon, 26 Jul 1997 05:00:00 GMT');
header('Last-Modified: '.gmdate('D, d M Y H:i:s').' GMT');
header('Cache-Control: no-cache');
header('Pragma: no-cache');
header('Cache-Control: post-check=0, pre-check=0');

$key = 'binary';
if (isset ($_GET['key'])) {
  $key .= $_GET['key'];
}

$bintype = (session::is_set($key.'type') ? session::get($key.'type') : 'octet-stream');
header('Content-type: '.$bintype);
if (session::is_set($key.'file')) {
  header('Content-disposition: attachment; filename="'.session::get($key.'file').'"');
}

echo session::get($key);
error_reporting (E_ALL | E_STRICT);
?>
