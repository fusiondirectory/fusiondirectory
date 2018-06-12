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
require_once ("../include/php_setup.inc");
require_once ("functions.inc");
require_once ("variables.inc");
require_once ("class_logging.inc");
header("Content-type: text/html; charset=UTF-8");

/*****************************************************************************
 *                               M   A   I   N                               *
 *****************************************************************************/

/* Set error handler to own one, initialize time calculation
   and start session. */
session::start();

if (isset($_REQUEST['signout']) && $_REQUEST['signout']) {
  if (session::global_is_set('connected')) {
    $config = session::global_get('config');
    if ($config->get_cfg_value('casActivated') == 'TRUE') {
      require_once('CAS.php');
      /* Move FD autoload after CAS autoload */
      spl_autoload_unregister('__fusiondirectory_autoload');
      spl_autoload_register('__fusiondirectory_autoload');
      phpCAS::client(
        CAS_VERSION_2_0,
        $config->get_cfg_value('casHost', 'localhost'),
        (int)($config->get_cfg_value('casPort', 443)),
        $config->get_cfg_value('casContext', '')
      );
      // Set the CA certificate that is the issuer of the cert
      phpCAS::setCasServerCACert($config->get_cfg_value('casServerCaCertPath'));
      phpCas::logout();
    }
  }
  session::destroy();
  session::start();
}

/* Reset errors */
session::set('errors', '');
session::set('errorsAlreadyPosted', '');
session::set('LastError', '');

/* Check if we need to run setup */
if (!file_exists(CONFIG_DIR.'/'.CONFIG_FILE)) {
  header('location:setup.php');
  exit();
}

/* Check if fusiondirectory.conf (.CONFIG_FILE) is accessible */
if (!is_readable(CONFIG_DIR.'/'.CONFIG_FILE)) {
  msg_dialog::display(
    _('Configuration error'),
    sprintf(
      _('FusionDirectory configuration %s/%s is not readable. Please run fusiondirectory-setup --check-config to fix this.'),
      CONFIG_DIR,
      CONFIG_FILE
    ),
    FATAL_ERROR_DIALOG
  );
  exit();
}

/* Parse configuration file */
$config = new config(CONFIG_DIR.'/'.CONFIG_FILE, $BASE_DIR);
session::global_set('config', $config);
session::global_set('DEBUGLEVEL', $config->get_cfg_value('DEBUGLEVEL'));
@DEBUG (DEBUG_CONFIG, __LINE__, __FUNCTION__, __FILE__, $config->data, 'config');

/* Set template compile directory */
$smarty->compile_dir = $config->get_cfg_value('templateCompileDirectory', SPOOL_DIR);

/* Check for compile directory */
if (!(is_dir($smarty->compile_dir) && is_writable($smarty->compile_dir))) {
  msg_dialog::display(
    _('Smarty error'),
    sprintf(
      _('Directory "%s" specified as compile directory is not accessible!'),
      $smarty->compile_dir
    ),
    FATAL_ERROR_DIALOG
  );
  exit();
}

/* Check for old files in compile directory */
clean_smarty_compile_dir($smarty->compile_dir);

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
  in_array($config->get_cfg_value('LoginMethod'), array('LoginCas', 'LoginHTTPAuth', 'LoginHTTPHeader'))) {
  session::global_set('DEBUGLEVEL', 0);
}

/* If SSL is forced, just forward to the SSL enabled site */
if (($config->get_cfg_value('forcessl') == 'TRUE') && ($ssl != '')) {
  header ("Location: $ssl");
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
?>
