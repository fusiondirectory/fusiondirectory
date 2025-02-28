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
 * \file class_tests.inc
 * Source code for class tests
 */

/*!
 * \brief This class provides various test functions
 *
 * This class provides various test functions. It enables to check
 * if a given value is:
 *
 * - a phone numnber
 * - a DNS name
 * - an URL
 * etc.
 *
 * The functions need to be handled with care, because they are not as strict
 * as one might expect.
 */
class tests
{
  /*!
   * \brief Test if the given string is a phone number
   *
   * \param string $nr The phone number to check
   */
  public static function is_phone_nr ($nr)
  {
    if ($nr == "") {
      return TRUE;
    }

    return preg_match("/^[\/0-9 ()+*-]+$/", $nr);
  }


  /*!
   * \brief Test if the given string contains characters allowed in a DNS record name
   *
   * \param string $str The DNS to check
   */
  public static function is_dns_name ($str)
  {
    return preg_match("/^[a-z0-9\.\-_]*$/i", $str);
  }


  /*!
   * \brief Test if the given string contains characters allowed in a hostname
   *
   * \param string $str The hostname to check
   */
  public static function is_valid_hostname ($str)
  {
    return preg_match("/^[a-z0-9\.\-]*$/i", $str);
  }


  /*!
   * \brief Test if the given string is an URL
   *
   * \param string $url The URL to check
   */
  public static function is_url ($url)
  {
    if ($url == "") {
      return TRUE;
    }

    /* Using @stephenhay regexp from http://mathiasbynens.be/demo/url-regex (removed ftp through) */
    return preg_match("@^(https?)://[^\s/$.?#].[^\s]*$@i", $url);
  }


  /*!
   * \brief Test if the given string is a DN
   *
   * \param string $dn The DN to check
   */
  public static function is_dn ($dn)
  {
    if ($dn == "") {
      return TRUE;
    }

    return preg_match("/^[a-z0-9 _-]+$/i", $dn);
  }


  /*!
   * \brief Test if the given string is an uid
   *
   * \param string $uid The UID to check
   */
  public static function is_uid ($uid)
  {
    if ($uid == "") {
      return TRUE;
    }

    /* STRICT adds spaces and case insenstivity to the uid check.
       This is dangerous and should not be used. */
    if (strict_uid_mode()) {
      return preg_match("/^[a-z0-9_-]+$/", $uid);
    } else {
      return preg_match("/^[a-z0-9 _.-]+$/i", $uid);
    }
  }

  /*!
   * \brief Test if the given string is an IP (v4 or v6)
   *
   * \param string $ip The IP to check
   */
  public static function is_ip ($ip)
  {
    return filter_var($ip, FILTER_VALIDATE_IP);
  }

  /*!
   * \brief Test if the given string is an IPv4
   *
   * \param string $ip The IPv4 to check
   */
  public static function is_ipv4 ($ip)
  {
    return filter_var($ip, FILTER_VALIDATE_IP, FILTER_FLAG_IPV4);
  }

  /*!
   * \brief Test if the given string is an IPv6
   *
   * \param string $ip The IPv6 to check
   */
  public static function is_ipv6 ($ip)
  {
    return filter_var($ip, FILTER_VALIDATE_IP, FILTER_FLAG_IPV6);
  }


  /*!
   * \brief Test if the given string is a mac address
   *
   * \param string $mac The MAC address to check
   */
  public static function is_mac ($mac)
  {
    return preg_match('/^([a-fA-F0-9]{2}:){5}[a-fA-F0-9]{2}$/', $mac);
  }


  /*!
   * \brief Checks if the given ip address doesn't match
   *  "is_ip" because there is also a sub net mask given
   *
   * \param string $ip The IP to check
   */
  public static function is_ip_with_subnetmask ($ip)
  {
    /* Generate list of valid submasks */
    $res = [];
    for ($e = 0; $e <= 32; $e++) {
      $res[$e] = $e;
    }
    $i[0] = 255;
    $i[1] = 255;
    $i[2] = 255;
    $i[3] = 255;
    for ($a = 3; $a >= 0; $a--) {
      $c = 1;
      while ($i[$a] > 0) {
        $str        = $i[0].".".$i[1].".".$i[2].".".$i[3];
        $res[$str]  = $str;
        $i[$a]      -= $c;
        $c          = 2 * $c;
      }
    }
    $res["0.0.0.0"] = "0.0.0.0";
    if (preg_match("/^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.".
                    "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.".
                    "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.".
                    "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)/", $ip)) {
      $mask = preg_replace("/^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.".
              "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.".
              "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.".
              "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)/", "", $ip);

      $mask = preg_replace("/^\//", "", $mask);
      if ((in_array("$mask", $res)) && preg_match("/^[0-9\.]/", $mask)) {
        return TRUE;
      }
    }
    return FALSE;
  }


  /*!
   * \brief Simple is domain check
   *
   * This checks if the given string looks like "string(...).string"
   *
   * \param string $str The domain to check
   */
  public static function is_domain ($str)
  {
    return preg_match("/^(([a-z0-9\-]{2,63})\.)*[a-z]{2,63}$/i", $str);
  }


