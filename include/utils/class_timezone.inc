<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class timezone {

  /* This function returns the offset for the default timezone. 
   * $stamp is used to detect summer or winter time.
   * In case of PHP5, the integrated timezone functions are used.
   */
  static public function get_default_timezone($stamp = NULL)
  {
    global $config;
    $tz ="";

    /* Default return value if zone could not be detected */
    $zone = array("name" => "unconfigured", "value" => 0);

    /* Use current timestamp if $stamp is not set */
    if($stamp === NULL){
      $stamp = time();
    }

    /* Is there a timezone configured in the gosa configuration (gosa.conf) */
    if ($config->get_cfg_value("timezone") != ""){

      /* Get zonename */
      $tz = $config->get_cfg_value("timezone");

      if(!@date_default_timezone_set($tz)){
        msg_dialog::display(_("Configuration error"), sprintf(_("The timezone setting '%s' in your gosa.conf is not valid. Cannot calculate correct timezone offset."), $tz), ERROR_DIALOG);
      }
      $tz_delta = date("Z", $stamp);
      $tz_delta = $tz_delta / 3600 ;
      return(array("name" => $tz, "value" => $tz_delta));

    }
    return($zone);
  }


  /* Return zone informations */
  static public function _get_tz_zones()
  {
    $timezone_identifiers = DateTimeZone::listIdentifiers();
    $timezones = array();
    $zones = DateTimeZone::listAbbreviations();
    foreach($zones as $group){
      foreach($group as $zone)  {
        $timezones[$zone['timezone_id']] = $zone['offset'];
        if($zone['dst']){
          $dst_timezones[$zone['timezone_id']] = 1;
        }
      }
    }
    return(array("TIMEZONES" => @$timezones, "DST_ZONES" => @$dst_timezones));
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
