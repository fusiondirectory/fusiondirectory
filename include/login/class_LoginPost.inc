<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018  FusionDirectory

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
 * \brief Login via POST
 */
class LoginPost extends LoginMethod
{
  /*! \brief Displayed name */
  static function getLabel()
  {
    return _('HTML form');
  }

  /*! \brief All login steps in the right order for standard POST login */
  static function loginProcess()
  {
    global $smarty, $config, $message;

    static::init();

    /* Reset error messages */
    $message = '';

    $smarty->assign ('focusfield', 'username');

    if (($_SERVER['REQUEST_METHOD'] == 'POST') && isset($_POST['login']) && isset($_POST['username']) && isset($_POST['password'])) {

      static::$username = $_POST['username'];
      static::$password = $_POST['password'];

      $success = static::runSteps(array(
        'validateUserInput',
        'ldapLoginUser',
        'checkForLockingBranch',
        'loginAndCheckExpired',
        'runSchemaCheck',
      ));

      if ($success) {
        /* Everything went well, redirect to main.php */
        static::redirect();
      }
    }

    /* Translation of cookie-warning. Whether to display it, is determined by JavaScript */
    $smarty->assign('cookies', '<b>'._('Warning').':</b> '._('Your browser has cookies disabled. Please enable cookies and reload this page before logging in!'));

    static::displayLogin();
  }

  /*! \brief Display the login page and exit() */
  static protected function displayLogin()
  {
    global $smarty,$message,$config,$ssl,$error_collector,$error_collector_mailto;
    $lang = session::global_get('lang');

    error_reporting(E_ALL | E_STRICT);
    /* Fill template with required values */
    $username = '';
    if (isset($_POST['username'])) {
      $username = trim($_POST['username']);
    }
    $smarty->assign('date',       gmdate('D, d M Y H:i:s'));
    $smarty->assign('username',   $username);
    $smarty->assign('revision',   FD_VERSION);
    $smarty->assign('year',       date('Y'));
    $smarty->append('css_files',  get_template_path('login.css'));

    /* Some error to display? */
    if (!isset($message)) {
      $message = '';
    }
    $smarty->assign('message', $message);

    /* Display SSL mode warning? */
    if (($ssl != '') && ($config->get_cfg_value('warnSSL') == 'TRUE')) {
      $smarty->assign('ssl', sprintf(_('Warning: <a href="%s">Session is not encrypted!</a>'), $ssl));
    } else {
      $smarty->assign('ssl', '');
    }

    if (!$config->check_session_lifetime()) {
      $smarty->assign('lifetime', _('Warning: The session lifetime configured in your fusiondirectory.conf will be overridden by php.ini settings.'));
    } else {
      $smarty->assign('lifetime', '');
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
    $smarty->assign('server_options', $servers);
    $smarty->assign('server_id',      $selected);

    /* show login screen */
    $smarty->assign('PHPSESSID', session_id());
    if ($error_collector != '') {
      $smarty->assign('php_errors', preg_replace('/%BUGBODY%/', $error_collector_mailto, $error_collector).'</div>');
    } else {
      $smarty->assign('php_errors', '');
    }
    $smarty->assign('msg_dialogs',  msg_dialog::get_dialogs());
    $smarty->assign('usePrototype', 'false');
    $smarty->assign('date',         date('l, dS F Y H:i:s O'));
    $smarty->assign('lang',         preg_replace('/_.*$/', '', $lang));
    $smarty->assign('rtl',          Language::isRTL($lang));

    $smarty->display(get_template_path('headers.tpl'));
    $smarty->assign('version', FD_VERSION);

    $smarty->display(get_template_path('login.tpl'));
    exit();
  }
}
