<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007 Fabian Hickert
  Copyright (C) 2011-2015  FusionDirectory

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

class setup_step extends plugin
{
  var $s_title      = "Still undefined";
  var $s_title_long = "This is a still undefined long title";
  var $s_info       = "This info is still undefined. This setup step detect the type of your ldap for FusionDirectory.This is a basic requirement for fusiondirectory.";
  var $is_active    = FALSE;
  var $is_enabled   = FALSE;
  var $is_completed = FALSE;

  var $header_image = "";

  var $attributes   = array();
  var $parent       = NULL;
  var $dialog       = FALSE;

  function __construct()
  {
  }

  function execute()
  {
  }

  function save_object()
  {
    /* Do not call plugin::save_object. This causes acl trouble  */
  }

  function get_title()
  {
    return $this->s_title;
  }

  function get_long_title()
  {
    return $this->s_title_long;
  }

  function get_small_info()
  {
    return $this->s_info;
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

  /* Return attributes handled by this setup step */
  function get_attributes()
  {
    $tmp = array();
    foreach ($this->attributes as $attr) {
      $tmp[$attr] = $this->$attr;
    }
    return $tmp;
  }

  function print_header()
  {
    $image    = get_template_path($this->header_image);
    $display  = "<div style='padding:3px;'>";
    $display  .= "  <p class=\"center\" style=\"margin:0px 0px 0px 5px;padding:0px;font-size:24px;font-weight:bold;\"><img class=\"center\" src=\"$image\" align=\"middle\" alt=\"*\">&nbsp;".$this->s_title_long."</p>\n";
    $display  .= "</div>\n";

    return $display;
  }

  function get_ldap_link()
  {
    /* Establish ldap connection */
    $cv = $this->parent->captured_values;
    $ldap_l = new LDAP($cv['admin'],
        $cv['password'],
        $cv['connection'],
        FALSE,
        $cv['tls']);

    $ldap = new ldapMultiplexer($ldap_l);
    $ldap->cd($cv['base']);
    return $ldap;
  }
}
?>
