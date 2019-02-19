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
 * \file class_userinfoNoAuth.inc
 * Source code for the class userinfoNoAuth
 */

/*!
 * \brief Class userinfoNoAuth
 * This represents a fake user used by public pages such as password recovery or setup
 */
class userinfoNoAuth extends userinfo
{
  function __construct ($context = 'fake')
  {
    parent::__construct($context);

    /* This fake user have all rights */
    $this->ignoreACL = TRUE;
  }

  function loadLDAPInfo ()
  {
    $this->cn   = $this->dn;
    $this->uid  = $this->dn;
  }

  function getBase ()
  {
    global $config;

    return $config->current['BASE'];
  }
}
