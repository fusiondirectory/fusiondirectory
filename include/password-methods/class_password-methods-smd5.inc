<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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
 * \file class_password-methods-smd5.inc
 * Source code for class passwordMethodsmd5
 */

/*!
 * \brief This class contains all the functions for sdm5 password methods
 * \see passwordMethod
 */
class passwordMethodsmd5 extends passwordMethod
{
  /*!
   * \brief passwordMethodsmd5 Constructor
   *
   * \param string $config
   */
  function __construct($config)
  {
  }

  /*!
   * \brief Is available
   *
   * \return TRUE if is avaibable, otherwise return false
   */
  function is_available()
  {
    if ((!function_exists('md5'))) {
      return FALSE;
    } else {
      return TRUE;
    }
  }

  /*!
   * \brief Generate template hash
   *
   * \param string $pwd Password
   */
  function generate_hash($pwd)
  {
    mt_srand(microtime() * 10000000);
    $salt0  = substr(pack("h*", md5(mt_rand())), 0, 8);
    $salt   = substr(pack("H*", md5($salt0 . $pwd)), 0, 4);
    $hash   = "{SMD5}".base64_encode(pack("H*", md5($pwd . $salt)) . $salt);
    return $hash;
  }

  function checkPassword($pwd, $hash)
  {
    $hash = base64_decode(substr($hash, 6));
    $salt = substr($hash, 16);
    $hash = substr($hash, 0, 16);
    $nhash = pack("H*", md5($pwd . $salt));
    return ($nhash == $hash);
  }

  /*!
   * \brief Get the hash name
   */
  static function get_hash_name()
  {
    return "smd5";
  }
}
?>
