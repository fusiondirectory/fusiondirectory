<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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

function html_trace($errstr = "")
{
  if (!function_exists('debug_backtrace')) {
    return array ("", "");
  }
  $trace = array_slice(debug_backtrace(), 1);

  $return_html    = "<table width=\"100%\" style='background-color:#402005;color:white;border:2px solid red;border-spacing:0;border-collapse:collapse;'><tr><td colspan=3><h1 style='color:white'>"._("PHP error")." \"$errstr\"</h1></td></tr>";
  $return_mailto  = rawurlencode("=== Trace ===");
  /* Generate trace history */
  for ($index = 0, $c = count($trace); $index < $c; $index++) {
    $ct   = $trace[$index];
    $loc  = "";
    if (isset($ct['class'])) {
      $loc .= _("class")." ".$ct['class'];
      if (isset($ct['function'])) {
        $loc .= " / ";
      }
    }
    if (isset($ct['function'])) {
      $loc .= _("function")." ".$ct['function'];
    }
    if (isset($ct['type'])) {
      switch ($ct['type']) {
        case "::":
          $type = _("static");
          break;

        case "->":
          $type = _("method");
          break;
      }
    } else {
      $type = "-";
    }
    $args = "";
    if (isset($ct['args'])) {
      $f = function ($arg) use(&$f) {
        static $i = 0;
        if ($i > 4) {
          return "…";
        }
        if (is_object($arg)) {
          return "CLASS:&nbsp;".get_class($arg);
        } elseif (is_array($arg)) { /* Avoid convertig array to string errors */
          $i++;
          $ret = "array(".implode(',', array_map($f, $arg)).")";
          $i--;
          return $ret;
        } else {
          if (strlen("$arg") > 512) {
            $arg = substr("$arg", 0, 512)."…";
          }
          return '"'.htmlentities("$arg", ENT_COMPAT, 'UTF-8').'"';
        }
      };
      $args = implode(',', array_map($f, $ct['args']));
    }
    if (empty($args)) {
      $args = "-";
    }
    if (isset($ct['file'])) {
      $file = $ct['file'];
    } else {
      $file = "";
    }
    if (isset($ct['line'])) {
      $line = $ct['line'];
    } else {
      $line = "";
    }
    $color = ($index & 1)?'#404040':'606060';
    $return_html .= "<tr style='background-color:$color'><td style='padding-left:20px' width=\"30%\">"._("Trace")."[$index]: $loc</td>";
    $return_html .= "<td>"._("File").": $file ("._('Line')." $line)</td><td width=\"10%\">"._("Type").": $type</td></tr>";
    $return_html .= "<tr style='background-color:$color'><td colspan=3 style='padding-left:20px;'>"._("Arguments").": $args</td></tr>";

    /* Add trace part to mailto body */
    $return_mailto .= rawurlencode(
                                 "\nTrace[".$index."]:".$loc.
                                 "\nFile : ".$file.
                                 "\nLine : ".$line.
                                 "\nType : ".$type.
                                 "\n  ".$args.
                                 "\n");
  }

  $return_html    .= "</table>";
  $return_mailto  .= rawurlencode("=== /Trace ===");

  return array($return_html, $return_mailto);
}

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
  if (error_reporting() == 0) {
    set_error_handler('gosaRaiseError', E_WARNING | E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT);
    return;
  }

  /* Workaround for buggy imap_open error outputs */
  if (preg_match('/imap_open/', $errstr)) {
    set_error_handler('gosaRaiseError', E_WARNING | E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT);
    return;
  }

  /* Hide ldap size limit messages */
  if (preg_match('/ldap_error/', $errstr)) {
    if (preg_match('/sizelimit/', $errstr)) {
      set_error_handler('gosaRaiseError', E_WARNING | E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT);
      return;
    }
  }

  /* Error messages are hidden in FusionDirectory, so we only send them to the logging class and abort here */
  if (isset($config->data) && $config->get_cfg_value("displayerrors") != "TRUE") {

    /* Write to syslog */
    if (class_exists("log") && !preg_match("/No such object/", $errstr)) {
      new log("view", "error", "", array(), "PHP error: $errstr ($errfile, line $errline)");
    }

    set_error_handler('gosaRaiseError', E_WARNING | E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT);
    return;
  }

  /* Send all errors to logging class, except "Ldap : No such object" messages*/
  if (class_exists("log") && !preg_match("/No such object/", $errstr)) {
    new log("debug", "all", $errfile, array(), "Type:".$errno.", Message:".$errstr.", File:".$errfile.", Line: ".$errline);
  }

  /* Create header as needed */
  if ($error_collector == "") {

    /* Mailto body header */
    $version = FD_VERSION;

    $error_collector_mailto .= rawurlencode(
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

    $error_collector = '
      <div class="error">
        <table width="100%">
          <tr>
            <td>
              <img src="geticon.php?context=status&amp;icon=dialog-warning&amp;size=16" alt="" class="center"/>&nbsp;
              <strong style="font-size:14px">'.
                _("Generating this page caused the PHP interpreter to raise some errors!").'
              </strong>
            </td>
            <td align=right>
              <a href="mailto:bugs@fusiondirectory.org?subject=FusionDirectory%20bugreport&amp;body=%BUGBODY%">
                <img src="geticon.php?context=applications&amp;icon=internet-mail&amp;size=16" title="'._("Send bug report to the FusionDirectory Team").
                '" class="center" alt="'.("Mail icon").'">&nbsp;'._("Send bugreport").'
              </a>
            </td>
            <td align="right">
              <button onClick="$(\'errorbox\').toggle();">'.
                _("Toggle information").'
              </button>
            </td>
          </tr>
        </table>
      </div>
      <div id="errorbox" style="position:absolute; z-index:150; display: none;">';
  }

  /* Create error header */
  $error_collector_mailto .= rawurlencode("=== Error === \n");
  $error_collector_mailto .= rawurlencode("PHP error: $errstr ($errfile, line $errline)\n");
  $error_collector_mailto .= rawurlencode("=== /Error === \n\n");

  list($html_trace, $mailto_trace) = html_trace($errstr);
  $error_collector        .= $html_trace;
  $error_collector_mailto .= $mailto_trace;

  /* Flush in case of fatal errors */
  set_error_handler('gosaRaiseError', E_WARNING | E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT);
  if (preg_match('/^fatal/i', $errstr)) {
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
if (!version_compare(phpversion(), "5.2.0", ">=")) {
  echo "PHP version needs to be 5.2.0 or above to run FusionDirectory. Aborted.";
  exit();
}

/* Set timezone */
date_default_timezone_set("GMT");

/* Get base dir for reference */
$BASE_DIR = dirname(dirname(__FILE__));
$ROOT_DIR = $BASE_DIR."/html";
error_reporting (E_ALL | E_STRICT);

/* Register error handler */
$error_collector        = "";
$error_collector_mailto = "";

set_error_handler('gosaRaiseError', E_WARNING | E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT);

$variables_order = "ES";
ini_set("track_vars", 1);
ini_set("display_errors", 1);
ini_set("report_memleaks", 1);
ini_set("include_path", ".:$BASE_DIR/include:".PHP_DIR.":".PEAR_DIR);

/* Do smarty setup */
require(SMARTY);

$smarty = new Smarty;

$smarty->template_dir = $BASE_DIR.'/ihtml/';
$smarty->caching      = FALSE;
$smarty->assign('css_files', array());
$smarty->assign('js_files', array());

$smarty->php_handling = Smarty::PHP_REMOVE;

/* Check for SSL connection */
$ssl = "";
if (!(isset($_SERVER['HTTPS']) && stristr($_SERVER['HTTPS'], "on"))) {
  if (empty($_SERVER['REQUEST_URI'])) {
    $ssl = "https://".$_SERVER['HTTP_HOST'].$_SERVER['PATH_INFO'];
  } else {
    $ssl = "https://".$_SERVER['HTTP_HOST'].$_SERVER['REQUEST_URI'];
  }
}
?>
