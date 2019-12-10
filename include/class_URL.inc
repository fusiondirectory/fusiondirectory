<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2018-2019  FusionDirectory

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
 * \brief Class URL
 * Static methods to get/build URLs
 */

class URL
{
  /*! \brief Returns TRUE if SSL was used to contact FD, whether directly or through a proxy
   */
  public static function sslOn (): bool
  {
    if (isset($_SERVER['HTTP_X_FORWARDED_PROTO'])) {
      return (strcasecmp($_SERVER['HTTP_X_FORWARDED_PROTO'], 'https') == 0);
    }
    if (isset($_SERVER['HTTPS'])) {
      return (strcasecmp($_SERVER['HTTPS'], 'on') == 0);
    }
    return FALSE;
  }

  protected static function gatherInfos (): array
  {
    $protocol = 'http';
    if (static::sslOn()) {
      $protocol .= 's';
    }

    $port = '80';
    if (!empty($_SERVER['HTTP_X_FORWARDED_HOST'])) {
      $host = $_SERVER['HTTP_X_FORWARDED_HOST'];
      if (isset($_SERVER['HTTP_X_FORWARDED_PORT'])) {
        $port = $_SERVER['HTTP_X_FORWARDED_PORT'];
      }
      if (isset($_SERVER['HTTP_X_FORWARDED_PROTO'])) {
        $protocol = $_SERVER['HTTP_X_FORWARDED_PROTO'];
      }
    } else {
      if (!empty($_SERVER['HTTP_HOST'])) {
        $host = $_SERVER['HTTP_HOST'];
      } else {
        $host = $_SERVER['SERVER_NAME'];
      }
      $port = $_SERVER['SERVER_PORT'];
    }

    return [$protocol, $host, $port];
  }

  /*! \brief Returns SSL URL to redirect to
   */
  public static function getSslUrl (): string
  {
    list(, $host, $port) = static::gatherInfos();
    $ssl = 'https://'.$host;

    if (!empty($_SERVER['REQUEST_URI'])) {
      $ssl .= $_SERVER['REQUEST_URI'];
    } elseif (!empty($_SERVER['PATH_INFO'])) {
      $ssl .= $_SERVER['PATH_INFO'];
    } else {
      $ssl .= $_SERVER['PHP_SELF'];
    }
    return $ssl;
  }

  /*! \brief Returns current page URL
   */
  public static function getPageURL ($queryString = FALSE): string
  {
    list($protocol, $host, $port) = static::gatherInfos();

    $pageURL = $protocol.'://'.$host;
    if ((($protocol == 'http') && ($port != '80')) || (($protocol == 'https') && ($port != '443'))) {
      $pageURL .= ':'.$port;
    }
    if (!empty($_SERVER['REQUEST_URI']) && $queryString) {
      $pageURL .= $_SERVER['REQUEST_URI'];
    } elseif (!empty($_SERVER['PATH_INFO'])) {
      $pageURL .= $_SERVER['PATH_INFO'];
    } else {
      $pageURL .= $_SERVER['PHP_SELF'];
    }

    return $pageURL;
  }

  /*! \brief Returns hostname to identify this website
   */
  public static function getHostName (): string
  {
    list($protocol, $host, $port) = static::gatherInfos();

    if ((($protocol == 'http') && ($port != '80')) || (($protocol == 'https') && ($port != '443'))) {
      $host .= ':'.$port;
    }

    return $host;
  }

  /*! \brief Returns absolute URL from relative URL
   */
  public static function buildAbsoluteUrl (string $path): string
  {
    return preg_replace('|/[^/]*$|', $path, static::getPageURL(FALSE));
  }
}
