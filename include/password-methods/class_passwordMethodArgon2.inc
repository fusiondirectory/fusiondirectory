<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2025  FusionDirectory

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


class passwordMethodArgon2 extends passwordMethod
{
  public $hash = 'argon2';

  /*!
   * \brief passwordMethodArgon2 Constructor
   */
  function __construct()
  {
  }

  /*!
   * \brief Is available
   *
   * \return TRUE if is available, otherwise return false
   */
  public function is_available(): bool
  {
    return defined('PASSWORD_ARGON2ID') && function_exists('password_hash') && function_exists('password_verify');
  }

  /*!
   * \brief Generate template hash
   *
   * \param string $pwd Password
   * \param bool $locked Should the password be locked
   *
   * \return string the password hash
   */
  public function generate_hash(string $pwd, bool $locked = FALSE): string
  {
    $hash = password_hash($pwd, PASSWORD_ARGON2ID);
    return '{ARGON2}'.($locked ? '!' : '').$hash;
  }

  /*!
   * \brief Check if a password matches a hash
   *
   * \param string $pwd The password to check
   * \param string $hash The hash to check against
   * 
   * \return bool True if the password matches the hash
   */
  function checkPassword($pwd, $hash): bool
  {
    // Remove the {ARGON2} prefix and any lock indicator
    $hash = preg_replace('/^\{ARGON2\}!?/', '', $hash);
    
    if (strlen($hash) === 0) {
      return false;
    }
    return password_verify($pwd, $hash);
  }

  /*!
   * \brief Check if the hash needs to be rehashed
   *
   * \param string $hash The hash to check
   *
   * \return bool True if the hash needs to be rehashed
   */
  public function needsRehash(string $hash): bool
  {
    // Remove the {ARGON2} prefix and any lock indicator
    $hash = preg_replace('/^\{ARGON2\}!?/', '', $hash);
    return password_needs_rehash($hash, PASSWORD_ARGON2ID);
  }

  /*!
   * \brief Get the hash name
   */
  static function get_hash_name()
  {
    return 'argon2';
  }

  /*!
   * \brief Extract a method
   *
   * \param string $password_hash
   */
  static function _extract_method($password_hash): string
  {
    if (preg_match('/^\{ARGON2\}/i', $password_hash)) {
      return static::get_hash_name();
    }
    
    return '';
  }
}
?>