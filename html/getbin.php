<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* Basic setup, remove eventually registered sessions */
@require_once ("../include/php_setup.inc");
@require_once ("functions.inc");
error_reporting (0);
session_cache_limiter("private");
session::start();
session::global_set('errorsAlreadyPosted',array());

/* Logged in? Simple security check */
if (!session::global_is_set('ui')){
  new log("security","unknown","",array(),"Error: getbin.php called without session") ;
  header ("Location: index.php");
  exit;
}

header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");
header("Last-Modified: ".gmdate("D, d M Y H:i:s")." GMT");
header("Cache-Control: no-cache");
header("Pragma: no-cache");
header("Cache-Control: post-check=0, pre-check=0");
$bintype = session::is_set('binarytype')
  ? session::get('binarytype') : "octet-stream";
header("Content-type: " . $bintype);
if (session::is_set('binaryfile')) {
  header( "Content-disposition: attachment; filename="
    . session::get('binaryfile'));
}

echo session::get('binary');
error_reporting (E_ALL | E_STRICT);

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
