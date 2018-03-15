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
 * \brief Login via HTTP Header
 */
class LoginHTTPHeader extends LoginMethod
{
  /*! \brief All login steps in the right order for HTTP Header login */
  static function loginProcess()
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
    return ($config->get_cfg_value('httpHeaderAuthActivated') == 'TRUE');
  }
}
