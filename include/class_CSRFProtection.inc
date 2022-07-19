<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018  FusionDirectory

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

class CSRFProtection
{
  public static function check ()
  {
    if (empty($_POST)) {
      return;
    }
    if (empty($_POST['CSRFtoken'])) {
      throw new FusionDirectoryException('CSRF protection token missing');
    }

    static::checkHeaders();

    if ($_POST['CSRFtoken'] !== static::getToken()) {
      throw new FusionDirectoryException('CSRF protection token invalid');
    }
  }

  public static function getToken ()
  {
    if (!session::is_set('CSRFtoken')) {
      session::set('CSRFtoken', standAlonePage::generateRandomHash());
    }
    return session::get('CSRFtoken');
  }

  public static function checkHeaders ()
  {
    $origin = FALSE;
    if (!empty($_SERVER['HTTP_ORIGIN'])) {
      $origin = $_SERVER['HTTP_ORIGIN'];
    } elseif (!empty($_SERVER['HTTP_REFERER'])) {
      $origin = $_SERVER['HTTP_REFERER'];
    }
    if ($origin) {
      $origin = preg_replace('|^[^/]+://([^/]+)(/.*)?$|', '\1', $origin);
      $target = FALSE;
      if (!empty($_SERVER['HTTP_X_FORWARDED_HOST'])) {
        /* Only take the first value, there may be several separated by commas */
        list($target) = explode(',', $_SERVER['HTTP_X_FORWARDED_HOST'], 2);
      } else
      if (!empty($_SERVER['HTTP_HOST'])) {
        $target = $_SERVER['HTTP_HOST'];
      }
      if ($target && !hash_equals($origin, $target)) {
        throw new FusionDirectoryException('CSRF detected: origin and target are not matching ('.$origin.' != '.$target.')');
      }
    }
  }
}
