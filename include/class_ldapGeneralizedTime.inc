<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016  FusionDirectory

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

/*! \file class_ldapGeneralizedTime.inc
    \brief LdapGeneralizedTime allows you to convert from and to LDAP GeneralizedTime format PHP DateTime objects

    ABNF syntax was taken from RFC 4517 - https://tools.ietf.org/html/rfc4517#page-13

    Examples:
    '199412161032Z'
    '199412160532-0500'
    '1994121610Z'
    '19941216101212Z'
    '19941216101260Z' -> becomes 19941216101300Z
    '19941216101255,5Z'
*/

/*! \class LdapGeneralizedTimeBadFormatException
    \brief Exception class which can be thrown by LdapGeneralizedTime if the format does not match
*/
class LdapGeneralizedTimeBadFormatException extends Exception {};

/*! \class LdapGeneralizedTime
    \brief LdapGeneralizedTime allows you to convert from and to LDAP GeneralizedTime format PHP DateTime objects

    This class provides function to convert from LDAP GeneralizedTime to DateTime and the other way.
    Please note that leap seconds will be lost as PHP has no support for it (see https://bugs.php.net/bug.php?id=70335). 01:60 will become 02:00.
    Also, this class does not support fraction of hours or fraction of minutes (fraction of seconds are supported).
*/
class LdapGeneralizedTime
{

  /*!
    \brief Convert from LDAP GeneralizedTime formatted string to DateTime object
    \param string GeneralizedTime formatted string to convert
    \param useException Whether or not to throw a LdapGeneralizedTimeBadFormatException on failure. Defaults to TRUE.
  */
  public static function fromString ($string, $useException = TRUE)
  {
    assert(is_string($string));

    // century = 2(%x30-39) ; "00" to "99"
    // year    = 2(%x30-39) ; "00" to "99"
    $year = '(?P<year>\d{4})';
    // month   =   ( %x30 %x31-39 ) ; "01" (January) to "09"
    //           / ( %x31 %x30-32 ) ; "10" to "12"
    $month = '(?P<month>0[1-9]|1[0-2])';
    // day     =   ( %x30 %x31-39 )    ; "01" to "09"
    //           / ( %x31-32 %x30-39 ) ; "10" to "29"
    //           / ( %x33 %x30-31 )    ; "30" to "31"
    $day = '(?P<day>0[1-9]|[0-2]\d|3[01])';
    // hour    = ( %x30-31 %x30-39 ) / ( %x32 %x30-33 ) ; "00" to "23"
    $hour = '(?P<hour>[0-1]\d|2[0-3])';
    // minute  = %x30-35 %x30-39                        ; "00" to "59"
    $minute = '(?P<minute>[0-5]\d)';
    // second      = ( %x30-35 %x30-39 ) ; "00" to "59"
    // leap-second = ( %x36 %x30 )       ; "60"
    $second = '(?P<second>[0-5]\d|60)';
    // fraction        = ( DOT / COMMA ) 1*(%x30-39)
    $fraction = '([.,](?P<fraction>\d+))';
    // g-time-zone     = %x5A  ; "Z"
    //                   / g-differential
    // g-differential  = ( MINUS / PLUS ) hour [ minute ]
    $timezone = '(?P<timezone>Z|[-+]([0-1]\d|2[0-3])([0-5]\d)?)';

    // GeneralizedTime = century year month day hour
    //                      [ minute [ second / leap-second ] ]
    //                      [ fraction ]
    //                      g-time-zone
    $pattern = '/^'.
      "$year$month$day$hour".
      "($minute$second?)?".
      "$fraction?".
      $timezone.
      '$/';

    if (preg_match($pattern, $string, $m)) {
      if (empty($m['minute'])) {
        $m['minute'] = '00';
      }
      if (empty($m['second'])) {
        $m['second'] = '00';
      }
      if (empty($m['fraction'])) {
        $m['fraction'] = '0';
      }
      $date = new DateTime($m['year'].'-'.$m['month'].'-'.$m['day'].'T'.$m['hour'].':'.$m['minute'].':'.$m['second'].'.'.$m['fraction'].$m['timezone']);
      $date->setTimezone(new DateTimeZone('UTC'));
      return $date;
    } elseif ($useException) {
      throw new LdapGeneralizedTimeBadFormatException("$string does not match LDAP GeneralizedTime format");
    } else {
      return FALSE;
    }
  }

  /*!
    \brief Convert from DateTime object to LDAP GeneralizedTime formatted string
    \param date DateTime object to convert
    \param useException Whether or not to set the date timezone to UTC. Defaults to TRUE.
  */
  public static function toString ($date, $setToUTC = TRUE)
  {
    assert($date instanceof DateTime);
    if ($setToUTC) {
      $date->setTimezone(new DateTimeZone('UTC'));
    }
    $fraction = preg_replace('/0+$/', '', $date->format('u'));
    $string   = $date->format('YmdHis');
    if (empty($fraction)) {
      return preg_replace('/(00){1,2}$/', '', $string).'Z';
    } else {
      return $string.'.'.$fraction.'Z';
    }
  }
}
