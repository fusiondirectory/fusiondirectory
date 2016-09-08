<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

/*!
 * \file class_plugin.inc
 * Source code for the class plugin
 */

/*! \brief plugin class, should not be used directly, see simplePlugin instead
 */
class plugin
{
  /*!
    \brief Mark plugin as account

    Defines whether this plugin is defined as an account or not.
    This has consequences for the plugin to be saved from tab
    mode. If it is set to 'FALSE' the tab will call the delete
    function, else the save function. Should be set to 'TRUE' if
    the construtor detects a valid LDAP object.

    \sa simplePlugin::is_this_account()
   */
  public $is_account            = FALSE;
  public $initially_was_account = FALSE;
  public $ignore_account        = FALSE;

  /*!
    \brief Represent temporary LDAP data

    This is only used internally.
   */
  protected $attrs = array();

  var $dialog = FALSE;

  protected $objectclasses    = array();
  protected $saved_attributes = array();

  var $acl_base     = "";
  var $acl_category = "";

  /*! \brief Can be set to render the tabulators in another stylesheet */
  public $pl_notify = FALSE;

  protected function prepareSavedAttributes()
  {
    /* Prepare saved attributes */
    $this->saved_attributes = $this->attrs;
    foreach (array_keys($this->saved_attributes) as $index) {
      if (is_numeric($index)) {
        unset($this->saved_attributes[$index]);
        continue;
      }

      if (!in_array_ics($index, $this->attributes) && strcasecmp('objectClass', $index)) {
        unset($this->saved_attributes[$index]);
        continue;
      }

      if (isset($this->saved_attributes[$index][0])) {
        if (!isset($this->saved_attributes[$index]["count"])) {
          $this->saved_attributes[$index]["count"] = count($this->saved_attributes[$index]);
        }
        if ($this->saved_attributes[$index]["count"] == 1) {
          $tmp = $this->saved_attributes[$index][0];
          unset($this->saved_attributes[$index]);
          $this->saved_attributes[$index] = $tmp;
          continue;
        }
      }
      unset($this->saved_attributes[$index]["count"]);
    }
  }

  protected function templateLoadAttrs(array $template_attrs)
  {
    if ($this->mainTab) {
      $this->_template_cn = $template_attrs['cn'][0];
    }
    $this->attrs = templateHandling::fieldsFromLDAP($template_attrs);
  }

