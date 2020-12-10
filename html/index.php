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

/* Load required includes */
require_once("../include/php_setup.inc");
require_once("functions.inc");
require_once("variables.inc");
require_once("class_logging.inc");

/* Set headers */
header('Content-type: text/html; charset=UTF-8');
header('X-XSS-Protection: 1; mode=block');
header('X-Content-Type-Options: nosniff');
header('X-Frame-Options: deny');

/**
 * @var Smarty $smarty    Defined in php_setup.inc
 * @var string $BASE_DIR  Defined in php_setup.inc
 * @var string $ssl       Defined in php_setup.inc
 */

/*****************************************************************************
 *                               M   A   I   N                               *
 *****************************************************************************/

/* Set error handler to own one, initialize time calculation
   and start session. */
session::start();

if (isset($_REQUEST['signout']) && $_REQUEST['signout']) {
  $reason = '';
  if (session::is_set('connected')) {
    $config = session::get('config');
    if ($config->get_cfg_value('casActivated') == 'TRUE') {
      require_once('CAS.php');
      /* Move FD autoload after CAS autoload */
      spl_autoload_unregister('__fusiondirectory_autoload');
      spl_autoload_register('__fusiondirectory_autoload');
      phpCAS::client(
        CAS_VERSION_2_0,
        $config->get_cfg_value('casHost', 'localhost'),
        $config->get_cfg_value('casPort', '443'),
        $config->get_cfg_value('casContext', '')
      );
      // Set the CA certificate that is the issuer of the cert
      phpCAS::setCasServerCACert($config->get_cfg_value('casServerCaCertPath'));
      phpCAS::logout();
    }
    $reason = 'Sign out';
    if (isset($_REQUEST['message'])) {
      switch ($_REQUEST['message']) {
        case 'expired':
          $reason = 'Session expired';
          break;
        case 'invalidparameter':
          $reason = sprintf('Invalid plugin parameter "%s"!', $_REQUEST['plug']);
          break;
        case 'nosession':
          $reason = 'No session found';
          break;
        default:
      }
    }
  }
  session::destroy($reason);
  session::start();
}

/* Reset errors */
reset_errors();

/* Check if we need to run setup */
if (!file_exists(CONFIG_DIR.'/'.CONFIG_FILE)) {
  header('location:setup.php');
  exit();
}

/* Check if fusiondirectory.conf (.CONFIG_FILE) is accessible */
if (!is_readable(CONFIG_DIR.'/'.CONFIG_FILE)) {
  throw new FatalError(
    htmlescape(sprintf(
      _('FusionDirectory configuration %s/%s is not readable. Please run fusiondirectory-setup --check-config to fix this.'),
      CONFIG_DIR,
      CONFIG_FILE
    ))
  );
}

/* Parse configuration file */
$config = new config(CONFIG_DIR.'/'.CONFIG_FILE, $BASE_DIR);
session::set('config', $config);
session::set('DEBUGLEVEL', $config->get_cfg_value('DEBUGLEVEL'));
logging::debug(DEBUG_CONFIG, __LINE__, __FUNCTION__, __FILE__, $config->data, 'config');
/* Configuration was reloaded, so plist needs to be as well */
session::un_set('plist');
unset($plist);

/* Set template compile directory */
$smarty->setCompileDir($config->get_cfg_value('templateCompileDirectory', SPOOL_DIR));

/* Check for compile directory */
if (!(is_dir($smarty->getCompileDir()) && is_writable($smarty->getCompileDir()))) {
  throw new FatalError(
    htmlescape(sprintf(
      _('Directory "%s" specified as compile directory is not accessible!'),
      $smarty->getCompileDir()
    ))
  );
}

/* Check for old files in compile directory */
clean_smarty_compile_dir($smarty->getCompileDir());

Language::init();

if (isset($_POST['server'])) {
  $server = $_POST['server'];
} else {
  $server = $config->data['MAIN']['DEFAULT'];
}

$config->set_current($server);
if (
  ($config->get_cfg_value('casActivated') == 'TRUE') ||
  ($config->get_cfg_value('httpAuthActivated') == 'TRUE') ||
  ($config->get_cfg_value('httpHeaderAuthActivated') == 'TRUE') ||
  in_array($config->get_cfg_value('LoginMethod'), ['LoginCas', 'LoginHTTPAuth', 'LoginHTTPHeader'])) {
  session::set('DEBUGLEVEL', 0);
}

/* If SSL is forced, just forward to the SSL enabled site */
if (($config->get_cfg_value('forcessl') == 'TRUE') && ($ssl != '')) {
  header("Location: $ssl");
  exit;
}

if (isset($_REQUEST['message'])) {
  switch ($_REQUEST['message']) {
    case 'expired':
      $message = _('Your FusionDirectory session has expired!');
      break;
    case 'invalidparameter':
      $message = sprintf(_('Invalid plugin parameter "%s"!'), $_REQUEST['plug']);
      break;
    case 'nosession':
      $message = _('No session found!');
      break;
    default:
      $message = $_REQUEST['message'];
  }
}

LoginMethod::loginProcess();
