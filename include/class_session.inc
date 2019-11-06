<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

/*!
 * \file class_session.inc
 * Source code for class session
 */

/*!
 * \brief This class contains all the function needed to manage sessions
 */
class session
{
  /*!
   * \brief Check if the name of the session is set
   *
   * \param string $name The name of the session
   */
  public static function is_set ($name)
  {
    return isset($_SESSION[$name]);
  }

  /*!
   * \brief Deprecated
   */
  public static function global_is_set ($name)
  {
    return static::is_set($name);
  }

  /*!
   * \brief Set a value in a session
   *
   * \param string $name Name of the session
   *
   * \param $value The new value
   */
  public static function set ($name, $value)
  {
    $_SESSION[$name] = $value;
  }

  /*!
   * \brief Deprecated
   */
  public static function global_set ($name, $value)
  {
    static::set($name, $value);
  }

  /*!
   * \brief Accessor of a session var
   *
   * \param string $name Name of the session var
   */
  public static function get ($name)
  {
    if (isset($_SESSION[$name])) {
      return $_SESSION[$name];
    } else {
      return NULL;
    }
  }

  /*!
   * \brief Deprecated
   */
  public static function global_get ($name)
  {
    return static::get($name);
  }

  /*!
   * \brief Accessor of a session var by reference
   *
   * \param string $name Name of the session var
   */
  public static function &get_ref ($name)
  {
    return $_SESSION[$name];
  }

  /*!
   * \brief Deprecated
   */
  public static function delete ($name)
  {
    return static::un_set($name);
  }

  /*!
   * \brief Deprecated
   */
  public static function global_delete ($name)
  {
    return static::un_set($name);
  }

  /*!
   * \brief Unset a session
   *
   * \param string $name Name of the session to delete
   */
  public static function un_set ($name)
  {
    if (isset($_SESSION[$name])) {
      unset($_SESSION[$name]);
    }
  }

  /*!
   * \brief Deprecated
   */
  public static function global_un_set ($name)
  {
    return static::un_set($name);
  }

  /*!
   * \brief Start a session
   */
  public static function start ($id = NULL)
  {
    session_name("FusionDirectory");
    /* Set cookie lifetime to one day (The parameter is in seconds ) */
    session_set_cookie_params(24 * 60 * 60);

    /* Set cache limiter to one day (parameter is minute !!) - default is 180 */
    session_cache_expire(60 * 24);

    /* Set session max lifetime, to prevent the garbage collector to delete session before timeout.
       !! The garbage collector is a cron job on debian systems, the cronjob will fetch the timeout from
       the php.ini, so if you use debian, you must hardcode session.gc_maxlifetime in your php.ini */
    ini_set("session.gc_maxlifetime", 24 * 60 * 60);
    if ($id !== NULL) {
      session_id($id);
    }
    session_start();

    /* Check for changed browsers and bail out */
    if (isset($_SESSION['HTTP_USER_AGENT'])) {
      if ($_SESSION['HTTP_USER_AGENT'] != md5($_SERVER['HTTP_USER_AGENT'])) {
        session_destroy();
        session_name("FusionDirectory");
        session_start();
      }
    } else {
      $_SESSION['HTTP_USER_AGENT'] = md5($_SERVER['HTTP_USER_AGENT']);
    }

    /* Regenerate ID to increase security */
    if (!isset($_SESSION['started'])) {
      session_regenerate_id();
      $_SESSION['started'] = TRUE;
    }
  }

  /*!
   * \brief Destroy a session
   */
  public static function destroy (string $reason = '')
  {
    global $ui;

    if (!isset($ui)) {
      $ui = static::get('ui');
    }

    try {
      if (isset($ui)) {
        logging::log(
          'security',
          'logout',
          $ui->dn,
          [],
          sprintf(
            '%s (%s) logged out (%s)',
            $ui->uid,
            ($_SERVER['REMOTE_ADDR'] ?? 'Unknown IP'),
            $reason
          )
        );
      } elseif (!empty($reason)) {
        logging::log(
          'security',
          'session',
          ($_SERVER['REMOTE_ADDR'] ?? ''),
          [],
          sprintf(
            'Session for %s destroyed (%s)',
            ($_SERVER['REMOTE_ADDR'] ?? 'unknown'),
            $reason
          )
        );
      }
    } catch (Exception $e) {
      /* Ignore exceptions here */
    }
    @session_destroy();
  }
}
