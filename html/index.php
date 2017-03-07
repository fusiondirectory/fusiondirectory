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

/* Display the login page and exit() */
function displayLogin()
{
  global $smarty,$message,$config,$ssl,$error_collector,$error_collector_mailto;
  $lang = session::global_get('lang');

  error_reporting(E_ALL | E_STRICT);
  /* Fill template with required values */
  $username = '';
  if (isset($_POST['username'])) {
    $username = trim($_POST['username']);
  }
  $smarty->assign ('date', gmdate("D, d M Y H:i:s"));
  $smarty->assign ('username', $username);
  $smarty->assign ('personal_img', "geticon.php?context=types&icon=user&size=48");
  $smarty->assign ('password_img', "geticon.php?context=status&icon=dialog-password&size=48");
  $smarty->assign ('directory_img', "geticon.php?context=places&icon=network-server&size=48");
  $smarty->append ('css_files',  get_template_path('login.css'));

  /* Some error to display? */
  if (!isset($message)) {
    $message = "";
  }
  $smarty->assign ("message", $message);

  /* Displasy SSL mode warning? */
  if (($ssl != '') && ($config->get_cfg_value('warnSSL') == 'TRUE')) {
    $smarty->assign ('ssl', sprintf(_('Warning: <a href="%s">Session is not encrypted!</a>'), $ssl));
  } else {
    $smarty->assign ('ssl', '');
  }

  if (!$config->check_session_lifetime()) {
    $smarty->assign ('lifetime', _('Warning: The session lifetime configured in your fusiondirectory.conf will be overridden by php.ini settings.'));
  } else {
    $smarty->assign ('lifetime', '');
  }

  /* Generate server list */
  $servers = array();
  if (isset($_POST['server'])) {
    $selected = $_POST['server'];
  } else {
    $selected = $config->data['MAIN']['DEFAULT'];
  }
  foreach ($config->data['LOCATIONS'] as $key => $ignored) {
    $servers[$key] = $key;
  }
  $smarty->assign ("server_options", $servers);
  $smarty->assign ("server_id", $selected);

  /* show login screen */
  $smarty->assign ("PHPSESSID", session_id());
  if (session::is_set('errors')) {
    $smarty->assign("errors", session::get('errors'));
  }
  if ($error_collector != "") {
    $smarty->assign("php_errors", preg_replace("/%BUGBODY%/", $error_collector_mailto, $error_collector)."</div>");
  } else {
    $smarty->assign("php_errors", "");
  }
  $smarty->assign("msg_dialogs", msg_dialog::get_dialogs());
  $smarty->assign("usePrototype", "false");
  $smarty->assign("date", date("l, dS F Y H:i:s O"));
  $smarty->assign("lang", preg_replace('/_.*$/', '', $lang));
  $smarty->assign("rtl", language_is_rtl($lang));

  $smarty->display (get_template_path('headers.tpl'));
  $smarty->assign("version", FD_VERSION);

  $smarty->display(get_template_path('login.tpl'));
  exit();
}

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
      /* Move CAS autoload before FD autoload */
      spl_autoload_unregister('CAS_autoload');
      spl_autoload_register('CAS_autoload', TRUE, TRUE);
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

initLanguage();

$smarty->assign ('focusfield', 'username');

if (isset($_POST['server'])) {
  $server = $_POST['server'];
} else {
  $server = $config->data['MAIN']['DEFAULT'];
}

$config->set_current($server);
if (($config->get_cfg_value('casActivated') == 'TRUE') || ($config->get_cfg_value('httpAuthActivated') == 'TRUE')) {
  session::global_set('DEBUGLEVEL', 0);
}

/* If SSL is forced, just forward to the SSL enabled site */
if (($config->get_cfg_value('forcessl') == 'TRUE') && ($ssl != '')) {
  header ("Location: $ssl");
  exit;
}

