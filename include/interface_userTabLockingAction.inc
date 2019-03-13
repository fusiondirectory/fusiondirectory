<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2018-2019  FusionDirectory

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
 * \file interface_userTabLockingAction.inc
 * Source code for the interface userTabLockingAction
 */

/*! \brief This interface is implemented when a user tabs needs to alter the locking LDAP modification for users
 */
interface userTabLockingAction
{
  /*! \brief Fills the $modify array with needed modifications for (un)locking this user
   *
   * This method is called after loading the object, so $this->attrs should
   *  contain information identical to the one in the LDAP
   *
   * \param string  $mode   LOCK or UNLOCK
   * \param array   &$modify LDAP modification to execute for (un)locking
   * */
  public function fillLockingLDAPAttrs (string $mode, array &$modify);
}
