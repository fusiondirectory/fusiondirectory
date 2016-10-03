<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
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

class tabs_dashboard extends simpleTabs_noSpecial
{
  function __construct($type, $dn, $copied_object = NULL)
  {
    parent::__construct($type, $dn, $copied_object);
    if (!class_available('dhcpService') && !class_available('dnsZone')) {
      unset($this->by_object['dashboardNetwork']);
      unset($this->by_name['dashboardNetwork']);
    }
  }

  function save()
  {
  }
}

?>
