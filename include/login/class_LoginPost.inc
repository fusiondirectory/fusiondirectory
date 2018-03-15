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
  /*! \brief All login steps in the right order for standard POST login */
  static function loginProcess()
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
    return (($_SERVER['REQUEST_METHOD'] == 'POST') && isset($_POST['login']));
  }
}
