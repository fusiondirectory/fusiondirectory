<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2018  FusionDirectory

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

/* Basic setup, remove eventually registered sessions */
require_once("../include/php_setup.inc");
require_once("functions.inc");
require_once("variables.inc");

/* Set headers */
header('Content-type: text/html; charset=UTF-8');
header('X-XSS-Protection: 1; mode=block');
header('X-Content-Type-Options: nosniff');
header('X-Frame-Options: deny');

/* Set the text domain as 'fusiondirectory' */
$domain = 'fusiondirectory';
bindtextdomain($domain, LOCALE_DIR);
textdomain($domain);

/* Remember everything we did after the last click */
session::start();
reset_errors();

if ($_SERVER['REQUEST_METHOD'] == 'POST') {
  @DEBUG(DEBUG_POST, __LINE__, __FUNCTION__, __FILE__, $_POST, '_POST');
}
@DEBUG(DEBUG_SESSION, __LINE__, __FUNCTION__, __FILE__, $_SESSION, '_SESSION');

/* Logged in? Simple security check */
if (!session::is_set('connected')) {
  logging::log('security', 'login', '', [], 'main.php called without session - logging out');
  header('Location: index.php?message=nosession');
  exit;
}

CSRFProtection::check();

$ui     = session::get('ui');
$config = session::get('config');

/* If SSL is forced, just forward to the SSL enabled site */
if (($config->get_cfg_value('forcessl') == 'TRUE') && ($ssl != '')) {
  header("Location: $ssl");
  exit;
}

timezone::setDefaultTimezoneFromConfig();

/* Check for invalid sessions */
if (session::get('_LAST_PAGE_REQUEST') != '') {
  /* check FusionDirectory.conf for defined session lifetime */
  $max_life = $config->get_cfg_value('sessionLifetime', 60 * 60 * 2);

  if ($max_life > 0) {
    /* get time difference between last page reload */
    $request_time = (time() - session::get('_LAST_PAGE_REQUEST'));

    /* If page wasn't reloaded for more than max_life seconds
     * kill session
     */
    if ($request_time > $max_life) {
      session::destroy();
      logging::log('security', 'login', '', [], 'main.php called with expired session - logging out');
      header('Location: index.php?signout=1&message=expired');
      exit;
    }
  }
}
session::set('_LAST_PAGE_REQUEST', time());


@DEBUG(DEBUG_CONFIG, __LINE__, __FUNCTION__, __FILE__, $config->data, "config");

/* Set template compile directory */
$smarty->compile_dir = $config->get_cfg_value("templateCompileDirectory", SPOOL_DIR);

Language::init();

/* Prepare plugin list */
pluglist::load();

/* Check previous plugin index */
if (session::is_set('plugin_index')) {
  $old_plugin_index = session::get('plugin_index');
} else {
  $old_plugin_index = '';
}

$plist->gen_menu();

$smarty->assign('hideMenus', FALSE);
/* check user expiration status */
$expired = $ui->expired_status();
if (($expired == POSIX_WARN_ABOUT_EXPIRATION) && !session::is_set('POSIX_WARN_ABOUT_EXPIRATION__DONE')) {
  @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $expired, 'This user account ('.$ui->uid.') is about to expire');

  // The users password is about to expire soon, display a warning message.
  logging::log('security', 'fusiondirectory', '', [], 'password for user "'.$ui->uid.'" is about to expire');
  msg_dialog::display(_('Password change'), _('Your password is about to expire, please change your password!'), INFO_DIALOG);
  session::set('POSIX_WARN_ABOUT_EXPIRATION__DONE', TRUE);
} elseif ($expired == POSIX_FORCE_PASSWORD_CHANGE) {
  @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $expired, 'This user account expired');

  // The password is expired, we are now going to enforce a new one from the user.

  // Hide the FusionDirectory menus to avoid leaving the enforced password change dialog.
  $smarty->assign('hideMenus', TRUE);
  $plug = (isset($_GET['plug'])) ? $_GET['plug'] : NULL;

  // Search for the 'user' class and set its id as active plug.
  foreach ($plist->dirlist as $key => $value) {
    if ($value == 'user') {
      if (!isset($_GET['plug']) || ($_GET['plug'] != $key)) {
        $_GET['plug'] = $key;
        msg_dialog::display(_('Warning'), _('Your password has expired, please set a new one.'), WARNING_DIALOG);
      }
      break;
    }
  }
}

