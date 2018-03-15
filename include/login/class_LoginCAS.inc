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
  /*! \brief All login steps in the right order for CAS login */
  static function loginProcess()
  {
    global $config, $message, $ui;

    require_once('CAS.php');
    /* Move FD autoload after CAS autoload */
    spl_autoload_unregister('__fusiondirectory_autoload');
    spl_autoload_register('__fusiondirectory_autoload');

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
      'checkForLockingBranch',
      'loginAndCheckExpired',
      'runSchemaCheck',
    ));

    if ($success) {
      /* Everything went well, redirect to main.php */
      static::redirect();
    }
  }

  static function active()
  {
    global $config;
    return ($config->get_cfg_value('casActivated') == 'TRUE');
  }
}
