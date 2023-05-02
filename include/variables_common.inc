<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2017  FusionDirectory

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
 * \file variables_common.inc
 * Define common locations and variables
 */

/*
 * \brief Allow setting the config file in the apache configuration
 * e.g. SetEnv CONFIG_DIR /etc/fusiondirectory
*/
if (isset($_SERVER['CONFIG_DIR'])) {
  define("CONFIG_DIR", $_SERVER['CONFIG_DIR']);
}

/*
 * \brief Allow setting the config file in the apache configuration
 *  e.g.  SetEnv CONFIG_FILE fusiondirectory.conf 1.0
 */
if (isset($_SERVER['CONFIG_FILE'])) {
  define("CONFIG_FILE", $_SERVER['CONFIG_FILE']);
}

/*!
 * \brief php library path
 */
define("PHP_DIR", "/usr/share/php"); /*! Define php directory */

/*!
 * \brief php pear path
 */
define("PEAR_DIR", "/usr/share/php"); /*! Define PEAR directory */

/*!
 * \brief fpdf library path
 */
define("FPDF", "/usr/share/php/fpdf/fpdf.php"); /*! Define fpdf library path */

/*!
 * \brief Global FPDF FONT define
 */
define("FPDF_FONTPATH", "/usr/share/php/fpdf/font/"); /*! Define fpdf font path  */

/*!
 * \brief FusionDirectory Version
 */
define("FD_VERSION", "1.4"); /*! Define FusionDirectory version */

/*!
 * \brief FusionDirectory config object RDN
 */
define("CONFIGRDN", "cn=config,ou=fusiondirectory,"); /*! Define FusionDirectory config object */

/*!
 * \brief Minimum PHP version
 */
define('PHP_MIN_VERSION', '7.3.0');

/*!
 * \brief Toggle crashing on PHP error, used for test suites
 */
define('PHP_ERROR_FATAL', 'FALSE');

/* Define constants for debugging */
define('DEBUG_TRACE',    1); /*! Debug level for tracing of common actions (save, check, etc.) */
define('DEBUG_LDAP',     2); /*! Debug level for LDAP queries */
define('DEBUG_DB',       4); /*! Debug level for database operations */
define('DEBUG_SHELL',    8); /*! Debug level for shell commands */
define('DEBUG_POST',     16); /*! Debug level for POST content */
define('DEBUG_SESSION',  32); /*! Debug level for SESSION content */
define('DEBUG_CONFIG',   64); /*! Debug level for CONFIG information */
define('DEBUG_ACL',      128); /*! Debug level for ACL infos */
define('DEBUG_SI',       256); /*! Debug level for communication with Argonaut */
define('DEBUG_MAIL',     512); /*! Debug level for all about mail (mailAccounts, imap, sieve etc.) */
define('DEBUG_FAI',      1024); /* FAI (incomplete) */

/* Define constants for LDAP operations */
define('LDAP_READ',   1);
define('LDAP_ADD',    2);
define('LDAP_MOD',    3);
define('LDAP_DEL',    4);
define('LDAP_SEARCH', 5);
define('LDAP_AUTH',   6);