if (isset($_GET['plug']) && $plist->plugin_access_allowed($_GET['plug'])) {
  $plugin_index = validate($_GET['plug']);
} else {
  /* set to welcome page as default plugin */
  $plugin_index = 'welcome';
}
session::set('plugin_index', $plugin_index);

/* Handle plugin locks.
    - Remove the plugin from session if we switched to another. (cleanup)
    - Remove all created locks if "reset" was posted.
    - Remove all created locks if we switched to another plugin.
*/
$cleanup      = FALSE;
$remove_lock  = FALSE;

/* Check if we have changed the selected plugin */
if (!empty($old_plugin_index) && ($old_plugin_index != $plugin_index)) {
  pluglist::runMainInc($old_plugin_index, TRUE);
} elseif ((isset($_GET['reset']) && $_GET['reset'] == 1) || isset($_POST['delete_lock'])) {
  /* Reset was posted, remove all created locks for the current plugin */
  $remove_lock = TRUE;
}

/* Check for sizelimits */
$ui->getSizeLimitHandler()->update();

/* Check for memory */
if (memory_get_usage() > (to_byte(ini_get('memory_limit')) - 2048000)) {
  msg_dialog::display(_("Configuration error"), _("Running out of memory!"), WARNING_DIALOG);
}

/* Load department list when plugin has changed. That is some kind of
   compromise between speed and beeing up to date */
if (isset($_GET['reset'])) {
  set_object_info();
}

/* show web frontend */
$smarty->assign("date", date("l, dS F Y H:i:s O"));
$lang = session::get('lang');
$smarty->assign('lang',  preg_replace('/_.*$/', '', $lang));
$smarty->assign('rtl',   Language::isRTL($lang));
if (isset($plugin_index)) {
  $plug = "?plug=$plugin_index";
} else {
  $plug = "";
}

if ($ui->ignore_acl_for_current_user()) {
  $smarty->assign('username', '<div style="color:#FF0000;">'._('User ACL checks disabled').'</div>&nbsp;'.$ui->uid);
} else {
  $smarty->assign('username', $ui->uid);
}
$smarty->assign("menu", $plist->menu);
$smarty->assign("plug", "$plug");

$smarty->assign("usePrototype", "false");

/* React on clicks */
if (($_SERVER['REQUEST_METHOD'] == 'POST')
  && (isset($_POST['delete_lock']) || isset($_POST['open_readonly']))) {

  /* Set old Post data */
  if (session::is_set('LOCK_VARS_USED_GET')) {
    foreach (session::get('LOCK_VARS_USED_GET') as $name => $value) {
      $_GET[$name]  = $value;
    }
  }
  if (session::is_set('LOCK_VARS_USED_POST')) {
    foreach (session::get('LOCK_VARS_USED_POST') as $name => $value) {
      $_POST[$name] = $value;
    }
  }
  if (session::is_set('LOCK_VARS_USED_REQUEST')) {
    foreach (session::get('LOCK_VARS_USED_REQUEST') as $name => $value) {
      $_REQUEST[$name] = $value;
    }
  }
}

/* Load plugin */
pluglist::runMainInc($plugin_index);

/* Print_out last ErrorMessage repeated string. */
$smarty->assign("msg_dialogs", msg_dialog::get_dialogs());
$smarty->assign("contents", $display);
$smarty->assign("sessionLifetime", $config->get_cfg_value("sessionLifetime", 60 * 60 * 2));

/* If there's some post, take a look if everything is there... */
if (isset($_POST) && count($_POST) && !isset($_POST['php_c_check'])) {
  msg_dialog::display(
    _('Configuration Error'),
    sprintf(_('Fatal error: not all POST variables have been transfered by PHP - please inform your administrator!')),
    FATAL_ERROR_DIALOG
  );
  exit();
}

/* Assign errors to smarty */
if ($error_collector != "") {
  $smarty->assign("php_errors", preg_replace("/%BUGBODY%/", $error_collector_mailto, $error_collector)."</div>");
} else {
  $smarty->assign("php_errors", "");
}

$focus = '<script type="text/javascript">';
$focus .= 'next_msg_dialog();';
$focus .= '</script>';
$smarty->assign('focus',      $focus);
$smarty->assign('CSRFtoken',  CSRFProtection::getToken());

if (class_available('Game')) {
  $smarty->assign('game_screen', Game::run());
} else {
  $smarty->assign('game_screen', '');
}

$display  = $smarty->fetch(get_template_path('headers.tpl')).
            $smarty->fetch(get_template_path('framework.tpl'));

/* Show page... */
echo $display;

/* Save plist and config */
session::set('plist', $plist);
session::set('config', $config);
reset_errors();