  protected function templateSaveAttrs()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cat($this->dn);
    $template_attrs = $ldap->fetch();
    if (!$template_attrs) {
      if (!$this->mainTab) {
        trigger_error('It seems main tab has not been saved.');
      }
      $template_attrs = array(
        'objectClass'     => array('fdTemplate'),
        'fdTemplateField' => array()
      );
    }
    $template_attrs = templateHandling::fieldsToLDAP($template_attrs, $this->attrs);
    if ($this->mainTab) {
      $template_attrs['cn'] = $this->_template_cn;
    }
    return $template_attrs;
  }

  /*!
   * \brief This function is called on the copied object to set its dn to where it will be saved
   */
  function resetCopyInfos()
  {
    $this->dn       = 'new';
    $this->orig_dn  = $this->dn;

    $this->saved_attributes       = array();
    $this->initially_was_account  = FALSE;
  }

  /*!
   * \brief Remove attributes, empty arrays, arrays
   * single attributes that do not differ
   */
  function cleanup()
  {
    foreach ($this->attrs as $index => $value) {

      /* Convert arrays with one element to non arrays, if the saved
         attributes are no array, too */
      if (is_array($this->attrs[$index]) &&
          count ($this->attrs[$index]) == 1 &&
          isset($this->saved_attributes[$index]) &&
          !is_array($this->saved_attributes[$index])) {
        $this->attrs[$index] = $this->attrs[$index][0];
      }

      /* Remove emtpy arrays if they do not differ */
      if (is_array($this->attrs[$index]) &&
          count($this->attrs[$index]) == 0 &&
          !isset($this->saved_attributes[$index])) {
        unset ($this->attrs[$index]);
        continue;
      }

      /* Remove single attributes that do not differ */
      if (!is_array($this->attrs[$index]) &&
          isset($this->saved_attributes[$index]) &&
          !is_array($this->saved_attributes[$index]) &&
          $this->attrs[$index] == $this->saved_attributes[$index]) {
        unset ($this->attrs[$index]);
        continue;
      }

      /* Remove arrays that do not differ */
      if (is_array($this->attrs[$index]) &&
          isset($this->saved_attributes[$index]) &&
          is_array($this->saved_attributes[$index])) {
        if (!array_differs($this->attrs[$index], $this->saved_attributes[$index])) {
          unset ($this->attrs[$index]);
          continue;
        }
      }
    }
  }

  function prepareNextCleanup()
  {
    /* Update saved attributes and ensure that next cleanups will be successful too */
    foreach ($this->attrs as $name => $value) {
      $this->saved_attributes[$name] = $value;
    }
  }

  /*!
   * \brief Show header message for tab dialogs
   *
   * \param string $button_text The button text
   *
   * \param string $text The text
   *
   * \param boolean $plugin_enabled
   *
   * \param boolean $button_disabled FALSE
   */
  function show_header($button_text, $text, $plugin_enabled, $button_disabled = FALSE, $name = 'modify_state')
  {
    if ($button_disabled || ((!$this->acl_is_createable() && !$plugin_enabled) || (!$this->acl_is_removeable() && $plugin_enabled))) {
        $state = 'disabled="disabled"';
    } else {
        $state = '';
    }
    $display = '<div width="100%"><p><b>'.$text.'</b><br/>'."\n";
    $display .= '<input type="submit" value="'.$button_text.'" name="'.$name.'" '.$state.'></p></div><hr class="separator"/>';

    return $display;
  }

  /*!
   * \brief Create unique DN
   *
   * \param string $attribute
   *
   * \param string $base
   */
  function create_unique_dn($attribute, $base)
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $base = preg_replace('/^,*/', '', $base);

    /* Try to use plain entry first */
    $dn = $attribute.'='.ldap_escape_dn($this->$attribute).','.$base;
    if ($dn == $this->orig_dn) {
      return $dn;
    }
    $ldap->cat($dn, array('dn'));
    if (!$ldap->fetch()) {
      return $dn;
    }

    /* Look for additional attributes */
    foreach ($this->attributes as $attr) {
      if ($attr == $attribute || $this->$attr == "" || is_array($this->$attr)) {
        continue;
      }

      $dn = $attribute.'='.ldap_escape_dn($this->$attribute).'+'.$attr.'='.ldap_escape_dn($this->$attr).','.$base;
      if ($dn == $this->orig_dn) {
        return $dn;
      }
      $ldap->cat($dn, array('dn'));
      if (!$ldap->fetch()) {
        return $dn;
      }
    }

    /* None found */
    return 'none';
  }

  /*!
   * \brief  Rename/Move a given src_dn to the given dest_dn
   *
   * Move a given ldap object indentified by $src_dn to the
   * given destination $dst_dn
   *
   * \param  string  $src_dn the source DN.
   *
   * \param  string  $dst_dn the destination DN.
   *
   * \return boolean TRUE on success else FALSE.
   */
  private function rename($src_dn, $dst_dn)
  {
    global $config;
    /* Try to move the source entry to the destination position */
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->create_missing_trees(preg_replace("/^[^,]+,/", '', $dst_dn));
    if (!$ldap->rename_dn($src_dn, $dst_dn)) {
      logging::log('debug', 'Ldap Protocol v3 implementation error, ldap_rename failed.',
              "FROM: $src_dn  -- TO: $dst_dn", array(), $ldap->get_error());
      @DEBUG(DEBUG_LDAP, __LINE__, __FUNCTION__, __FILE__, "Rename failed FROM: $src_dn  -- TO:  $dst_dn",
          'Ldap Protocol v3 implementation error. Error:'.$ldap->get_error());
      return FALSE;
    }

    return TRUE;
  }


   /*!
    * \brief Move ldap entries from one place to another
    *
    * \param  string  $src_dn the source DN.
    *
    * \param  string  $dst_dn the destination DN.
    */
  function move($src_dn, $dst_dn)
  {
    global $config;
    /* Do not move if only upper- lowercase has changed */
    if (strtolower($src_dn) == strtolower($dst_dn)) {
      return TRUE;
    }

    /* Try to move with ldap routines */
    if (!$this->rename($src_dn, $dst_dn)) {
      return FALSE;
    }

    /* Get list of users,groups and roles within this tree,
        maybe we have to update ACL references.
        * TODO : replace this with a call to handleForeignKeys on sub objects
     */
    $leaf_objs = get_list("(|(objectClass=posixGroup)(objectClass=inetOrgPerson)(objectClass=gosaRole))", array("all"), $dst_dn,
                          array("dn","objectClass"), GL_SUBSEARCH | GL_NO_ACL_CHECK);
    foreach ($leaf_objs as $obj) {
      $new_dn = $obj['dn'];
      $old_dn = preg_replace("/".preg_quote(LDAP::convert($dst_dn), '/')."$/i", $src_dn, LDAP::convert($new_dn));
      $this->update_acls($old_dn, $new_dn);
    }

    /* Check if there are gosa departments moved.
       If there were deps moved, the force reload of config->deps.
     */
    $leaf_deps = get_list("(objectClass=gosaDepartment)", array("all"), $dst_dn,
                            array("dn","objectClass"), GL_SUBSEARCH | GL_NO_ACL_CHECK);

    if (count($leaf_deps)) {
      $config->get_departments();
      $config->make_idepartments();
      session::global_set("config", $config);
      $ui = get_userinfo();
      $ui->reset_acl_cache();
    }

    $this->handleForeignKeys($src_dn, $dst_dn);
    return TRUE;
  }

  /*! \brief Forward command execution requests
   *         to the pre/post hook execution method.
   *
   * \param  string  $when must be PRE or POST
   *
   * \param  string  $mode add, remove or modify
   *
   * \param  array  $addAttrs
   */
  protected function handle_hooks($when, $mode, array $addAttrs = array())
  {
    switch ($mode) {
      case 'add':
        return $this->callHook($when.'CREATE', $addAttrs);
        break;

      case 'modify':
        return $this->callHook($when.'MODIFY', $addAttrs);
        break;

      case 'remove':
        return $this->callHook($when.'REMOVE', $addAttrs);
        break;

      default:
        trigger_error(sprintf('Invalid %s event type given: "%s"! Valid types are: add, modify, remove.', strtolower($when), $mode));
        break;
    }
  }

  /*! \brief Forward command execution requests
   *         to the post hook execution method.
   */
  function handle_post_events($mode, array $addAttrs = array())
  {
    /* Update foreign keys */
    if ($mode == 'remove') {
      $this->handleForeignKeys($this->dn, NULL);
    } elseif ($mode == 'modify') {
      $this->handleForeignKeys();
    }
    return $this->handle_hooks('POST', $mode, $addAttrs);
  }

  /*!
   *  \brief Forward command execution requests
   *         to the pre hook execution method.
   */
  function handle_pre_events($mode, array $addAttrs = array())
  {
    return $this->handle_hooks('PRE', $mode, $addAttrs);
  }

  /*!
   * \brief    Calls external hooks which are defined for this plugin (fusiondirectory.conf)
   *           Replaces placeholder by class values of this plugin instance.
   *       Allows to a add special replacements.
   */
  function callHook($cmd, array $addAttrs = array(), &$returnOutput = array(), &$returnCode = NULL)
  {
    if ($this->is_template) {
      return array();
    }
    global $config;

    $commands = $config->searchHooks(get_class($this), $cmd);
    $messages = array();

    foreach ($commands as $command) {
      // Walk trough attributes list and add the plugins attributes.
      foreach ($this->attributes as $attr) {
        $addAttrs[$attr] = $this->$attr;
      }

      $ui = get_userinfo();

      $addAttrs['callerDN']         = $ui->dn;
      $addAttrs['callerCN']         = $ui->cn;
      $addAttrs['callerUID']        = $ui->uid;
      $addAttrs['callerSN']         = $ui->sn;
      $addAttrs['callerGIVENNAME']  = $ui->givenName;

      $addAttrs['dn']         = $this->dn;
      $addAttrs['location']   = $config->current['NAME'];

      if (isset($this->parent->by_object)) {
        foreach ($this->parent->by_object as $object) {
          foreach ($object->attributes as $attr) {
            if (!isset($addAttrs[$attr])) {
              $addAttrs[$attr] = $object->$attr;
            }
          }
        }
      }

      if (!isset($addAttrs['base']) && isset($this->base)) {
        $addAttrs['base'] = $this->base;
      }

      $command = templateHandling::parseString($command, $addAttrs, 'escapeshellarg');

      @DEBUG(DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, "Execute");
      exec($command, $arr, $returnCode);
      $returnOutput = $arr;

      if ($returnCode != 0) {
        $str = implode("\n", $arr);
        @DEBUG(DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, "Execution failed code: ".$returnCode);
        $message = msgPool::cmdexecfailed($cmd, $command, get_class($this));
        if (!empty($str)) {
          $message .= "Result: ".$str;
        }
        $messages[] = $message;
      } elseif (is_array($arr)) {
        $str = implode("\n", $arr);
        @DEBUG(DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, "Result: ".$str);
        if (!empty($str) && $config->get_cfg_value("displayHookOutput", "FALSE") == "TRUE") {
          msg_dialog::display('['.get_class($this).' '.strtolower($cmd)."hook] $command", $str, INFO_DIALOG);
        }
      }
    }
    return $messages;
  }

  /*!
   * \brief Set acl base
   *
   * \param string $base
   */
  function set_acl_base($base)
  {
    $this->acl_base = $base;
  }

  /*!
   * \brief Set acl category
   *
   * \param string $category
   */
  function set_acl_category($category)
  {
    $this->acl_category = "$category/";
  }

  /*! \brief Can we write the acl */
  function acl_is_writeable($attribute, $skip_write = FALSE)
  {
    if ($this->readOnly()) {
      return FALSE;
    }
    $ui = get_userinfo();
    return preg_match('/w/', $ui->get_permissions($this->acl_base, $this->acl_category.get_class($this), $attribute, $skip_write));
  }

  /*!
   * \brief Can we read the acl
   *
   * \param string $attribute
   */
  function acl_is_readable($attribute)
  {
    $ui = get_userinfo();
    return preg_match('/r/', $ui->get_permissions($this->acl_base, $this->acl_category.get_class($this), $attribute));
  }

  /*!
   * \brief Can we create the acl
   *
   * \param string $base Empty string
   */
  function acl_is_createable($base = "")
  {
    if ($this->readOnly()) {
      return FALSE;
    }
    $ui = get_userinfo();
    if ($base == "") {
      $base = $this->acl_base;
    }
    return preg_match('/c/', $ui->get_permissions($base, $this->acl_category.get_class($this), '0'));
  }

  /*!
   * \brief Can we remove the acl
   *
   * \param string $base Empty string
   */
  function acl_is_removeable($base = "")
  {
    if ($this->readOnly()) {
      return FALSE;
    }
    $ui = get_userinfo();
    if ($base == "") {
      $base = $this->acl_base;
    }
    return preg_match('/d/', $ui->get_permissions($base, $this->acl_category.get_class($this), '0'));
  }

  /*!
   * \brief Can we move the acl
   *
   * \param string $base Empty string
   */
  function acl_is_moveable($base = "")
  {
    if ($this->readOnly()) {
      return FALSE;
    }
    $ui = get_userinfo();
    if ($base == "") {
      $base = $this->acl_base;
    }
    return preg_match('/m/', $ui->get_permissions($base, $this->acl_category.get_class($this), '0'));
  }

  /*! \brief get the acl */
  function getacl($attribute, $skip_write = FALSE)
  {
    $ui         = get_userinfo();
    $skip_write |= $this->readOnly();
    return $ui->get_permissions($this->acl_base, $this->acl_category.get_class($this), $attribute, $skip_write);
  }

  /*!
   * \brief Returns a list of all available departments for this object.
   *
   * If this object is new, all departments we are allowed to create a new user in
   * are returned. If this is an existing object, return all deps.
   * We are allowed to move tis object too.
   *
   * \return array [dn] => "..name"  // All deps. we are allowed to act on.
  */
  function get_allowed_bases()
  {
    global $config;
    $deps = array();

    /* Is this a new object ? Or just an edited existing object */
    if (!$this->initially_was_account && $this->is_account) {
      $new = TRUE;
    } else {
      $new = FALSE;
    }

    foreach ($config->idepartments as $dn => $name) {
      if ($new && $this->acl_is_createable($dn)) {
        $deps[$dn] = $name;
      } elseif (!$new && $this->acl_is_moveable($dn)) {
        $deps[$dn] = $name;
      }
    }

    /* Add current base */
    if (isset($this->base) && isset($config->idepartments[$this->base])) {
      $deps[$this->base] = $config->idepartments[$this->base];
    } elseif (strtolower($this->dn) != strtolower($config->current['BASE'])) {
      trigger_error("Cannot return list of departments, no default base found in class ".get_class($this).". (base is '".$this->base."')");
    }
    return $deps;
  }

  /*
   * \brief This function updates ACL settings if $old_dn was used.
   *
   * \param string $old_dn specifies the actually used dn
   *
   * \param string $new_dn specifies the destiantion dn
   */
  function update_acls($old_dn, $new_dn)
  {
    /* Check if old_dn is empty. This should never happen */
    if (empty($old_dn) || empty($new_dn)) {
      trigger_error("Failed to check acl dependencies, wrong dn given.");
      return;
    }

    /* Update userinfo if necessary */
    $ui = session::global_get('ui');
    if ($ui->dn == $old_dn) {
      $ui->dn = $new_dn;
      session::global_set('ui', $ui);
      logging::log('view', 'acl/'.get_class($this), $this->dn, array(), 'Updated current object dn from "'.$old_dn.'" to "'.$new_dn.'"');
    }
  }
}
?>
