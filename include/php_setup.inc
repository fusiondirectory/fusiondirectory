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

/*!
 * \file php_setup.inc
 * Source code for php_setup
 */
require_once ("variables.inc");

/*!
 * \brief Raise an error
 *
 * \param string $errno
 *
 * \param string $errstr The error string
 *
 * \param string $errfile The error filename
 *
 * \param string $errline The error line
 */
function gosaRaiseError($errno, $errstr, $errfile, $errline)
{
  global $error_collector,$config, $error_collector_mailto;

  // To avoid recursion - restore original error handler.
  restore_error_handler();

  /* Return if error reporting is set to zero */
  if (error_reporting() == 0){
    set_error_handler('gosaRaiseError', E_WARNING |  E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT) ;
    return;
  }

  /* Workaround for buggy imap_open error outputs */
  if (preg_match('/imap_open/', $errstr)){
    set_error_handler('gosaRaiseError', E_WARNING |  E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT) ;
    return;
  }

  /* Hide ldap size limit messages */
  if (preg_match('/ldap_error/', $errstr)){
    if (preg_match('/sizelimit/', $errstr)){
      set_error_handler('gosaRaiseError', E_WARNING |  E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT) ;
      return;
    }
  }

  /* Error messages are hidden in FusionDirectory, so we only send them to the logging class and abort here */
  if (isset($config->data) && $config->get_cfg_value("displayerrors") != "TRUE") {

    /* Write to syslog */
    if (class_exists("log") && !preg_match("/No such object/",$errstr)) {
      new log("view","error","",array(),"PHP error: $errstr ($errfile, line $errline)");
    }

    set_error_handler('gosaRaiseError', E_WARNING |  E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT) ;
    return;
  }

  /* Send all errors to logging class, except "Ldap : No such object" messages*/
  if (class_exists("log") && !preg_match("/No such object/", $errstr)) {
    new log("debug","all",$errfile,array(),"Type:".$errno.", Message:".$errstr.", File:".$errfile.", Line: ".$errline);
  }

  /* Create header as needed */
  if ($error_collector == ""){

    /* Mailto body header */
    $version = FD_VERSION;

    $error_collector_mailto .=rawurlencode(
                              "Oups. Seems like you've catched some kind of bug inside FusionDirectory/PHP. You may want to help ".
                              "us to improve the software stability. If so, please provide some more information below.".
                              "\n\n".
                              "*** FusionDirectory bug report ***".
                              "\nFusionDirectory Version: $version".
                              "\n\n".
                              "Please describe what you did to produce this error as detailed as possible. Can you ".
                              "reproduce this bug using the demo on http://demo.FusionDirectory.org ?".
                              "\n\n".
                              "*** PHP error information ***\n\n");

    if (class_exists('session') && session::is_set('js') && session::get('js')==FALSE){
      $error_collector= "<div>";
    } else {

      $warning_path = (is_callable('get_template_path')) ? get_template_path('images/warning.png') : 'images/warning.png' ;

      $error_collector= "
        <table summary=\"\" width=\"100%\" style='background-color:#E0E0E0;border-bottom:1px solid black;z-index:150;'>
          <tr>
            <td>
              <img alt=\"\" align=\"middle\" src='{$warning_path}'>&nbsp;
              <font style='font-size:14px;font-weight:bold'>".
                _("Generating this page caused the PHP interpreter to raise some errors!")."
              </font>
            </td>
            <td align=right>
              <a href=\"mailto:bugs@fusiondirectory.org?subject=FusionDirectory%20bugreport&amp;body=%BUGBODY%\">
                <img border='0' src='images/mailto.png' title='"._("Send bug report to the FusionDirectory Team")."' class='center' alt=''>&nbsp;"._("Send bugreport")."
              </a>
            </td>
            <td align=right>
              <button onClick=\"$('errorbox').toggle();\">".
                _("Toggle information")."
              </button>
            </td>
          </tr>
        </table>
        <div id='errorbox' style='position:absolute; z-index:150; display: none;'>";
    }
  }

  /* Create error header */
  $error_collector_mailto .= rawurlencode("=== Error === \n");
  $error_collector_mailto .= rawurlencode("PHP error: $errstr ($errfile, line $errline)\n");
  $error_collector_mailto .= rawurlencode("=== /Error === \n\n");

  list($html_trace, $mailto_trace) = html_trace();
  $error_collector        .= $html_trace;
  $error_collector_mailto .= $mailto_trace;

  /* Flush in case of fatal errors */
  set_error_handler('gosaRaiseError', E_WARNING |  E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT) ;
  if (preg_match('/^fatal/i', $errstr)){
    echo $error_collector."</div>";
    flush();
    exit;
  }
}

/*!
 * \brief Dummy error handler
 */
function dummy_error_handler()
{
}

/* Bail out for incompatible/old PHP versions */
if (!version_compare(phpversion(),"5.2.0",">=")){
  echo "PHP version needs to be 5.2.0 or above to run FusionDirectory. Aborted.";
  exit();
}

/* Set timezone */
date_default_timezone_set("GMT");

/* Get base dir for reference */
$BASE_DIR= dirname(dirname(__FILE__));
$ROOT_DIR= $BASE_DIR."/html";
error_reporting (E_ALL | E_STRICT);

/* Register error handler */
$error_collector= "";
$error_collector_mailto= "";

set_error_handler('gosaRaiseError', E_WARNING |  E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT) ;

$variables_order= "ES";
ini_set("track_vars",1);
ini_set("display_errors",1);
ini_set("report_memleaks",1);
ini_set("include_path",".:$BASE_DIR/include:".PHP_DIR.":".PEAR_DIR);

/* Do smarty setup */

if (file_exists(SMARTY)) {
 require(SMARTY);
} else {
 require(SMARTY3);
}

/*require(SMARTY_PATH);*/
$smarty = new Smarty;
$smarty->template_dir = $BASE_DIR.'/ihtml/';
$smarty->caching= false;

// To be able to switch between smarty version 2/3
if(defined('SMARTY_PHP_REMOVE')){
    $smarty->php_handling= SMARTY_PHP_REMOVE;
}else{
    $smarty->php_handling= Smarty::PHP_REMOVE;
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
