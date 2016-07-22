<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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
 * \file class_password-methods-crypt.inc
 * Source code for class passwordMethodCrypt
 */

/*!
 * \brief This class contains all the functions for crypt password methods
 * \see passwordMethod
 */
class passwordMethodCrypt extends passwordMethod
{

  /*!
   * \brief passwordMethodCrypt Constructor
   */
  function __construct()
  {
  }

  /*!
   * \brief Is available
   *
   * \return TRUE if is avaibable, otherwise return false
   */
  function is_available()
  {
    if (function_exists("crypt")) {
      return TRUE;
    } else {
      return FALSE;
    }
  }

  /*!
   * \brief Create template hash
   *
   * \param string $attrs
   */
  function create_template_hash($attrs)
  {
    return $this->generate_hash('N0T$3T4N0W').'N0T$3T4N0W';
  }

  /*!
   * \brief Generate template hash
   *
   * \param string $pwd Password
   */
  function generate_hash($pwd)
  {
    if ($this->hash == "crypt/standard-des") {
      $salt = "";
      for ($i = 0; $i < 2; $i++) {
          $salt .= get_random_char();
      }
    }

    if ($this->hash == "crypt/enhanced-des") {
      $salt = "_";
      for ($i = 0; $i < 8; $i++) {
          $salt .= get_random_char();
      }
    }

    if ($this->hash == "crypt/md5") {
      $salt = "\$1\$";
      for ($i = 0; $i < 6; $i++) {
          $salt .= get_random_char();
      }
      $salt .= "\$";
    }

    if ($this->hash == "crypt/blowfish") {
      $salt = "\$2a\$07\$";
      for ($i = 0; $i < CRYPT_SALT_LENGTH; $i++) {
          $salt .= get_random_char();
      }
      $salt .= "\$";
    }

    if ($this->hash == "crypt/sha-256") {
      $salt = "\$5\$";
      for ($i = 0; $i < 12; $i++) {
          $salt .= get_random_char();
      }
      $salt .= "\$";
    }

    if ($this->hash == "crypt/sha-512") {
      $salt = "\$6\$";
      for ($i = 0; $i < 12; $i++) {
          $salt .= get_random_char();
      }
      $salt .= "\$";
    }

    return "{CRYPT}".crypt($pwd, $salt);
  }

  function checkPassword($pwd, $hash)
  {
    // Not implemented
    return FALSE;
  }

  /*!
   * \brief Get the hash name
   */
  static function get_hash_name()
  {
    $hashes = array();
    if (CRYPT_STD_DES == 1) {
      $hashes[] = "crypt/standard-des";
    }

    if (CRYPT_EXT_DES == 1) {
      $hashes[] = "crypt/enhanced-des";
    }

    if (CRYPT_MD5 == 1) {
      $hashes[] = "crypt/md5";
    }

    if (CRYPT_BLOWFISH == 1) {
      $hashes[] = "crypt/blowfish";
    }

    if (CRYPT_SHA256 == 1) {
      $hashes[] = "crypt/sha-256";
    }

    if (CRYPT_SHA512 == 1) {
      $hashes[] = "crypt/sha-512";
    }

    return $hashes;
  }

  /*!
   * \brief Extract a method
   *
   * \param string $password_hash
   */
  static function _extract_method($classname, $password_hash)
  {
    if (!preg_match('/^{crypt}/i', $password_hash)) {
      return "";
    }

    $password_hash = preg_replace('/^{[^}]+}!?/', '', $password_hash);

    if (preg_match("/^[a-zA-Z0-9.\/][a-zA-Z0-9.\/]/", $password_hash)) {
      return "crypt/standard-des";
    }

    if (preg_match("/^_[a-zA-Z0-9.\/]/", $password_hash)) {
      return "crypt/enhanced-des";
    }

    if (preg_match('/^\$1\$/', $password_hash)) {
      return "crypt/md5";
    }

    if (preg_match('/^(\$2\$|\$2a\$)/', $password_hash)) {
      return "crypt/blowfish";
    }

    if (preg_match('/^\$5\$/', $password_hash)) {
      return "crypt/sha-256";
    }

    if (preg_match('/^\$6\$/', $password_hash)) {
      return "crypt/sha-512";
    }

    return "";
  }
}
?>
