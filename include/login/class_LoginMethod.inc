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

/*!
 * \brief Base class for login methods
 *
 * See index.php
 */
class LoginMethod
{
  static protected $username;
  static protected $password;

  static function init ()
  {
    static::$username = NULL;
    static::$password = NULL;
  }

  /*! \brief Runs schemaCheck if activated in configuration */
  static function runSchemaCheck (): bool
  {
    global $config;
    if ($config->get_cfg_value('schemaCheck') != 'TRUE') {
      return TRUE;
    }
    $cfg = [];
    $cfg['admin']       = $config->current['ADMINDN'];
    $cfg['password']    = $config->current['ADMINPASSWORD'];
    $cfg['connection']  = $config->current['SERVER'];
    $cfg['tls']         = ($config->get_cfg_value('ldapTLS') == 'TRUE');
    $str = check_schema($cfg);
    foreach ($str as $tr) {
      if (!$tr['STATUS']) {
        if ($tr['IS_MUST_HAVE']) {
          throw new FusionDirectoryError(htmlescape(_('LDAP schema check reported errors:')).'<br/><br/><i>'.htmlescape($tr['MSG']).'</i>');
        } else {
          $warning = new FusionDirectoryWarning(nl2br(htmlescape(sprintf(_("LDAP schema error:\n%s"), $tr['MSG']))));
          $warning->display();
        }
      }
    }
    return TRUE;
  }

  /*! \brief Check if locking LDAP branch is here or create it */
  static function checkForLockingBranch (): bool
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cat(get_ou('lockRDN').get_ou('fusiondirectoryRDN').$config->current['BASE'], ['dn']);
    $attrs = $ldap->fetch();
    if (!count($attrs)) {
      $ldap->cd($config->current['BASE']);
      try {
        $ldap->create_missing_trees(get_ou('lockRDN').get_ou('fusiondirectoryRDN').$config->current['BASE']);
      } catch (FusionDirectoryError $error) {
        $error->display();
      }
    }

    return TRUE;
  }

  /*! \brief Check username for invalid characters and check password is not empty
   * Also trims username */
  static function validateUserInput (): bool
  {
    global $message, $smarty;
    static::$username = trim(static::$username);
    if (!preg_match('/^[@A-Za-z0-9_.-]+$/', static::$username)) {
      $message = _('Please specify a valid username!');
      return FALSE;
    } elseif (mb_strlen(static::$password, 'UTF-8') == 0) {
      $message = _('Please specify your password!');
      $smarty->assign('focusfield', 'password');
      return FALSE;
    }
    return TRUE;
  }

  /*! \brief Performs an LDAP bind with $username and $password */
  static function ldapLoginUser (): bool
  {
    global $ui, $config, $message, $smarty;
    /* Login as user, initialize user ACL's */
    try {
      $ui = userinfo::loginUser(static::$username, static::$password);
    } catch (LoginFailureException $e) {
      /* Load plist to be able to log */
      pluglist::load();
      logging::log('security', 'login failure', static::$username, [], 'Authentication failed: '.$e->getMessage());
      /* Show the same message whether the user exists or not to avoid information leak */
      $message = $e->getMessage();
      $smarty->assign('focusfield', 'password');
      return FALSE;
    }
    return TRUE;
  }

  /*! \brief Called after successful login, return FALSE if account is expired */
  static function loginAndCheckExpired (): bool
  {
    global $ui, $config, $plist, $message, $smarty;

    /* Remove all locks of this user */
    Lock::deleteByUser($ui->dn);

    /* Save userinfo and plugin structure */
    session::set('ui', $ui);

    /* User might have its own language, re-run initLanguage */
    $plistReloaded = Language::init();

    /* We need a fully loaded plist and config to test account expiration */
    if (!$plistReloaded) {
      session::un_set('plist');
    }
    pluglist::load();

    /* Check that newly installed plugins have their configuration in the LDAP (will reload plist if needed) */
    $config->checkLdapConfig();

    /* Check account expiration */
    $expired = $ui->expired_status();

    if ($expired == POSIX_ACCOUNT_EXPIRED) {
      logging::log('security', 'account', $ui->dn, [], 'Account for user "'.static::$username.'" has expired');
      $message = _('Account locked. Please contact your system administrator!');
      $smarty->assign('focusfield', 'username');
      return FALSE;
    }

    return TRUE;
  }

  /*! \brief Connect user */
  static function connect ()
  {
    global $config, $ui;

    $ui = session::get('ui');

    //Create new session ID to avoir session_fixation security issues after sucess login
    session_regenerate_id();

    /* Not account expired or password forced change go to main page */
    logging::log('security', 'login', $ui->uid, [], 'Logged in successfully');
    session::set('connected', 1);
    session::set('DEBUGLEVEL', $config->get_cfg_value('DEBUGLEVEL'));
  }

  /*! \brief Final step of successful login: redirect to main.php */
  static function redirect ()
  {
    static::connect();
    header('Location: main.php');
    exit;
  }

  /*! \brief Run each step in $steps, stop on errors */
  static function runSteps ($steps)
  {
    try {
      foreach ($steps as $step) {
        $status = static::$step();
        if (is_string($status)) {
          /* Deprecated */
          msg_dialog::display(_('LDAP error'), $status, LDAP_ERROR);
          return FALSE;
        } elseif ($status === FALSE) {
          return FALSE;
        }
      }
    } catch (FusionDirectoryError $e) {
      $e->display();
      return FALSE;
    }
    return TRUE;
  }

  /*! \brief All login steps in the right order */
  static function loginProcess ()
  {
    global $config, $smarty;

    $method = $config->get_cfg_value('LoginMethod', '');
    if (empty($method)) {
      // Try to detect configurations from FD<1.4
      if ($config->get_cfg_value('httpAuthActivated') == 'TRUE') {
        $method = 'LoginHTTPAuth';
      } elseif ($config->get_cfg_value('casActivated') == 'TRUE') {
        $method = 'LoginCAS';
      } elseif ($config->get_cfg_value('httpHeaderAuthActivated') == 'TRUE') {
        $method = 'LoginHTTPHeader';
      } else {
        $method = 'LoginPost';
      }
    }
    try {
      $method::loginProcess();
    } catch (Exception $e) {
      $lang = session::get('lang');

      $display = '<h1>'.htmlescape(_('An unrecoverable error occurred. Please contact your administator.')).'</h1><p>';
      if (ini_get('display_errors') == 1) {
        $display .= nl2br(htmlescape((string)$e));
      } else {
        $display .= 'Error detail display is turned off.';
      }
      $display .= '</p>'."\n";
      $smarty->assign('headline',       _('Fatal error!'));
      $smarty->assign('headline_image', 'geticon.php?context=status&icon=dialog-error&size=32');
      $smarty->assign('usePrototype',   'false');
      $smarty->assign('date',           date('l, dS F Y H:i:s O'));
      $smarty->assign('lang',           preg_replace('/_.*$/', '', $lang));
      $smarty->assign('rtl',            Language::isRTL($lang));

      $smarty->display(get_template_path('headers.tpl'));
      echo $display;
      exit();
    }
  }

  /*! \brief Displayed name for each login method. Returning FALSE disables a method */
  static function getLabel ()
  {
    return FALSE;
  }

  static function getMethods ()
  {
    $methods = [
      'LoginPost',
      'LoginCAS',
      'LoginHTTPAuth',
      'LoginHTTPHeader',
    ];
    $return = [];
    foreach ($methods as $method) {
      $label = $method::getLabel();
      if ($label) {
        $return[$method] = $label;
      }
    }
    return $return;
  }
}
