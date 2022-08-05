<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2020  FusionDirectory

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
require_once('variables.inc');
require_once('class_URL.inc');

/*!
 * \brief Escape string for HTML output
 */
function htmlescape (string $str): string
{
  return htmlspecialchars($str, ENT_QUOTES | ENT_HTML5, 'UTF-8');
}

/*!
 * \brief Unescape string for HTML output, reverse of htmlescape
 */
function htmlunescape (string $html): string
{
  return html_entity_decode($html, ENT_QUOTES | ENT_HTML5, 'UTF-8');
}

function html_trace ($errstr = "")
{
  static $hideArgs = [
    'LDAP/init'           => [3],
    'userinfo/loginUser'  => [1],
    'change_password'     => [1],
    'cred_decrypt'        => [0,1],
    'LDAP/__construct'    => [1],
  ];
  if (!function_exists('debug_backtrace')) {
    return ['', ''];
  }
  $trace = array_slice(debug_backtrace(), 1);

  $loc = '';
  if (isset($trace[0]['file'])) {
    $loc = ' - '._('File').': '.$trace[0]['file'];
    if (isset($trace[0]['line'])) {
      $loc .= ' ('._('Line').' '.$trace[0]['line'].')';
    }
  }

  $return_html    = '<table width="100%" style="background-color:#402005;color:white;border:2px solid red;border-spacing:0;border-collapse:collapse;">'.
                    '<tr><td colspan="3">'.
                    '<h1 style="color:white">'.htmlescape(_('PHP error').' "'.$errstr.'"'.$loc).'</h1>'.
                    '</td></tr>';
  $return_mailto  = rawurlencode('=== Trace ===');
  /* Generate trace history */
  for ($index = 1, $c = count($trace); $index < $c; $index++) {
    $ct   = $trace[$index];
    $loc  = '';
    $func = '';
    if (isset($ct['class'])) {
      $loc .= _('class').' '.$ct['class'];
      $func .= $ct['class'];
      if (isset($ct['function'])) {
        $loc  .= ' / ';
        $func .= '/';
      }
    }
    if (isset($ct['function'])) {
      $loc .= _('function').' '.$ct['function'];
      $func .= $ct['function'];
    }
    if (isset($ct['type'])) {
      switch ($ct['type']) {
        case '::':
          $type = _('static');
          break;

        case '->':
          $type = _('method');
          break;

        default:
          $type = 'unknown';
          break;
      }
    } else {
      $type = '-';
    }
    $args = '';
    if (isset($ct['args'])) {
      if (isset($hideArgs[$func])) {
        $hideArgsIndexes = $hideArgs[$func];
      } else {
        $hideArgsIndexes = [];
      }
      $f = function ($index, $arg) use (&$f, $hideArgsIndexes)
      {
        static $i = 0;
        if (($i == 0) && in_array($index, $hideArgsIndexes)) {
          return '***';
        }
        if ($i > 4) {
          return '…';
        }
        if (is_object($arg)) {
          return 'CLASS:&nbsp;'.get_class($arg);
        } elseif (is_array($arg)) { /* Avoid converting array to string errors */
          $i++;
          $ret = 'array('.implode(',', array_map($f, array_keys($arg), $arg)).')';
          $i--;
          return $ret;
        } else {
          if (strlen("$arg") > 512) {
            $arg = substr("$arg", 0, 512)."…";
          }
          return '"'.htmlescape("$arg").'"';
        }
      };
      $args = implode(',', array_map($f, array_keys($ct['args']), $ct['args']));
    }
    if (empty($args)) {
      $args = '-';
    }
    if (isset($ct['file'])) {
      $file = $ct['file'];
    } else {
      $file = '';
    }
    if (isset($ct['line'])) {
      $line = $ct['line'];
    } else {
      $line = '';
    }
    $color = ($index & 1) ? '#404040' : '606060';
    $return_html .= "<tr style='background-color:$color'><td style='padding-left:20px' width=\"30%\">".htmlescape(_("Trace")."[$index]: $loc").'</td>';
    $return_html .= "<td>".htmlescape(_("File").": $file ("._('Line')." $line)").'</td><td width="10%">'.htmlescape(_("Type").": $type").'</td></tr>';
    $return_html .= "<tr style='background-color:$color'><td colspan=3 style='padding-left:20px;'>".htmlescape(_("Arguments").": $args").'</td></tr>';

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

  return [$return_html, $return_mailto];
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
function gosaRaiseError ($errno, $errstr, $errfile, $errline)
{
  global $error_collector,$config, $error_collector_mailto;

  // To avoid recursion - restore original error handler.
  restore_error_handler();

  /* Return if error reporting is set to zero */
  if (error_reporting() == 0) {
    set_error_handler('gosaRaiseError', E_WARNING | E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT);
    return FALSE;
  }

  /* Workaround for buggy imap_open error outputs */
  if (preg_match('/imap_open/', $errstr)) {
    set_error_handler('gosaRaiseError', E_WARNING | E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT);
    return;
  }

  /* Hide ldap size limit messages */
  if (preg_match('/ldap_error/', $errstr) && preg_match('/sizelimit/', $errstr)) {
    set_error_handler('gosaRaiseError', E_WARNING | E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT);
    return;
  }

  /* Send all errors to logging class, except "Ldap : No such object" messages*/
  if (class_exists('logging') && !preg_match('/No such object/', $errstr)) {
    logging::log('error', 'php', $errfile, [], 'Type:'.$errno.', Message:'.$errstr.', File:'.$errfile.', Line: '.$errline);
  }

  /* Error messages are hidden in FusionDirectory, so we only send them to the logging class and abort here */
  if (isset($config->data) && $config->get_cfg_value('displayerrors') != 'TRUE') {
    set_error_handler('gosaRaiseError', E_WARNING | E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT);
    return;
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
                              "Please describe what you did to produce this error as detailed as possible.".
                              "\n\n".
                              "*** PHP error information ***\n\n");

    $error_collector = '
      <div class="error">
        <table width="100%">
          <tr>
            <td>
              <img src="geticon.php?context=status&amp;icon=dialog-warning&amp;size=16" alt="" class="center"/>&nbsp;
              <strong style="font-size:14px">'.
                htmlescape(_('Generating this page caused the PHP interpreter to raise some errors!')).'
              </strong>
            </td>
            <td align=right>
              <a href="mailto:bugs@fusiondirectory.org?subject=FusionDirectory%20bugreport&amp;body=%BUGBODY%">
                <img src="geticon.php?context=applications&amp;icon=internet-mail&amp;size=16" title="'.htmlescape(_('Send bug report to the FusionDirectory Team')).
                '" class="center" alt="'.htmlescape(_('Mail icon')).'">&nbsp;'.htmlescape(_('Send bugreport')).'
              </a>
            </td>
            <td align="right">
              <button onClick="$(\'errorbox\').toggle();">'.
                htmlescape(_('Toggle information')).'
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
  if (preg_match('/^fatal/i', $errstr) || (PHP_ERROR_FATAL == 'TRUE')) {
    session::destroy('Fatal error');
    if (PHP_ERROR_FATAL == 'TRUE') {
      $error_collector = str_replace('display: none;', '', $error_collector);
    }
    echo $error_collector."</div>";
    flush();
    exit;
  }

  set_error_handler('gosaRaiseError', E_WARNING | E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT);
}

/*!
 * \brief Catches throwables that no one catched
 *
 * \param Throwable $throwable
 */
function fusiondirectoryExceptionHandler (Throwable $throwable)
{
  try {
    logging::log('error', 'fatal', '', [], 'Uncaught '.get_class($throwable).': '.$throwable->getMessage());
  } catch (Throwable $t) {
    /* Ignore exceptions/errors here */
  }

  try {
    if ($throwable instanceof FatalError) {
      $throwable->display();
    } else {
      $error = new FatalError(htmlescape(sprintf(_('Uncaught %s: %s'), get_class($throwable), $throwable->getMessage())), 0, $throwable);
      $error->display();
    }
  } catch (Throwable $t) {
    /* Minimal display if exceptions happens when building the pretty one */
    echo 'Uncaught '.get_class($throwable).': '.$throwable->getMessage();
  }

  exit(255);
}

/*!
 * \brief Dummy error handler
 */
function dummy_error_handler ()
{
}

/* Bail out for incompatible/old PHP versions */
if (!version_compare(phpversion(), PHP_MIN_VERSION, ">=")) {
  echo "PHP version needs to be ".PHP_MIN_VERSION." or above to run FusionDirectory. Aborted.";
  exit();
}

/* Set timezone */
date_default_timezone_set("GMT");

/* Get base dir for reference */
$BASE_DIR = dirname(dirname(__FILE__));
$ROOT_DIR = $BASE_DIR."/html";
error_reporting(E_ALL | E_STRICT);

/* Register error handler */
$error_collector        = "";
$error_collector_mailto = "";

set_error_handler('gosaRaiseError', E_WARNING | E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT);
set_exception_handler('fusiondirectoryExceptionHandler');

$variables_order = 'ES';
ini_set('track_vars',       '1');
ini_set('display_errors',   '1');
ini_set('report_memleaks',  '1');
ini_set('include_path',     ".:$BASE_DIR/include:".PHP_DIR.':'.PEAR_DIR);

/* Do smarty setup */
require(SMARTY);

$smarty = new Smarty;

$smarty->setTemplateDir($BASE_DIR.'/ihtml/');
$smarty->caching      = Smarty::CACHING_OFF;
$smarty->assign('css_files', []);
$smarty->assign('js_files', []);

if (defined("SMARTY_PLUGINS_EXTRA")) {
  $smarty->addPluginsDir((array)SMARTY_PLUGINS_EXTRA);
}

$smarty->registerPlugin('modifier', 'base64_encode', 'base64_encode');

$smarty->php_handling = Smarty::PHP_REMOVE;

/* Check for SSL connection */
$ssl = '';
if (!URL::sslOn()) {
  $ssl = URL::getSslUrl();
}
