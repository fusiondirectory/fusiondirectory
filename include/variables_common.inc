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

/*! \file
 * Define common locations and variables */

/* Allow setting the config file in the apache configuration
    e.g. SetEnv CONFIG_DIR /etc/fusiondirectory
*/
if(isset($_SERVER['CONFIG_DIR'])){
  define ("CONFIG_DIR",$_SERVER['CONFIG_DIR']);
}

/* Allow setting the config file in the apache configuration
    e.g.  SetEnv CONFIG_FILE fusiondirectory.conf 1.0
 */
if(isset($_SERVER['CONFIG_FILE'])){
  define ("CONFIG_FILE",$_SERVER['CONFIG_FILE']);
}

/* php library path */
define ("PHP_DIR", "/usr/share/php"); /*! Define php directory */

/* php library path */
define ("PEAR_DIR", "/usr/share/php"); /*! Define PEAR directory */

/* fpdf library path */
define("FPDF","/usr/share/php/fpdf/fpdf.php"); /*! Define fpdf library path */

/* Global FPDF FONT define */
define("FPDF_FONTPATH", "/usr/share/php/fpdf/font/"); /*! Define fpdf font path  */

/* path for smarty libraries */
define("SMARTY", "/usr/share/php/smarty/libs/Smarty.class.php"); /*! Define smarty2 library path */
define("SMARTY3", "/usr/share/php/smarty3/Smarty.class.php"); /*! Define smarty3 library path */

/* name of the utility used to create windows passwords */
define ("MKNTPWD", "/usr/sbin/fusiondirectory-mkntpassword"); /*! Define the name of the binary used to create smb password */

/* FusionDirectory Version */
define ("FD_VERSION", "1.0.3"); /*! Define FusionDirectory version */
?>
