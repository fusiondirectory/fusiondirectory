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
$tokenBranch = get_ou('tokenRDN').$config->current['BASE'];
recoveryConfig::userMainInc('recoveryConfig',"cn=config,$tokenBranch");

/*
if ($remove_lock) {
  if (session::is_set('recoveryConfig')) {
    //nothing to do here
  }
}

if ($cleanup) {
  session::un_set('recoveryConfig');
} else {

  if (isset($_GET['reset']) && $_GET['reset'] == 1) {
    session::un_set('recoveryConfig');
  }

  if (!session::is_set('recoveryConfig')) {
    // Check if token branch is here
    $ldap = $config->get_ldap_link();
    $tokenBranch = get_ou("tokenRDN").$config->current['BASE'];
    $ldap->cat($tokenBranch, array('dn'));
    if (!$ldap->count()) {
      $ldap->cd ($config->current['BASE']);
      $ldap->create_missing_trees($tokenBranch);
      if (!$ldap->success()) {
        $display = msgPool::ldaperror($ldap->get_error(),
                                  $token, LDAP_MOD, get_class());
      }
      fusiondirectory_log("Created token branch ".$token);
    }
    $dn = "cn=config,$tokenBranch";
    $recoveryConfig = new recoveryConfig ($config, $dn);
    session::set('recoveryConfig',$recoveryConfig);
  }
  $display = print_header(get_template_path('plugins/password/images/plugin.png'),
                         _("Password recovery settings"), "");

  $recoveryConfig = session::get('recoveryConfig');
  $display .= $recoveryConfig->execute();

  session::set('recoveryConfig',$recoveryConfig);
}*/
?>
