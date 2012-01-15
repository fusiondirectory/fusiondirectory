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

/* Define common locations and variables */

/* Allow setting the config file in the apache configuration
    e.g. SetEnv CONFIG_DIR /etc/fusiondirectory
*/
if(!isset($_SERVER['CONFIG_DIR'])){
  define ("CONFIG_DIR", "/etc/fusiondirectory");
}else{
  define ("CONFIG_DIR",$_SERVER['CONFIG_DIR']);
}

/* Allow setting the config file in the apache configuration
    e.g.  SetEnv CONFIG_FILE fusiondirectory.conf 1.0
 */
if(!isset($_SERVER['CONFIG_FILE'])){
  define ("CONFIG_FILE", "fusiondirectory.conf");
}else{
  define ("CONFIG_FILE",$_SERVER['CONFIG_FILE']);
}

/* Smarty compile dir */
define ("SPOOL_DIR","/var/spool/fusiondirectory");

/* Global cache dir */
define ("CACHE_DIR","/var/cache/fusiondirectory");

/* Global locale cache dir */
define ("LOCALE_DIR", "/var/cache/fusiondirectory/locale");

/* Global tmp dir */
define ("TEMP_DIR","/var/cache/fusiondirectory/tmp");

/* Directory containing the configuration template */
define ("CONFIG_TEMPLATE_DIR", "/var/cache/fusiondirectory/template/");

/* Directory containing the fai logs */
define ("FAI_LOG_DIR", "/var/cache/fusiondirectory/fai/");

/* Directory containing the vacation files */
define ("VACATION_DIR", "/var/cache/fusiondirectory/vacation/");

/* php library path */
define ("PHP_DIR", "/usr/share/php");

/* excel library path */
define ("EXCEL", "/include/utils/excel");

/* fpdf library path */ 
define("FPDF","/usr/share/php/fpdf/fpdf.php");

/* Global FPDF FONT define */
define("FPDF_FONTPATH", "/usr/share/php/fpdf/font/");

/* path for smarty libraries */
define("SMARTY", "/usr/share/php/smarty/libs/Smarty.class.php");
define("SMARTY3", "/usr/share/php/smarty3/Smarty.class.php");

/* name of the class.cache file */
define("CLASS_CACHE", "class.cache");

/* name of the utility used to create windows passwords */
define ("MKNTPWD", "/usr/bin/fusiondirectory-mkntpassword");

?>
