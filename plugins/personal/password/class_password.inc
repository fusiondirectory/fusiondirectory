<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

class password extends plugin
{
  var $proposal         = "";
  var $proposalEnabled  = FALSE;
  var $proposalSelected = FALSE;

  var $forcedHash = NULL;

  function password(&$config, $dn = NULL, $parent = NULL)
  {
    plugin::plugin($config, $dn, $parent);

    // Try to generate a password proposal, if this is successfull
    //  then preselect the proposal usage.
    $this->refreshProposal();
    if ($this->proposal != "") {
      $this->proposalSelected = TRUE;
    }
  }

  function forceHash($hash)
  {
    $this->forcedHash = $hash;
  }

  function refreshProposal()
  {
    $this->proposal = passwordMethod::getPasswordProposal($this->config);
    $this->proposalEnabled = (!empty($this->proposal));
  }

  function execute()
  {
    plugin::execute();
    $smarty = get_smarty();
    $smarty->assign("usePrototype", "true");
    $ui = get_userinfo();

    /* Get acls */
    $password_ACLS = $ui->get_permissions($ui->dn, "users/password");
    $smarty->assign("ChangeACL", $password_ACLS);
    $smarty->assign("NotAllowed", !preg_match("/w/i", $password_ACLS));

    /* Display expiration template */
    $smarty->assign("passwordExpired", FALSE);
    if ($this->config->get_cfg_value("handleExpiredAccounts") == "TRUE") {
      $expired = $ui->expired_status();
      $smarty->assign("passwordExpired", ($expired == POSIX_FORCE_PASSWORD_CHANGE));
      if ($expired == POSIX_DISALLOW_PASSWORD_CHANGE) {
        return $smarty->fetch(get_template_path("nochange.tpl", TRUE));
      }
    }


    // Refresh proposal if requested
    if (isset($_POST['refreshProposal'])) {
      $this->refreshProposal();
    }
    if (isset($_POST['proposalSelected'])) {
      $this->proposalSelected = get_post('proposalSelected') == 1;
    }
    $smarty->assign("proposal", $this->proposal);
    $smarty->assign("proposalEnabled", $this->proposalEnabled);
    $smarty->assign("proposalSelected", $this->proposalSelected);

    /* Pwd change requested */
    if (isset($_POST['password_finish'])) {
      if ($this->proposalSelected) {
        $current_password   = $_POST['current_password'];
        $new_password       = $this->proposal;
        $repeated_password  = $this->proposal;
      } else {
        $current_password   = $_POST['current_password'];
        $new_password       = $_POST['new_password'];
        $repeated_password  = $_POST['repeated_password'];
      }

      /* Should we check different characters in new password */
      $check_differ = ($this->config->get_cfg_value("passwordMinDiffer") != "");
      $differ       = $this->config->get_cfg_value("passwordMinDiffer", 0);

      /* Enable length check ? */
      $check_length = ($this->config->get_cfg_value("passwordMinLength") != "");
      $length       = $this->config->get_cfg_value("passwordMinLength", 0);

      // Perform FusionDirectory password policy checks
      $message = array();
      if (empty($current_password)) {
        $message[] = _("You need to specify your current password in order to proceed.");
      } elseif ($new_password != $repeated_password) {
        $message[] = _("The passwords you've entered as 'New password' and 'Repeated new password' do not match.");
      } elseif ($new_password == "") {
        $message[] = _("The password you've entered as 'New password' is empty.");
      } elseif ($check_differ && (substr($current_password, 0, $differ) == substr($new_password, 0, $differ))) {
        $message[] = _("The password used as new and current are too similar.");
      } elseif ($check_length && (strlen($new_password) < $length)) {
        $message[] = _("The password used as new is to short.");
      } elseif (!passwordMethod::is_harmless($new_password)) {
        $message[] = _("The password contains possibly problematic Unicode characters!");
      }

      /* Call external password quality hook ?*/
      if (!count($message)) {
        $check_hook   = $this->config->get_cfg_value("passwordHook") != "";
        $hook         = $this->config->get_cfg_value("passwordHook")." ".
            escapeshellarg($ui->username)." ".escapeshellarg($new_password)." ".escapeshellarg($current_password);
        if ($check_hook) {
          exec($hook, $resarr);
          $check_hook_output = "";
          if (count($resarr) > 0) {
            $check_hook_output = join('\n', $resarr);
          }
          if (!empty($check_hook_output)) {
            $message[] = sprintf(_("Check-hook reported a problem: %s. Password change canceled!"), $check_hook_output);
          }
        }
      }

      // Some errors/warning occured, display them and abort password change.
      if (count($message)) {
        msg_dialog::displayChecks($message);
      } else {

        /* Try to connect via current password */
        $tldap = new LDAP(
          $ui->dn,
          $current_password,
          $this->config->current['SERVER'],
          $this->config->get_cfg_value("ldapFollowReferrals") == "TRUE",
          $this->config->get_cfg_value("ldapTLS") == "TRUE"
        );

        /* connection Successfull ? */
        if (!$tldap->success()) {
          msg_dialog::display(_("Password change"),
                              _("The password you've entered as your current password doesn't match the real one."), WARNING_DIALOG);
        } else {

          /* Check FusionDirectory permissions */
          if (!preg_match("/w/i", $password_ACLS)) {
            msg_dialog::display(_("Password change"),
                                _("You have no permission to change your password."), WARNING_DIALOG);
          } else {
            $this->change_password($ui->dn, $new_password, $this->forcedHash);
            fusiondirectory_log("User/password has been changed");
            $ui->password = $new_password;
            session::set('ui', $ui);
            return $smarty->fetch(get_template_path("changed.tpl", TRUE));
          }
        }
      }
    }
    return $smarty->fetch(get_template_path("password.tpl", TRUE));
  }

  function change_password($dn, $pwd, $hash)
  {
    if ($hash) {
      change_password ($dn, $pwd, 0, $hash);
    } else {
      change_password ($dn, $pwd);
    }
  }


  function remove_from_parent()
  {
    $this->handle_post_events("remove");
  }

  function save()
  {
  }

  static function plInfo()
  {
    return array(
      "plShortName"     => _("Password"),
      "plDescription"   => _("Change user password"),
      "plSelfModify"    => TRUE,
      "plPriority"      => 10,
      "plCategory"      => array("users"),
      "plSection"       => "personal",

      "plProvidedAcls"  => array()
    );
  }

}
?>
