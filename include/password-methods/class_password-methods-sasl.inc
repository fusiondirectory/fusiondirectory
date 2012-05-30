<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class passwordMethodsasl extends passwordMethod {
  var $uid    = ""; // uid, or exop specified field value
  var $realm  = "";
  var $exop   = "";

  function __construct($config, $dn="")
  {
    if ($dn == "" || $dn == "new") {
      return;
    }

    $this->realm  = trim($config->get_cfg_value('saslRealm',""));
    $this->exop   = trim($config->get_cfg_value('saslExop',""));
    if (empty($this->realm) && empty($this->exop)) {
      trigger_error(msgPool::cmdnotfound("saslRealm", _("SASL")));
      trigger_error(msgPool::cmdnotfound("saslExop", _("SASL")));
    }

    $attr = (empty($this->exop)?'uid':$this->exop);

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->cat($dn, array($attr));
    if ($ldap->count() == 1) {
      $attrs = $ldap->fetch();
      $this->uid = $attrs[$attr][0];
    } else {
      trigger_error("Cannot change password, unknown user '".$dn."'");
    }
  }

  function is_available()
  {
    return TRUE;
  }

  function generate_hash($pwd)
  {
    if (empty($this->exop)) {
      return "{SASL}".$this->uid."@".$this->realm;
    } else {
      return "{SASL}".$this->uid; // may not be the uid, see saslExop option
    }
  }

  function get_hash_name()
  {
    return "sasl";
  }

  function need_password()
  {
    return FALSE;
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
