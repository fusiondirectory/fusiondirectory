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


class Step_Config2 extends setup_step
{
  var $governmentmode = FALSE;
  var $sambaidmapping = FALSE;
  var $header_image   = "images/setup/server.png";

  var $generic_settings  = array( "snapshot_active"     => FALSE,
                                  "snapshot_base"       => "ou=snapshots,%base%",
                                  "snapshot_user"       => "%admin%",
                                  "snapshot_password"   => "",
                                  "snapshot_server"     => "%connection%");

  var $timezone       = "Europe/Paris";
  var $timezones      = array();

  var $attributes = array("timezone");
  var $called = FALSE;

  function Step_Config2()
  {
    $this->update_strings();

    $this->timezones = timezone::_get_tz_zones();
  }


  function update_strings()
  {
    $this->s_title      = _("FusionDirectory settings 2/3");
    $this->s_title_long = _("FusionDirectory settings 2/3");
    $this->s_info       = _("Customize special parameters");
  }


  function execute()
  {
    global $BASE_DIR;

    /* Update snapshot values, with already collected values */
    foreach ($this->generic_settings as $key => $value) {
      foreach (array("snapshot_base","snapshot_user","snapshot_server") as $attr) {
        foreach ($this->parent->captured_values as $replace_name => $replace_value) {
          if (is_string($replace_value))  {
            $this->generic_settings[$attr] = preg_replace("/%".$replace_name."%/", $replace_value, $this->generic_settings[$attr]);
          }
        }
      }
    }

    $smarty = get_smarty();
    $smarty->assign("generic_settings",  reverse_html_entities($this->generic_settings));

    $smarty->assign("bool", array(FALSE => _("No"), TRUE => _("Yes")));

    foreach ($this->attributes as $attr) {
      $smarty->assign($attr, reverse_html_entities($this->$attr));
    }
    $smarty->assign("timezone",   $this->timezone);
    $smarty->assign("timezones",  $this->timezones);
    return $smarty->fetch("$BASE_DIR/setup/setup_config2.tpl");
  }

  function save_object()
  {
    if (isset($_POST['step6_posted'])) {

      if (isset($_POST['timezone']) && isset($this->timezones[$_POST['timezone']])) {
        $this->timezone = $_POST['timezone'];
      }

      /* Get attributes */
      foreach ($this->attributes as $attr) {
        if (isset($_POST[$attr])) {
          $this->$attr = validate(get_post($attr));
        }
      }

      if (isset($_POST['snapshot_active'])) {
        $this->generic_settings['snapshot_active'] = TRUE;
        if (isset($_POST['snapshot_base'])) {
          $this->generic_settings['snapshot_base'] = get_post('snapshot_base');
        }
        if (isset($_POST['snapshot_user'])) {
          $this->generic_settings['snapshot_user'] = get_post('snapshot_user');
        }
        if (isset($_POST['snapshot_password'])) {
          $this->generic_settings['snapshot_password'] = get_post('snapshot_password');
        }
        if (isset($_POST['snapshot_server'])) {
          $this->generic_settings['snapshot_server'] = get_post('snapshot_server');
        }
      } else {
        $this->generic_settings['snapshot_active'] = FALSE;
      }
    }

    $tmp = $this->check();
    if (count($tmp) == 0) {
      $this->is_completed = TRUE;
    } else {
      $this->is_completed = FALSE;
    }
  }


  function check()
  {
    return array();
  }


  /* Attributes that are interesting for configuration generation */
  function get_attributes()
  {
    $tmp = setup_step::get_attributes();
    foreach (array("generic_settings") as $attr) {
      $tmp[$attr] = $this->$attr;
    }
    return $tmp;
  }
}

?>
