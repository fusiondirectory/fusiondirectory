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
  var $acl_base     = "";
  var $acl_category = "";

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
}
?>
