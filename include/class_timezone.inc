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
 * \file class_timezone.inc
 * Source code for class timezone
 */

/*!
 * \brief This class contains all the function needed to manage the
 * timezones
 */
class timezone
{
  /*
   * \brief This function sets the default timezone according to fusiondirectory configuration.
   *
   * \return TRUE upon success, FALSE otherwise
   */
  static public function setDefaultTimezoneFromConfig()
  {
    global $config;

    /* Is there a timezone set in the fusiondirectory configuration */
    if ($config->get_cfg_value('timezone') != '') {
      $tz = $config->get_cfg_value('timezone');

      if (@date_default_timezone_set($tz)) {
        return TRUE;
      } else {
        msg_dialog::display(
          _('Configuration error'),
          sprintf(_('The timezone setting "%s" in your configuration is not valid.'), $tz),
          ERROR_DIALOG
        );
      }
    }
    return FALSE;
  }

  /*
   * \brief This function returns the offset for the default timezone.
   *
   * deprecated
   * \param $stamp is used to detect summer or winter time.
   */
  static public function get_default_timezone($stamp = NULL)
  {
    global $config;

    /* Use current timestamp if $stamp is not set */
    if ($stamp === NULL) {
      $stamp = time();
    }

    /* Is there a correct timezone set in the fusiondirectory configuration */
    if (static::setDefaultTimezoneFromConfig()) {
      $tz       = $config->get_cfg_value('timezone');
      $tz_delta = date('Z', $stamp);
      $tz_delta = $tz_delta / 3600;
      return array('name' => $tz, 'value' => $tz_delta);
    } else {
      return array('name' => 'unconfigured', 'value' => 0);
    }
  }

  /*
   * \brief Get the time zone informations
   *
   * \return Time zone informations
   */
  static public function _get_tz_zones()
  {
    return DateTimeZone::listIdentifiers();
  }

  /* \brief Return default timezone as a DateTimeZone object */
  static public function getDefaultTimeZone()
  {
    return new DateTimeZone(date_default_timezone_get());
  }

  /* \brief Return UTC timezone as a DateTimeZone object */
  static public function utc()
  {
    static $utc;
    if (!isset($utc)) {
      $utc = new DateTimeZone('UTC');
    }
    return $utc;
  }
}
?>
