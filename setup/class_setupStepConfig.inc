<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2007 Fabian Hickert
  Copyright (C) 2011-2019  FusionDirectory

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

class setupStepConfig_before_init extends setupStep
{
  function update_strings ()
  {
    global $config;
    if (!isset($config->current['BASE'])) {
      $config->current['BASE'] = '';
    }
    $infos = configInLdap::plInfo();
    $this->header_image   = $infos['plIcon'];
    $this->s_short_name   = $infos['plShortName'];
    $this->s_title        = $infos['plTitle'];
    $this->s_description  = $infos['plDescription'];
  }
}

class setupStepConfig extends configInLdap
{
  var $is_active    = FALSE;
  var $is_enabled   = FALSE;
  var $is_completed = FALSE;
  var $header_image;

  protected $objectclasses = ['fusionDirectoryConf'];

  static function plInfo (): array
  {
    return [];
  }

  function __construct ($parent, $cv)
  {
    global $config;
    parent::__construct(CONFIGRDN.$config->current['BASE'], NULL, $parent, TRUE);

    $this->attributesInfo['miscellaneous']['class'] = ['invisible'];
    $this->attributesInfo['debug']['class'] = ['invisible'];

    $this->fdLanguage     = $cv['lang_selected'];
  }

  function update_strings ()
  {
    $infos = parent::plInfo();
    $this->header_image = $infos['plIcon'];
  }

  function get_short_name ()
  {
    $infos = parent::plInfo();
    return $infos['plTitle'];
  }

  function get_title ()
  {
    return $this->get_description();
  }

  function get_description ()
  {
    $infos = parent::plInfo();
    return $infos['plDescription'];
  }

  /* Return attributes handled by this setup step */
  function get_attributes ()
  {
    $tmp = [];
    foreach (array_keys($this->attributesAccess) as $attr) {
      $tmp[$attr] = $this->$attr;
    }
    return $tmp;
  }

  function save_object ()
  {
    global $config, $plist;
    parent::save_object();
    $this->is_completed = FALSE;
    $tmp = $this->check();
    if (count($tmp) == 0) {
      /* Create root object if missing */
      $ldap = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);
      $ldap->create_missing_trees($config->current['BASE'], FALSE);
      if (!$ldap->success()) {
        msg_dialog::display(
          _('LDAP error'),
          msgPool::ldaperror($ldap->get_error(), $config->current['BASE'], 'create_missing_trees', get_class()),
          LDAP_ERROR
        );
        return;
      }
      /* Save in LDAP */
      $errors = $this->save();
      if (!empty($errors)) {
        msg_dialog::displayChecks($errors);
        return;
      }
      /* Insert default config values, even for installed plugin */
      session::un_set('plist');
      pluglist::load();
      $config->loadPlist($plist);
      $config->checkLdapConfig(TRUE);
      /* Reload config from LDAP */
      $config->set_current($config->current['NAME']);
      /* Set as completed and reload step */
      $this->is_completed = TRUE;
      $this->parent->reBuildConfigStep(TRUE);
    }
  }

  function is_active ()
  {
    return $this->is_active;
  }

  function is_enabled ()
  {
    return $this->is_enabled;
  }

  function is_completed ()
  {
    return $this->is_completed;
  }

  function set_active ($value = TRUE)
  {
    $this->is_active = (bool) $value;
  }

  function set_enabled ($value = TRUE)
  {
    $this->is_enabled = (bool) $value;
  }

  function set_completed ($value = TRUE)
  {
    $this->is_completed = (bool) $value;
  }

  /* bypass acl system as there is no user login */
  function acl_is_writeable ($attribute, bool $skip_write = FALSE): bool
  {
    return TRUE;
  }
  function acl_is_readable ($attribute): bool
  {
    return TRUE;
  }
  function acl_is_createable (string $base = NULL): bool
  {
    return TRUE;
  }
  function acl_is_removeable (string $base = NULL): bool
  {
    return TRUE;
  }
  function acl_is_moveable (string $base = NULL): bool
  {
    return TRUE;
  }
  function aclGetPermissions ($attribute = '0', string $base = NULL, bool $skipWrite = FALSE): string
  {
    return 'cmdrw';
  }

  /* We need static method to work as if we were configInLdap */
  static function isAccount ($attrs)
  {
    return configInLdap::isAccount($attrs);
  }
}