if (isset($_REQUEST['message'])) {
  switch($_REQUEST['message']) {
    case 'expired':
      $message = _('Your FusionDirectory session has expired!');
      break;
    case 'newip':
      $message = _('Your IP has changed!');
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

/* Class with a function for each login step
 * Each function can return a string to display an LDAP error, or FALSE to redirect to login
 * In this case it can set global $message and assign focusfield in smarty before hand */
class Index {
  static protected $username;
  static protected $password;

  static function init()
  {
    static::$username = NULL;
    static::$password = NULL;
  }

  /* Runs schemaCheck if activated in configuration */
  static function runSchemaCheck()
  {
    global $config;
    if ($config->get_cfg_value('schemaCheck') != 'TRUE') {
      return TRUE;
    }
    $cfg = array();
    $cfg['admin']       = $config->current['ADMINDN'];
    $cfg['password']    = $config->current['ADMINPASSWORD'];
    $cfg['connection']  = $config->current['SERVER'];
    $cfg['tls']         = ($config->get_cfg_value('ldapTLS') == 'TRUE');
    $str = check_schema($cfg);
    foreach ($str as $tr) {
      if (!$tr['STATUS']) {
        if ($tr['IS_MUST_HAVE']) {
          return _('LDAP schema check reported errors:').'<br/><br/><i>'.$tr['MSG'].'</i>';
        } else {
          msg_dialog::display(_('LDAP schema error'), $tr['MSG'], WARNING_DIALOG);
        }
      }
    }
    return TRUE;
  }

  /* Check locking LDAP branch is here or create it */
  static function checkForLockingBranch()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cat(get_ou('lockRDN').get_ou('fusiondirectoryRDN').$config->current['BASE'], array('dn'));
    $attrs = $ldap->fetch();
    if (!count($attrs)) {
      $ldap->cd($config->current['BASE']);
      $ldap->create_missing_trees(get_ou('lockRDN').get_ou('fusiondirectoryRDN').$config->current['BASE']);
    }
  }

  /* Check username for invalid characters and check password is not empty
   * Also trims username */
  static function validateUserInput()
  {
    global $message, $smarty;
    static::$username = trim(static::$username);
    if (!preg_match('/^[@A-Za-z0-9_.-]+$/', static::$username)) {
      $message = _('Please specify a valid username!');
      return FALSE;
    } elseif (mb_strlen(static::$password, 'UTF-8') == 0) {
      $message = _('Please specify your password!');
      $smarty->assign ('focusfield', 'password');
      return FALSE;
    }
    return TRUE;
  }

  /* Performs an LDAP bind with $username and $password */
  static function ldapLoginUser()
  {
    global $ui, $config, $message, $smarty;
    /* Login as user, initialize user ACL's */
    $ui = ldap_login_user(static::$username, static::$password);
    if ($ui === NULL) {
      if (isset($_SERVER['REMOTE_ADDR'])) {
        logging::log('security', 'login', '', array(), 'Authentication failed for user "'.static::$username.'" [from '.$_SERVER['REMOTE_ADDR'].']');
      } else {
        logging::log('security', 'login', '', array(), 'Authentication failed for user "'.static::$username.'"');
      }
      $message = _('Please check the username/password combination.');
      $smarty->assign ('focusfield', 'password');
      return FALSE;
    }
    return TRUE;
  }

  /* Called after successful login, return FALSE if account is expired */
  static function loginAndCheckExpired()
  {
    global $ui, $config, $plist, $message, $smarty;
    /* Remove all locks of this user */
    del_user_locks($ui->dn);

    /* Save userinfo and plugin structure */
    session::global_set('ui', $ui);

    /* User might have its own language, re-run initLanguage */
    initLanguage();

    /* We need a fully loaded plist and config to test account expiration */
    session::global_un_set('plist');
    $plist = load_plist();

    /* Check that newly installed plugins have their configuration in the LDAP (will reload plist if needed) */
    $config->checkLdapConfig();

    /* Check account expiration */
    if ($config->get_cfg_value('handleExpiredAccounts') == 'TRUE') {
      $expired = $ui->expired_status();

      if ($expired == POSIX_ACCOUNT_EXPIRED) {
        logging::log('security', 'login', '', array(), 'Account for user "'.static::$username.'" has expired');
        $message = _('Account locked. Please contact your system administrator!');
        $smarty->assign ('focusfield', 'username');
        return FALSE;
      }
    }
    return TRUE;
  }

  /* Final step of successful login: redirect to main.php */
  static function redirect()
  {
    global $config;
    /* Not account expired or password forced change go to main page */
    logging::log('security', 'login', '', array(), 'User "'.static::$username.'" logged in successfully.');
    session::global_set('connected', 1);
    session::global_set('DEBUGLEVEL', $config->get_cfg_value('DEBUGLEVEL'));
    header ('Location: main.php?global_check=1');
    exit;
  }

  /* Return HTTP authentication header */
  static function authenticateHeader($message = 'Authentication required')
  {
    header('WWW-Authenticate: Basic realm="FusionDirectory"');
    header('HTTP/1.0 401 Unauthorized');
    echo "$message\n";
    exit;
  }

  /* Run each step in $steps, stop on errors */
  static function runSteps($steps)
  {
    foreach($steps as $step) {
      $status = static::$step();
      if (is_string($status)) {
        msg_dialog::display(_('LDAP error'), $status, LDAP_ERROR);
        return FALSE;
      } elseif ($status === FALSE) {
        return FALSE;
      }
    }
    return TRUE;
  }

  /* All login steps in the right order for standard POST login */
  static function fullLoginProcess()
  {
    global $config, $message;

    static::init();

    /* Reset error messages */
    $message = '';

    static::$username = $_POST['username'];
    static::$password = $_POST['password'];

    $success = static::runSteps(array(
      'validateUserInput',
      'ldapLoginUser',
      'loginAndCheckExpired',
      'runSchemaCheck',
      'checkForLockingBranch',
    ));

    if ($success) {
      /* Everything went well, redirect to main.php */
      static::redirect();
    }
  }

  /* All login steps in the right order for HTTP auth login */
  static function authLoginProcess()
  {
    global $config, $message;

    static::init();

    if (!isset($_SERVER['PHP_AUTH_USER'])) {
      static::authenticateHeader();
    }

    static::$username = $_SERVER['PHP_AUTH_USER'];
    static::$password = $_SERVER['PHP_AUTH_PW'];

    $success = static::runSteps(array(
      'validateUserInput',
      'ldapLoginUser',
      'loginAndCheckExpired',
      'runSchemaCheck',
      'checkForLockingBranch',
    ));

    if ($success) {
      /* Everything went well, redirect to main.php */
      static::redirect();
    } else {
      static::authenticateHeader($message);
    }
  }

  /* All login steps in the right order for HTTP Header login */
  static function headerAuthLoginProcess()
  {
    global $config, $message, $ui;

    static::init();

    /* Reset error messages */
    $message = '';

    $header = $config->get_cfg_value('httpHeaderAuthHeaderName', 'AUTH_USER');

    static::$username = $_SERVER['HTTP_'.$header];

    if (!static::$username) {
      msg_dialog::display(
        _('Error'),
        sprintf(
          _('No value found in HTTP header "%s"'),
          $header
        ),
        FATAL_ERROR_DIALOG
      );
      exit();
    }

    $ui = ldap_get_user(static::$username);

    if ($ui === FALSE) {
      msg_dialog::display(
        _('Error'),
        sprintf(
          _('Header user "%s" could not be found in the LDAP'),
          static::$username
        ),
        FATAL_ERROR_DIALOG
      );
      exit();
    } elseif (is_string($ui)) {
      msg_dialog::display(
        _('Error'),
        sprintf(
          _('Login with user "%s" triggered error: %s'),
          static::$username,
          $ui
        ),
        FATAL_ERROR_DIALOG
      );
      exit();
    }

    $ui->loadACL();

    $success = static::runSteps(array(
      'loginAndCheckExpired',
      'runSchemaCheck',
      'checkForLockingBranch',
    ));

    if ($success) {
      /* Everything went well, redirect to main.php */
      static::redirect();
    }
  }

  /* All login steps in the right order for CAS login */
  static function casLoginProcess()
  {
    global $config, $message, $ui;

    static::init();

    /* Reset error messages */
    $message = '';

    //~ phpCAS::setDebug();

    // Initialize phpCAS
    phpCAS::client(
      CAS_VERSION_2_0,
      $config->get_cfg_value('casHost', 'localhost'),
      (int)($config->get_cfg_value('casPort', 443)),
      $config->get_cfg_value('casContext', '')
    );

    // Set the CA certificate that is the issuer of the cert
    phpCAS::setCasServerCACert($config->get_cfg_value('casServerCaCertPath'));
    //~ phpCAS::setNoCasServerValidation();

    // force CAS authentication
    phpCAS::forceAuthentication();
    static::$username = phpCAS::getUser();

    $ui = ldap_get_user(static::$username);

    if ($ui === FALSE) {
      msg_dialog::display(
        _('Error'),
        sprintf(
          _('CAS user "%s" could not be found in the LDAP'),
          static::$username
        ),
        FATAL_ERROR_DIALOG
      );
      exit();
    } elseif (is_string($ui)) {
      msg_dialog::display(
        _('Error'),
        sprintf(
          _('Login with user "%s" triggered error: %s'),
          static::$username,
          $ui
        ),
        FATAL_ERROR_DIALOG
      );
      exit();
    }

    $ui->loadACL();

    $success = static::runSteps(array(
      'loginAndCheckExpired',
      'runSchemaCheck',
      'checkForLockingBranch',
    ));

    if ($success) {
      /* Everything went well, redirect to main.php */
      static::redirect();
    }
  }
}

if ($config->get_cfg_value('httpAuthActivated') == 'TRUE') {
  Index::authLoginProcess();
} elseif ($config->get_cfg_value('casActivated') == 'TRUE') {
  require_once('CAS.php');
  /* Move CAS autoload before FD autoload */
  spl_autoload_unregister('CAS_autoload');
  spl_autoload_register('CAS_autoload', TRUE, TRUE);
  Index::casLoginProcess();
} elseif ($config->get_cfg_value('httpHeaderAuthActivated') == 'TRUE') {
  Index::headerAuthLoginProcess();
} elseif ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_POST['login'])) {
  /* Got a formular answer, validate and try to log in */
  Index::fullLoginProcess();
}

/* Translation of cookie-warning. Whether to display it, is determined by JavaScript */
$smarty->assign ('cookies', '<b>'._('Warning').':</b> '._('Your browser has cookies disabled. Please enable cookies and reload this page before logging in!'));

/* Set focus to the error button if we've an error message */
$focus = '';
if (session::is_set('errors') && session::get('errors') != '') {
  $focus = '<script type="text/javascript">';
  $focus .= 'document.forms[0].error_accept.focus();';
  $focus .= '</script>';
}
$smarty->assign('focus', $focus);

displayLogin();
?>

</body>
</html>
