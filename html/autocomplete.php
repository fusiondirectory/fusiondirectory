<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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
@require_once ("../include/php_setup.inc");
@require_once ("functions.inc");
@require_once ("variables.inc");

session_cache_limiter("private");
session::start();
session::global_set('errorsAlreadyPosted',array());

/* Logged in? Simple security check */
if (!session::global_is_set('ui')){
  new log("security","unknown","",array(),"Error: autocomplete.php called without session") ;
  header ("Location: index.php");
  exit;
}

/* Base completition or filter completition? */
if (isset($_GET['type']) && $_GET['type'] == "base") {

  // Find dn based on name and description
  if (session::is_set("pathMapping") && count($_POST) == 1) {
    $res= "";
    $pathMapping= session::get("pathMapping");
    $search= preg_replace('/&quot;/', '"', current($_POST));

    $config = session::global_get('config');
    foreach ($config->department_info as $dn => $info) {
      if (!isset($pathMapping[$dn])) {
        continue;
      }
      if (mb_stristr($info['name'], $search) !== false) {
        $res.= "<li>".mark($search, $pathMapping[$dn]).($info['description']==''?"":"<span class='informal'> [".mark($search, $info['description'])."]</span>")."</li>";
        continue;
      }
      if (mb_stristr($info['description'], $search) !== false) {
        $res.= "<li>".mark($search, $pathMapping[$dn]).($info['description']==''?"":"<span class='informal'> [".mark($search, $info['description'])."]</span>")."</li>";
        continue;
      }
      if (mb_stristr($pathMapping[$dn], $search) !== false) {
        $res.= "<li>".mark($search, $pathMapping[$dn]).($info['description']==''?"":"<span class='informal'> [".mark($search, $info['description'])."]</span>")."</li>";
        continue;
      }
    }

    /* Return results */
    if (!empty($res)) {
      echo "<ul>$res</ul>";
    }
  }

} else {

  $ui = session::global_get('ui');

  /* Is there a filter object arround? */
  if (session::is_set("autocomplete")){
    $filter= session::get("autocomplete");
    $filter->processAutocomplete();
  }
}

?>
