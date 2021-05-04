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
 * \brief Login via CAS
 */
class LoginCAS extends LoginMethod
{
  /*! \brief Displayed name */
  static function getLabel ()
  {
    return _('CAS');
  }

  /*! \brief Initialize phpCAS library */
  static function initCAS ()
  {
    global $config;

    require_once('CAS.php');
    /* Move FD autoload after CAS autoload */
    spl_autoload_unregister('__fusiondirectory_autoload');
    spl_autoload_register('__fusiondirectory_autoload');

    if ($config->get_cfg_value('casVerbose') == 'TRUE') {
      phpCAS::setVerbose(TRUE);
    }

    // Initialize phpCAS
    phpCAS::client(
      CAS_VERSION_2_0,
      $config->get_cfg_value('casHost', 'localhost'),
      (int) ($config->get_cfg_value('casPort', 443)),
      $config->get_cfg_value('casContext', '')
    );

    // Set the CA certificate that is the issuer of the cert
    phpCAS::setCasServerCACert($config->get_cfg_value('casServerCaCertPath'));
  }

  /*! \brief All login steps in the right order for CAS login */
  static function loginProcess ()
  {
    global $config, $message, $ui;

    static::init();

    static::initCAS();

    /* Reset error messages */
    $message = '';

    /* Remove query string from redirection URL to avoid signout loops */
    phpCAS::setFixedServiceURL(preg_replace('/\?.*$/', '', phpCAS::getServiceURL()));

    /* Force CAS authentication */
    phpCAS::forceAuthentication();
    static::$username = phpCAS::getUser();

    $ui = userinfo::getLdapUser(static::$username);

    if ($ui === FALSE) {
      throw new FatalError(
        htmlescape(sprintf(
          _('CAS user "%s" could not be found in the LDAP'),
          static::$username
        ))
      );
    } elseif (is_string($ui)) {
      throw new FatalError(
        htmlescape(sprintf(
          _('Login with user "%s" triggered error: %s'),
          static::$username,
          $ui
        ))
      );
    }

    $ui->loadACL();

    $success = static::runSteps([
      'checkForLockingBranch',
      'loginAndCheckExpired',
      'runSchemaCheck',
    ]);

    if ($success) {
      /* Everything went well, redirect to main.php */
      static::redirect();
    } else {
      echo msg_dialog::get_dialogs();
      if (!empty($message)) {
        throw new FatalError(
          htmlescape(sprintf(
            _('Login with user "%s" triggered error: %s'),
            static::$username,
            $message
          ))
        );
      }
      exit();
    }
  }
}
