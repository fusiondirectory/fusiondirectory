<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007 Fabian Hickert
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

class Step_Config_before_init extends setupStep
{
  function update_strings()
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

class Step_Config extends configInLdap
{
  var $is_active    = FALSE;
  var $is_enabled   = FALSE;
  var $is_completed = FALSE;
  var $header_image;

  static function plInfo()
  {
    return array();
  }

  function __construct($parent, $cv)
  {
    global $config;
    parent::__construct(CONFIGRDN.$config->current['BASE'], NULL, $parent, TRUE);

    $this->attributesInfo['miscellaneous']['class'] = array('invisible');
    $this->attributesInfo['debug']['class'] = array('invisible');

    $this->fdLanguage     = $cv['lang_selected'];
  }

  function update_strings()
  {
    $infos = parent::plInfo();
    $this->header_image = $infos['plIcon'];
  }

  function get_short_name()
  {
    $infos = parent::plInfo();
    return $infos['plTitle'];
  }

  function get_title()
  {
    $infos = parent::plInfo();
    return $infos['plDescription'];
  }

  function get_description()
  {
    $infos = parent::plInfo();
    return $infos['plDescription'];
  }

  /* Return attributes handled by this setup step */
  function get_attributes()
  {
    $tmp = array();
    foreach (array_keys($this->attributesAccess) as $attr) {
      $tmp[$attr] = $this->$attr;
    }
    return $tmp;
  }

  function save_object()
  {
    global $config;
    parent::save_object();
    $tmp = $this->check();
    if (count($tmp) == 0) {
      /* Create root object if missing */
      $ldap = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);
      $ldap->create_missing_trees($config->current['BASE'], FALSE);
      if (!$ldap->success()) {
        return msgPool::ldaperror($ldap->get_error(), $config->current['BASE'], 'create_missing_trees', get_class());
      }
      /* Save in LDAP */
      $this->save();
      /* Reload config from LDAP */
      $config->set_current($config->current['NAME']);
      session::global_un_set('plist');
      load_plist();
      $this->is_completed = TRUE;
      $this->parent->reBuildConfigStep(TRUE);
    } else {
      $this->is_completed = FALSE;
    }
  }

  function is_active()
  {
    return $this->is_active;
  }

  function is_enabled()
  {
    return $this->is_enabled;
  }

  function is_completed()
  {
    return $this->is_completed;
  }

  function set_active($value = TRUE)
  {
    $this->is_active = ($value == TRUE);
  }

  function set_enabled($value = TRUE)
  {
    $this->is_enabled = ($value == TRUE);
  }

  function set_completed($value = TRUE)
  {
    $this->is_completed = ($value == TRUE);
  }

  /* bypass acl system as there is no user login */
  function acl_is_writeable($attribute, $skip_write = FALSE)
  {
    return TRUE;
  }
  function acl_is_readable($attribute)
  {
    return TRUE;
  }
  function acl_is_createable($base = "")
  {
    return TRUE;
  }
  function acl_is_removeable($base = "")
  {
    return TRUE;
  }
  function acl_is_moveable($base = "")
  {
    return TRUE;
  }
  function getacl($attribute, $skip_write = FALSE)
  {
    return 'cmdrw';
  }
}
?>
