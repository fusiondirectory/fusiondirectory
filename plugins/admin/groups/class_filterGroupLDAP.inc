<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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

class filterGroupLDAP {

  static function query($parent,$base, $scope, $filter, $attributes, $category, $objectStorage= "")
  {
    $config= session::global_get('config');
    $flag= ($scope == "sub")?GL_SUBSEARCH:0;

    $entries= filterGroupLDAP::get_list($base, $filter, $attributes, $category, $objectStorage, $flag | GL_SIZELIMIT);

    foreach ($entries as $id => $entry) {
        $ldap = $config->get_ldap_link();
        $ldap->cd($entry['dn']);
        $ldap->search("(|(objectClass=gotoMenuEntry)(objectClass=gotoSubmenuEntry))");
        if ($ldap->count()) {
          $entries[$id]["objectClass"][] = "gotoMenuGroup";
        }
    }
    return $entries;
  }

  static function get_list($parent,$base, $filter, $attributes, $category, $objectStorage, $flags= GL_SUBSEARCH)
  {
    return filterLDAP::get_list($parent,$base, $filter, $attributes, $category, $objectStorage, $flags);
  }


}

?>