  /*!
   * \brief Check if the given argument is an id
   *
   * \param string $id The id to check
   */
  public static function is_id ($id)
  {
    if ($id == "") {
      return FALSE;
    }

    return preg_match("/^[0-9]+$/", $id);
  }


  /*!
   * \brief Check if the given argument is a path
   *
   * \param string $path The path to check
   */
  public static function is_path ($path)
  {
    if ($path == "") {
      return TRUE;
    }
    if (!preg_match('/^[a-z0-9%\/_.+-]+$/i', $path)) {
      return FALSE;
    }

    return preg_match("/\/.+$/", $path);
  }


  /*!
   * \brief Check if the given argument is an email
   *
   * \param string $address The email address
   */
  public static function is_email ($address)
  {
    /* last test is to allow addresses like example@localhost, which are refused by some PHP version */
    return (($address == '')
      || (filter_var($address, FILTER_VALIDATE_EMAIL) !== FALSE)
      || (filter_var($address.'.com', FILTER_VALIDATE_EMAIL) !== FALSE));
  }


  /*
   * \brief Check if the given department name is valid
   *
   * \param string $name The deparment name
   *
   * \param string $base
   */
  public static function is_department_name_reserved ($name)
  {
    global $config;
    $reservedNames = [];
    foreach ($config->data['OBJECTS'] as $infos) {
      if (isset($infos['ou']) && preg_match('/ou=([^,]+),$/', $infos['ou'], $m)) {
        $reservedNames[] = $m[1];
      }
    }

    return in_array_ics($name, array_unique($reservedNames));
  }


  /*
   * \brief Check if $ip1 and $ip2 represents a valid IPv4 range
   *
   * \param string $ip1 The first IPv4
   *
   * \param string $ip2 The second IPv4
   *
   * \return TRUE in case of a valid range, FALSE in case of an error.
   */
  public static function is_ip_range ($ip1, $ip2)
  {
    if (!tests::is_ipv4($ip1) || !tests::is_ipv4($ip2)) {
      return FALSE;
    } else {
      $ar1  = array_map('intval', explode('.', $ip1));
      $var1 = $ar1[0] * (16777216) + $ar1[1] * (65536) + $ar1[2] * (256) + $ar1[3];

      $ar2  = array_map('intval', explode('.', $ip2));
      $var2 = $ar2[0] * (16777216) + $ar2[1] * (65536) + $ar2[2] * (256) + $ar2[3];
      return ($var1 < $var2);
    }
  }


  /*
   * \brief Check if the specified IP address is inside the given network
   *
   * \param string $network Name of the network
   *
   * \param string $netmask The netmask of the IPv4 address
   *
   * \param string $address The IPv4 address
   */
  public static function is_in_network ($network, $netmask, $address)
  {
    $nw = array_map('intval', explode('.', $network));
    $nm = array_map('intval', explode('.', $netmask));
    $ad = array_map('intval', explode('.', $address));

    /* Generate inverted netmask */
    $ni = [];
    $la = [];
    for ($i = 0; $i < 4; $i++) {
      $ni[$i] = 255 - $nm[$i];
      $la[$i] = $nw[$i] | $ni[$i];
    }

    /* Transform to integer */
    $first  = $nw[0] * (16777216) + $nw[1] * (65536) + $nw[2] * (256) + $nw[3];
    $curr   = $ad[0] * (16777216) + $ad[1] * (65536) + $ad[2] * (256) + $ad[3];
    $last   = $la[0] * (16777216) + $la[1] * (65536) + $la[2] * (256) + $la[3];

    return ($first < $curr && $last > $curr);
  }

  /* \brief Check if the specified IPv4 address $address is inside the given network */
  public static function is_in_ip_range ($from, $to, $address)
  {
    $from = array_map('intval', explode('.', $from));
    $to   = array_map('intval', explode('.', $to));
    $ad   = array_map('intval', explode('.', $address));

    /* Transform to integer */
    $from = $from[0] * (16777216) + $from[1] * (65536) + $from[2] * (256) + $from[3];
    $to   = $to[0] * (16777216) + $to[1] * (65536) + $to[2] * (256) + $to[3];
    $ad   = $ad[0] * (16777216) + $ad[1] * (65536) + $ad[2] * (256) + $ad[3];

    return ($ad >= $from && $ad <= $to);
  }

  /* \brief Check if the value is a valid orcid id */
  public static function is_orcid ($orcid)
  {
    /* Remove hyphens, remove last digit, convert to array */
    $baseDigits = str_split(str_replace('-', '', substr($orcid, 0, -1)));
    $sum = 0;
    foreach ($baseDigits as $baseDigit) {
      $sum = ($sum + (int)$baseDigit) * 2;
    }
    $remainder  = $sum % 11;
    $result     = (12 - $remainder) % 11;
    $orcidCheckSum = (($result == 10) ? "X" : (string)$result);

    return ($orcidCheckSum != substr($orcid, -1));
  }
}
