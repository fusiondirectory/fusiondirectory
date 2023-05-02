<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2019  FusionDirectory

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
 * \file class_passwordMethodClear.inc
 * Source code for class passwordMethodClear
 */

/*!
 * \brief This class contains all the functions for clear password method
 * \see passwordMethod
 */
class passwordMethodClear extends passwordMethod
{
  protected $lockable = FALSE;

  public $hash = 'clear';

  /*!
   * \brief passwordMethodClear Constructor
   */
  function __construct ()
  {
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
    return $pwd;
  }

  /*!
   * \brief Get the hash name
   */
  static function get_hash_name ()
  {
    return 'clear';
  }
}
