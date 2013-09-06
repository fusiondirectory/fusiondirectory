<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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
class session {

  public static function get_session_size()
  {
  }

  public static function get_element_size()
  {
  }

  /*!
   * \brief Add channel in the session
   *
   * \param string $name Name of the new channel
   */
  public static function add_channel($name)
  {
    /* If there's already such kind of channel, skip... */
    if (isset($_SESSION[$name])) {
      return FALSE;
    }

    /* Allocate it... */
    $_SESSION[$name]    = array();
    $_POST["_channel_"] = $name;
    return TRUE;
  }


  /*
   * \brief Remove a channel from a session
   *
   * \param string $name Name of the channel to remove
   */
  public static function remove_channel($name)
  {
    /* If there's already such kind of channel, skip... */
    if (isset($_SESSION[$name])) {
      unset($_SESSION[$name]);
      if (isset($_POST["_channel_"])) {
        unset($_POST["_channel_"]);
      }
      return TRUE;
    }

    return FALSE;
  }

  /*!
   * \brief Check if the name of the session is set
   *
   * \param string $name The name of the session
   */
  public static function is_set($name)
  {
    $channel = "";
    if (isset($_POST['_channel_'])) {
      $channel = $_POST['_channel_'];
    }

    /* Global fallback if not set */
    if ($channel == "") {
      return isset($_SESSION[$name]);
    }

    /* Sanity check */
    if (!session::channel_exists($channel)) {
      msg_dialog::display(_("Internal error"), _("Requested channel does not exist! Please contact your Administrator."), FATAL_ERROR_DIALOG);
    }

    $channel = "gch_".$channel;
    return isset($_SESSION[$channel][$name]);
  }

  /*!
   * \brief Check if a session is defined
   *
   * \param string $name Name of the session
   */
  public static function global_is_set($name)
  {
    return isset($_SESSION[$name]);
  }

  /*!
   * \brief Set a value in a session
   *
   * \param string $name Name of the session
   *
   * \param $value The new value
   */
  public static function set($name, $value)
  {
    $channel = "";
    if (isset($_POST['_channel_'])) {
      $channel = $_POST['_channel_'];
    }

    /* Global fallback if not set */
    if ($channel == "") {
      $_SESSION[$name] = $value;
    } else {
      /* Sanity check */
      if (!session::channel_exists($channel)) {
        msg_dialog::display(_("Internal error"), _("Requested channel does not exist! Please contact your Administrator."), FATAL_ERROR_DIALOG);
      }
      $_SESSION[$channel][$name] = $value;
    }
  }

  /*!
   * \brief Set a value in a session
   *
   * \param string $name Name of the session
   *
   * \param $value The new value
   */
  public static function global_set($name, $value)
  {
    $_SESSION[$name] = $value;
  }

  /*!
   * \brief Accessor of a session
   *
   * \param string $name Name of the session
   */
  public static function &get($name)
  {
    $channel = "";
    if (isset($_POST['_channel_'])) {
      $channel = $_POST['_channel_'];
    }

    /* Global fallback if not set */
    if ($channel == "") {
      $ret = &$_SESSION[$name];
      return $ret;
    }

    /* Sanity check */
    if (!session::channel_exists($channel)) {
      msg_dialog::display(_("Internal error"), _("Requested channel does not exist! Please contact your Administrator."), FATAL_ERROR_DIALOG);
    }

    $channel  = "gch_".$channel;
    $ret      = &$_SESSION[$channel][$name];
    return $ret;
  }

  /*!
   * \brief Accessor of a session
   *
   * \param string $name Name of the session
   */
  public static function &global_get($name)
  {
    $ret = &$_SESSION[$name];
    return $ret;
  }

  /*!
   * \brief Delete a session
   *
   * \param string $name Name of the session to delete
   */
  public static function delete($name)
  {
    $channel = "";
    if (isset($_POST['_channel_'])) {
      $channel = $_POST['_channel_'];
    }

    /* Global fallback if not set */
    if ($channel == "") {
      if (isset($_SESSION[$name])) {
        unset($_SESSION[$name]);
      }
    } else {
      if (isset($_SESSION[$channel][$name])) {
        unset($_SESSION[$channel][$name]);
      }
    }
  }

  /*!
   * \brief Delete a session
   *
   * \param string $name Name of the session to delete
   */
  public static function global_delete($name)
  {
    if ($_SESSION[$name]) {
      unset($_SESSION[$name]);
    }
  }

  /*!
   * \brief Unset a session
   *
   * \param string $name Name of the session to delete
   */
  public static function un_set($name)
  {
    return session::delete($name);
  }

  /*!
   * \brief Unset a session
   *
   * \param string $name Name of the session to delete
   */
  public static function global_un_set($name)
  {
    return session::global_delete($name);
  }

  /*!
   * \brief Start a session
   */
  public static function start()
  {
    session_name("FusionDirectory");
    /* Set cookie lifetime to one day (The parameter is in seconds ) */
    session_set_cookie_params(24 * 60 * 60);

    /* Set cache limter to one day (parameter is minute !!)*/
    session_cache_expire(60 * 24);  // default is 180

    /* Set session max lifetime, to prevent the garbage collector to delete session before timeout.
       !! The garbage collector is a cron job on debian systems, the cronjob will fetch the timeout from
       the php.ini, so if you use debian, you must hardcode session.gc_maxlifetime in your php.ini */
    ini_set("session.gc_maxlifetime", 24 * 60 * 60);
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
  public static function destroy()
  {
    @session_destroy();
  }

  public static function set_lifetime($seconds = -1)
  {
    echo "Not implemented yet";
  }


  /*!
   * \brief Get all sessions
   */
  public static function &get_all()
  {
    $ret = &$_SESSION;
    return $ret;
  }
}

?>
