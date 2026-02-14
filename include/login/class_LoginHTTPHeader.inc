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
  /*! \brief Displayed name */
  static function getLabel ()
  {
    return _('HTTP Header');
  }

  /*! \brief All login steps in the right order for HTTP Header login */
  static function loginProcess ()
  {
    global $config, $message, $ui;

    static::init();

    /* Reset error messages */
    $message = '';

    // Get username header configuration
    $header = $config->get_cfg_value('httpHeaderAuthHeaderName', 'AUTH_USER');
    // History - be aware to set the header to AUTH-USER without X and score, not underscore.
    $headerKey = 'HTTP_'.strtoupper(str_replace('-', '_', $header));

    // Safely retrieve username from header
    if (isset($_SERVER[$headerKey])) {
      static::$username = $_SERVER[$headerKey];
    } else {
      // Header doesn't exist
      throw new FatalError(
        htmlescape(sprintf(
          _('No value found in HTTP header "%s" (Location: %s)'),
          $header,
          $config->current['NAME']
        ))
      );
    }

    if (empty(static::$username)) {
      throw new FatalError(
        htmlescape(sprintf(
          _('Empty value in HTTP header "%s" (Location: %s)'),
          $header,
          $config->current['NAME']
        ))
      );
    }

    $ui = userinfo::getLdapUser(static::$username);

    if ($ui === FALSE) {
      throw new FatalError(
        htmlescape(sprintf(
          _('Header user "%s" could not be found in LDAP (Location: %s)'),
          static::$username,
          $config->current['NAME']
        ))
      );
    } elseif (is_string($ui)) {
      throw new FatalError(
        htmlescape(sprintf(
          _('Login with user "%s" triggered error: %s (Location: %s)'),
          static::$username,
          $ui,
          $config->current['NAME']
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
      throw new FatalError(
        htmlescape(sprintf(
          _('Login with user "%s" triggered error: %s (Location: %s)'),
          static::$username,
          $message,
          $config->current['NAME']
        ))
      );
    }
  }
}
