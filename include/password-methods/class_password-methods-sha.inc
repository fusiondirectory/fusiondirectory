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

/*
 * \file class_pasword-methods-sha.inc
 * Source code for class passwordMethodsha
 */

/*!
 * \brief This class contains all the functions for sha password methods
 * \see passwordMethod
 */
class passwordMethodsha extends passwordMethod
{
  /*!
   * \brief passwordMethodsha Constructor
   */
  function __construct ()
  {
  }

  /*!
   * \brief Is available
   *
   * \return TRUE if is avaibable, otherwise return false
   */
  public function is_available (): bool
  {
    return (function_exists('sha1') || function_exists('mhash'));
  }

  /*!
   * \brief Generate template hash
   *
   * \param string $pwd Password
   * \param bool $locked Should the password be locked
   *
   * \return string the password hash
   */
  public function generate_hash (string $pwd, bool $locked = FALSE): string
  {
    if (function_exists('sha1')) {
      $hash = '{SHA}'.($locked ? '!' : '').base64_encode(pack('H*', sha1($pwd)));
    } elseif (function_exists('mhash')) {
      $hash = '{SHA}'.($locked ? '!' : '').base64_encode(mHash(MHASH_SHA1, $pwd));
    } else {
      throw new FusionDirectoryException(msgPool::missingext('mhash'));
    }

    return $hash;
  }

  /*!
   * \brief Get the hash name
   */
  static function get_hash_name ()
  {
    return 'sha';
  }
}
