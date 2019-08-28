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
  static function runSchemaCheck ()
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
          return _('LDAP schema check reported errors:').'<br/><br/><i>'.$tr['MSG'].'</i>';
        } else {
          msg_dialog::display(_('LDAP schema error'), $tr['MSG'], WARNING_DIALOG);
        }
      }
    }
    return TRUE;
  }

  /*! \brief Check if locking LDAP branch is here or create it */
  static function checkForLockingBranch ()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cat(get_ou('lockRDN').get_ou('fusiondirectoryRDN').$config->current['BASE'], ['dn']);
    $attrs = $ldap->fetch();
    if (!count($attrs)) {
      $ldap->cd($config->current['BASE']);
      $ldap->create_missing_trees(get_ou('lockRDN').get_ou('fusiondirectoryRDN').$config->current['BASE']);
    }
  }

  /*! \brief Check username for invalid characters and check password is not empty
   * Also trims username */
  static function validateUserInput ()
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
  static function ldapLoginUser ()
  {
    global $ui, $config, $message, $smarty;
    /* Login as user, initialize user ACL's */
    try {
      $ui = userinfo::loginUser(static::$username, static::$password);
    } catch (LoginFailureException $e) {
      if (isset($_SERVER['REMOTE_ADDR'])) {
        logging::log('security', 'login', '', [], 'Authentication failed for user "'.static::$username.'" [from '.$_SERVER['REMOTE_ADDR'].']: '.$e->getMessage());
      } else {
        logging::log('security', 'login', '', [], 'Authentication failed for user "'.static::$username.'": '.$e->getMessage());
      }
      /* Show the same message whether the user exists or not to avoid information leak */
      $message = $e->getMessage();
      $smarty->assign('focusfield', 'password');
      return FALSE;
    }
    return TRUE;
  }

  /*! \brief Called after successful login, return FALSE if account is expired */
  static function loginAndCheckExpired ()
  {
    global $ui, $config, $plist, $message, $smarty;
    /* Remove all locks of this user */
    del_user_locks($ui->dn);

    /* Save userinfo and plugin structure */
    session::set('ui', $ui);

    /* User might have its own language, re-run initLanguage */
    $plistReloaded = Language::init();

    /* We need a fully loaded plist and config to test account expiration */
    if (!$plistReloaded) {
      session::global_un_set('plist');
    }
    pluglist::load();

    /* Check that newly installed plugins have their configuration in the LDAP (will reload plist if needed) */
    $config->checkLdapConfig();

    /* Check account expiration */
    $expired = $ui->expired_status();

    if ($expired == POSIX_ACCOUNT_EXPIRED) {
      logging::log('security', 'login', '', [], 'Account for user "'.static::$username.'" has expired');
      $message = _('Account locked. Please contact your system administrator!');
      $smarty->assign('focusfield', 'username');
      return FALSE;
    }

    return TRUE;
  }

  /*! \brief Final step of successful login: redirect to main.php */
  static function redirect ()
  {
    global $config;
    /* Not account expired or password forced change go to main page */
    logging::log('security', 'login', '', [], 'User "'.static::$username.'" logged in successfully.');
    session::set('connected', 1);
    session::set('DEBUGLEVEL', $config->get_cfg_value('DEBUGLEVEL'));
    header('Location: main.php');
    exit;
  }

  /*! \brief Run each step in $steps, stop on errors */
  static function runSteps ($steps)
  {
    foreach ($steps as $step) {
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

  /*! \brief All login steps in the right order */
  static function loginProcess ()
  {
    global $config;
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
    $method::loginProcess();
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
