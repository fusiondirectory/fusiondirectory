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

  var $acl_base     = "";
  var $acl_category = "";

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
    $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $dst_dn));
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
    global $config, $ui;

    /* Do not move if only case has changed */
    if (strtolower($src_dn) == strtolower($dst_dn)) {
      return TRUE;
    }

    /* Try to move with ldap routines */
    if (!$this->rename($src_dn, $dst_dn)) {
      return FALSE;
    }

    /* Update userinfo if necessary */
    if (preg_match('/'.preg_quote($src_dn, '/').'$/i', $ui->dn)) {
      $ui_dn = preg_replace('/'.preg_quote($src_dn, '/').'$/i', $dst_dn, $ui->dn);
      logging::log('view', 'acl/'.get_class($this), $this->dn, array(), 'Updated userinfo dn from "'.$ui->dn.'" to "'.$ui_dn.'"');
      $ui->dn = $ui_dn;
    }

    /* Check if departments were moved. If so, force the reload of config->departments */
    $ldap = $config->get_ldap_link();
    $ldap->cd($dst_dn);
    $ldap->search('(objectClass=gosaDepartment)', array('dn'));
    if ($ldap->count()) {
      $config->get_departments();
      $config->make_idepartments();
      $ui->reset_acl_cache();
    }

    $this->handleForeignKeys($src_dn, $dst_dn);
    return TRUE;
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
}
?>
