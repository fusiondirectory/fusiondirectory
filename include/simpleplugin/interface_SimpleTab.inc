<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2018-2020  FusionDirectory

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
 * \file interface_SimpleTab.inc
 * Source code for the interface SimpleTab
 */

/*! \brief This interface is implemented by classes intended to be used as tabs, mainly simplePlugin itself.
 */

/**
 * @property bool $is_account
 * @property string $dn
 * @property bool $is_template
 * @property string $_template_cn (only for main tab of templates)
 * @property array $attributesAccess (only for main tab of templates)
 * @property simpleTabs $parent
 */
interface SimpleTab extends FusionDirectoryDialog
{
  /*
   * Public methods needed in some cases:
   * compute_dn (): string (only for main tab)
   * move (string $src_dn, string $dst_dn): TRUE|string (only for main tab)
   * getAclBase (): string (only for main tab)
   */

  public static function plInfo (): array;

  /*! \brief Checks data and return errors
   */
  public function check (): array;

  /*! \brief Delete tab data from LDAP and return errors
   */
  public function remove (bool $fulldelete = FALSE): array;

  /*! \brief Save data to the LDAP and return errors
   */
  public function save (): array;

  /*! \brief Reset information after a copy/paste
   */
  public function resetCopyInfos ();

  /*! \brief Sets ACL category provided by simpleTabs
   */
  public function set_acl_category (string $category);

  /*!
   * \brief Can we delete the object
   *
   * Only used on main tab
   *
   * \param string $base
   */
  public function acl_is_removeable (string $base = NULL): bool;

  /*!
   * \brief Sets whether the opened objet is a template
   *
   * \param bool $isTemplate
   */
  public function setTemplate (bool $isTemplate);

  /*!
   * \brief Sets whether the opened objet has an edit button
   *
   * \param bool $needEditMode
   */
  public function setNeedEditMode (bool $needEditMode);

  /*!
   * \brief Is there a modal dialog opened
   */
  public function is_modal_dialog (): bool;

  /*!
   * \brief Returns list of required LDAP attributes
   *
   * Used by template
   */
  public function getRequiredAttributes (): array;

  /*!
   * \brief Returns TRUE if this attribute should be asked in the creation by template dialog
   */
  public function showInTemplate (string $attr, array $templateAttrs): bool;

  /*! \brief Check if logged in user have enough right to read this attribute value
   *
   * \param mixed $attr Attribute object or name (in this case it will be fetched from attributesAccess)
   */
  public function attrIsReadable ($attr): bool;

  /*! \brief Check if logged in user have enough right to write this attribute value
   *
   * \param mixed $attr Attribute object or name (in this case it will be fetched from attributesAccess)
   */
  public function attrIsWriteable ($attr): bool;

  /*!
   * \brief Adapt from template
   */
  public function adapt_from_template (array $attrs, array $skip = []);

  /*!
   * \brief Deserialize values
   *
   * Returns TRUE or error
   */
  public function deserializeValues (array $values, bool $checkAcl = TRUE);

  /*!
   * \brief Test if the plugins have ACLs declared
   *
   * If FALSE tab is always shown, if TRUE aclGetPermissions('') is called
   */
  public function aclHasPermissions (): bool;

  /*!
   * \brief Get the acl permissions for an attribute or the plugin itself
   */
  public function aclGetPermissions ($attribute = '0', string $base = NULL, bool $skipWrite = FALSE): string;

  /*!
   * \brief Merge in objectClasses needed by this tab
   *
   *  Used by prepare_save and template::apply
   */
  public function mergeObjectClasses (array $oc): array;

  /*!
   * \brief Fill attributes which may be used in hooks
   *
   *  Used by simplePlugin::callHook
   */
  public function fillHookAttrs (array &$addAttrs);

  /*!
   * \brief Test whether a tab is active
   */
  public function isActive (): bool;

  /*!
   * \brief Test whether a tab can be deactivated
   */
  public function isActivatable (): bool;
}
