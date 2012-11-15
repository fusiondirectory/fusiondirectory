<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007  Fabian Hickert
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


class Step_Config3 extends setup_step
{
  var $header_image = "images/setup/server.png";

  var $errorlvl   = FALSE;

  var $login_attributes = array("uid"       => "uid",
                                "mail"      => "mail",
                                "uid,mail"  => "both");
  var $optional = array(
              "login_attribute"             => "uid",
              "strict_units"                => FALSE,
              "list_summary"                => TRUE,
              "forceglobals"                => TRUE,
              "forcessl"                    => FALSE,
              "ldapstats"                   => FALSE,
              "warnssl"                     => TRUE,
              "user_filter_cookie"          => TRUE,
              "debuglevel"                  => 0,
              "session_lifetime"            => 1800,
              "compile"                     => SPOOL_DIR,
              "max_ldap_query_time"         => "5.0",
              "max_ldap_query_time_active"  => FALSE,

              "mailQueueScriptPath_active"  => FALSE,
              "notifydir_active"            => FALSE,
              "ppd_path_active"             => FALSE,
              "kioskpath_active"            => FALSE,
              "gosaSupportURI_active"       => FALSE,
              "kioskpath_active"            => FALSE,

              "enableCopyPaste"         => FALSE,
              "logging"                 => TRUE,
              "uniq_identifier"         => "entryCSN",
              "uniq_identifier_active"  => TRUE,

              "sudoou_active" => FALSE,

              "noprimarygroup"  => FALSE);

  var $attributes = array("errorlvl");

  function Step_Config3()
  {
    $this->update_strings();
  }


  function update_strings()
  {
    $this->s_title      = _("FusionDirectory settings 3/3");
    $this->s_title_long = _("FusionDirectory settings 3/3");
    $this->s_info       = _("Tweak some FusionDirectory core behaviour");
  }


  function execute()
  {
    global $BASE_DIR;
    $cv = $this->parent->captured_values;
    /*$this->optional['sudoou'] = preg_replace("/%BASE%/",$cv['base'],$this->optional['sudoou']);*/
    $smarty = get_smarty();
    $smarty->assign("bool", array(FALSE => _("No"), TRUE => _("Yes")));
    $smarty->assign("uniq_identifiers", array("entryCSN" => "entryCSN",
                                              "contextCSN" => "contextCSN" ));

    $smarty->assign("logging", $this->optional['logging']);

    $smarty->assign("optional", reverse_html_entities($this->optional));
    foreach ($this->attributes as $attr) {
      $smarty->assign($attr, reverse_html_entities($this->$attr));
    }
    $smarty->assign("login_attributes", $this->login_attributes);
    return $smarty->fetch("$BASE_DIR/setup/setup_config3.tpl");
  }


  function save_object()
  {
    if (isset($_POST['step6a_posted'])) {

      /* Get attributes */
      foreach ($this->attributes as $attr) {
        if (isset($_POST[$attr])) {
          $this->$attr = validate(get_post($attr));
        }
      }

      foreach (array("login_attribute","list_summary","strict_units","noprimarygroup","forceglobals","forcessl","ldapstats","user_filter_cookie","warnssl","compile","debuglevel","session_lifetime","enableCopyPaste","logging") as $name) {
        if (isset($_POST[$name])) {
          $this->optional[$name] = get_post($name);
        }
      }

      $this->optional['ppd_path_active'] = FALSE;

      if (isset($_POST['max_ldap_query_time_active'])) {
        $this->optional['max_ldap_query_time_active'] = TRUE;
        if (isset($_POST['max_ldap_query_time'])) {
          $this->optional['max_ldap_query_time'] = get_post('max_ldap_query_time');
        }
      } else {
        $this->optional['max_ldap_query_time_active'] = FALSE;
      }

      $this->optional['mailQueueScriptPath_active'] = FALSE;

      $this->optional['kioskpath_active'] = FALSE;

      $this->optional['sudoou_active'] = FALSE;

      $this->optional['notifydir_active'] = FALSE;

      $this->optional['gosaSupportURI_active'] = FALSE;

      if (isset($_POST['enableCopyPaste'])) {
        $this->optional['enableCopyPaste'] = get_post('enableCopyPaste');
      }

      /* Get logging */
      $this->optional['logging'] = isset($_POST['logging']);

      if (isset($_POST['uniq_identifier_active'])) {
        $this->optional['uniq_identifier_active'] = TRUE;
        if (isset($_POST['uniq_identifier'])) {
          $this->optional['uniq_identifier'] = get_post('uniq_identifier');
        }
      } else {
        $this->optional['uniq_identifier_active'] = FALSE;
      }
    }

    $tmp = $this->check();
    if (count($tmp) == 0) {
      $error = $this->save_config_in_ldap();
      if (!empty($error)) {
        msg_dialog::display(_("Setup error"), $error, ERROR_DIALOG);
      }
      $this->is_completed = TRUE;
    } else {
      $this->is_completed = FALSE;
    }
  }

