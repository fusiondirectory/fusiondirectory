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
  new log("security","unknown","",array(),"Error: getFAIstatus.php called without session") ;
  header ("Location: index.php");
  exit;
}

/* There must be a mac address given */
if(!isset($_GET['mac'])){
  return;
}

$config = session::global_get("config");
$o =  new supportDaemon();
$res = $o->get_entries_by_mac(explode(",", $_GET['mac']));
foreach($res as $entry){
  echo $entry['MACADDRESS']."|".$entry['PROGRESS']."\n";
}
?>
