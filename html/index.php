<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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
require_once ("class_log.inc");
header("Content-type: text/html; charset=UTF-8");


/**
 * Display the login page and exit().
 *
 */
function displayLogin()
{
  global $smarty,$message,$config,$ssl,$error_collector,$error_collector_mailto;
  error_reporting(E_ALL | E_STRICT);
  /* Fill template with required values */
  $username = "";
  if (isset($_POST["username"])) {
    $username = trim(get_post("username"));
  }
  $smarty->assign ('date', gmdate("D, d M Y H:i:s"));
  $smarty->assign ('username', $username);
  $smarty->assign ('personal_img', get_template_path('images/login-head.png'));
  $smarty->assign ('password_img', get_template_path('images/password.png'));
  $smarty->assign ('directory_img', get_template_path('images/ldapserver.png'));
  $smarty->append ('css_files',  get_template_path('login.css'));

  /* Some error to display? */
  if (!isset($message)) {
    $message = "";
  }
  $smarty->assign ("message", $message);

  /* Displasy SSL mode warning? */
  if (($ssl != "") && ($config->get_cfg_value('warnSSL') == 'TRUE')) {
    $smarty->assign ("ssl", _("Warning").": <a style=\"color:red;\" href=\"$ssl\">"._("Session is not encrypted!")."</a>");
  } else {
    $smarty->assign ("ssl", "");
  }

  if (!$config->check_session_lifetime()) {
    $smarty->assign ("lifetime", _("Warning").": ".
        _("The session lifetime configured in your fusiondirectory.conf will be overridden by php.ini settings."));
  } else {
    $smarty->assign ("lifetime", "");
  }

  /* Generate server list */
  $servers = array();
  if (isset($_POST['server'])) {
    $selected = get_post('server');
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
session::set('errorsAlreadyPosted', array());

/* Destroy old session if exists.
   Else you will get your old session back, if you not logged out correctly. */
if (is_array(session::get_all()) && count(session::get_all())) {
  session::destroy();
  session::start();
}

$username = "";

/* Reset errors */
session::set('errors', "");
session::set('errorsAlreadyPosted', "");
session::set('LastError', "");

/* Check if we need to run setup */
if (!file_exists(CONFIG_DIR."/".CONFIG_FILE)) {
  header("location:setup.php");
  exit();
}

/* Reset errors */
session::set('errors', "");

/* Check if fusiondirectory.conf (.CONFIG_FILE) is accessible */
if (!is_readable(CONFIG_DIR."/".CONFIG_FILE)) {
  msg_dialog::display(_("Configuration error"), sprintf(_("FusionDirectory configuration %s/%s is not readable. Please run fusiondirectory-setup --check-config to fix this."), CONFIG_DIR, CONFIG_FILE), FATAL_ERROR_DIALOG);
  exit();
}

/* Parse configuration file */
$config = new config(CONFIG_DIR."/".CONFIG_FILE, $BASE_DIR);
session::global_set('DEBUGLEVEL', $config->get_cfg_value('DEBUGLEVEL'));
if ($_SERVER["REQUEST_METHOD"] != "POST") {
  @DEBUG (DEBUG_CONFIG, __LINE__, __FUNCTION__, __FILE__, $config->data, "config");
}

/* Set template compile directory */
$smarty->compile_dir = $config->get_cfg_value("templateCompileDirectory", SPOOL_DIR);

/* Check for compile directory */
if (!(is_dir($smarty->compile_dir) && is_writable($smarty->compile_dir))) {
  msg_dialog::display(_("Smarty error"), sprintf(_("Directory '%s' specified as compile directory is not accessible!"),
        $smarty->compile_dir), FATAL_ERROR_DIALOG);
  exit();
}

/* Check for old files in compile directory */
clean_smarty_compile_dir($smarty->compile_dir);


/* Language setup */
$lang = get_browser_language();
putenv("LANGUAGE=");
putenv("LANG=$lang");
setlocale(LC_ALL, $lang);
$GLOBALS['t_language']            = $lang;
$GLOBALS['t_gettext_message_dir'] = $BASE_DIR.'/locale/';

/* Set the text domain as 'fusiondirectory' */
$domain = 'fusiondirectory';
bindtextdomain($domain, LOCALE_DIR);
textdomain($domain);
$smarty->assign ('nextfield', 'username');

if ($_SERVER["REQUEST_METHOD"] != "POST") {
  @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $lang, "Setting language to");
}

/* Do we have htaccess authentification enabled? */
$htaccess_authenticated = FALSE;
if ($config->get_cfg_value("htaccessAuthentication") == "TRUE" ) {
  if (!isset($_SERVER['REMOTE_USER'])) {
    msg_dialog::display(_("Configuration error"), _("There is a problem with the authentication setup!"), FATAL_ERROR_DIALOG);
    exit;
  }

  $tmp      = process_htaccess($_SERVER['REMOTE_USER'], isset($_SERVER['KRB5CCNAME']));
  $username = $tmp['username'];
  $server   = $tmp['server'];
  if ($username == "") {
    msg_dialog::display(_("Error"), _("Cannot find a valid user for the current authentication setup!"), FATAL_ERROR_DIALOG);
    exit;
  }
  if ($server == "") {
    msg_dialog::display(_("Error"), _("User information is not unique across the configured LDAP trees!"), FATAL_ERROR_DIALOG);
    exit;
  }

  $htaccess_authenticated = TRUE;
}
if (!$htaccess_authenticated) {
  if (isset($_POST['server'])) {
    $server = get_post("server");
  } else {
    $server = $config->data['MAIN']['DEFAULT'];
  }
}
$config->set_current($server);

/* If SSL is forced, just forward to the SSL enabled site */
if (($config->get_cfg_value("forcessl") == "TRUE") && ($ssl != '')) {
  header ("Location: $ssl");
  exit;
}


/* Got a formular answer, validate and try to log in */
if (($_SERVER["REQUEST_METHOD"] == "POST" && isset($_POST['login'])) || $htaccess_authenticated) {

  /* Reset error messages */
  $message = "";


  /* Destroy old sessions, they cause a successfull login to relog again ...*/
  if (session::global_is_set('_LAST_PAGE_REQUEST')) {
    session::global_set('_LAST_PAGE_REQUEST', time());
  }

  /* Admin-logon and verify */
  $ldap = $config->get_ldap_link();
  if (is_null($ldap) || (is_int($ldap) && $ldap == 0)) {
    msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    displayLogin();
    exit();
  }


  /* Check for schema file presence */
  if ($config->get_cfg_value("schemaCheck") == "TRUE") {
    $recursive  = ($config->get_cfg_value("ldapFollowReferrals") == "TRUE");
    $tls        = ($config->get_cfg_value("ldapTLS") == "TRUE");

    if (!count($ldap->get_objectclasses())) {
      msg_dialog::display(_("LDAP error"), _("Cannot detect information about the installed LDAP schema!"), ERROR_DIALOG);
      displayLogin();
      exit();
    } else {
      $cfg = array();
      $cfg['admin']       = $config->current['ADMINDN'];
      $cfg['password']    = $config->current['ADMINPASSWORD'];
      $cfg['connection']  = $config->current['SERVER'];
      $cfg['tls']         = $tls;
      $str = check_schema($cfg, $config->get_cfg_value("rfc2307bis") == "TRUE");
      $checkarr = array();
      foreach ($str as $tr) {
        if (isset($tr['IS_MUST_HAVE']) && !$tr['STATUS']) {
          msg_dialog::display(_("LDAP error"), _("Your LDAP setup contains old schema definitions:")."<br><br><i>".$tr['MSG']."</i>", ERROR_DIALOG);
          displayLogin();
          exit();
        }
      }
    }
  }


  /* Check for locking area */
  $ldap->cat($config->get_cfg_value("config"), array("dn"));
  $attrs = $ldap->fetch();
  if (!count ($attrs)) {
    $ldap->cd($config->current['BASE']);
    $ldap->create_missing_trees($config->get_cfg_value("config"));
  }


  /* Check for valid input */
  $ok = TRUE;
  if (!$htaccess_authenticated) {
    $username = trim(get_post("username"));
    if (!preg_match("/^[@A-Za-z0-9_.-]+$/", $username)) {
      $message = _("Please specify a valid username!");
      $ok = FALSE;
    } elseif (mb_strlen($_POST["password"], 'UTF-8') == 0) {
      $message = _("Please specify your password!");
      $smarty->assign ('nextfield', 'password');
      $ok = FALSE;
    }
  }

  if ($ok) {

    /* Login as user, initialize user ACL's */
    if ($htaccess_authenticated) {
      $ui = ldap_login_user_htaccess($username);
      if ($ui === NULL || !$ui) {
        msg_dialog::display(_("Authentication error"), _("Cannot retrieve user information for htaccess authentication!"), FATAL_ERROR_DIALOG);
        exit;
      }
    } else {
      $ui = ldap_login_user($username, $_POST["password"]);
    }
    if ($ui === NULL || !$ui) {
      $message = _("Please check the username/password combination.");
      $smarty->assign ('nextfield', 'password');
      session::global_set('config', $config);

      if (isset($_SERVER['REMOTE_ADDR'])) {
        $ip = $_SERVER['REMOTE_ADDR'];
        new log("security", "login", "", array(), "Authentication failed for user \"$username\" [from $ip]");
      } else {
        new log("security", "login", "", array(), "Authentication failed for user \"$username\"");
      }
    } else {
      /* Remove all locks of this user */
      del_user_locks($ui->dn);

      /* Save userinfo and plugin structure */
      session::global_set('ui', $ui);
      session::global_set('session_cnt', 0);

      /* Let FusionDirectory trigger a new connection for each POST, save config to session. */
      session::global_set('config', $config);

      /* Restore filter settings from cookie, if available */
      if ($config->get_cfg_value("storeFilterSettings") == "TRUE") {

        if (isset($_COOKIE['FusionDirectory_Filter_Settings']) || isset($HTTP_COOKIE_VARS['FusionDirectory_Filter_Settings'])) {

          if (isset($_COOKIE['FusionDirectory_Filter_Settings'])) {
            $cookie_all = unserialize(base64_decode($_COOKIE['FusionDirectory_Filter_Settings']));
          } else {
            $cookie_all = unserialize(base64_decode($HTTP_COOKIE_VARS['FusionDirectory_Filter_Settings']));
          }
          if (isset($cookie_all[$ui->dn])) {
            $cookie = $cookie_all[$ui->dn];
            $cookie_vars = array("MultiDialogFilters","CurrentMainBase","plug");
            foreach ($cookie_vars as $var) {
              if (isset($cookie[$var])) {
                session::global_set($var, $cookie[$var]);
              }
            }
            if (isset($cookie['plug'])) {
              $plug = $cookie['plug'];
            }
          }
        }
      }

      /* are we using accountexpiration */
      if ($config->get_cfg_value("handleExpiredAccounts") == "TRUE") {
        $expired = $ui->expired_status();

        if ($expired == POSIX_ACCOUNT_EXPIRED) {
          $message = _("Account locked. Please contact your system administrator!");
          $smarty->assign ('nextfield', 'password');
          new log("security", "login", "", array(), "Account for user \"$username\" has expired");
          displayLogin();
          exit();
        }
      }

      /* Not account expired or password forced change go to main page */
      new log("security", "login", "", array(), "User \"$username\" logged in successfully");
      $plist = new pluglist($config, $ui);
      if (isset($plug) && isset($plist->dirlist[$plug])) {
        header ("Location: main.php?plug=".$plug."&global_check=1");
      } else {
        header ("Location: main.php?global_check=1");
      }
      exit;
    }
  }
}

/* Translation of cookie-warning. Whether to display it, is determined by JavaScript */
$smarty->assign ("cookies", "<b>"._("Warning").":<\/b> "._("Your browser has cookies disabled. Please enable cookies and reload this page before logging in!"));

/* Set focus to the error button if we've an error message */
$focus = "";
if (session::is_set('errors') && session::get('errors') != "") {
  $focus = '<script type="text/javascript">';
  $focus .= 'document.forms[0].error_accept.focus();';
  $focus .= '</script>';
}
$smarty->assign("focus", $focus);
displayLogin();

?>

</body>
</html>