  function save_config_in_ldap ()
  {
    $cv = $this->parent->captured_values;

    $attrs = array(
      'objectClass'         => 'fusionDirectoryConf',
      'fdPasswordDefaultHash' => $cv['encryption'],
      'fdUserRDN'             => $cv['peopleou'],
      'fdGroupRDN'            => $cv['groupou'],
      'fdGidNumberBase'       => $cv['uidbase'],
      'fdUidNumberBase'       => $cv['uidbase'],
    );
    if ($cv['cngen_active']) {
      $attrs['fdAccountRDN'] = 'cn='.$cv['cngen'];
    } else {
      $attrs['fdAccountPrimaryAttribute'] = $cv['peopledn'];
    }

    if ($this->optional['login_attribute']) {
      $attrs['fdLoginAttribute'] = $this->optional['login_attribute'];
    } else {
      $attrs['fdLoginAttribute'] = 'uid';
    }
    if ($cv['timezone']) {
      $attrs['fdTimezone'] = $cv['timezone'];
    }
    $attrs['fdHonourUnitTags']      = ($this->optional['strict_units']?"TRUE":"FALSE");
    $attrs['fdRfc2307bis']          = ($cv['rfc2307bis']?"TRUE":"FALSE");
    $attrs['fdPersonalTitleInDN']   = ($cv['include_personal_title']?"TRUE":"FALSE");
    if ($cv['base_hook_active']) {
      $attrs['fdNextIdHook'] = $cv['base_hook'];
    }
    if ($cv['id_settings']['idgen_active']) {
      $attrs['fdIdGenerator'] = $cv['id_settings']['idgen'];
    }
    $attrs['fdStrictNamingRules'] = ($cv['strict']?"TRUE":"FALSE");
    if ($cv['id_settings']['minid_active']) {
      $attrs['fdMinId'] = $cv['id_settings']['minid'];
    }
    //~ $attrs['fdGosaSharedPrefix'] = "";
    //~ $attrs['fdLdapTLS']   = ($cv['tls']?"TRUE":"FALSE");
    //~ $attrs['fdHonourIvbbAttributes']  = ($cv['governmentmode']?"TRUE":"FALSE");
    $attrs['fdHandleExpiredAccounts'] = ($cv['account_expiration']?"TRUE":"FALSE");
    $attrs['fdEnableSnapshots']       = ($cv['generic_settings']['snapshot_active']?"TRUE":"FALSE");
    $attrs['fdSnapshotBase']          = $cv['generic_settings']['snapshot_base'];
    $attrs['fdSnapshotAdminDn']       = $cv['generic_settings']['snapshot_user'];
    $attrs['fdSnapshotAdminPassword'] = $cv['generic_settings']['snapshot_password'];
    $attrs['fdSnapshotURI']           = $cv['generic_settings']['snapshot_server'];

    $attrs['fdLanguage']  = $cv['lang_selected'];
    $attrs['fdTheme']     = $cv['theme'];

    if ($this->optional['max_ldap_query_time_active']) {
      $attrs['fdLdapMaxQueryTime'] = $this->optional['max_ldap_query_time'];
    }
    $attrs['fdPrimaryGroupFilter']  = ($this->optional['noprimarygroup']?"TRUE":"FALSE");
    $attrs['fdStoreFilterSettings'] = ($this->optional['user_filter_cookie']?"TRUE":"FALSE");
    if ($this->optional['uniq_identifier_active']) {
      $attrs['fdModificationDetectionAttribute'] = $this->optional['uniq_identifier'];
    }
    $attrs['fdCopyPaste']   = ($this->optional['enableCopyPaste']?"TRUE":"FALSE");
    $attrs['fdListSummary'] = ($this->optional['list_summary']?"TRUE":"FALSE");
    $attrs['fdLdapStats']   = ($this->optional['ldapstats']?"TRUE":"FALSE");
    $attrs['fdWarnSSL']     = ($this->optional['warnssl']?"TRUE":"FALSE");

    if ($cv['pwd_rules']['pwminlen_active']) {
      $attrs['fdPasswordMinLength'] = $cv['pwd_rules']['pwminlen'];
    }
    if ($cv['pwd_rules']['pwdiffer_active']) {
      $attrs['fdPasswordMinDiffer'] = $cv['pwd_rules']['pwdiffer'];
    }
    if ($cv['pwd_rules']['externalpwdhook_active']) {
      $attrs['fdPasswordHook'] = $cv['pwd_rules']['externalpwdhook'];
    }
    $attrs['fdSchemaCheck'] = ($cv['enable_schema_check']?"TRUE":"FALSE");

    $attrs['fdLogging']         = ($this->optional['logging']?"TRUE":"FALSE");
    $attrs['fdDisplayErrors']   = ($this->errorlvl?"TRUE":"FALSE");
    $attrs['fdSessionLifetime'] = $this->optional['session_lifetime'];
    $attrs['fdDebugLevel']      = $this->optional['debuglevel'];

    $ldap_l = new LDAP(
      $cv['admin'], $cv['password'], $cv['connection'],
      FALSE, $cv['tls']
    );

    $ldap = new ldapMultiplexer($ldap_l);
    $dn = CONFIGRDN.$cv['base'];
    $ldap->cd($cv['base']);
    $ldap->create_missing_trees($dn);
    if (!$ldap->success()) {
      return msgPool::ldaperror($ldap->get_error(), $dn, 'create_missing_trees', get_class());
    }
    $ldap->cd($dn);
    $ldap->rmdir($dn);
    if (!$ldap->success()) {
      return msgPool::ldaperror($ldap->get_error(), $dn, 'rmdir', get_class());
    }
    $ldap->cd($dn);
    $ldap->add($attrs);
    if (!$ldap->success()) {
      return msgPool::ldaperror($ldap->get_error(), $dn, 'add', get_class());
    }
  }


  function check()
  {
    $message = array();

    if (!is_numeric($this->optional['session_lifetime'])) {
      $message[] = _("Session lifetime must be a numeric value!");
    }

    if (!is_numeric($this->optional['max_ldap_query_time'])) {
      $message[] = _("Maximum LDAP query time must be a numeric value!");
    }

    return $message;
  }


  /* Attributes that are interesting for configuration generation */
  function get_attributes()
  {
    $tmp = setup_step::get_attributes();
    foreach (array("optional") as $attr) {
      $tmp[$attr] = $this->$attr;
    }
    return($tmp);
  }

}

?>
