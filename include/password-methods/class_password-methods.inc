<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

/*
 * \file class_pasword-methods.inc
 * Source code for class password-methods
 */

/*!
 * \brief This class contains all the basic function for password methods
 */
class passwordMethod
{
  var $config   = FALSE;
  var $attrs    = array();
  var $display  = FALSE;
  var $hash     = "";
  var $lockable = TRUE;

  /*!
   * \brief Password method contructor
   *
   * \param string $config
   *
   * \param string $dn The DN
   */
  function __construct($config, $dn = "")
  {
  }

  /*!
   * \brief Create a template hash
   *
   * \param $attrs Attr
   */
  function create_template_hash($attrs)
  {
    if ($this->get_hash_name() == "") {
      return "{crypt}N0T$3T4N0W";
    } else {
      return '{'.$this->get_hash().'}'.'N0T$3T4N0W';
    }
  }

  /*!
   * \brief Get the Hash name
   */
  static function get_hash_name()
  {
    trigger_error("get_hash_name can't be called on main class");
  }

  /*!
   * \brief If we need password
   *
   * \return boolean TRUE
   */
  function need_password()
  {
    return TRUE;
  }

  /*!
   * \brief Is locked
   *
   * \param string $config
   *
   * \param string $dn The DN
   */
  function is_locked($config, $dn = "")
  {
    if (!$this->lockable) {
      return FALSE;
    }

    /* Get current password hash */
    $pwd = "";
    if (!empty($dn)) {
      $ldap = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);
      $ldap->cat($dn);
      $attrs = $ldap->fetch();
      if (isset($attrs['userPassword'][0])) {
        $pwd = $attrs['userPassword'][0];
      }
    } elseif (isset($this->attrs['userPassword'][0])) {
      $pwd = $this->attrs['userPassword'][0];
    }
    return preg_match("/^[^\}]*+\}!/", $pwd);
  }

  /*! \brief       Locks an account (gosaAccount) by added a '!' as prefix to the password hashes.
   *               This makes logins impossible, due to the fact that the hash becomes invalid.
   *               userPassword: {SHA}!q02NKl9IChNwZEAJxzRdmB6E
   *               sambaLMPassword: !EBD223B61F8C259AD3B435B51404EE
   *               sambaNTPassword: !98BB35737013AAF181D0FE9FDA09E
   *
   * \param string $config
   *
   * \param string $dn
   */
  function lock_account($config, $dn = "")
  {
    return $this->generic_modify_account($config, $dn, 'LOCK');
  }

  /*!
   * \brief Unlocks an account (gosaAccount) which was locked by 'lock_account()'.
   *        For details about the locking mechanism see 'lock_account()'.
   */
  function unlock_account($config, $dn = "")
  {
    return $this->generic_modify_account($config, $dn, 'UNLOCK');
  }

  /*!
   * \brief Unlocks an account (gosaAccount) which was locked by 'lock_account()'.
   *        For details about the locking mechanism see 'lock_account()'.
   */
  private function generic_modify_account($config, $dn, $mode)
  {
    if (!$this->lockable) {
      return FALSE;
    }
    if ($mode != 'LOCK' && $mode != 'UNLOCK') {
      die('Invalid mode "'.$mode.'"');
    }

    /* Get current password hash */
    $pwd  = "";
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    if (!empty($dn)) {
      $ldap->cat($dn);
      $attrs = $ldap->fetch();
      if (isset($attrs['userPassword'][0])) {
        $pwd = $attrs['userPassword'][0];
      }
    } elseif (isset($this->attrs['userPassword'][0])) {
      $pwd  = $this->attrs['userPassword'][0];
      $dn   = $this->attrs['dn'];
    }

    /* We can only lock/unlock non-empty passwords */
    if (!empty($pwd)) {

      /* Check if this entry is already locked. */
      if (!preg_match("/^[^\}]*+\}!/", $pwd)) {
        if ($mode == 'UNLOCK') {
          return TRUE;
        }
      } elseif ($mode == 'LOCK') {
        return TRUE;
      }

      // (Un)lock the account by modifying the password hash.
      $pwdClass = new password($config, $dn);
      $pwdClass->callHook($pwdClass, 'PRE'.$mode, array(), $ret);

      if ($mode == 'LOCK') {
        /* Lock entry */
        $pwd = preg_replace("/(^[^\}]+\})(.*$)/",   "\\1!\\2",  $pwd);
      } else {
        /* Unlock entry */
        $pwd = preg_replace("/(^[^\}]+\})!(.*$)/",  "\\1\\2",   $pwd);
      }
      $ldap->cd($dn);
      $ldap->modify(array("userPassword" => $pwd));

      // Call the password post-lock hook, if defined.
      if ($ldap->success()) {
          $pwdClass->callHook($pwdClass, 'POST'.$mode, array(), $ret);
      }
      return $ldap->success();
    }
    return FALSE;
  }


  /*!
   * \brief This function returns all loaded classes for password encryption
   */
  static function get_available_methods()
  {
    global $class_mapping, $config;
    $ret  = FALSE;
    $i    = 0;

    /* Only */
    if (!session::is_set("passwordMethod::get_available_methods")) {
      foreach (array_keys($class_mapping) as $class) {
        if (preg_match('/passwordMethod/i', $class) && !preg_match("/^passwordMethod$/i", $class)) {
          $test = new $class($config, "");
          if ($test->is_available()) {
            $plugs = $test->get_hash_name();
            if (!is_array($plugs)) {
              $plugs = array($plugs);
            }

            foreach ($plugs as $plugname) {
              $cfg = $test->is_configurable();

              $ret['name'][$i]            = $plugname;
              $ret['class'][$i]           = $class;
              $ret['is_configurable'][$i] = $cfg;
              $ret['object'][$i]          = $test;
              $ret['desc'][$i]            = $test->get_description();
              $ret[$i]['name']            = $plugname;
              $ret[$i]['class']           = $class;
              $ret[$i]['object']          = $test;
              $ret[$i]['is_configurable'] = $cfg;
              $ret[$i]['desc']            = $test->get_description();
              $ret[$plugname]             = $class;
              $i++;
            }
          }
        }
      }
      session::set("passwordMethod::get_available_methods", $ret);
    }
    return session::get("passwordMethod::get_available_methods");
  }

  /*!
   * \brief Get desciption
   */
  function get_description()
  {
    return "";
  }


  /*!
   * \brief Method to let password backends remove additional information besides
   * the userPassword attribute
   */
  function remove_from_parent()
  {
  }


  /*!
   * \brief Method to let passwords backends manage additional information
   *  besides the userAttribute entry
   */
  function set_password($password)
  {
    return TRUE;
  }


  /*!
   * \brief Return true if this password method provides a configuration dialog
   */
  function is_configurable()
  {
    return FALSE;
  }

  /*!
   * \brief Provide a subdialog to configure a password method
   */
  function configure()
  {
    return "";
  }


  /*!
   * \brief Save information to LDAP
   *
   * \param string $dn The DN
   */
  function save($dn)
  {
  }


  /*!
   * \brief Try to find out if it's our hash...
   *
   * \param string $password_hash
   *
   * \param string $dn The DN
   */
  static function get_method($password_hash, $dn = "")
  {
    global $config;

    $methods = passwordMethod::get_available_methods();

    foreach ($methods['class'] as $class) {
      $method = $class::_extract_method($class, $password_hash);
      if ($method != "") {
        $test = new $class($config, $dn);
        $test->set_hash($method);
        return $test;
      }
    }

    msg_dialog::display(_("Error"), _("Cannot find a suitable password method for the current hash!"), ERROR_DIALOG);

    return NULL;
  }

  /*!
   * \brief Extract a method
   *
   * \param string $password_hash
   */
  static function _extract_method($classname, $password_hash)
  {
    $hash = $classname::get_hash_name();
    if (preg_match("/^\{$hash\}/i", $password_hash)) {
      return $hash;
    }

    return "";
  }

  /*!
   * \brief Make a hash
   *
   * \param string $password The password
   *
   * \param string $hash
   */
  static function make_hash($password, $hash)
  {
    global $config;

    $methods  = passwordMethod::get_available_methods();
    $tmp      = new $methods[$hash]($config);
    $tmp->set_hash($hash);
    return $tmp->generate_hash($password);
  }

  /*!
   * \brief Set a hash
   *
   * \param string $hash
   */
  function set_hash($hash)
  {
    $this->hash = $hash;
  }


  /*!
   * \brief Get a hash
   */
  function get_hash()
  {
    return $this->hash;
  }

  /*!
   * Get the current object
   *
   * \param string $dn The DN
   */
  function adapt_from_template($attrs)
  {
    return $this;
  }

  /*!
   * \brief Test for problematic unicode caracters in password
   *  This can be activated with the keyword strictPasswordRules in the
   *  fusiondirectory.conf
   *
   * \param string $password The password
   */
  static function is_harmless($password)
  {
    global $config;

    if ($config->get_cfg_value("strictPasswordRules") == "TRUE") {
      // Do we have UTF8 characters in the password?
      return ($password == utf8_decode($password));
    }

    return TRUE;
  }

  /*!
   * \brief Get the password proposal
   *
   * \param string $config
   */
  static function getPasswordProposal($config)
  {
    if ($config->get_cfg_value('passwordProposalHook', '') != '') {
        $command = $config->get_cfg_value('passwordProposalHook', '');
      if (check_command($command)) {
        @DEBUG (DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, "Execute");
        exec($command, $arr, $returnCode);

        if ($returnCode != 0) {
          $str = implode("\n", $arr);
          @DEBUG (DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, "Execution failed code: ".$returnCode);
          $message = msgPool::cmdexecfailed($cmd, $command, get_class($plugin));
          msg_dialog::display(_("Error"), $message, ERROR_DIALOG);
        } elseif (is_array($arr)) {
          $str = implode("\n", $arr);
          @DEBUG (DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, "Result: ".$str);
          if (count($arr) && !empty($arr[0])) {
            return $arr[0];
          }
        }
      } else {
        $message = msgPool::cmdinvalid($cmd, $command, get_class($plugin));
        msg_dialog::display(_("Error"), $message, ERROR_DIALOG);
      }
    }
    return '';
  }
}
?>
