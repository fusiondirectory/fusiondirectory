<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013  FusionDirectory

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

class NonExistingObjectTypeException extends Exception {};

class objects
{
  /*!
   * \brief Get list of object of objectType $type in $ou
   *
   * \param string  $type the objectType to list
   * \param mixed   $attrs The attributes to fetch.
   * If this is a single value, the resulting associative array will have for each dn the value of this attribute.
   * If this is an array, the keys must be the wanted attributes, and the values can be either 1, '*' or 'raw'
   *  depending if you want a single value or an array of values. 'raw' means untouched LDAP value and is only useful for dns.
   *  Other values are considered to be 1.
   * \param string  $ou the LDAP branch to search in, base will be used if it is NULL
   * \param string  $filter an additional filter to use in the LDAP search. Should be surrounded by parenthesis.
   *
   * \return The list of objects as an associative array (keys are dns)
   */
  static function ls ($type, $attrs = array(), $ou = NULL, $filter = '')
  {
    global $config;

    if ($ou === NULL) {
      $ou = $config->current['BASE'];
    }
    $type = strtoupper($type);
    if (!isset($config->data['OBJECTS'][$type])) {
      throw new NonExistingObjectTypeException('Non-existing type '.$type);
    }
    $object_ou = $config->data['OBJECTS'][$type]['ou'];
    if (!empty($object_ou)) {
      if (!preg_match("/^$object_ou/", $ou)) {
        $ou = $object_ou.$ou;
      }
    }
    if (empty($filter)) {
      $filter = $config->data['OBJECTS'][$type]['filter'];
    } else {
      if (!preg_match('/^\(.*\)$/', $filter)) {
        $filter = '('.$filter.')';
      }
      $filter = '(&'.$filter.$config->data['OBJECTS'][$type]['filter'].')';
    }
    $ldap = $config->get_ldap_link();
    $ldap->cd($ou);
    if (is_array($attrs)) {
      $search_attrs = array_keys($attrs);
    } else {
      $search_attrs = array($attrs);
    }
    $ldap->search($filter, $search_attrs);
    if (!$ldap->success()) {
      throw new Exception($ldap->get_error());
    }
    $result = array();
    while ($fetched_attrs = $ldap->fetch()) {
      $key = $fetched_attrs['dn'];
      if (is_array($attrs)) {
        $result[$key] = array();
        foreach ($attrs as $attr => $mode) {
          if (isset($fetched_attrs[$attr])) {
            switch ($mode) {
              case '*':
                unset($fetched_attrs[$attr]['count']);
              case 'raw':
                $result[$key][$attr] = $fetched_attrs[$attr];
                break;
              case 1:
              default:
                $result[$key][$attr] = $fetched_attrs[$attr][0];
            }
          }
        }
      } elseif (isset($fetched_attrs[$attrs])) {
        $result[$key] = $fetched_attrs[$attrs][0];
      }
    }
    return $result;
  }

  /*!
   * \brief Create the tab object for the given dn
   *
   * \param string  $type the objectType to open
   * \param string  $dn   the dn to open
   *
   * \return The created tab object
   */
  static function open ($type, $dn)
  {
    $infos  = self::infos($type);
    $tabClass = $infos['tabClass'];
    if ($tabClass === NULL) {
      $tabClass = 'simpleTabs';
    }

    $tabObject = new $tabClass($config, $config->data['TABS'][$infos['tabGroup']], $dn, $infos['aclCategory']);
    $tabObject->set_acl_base($config->current['BASE']);
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, "Openned as $type object");

    return $tabObject;
  }

  static function create ($type)
  {
    return self::open($type, 'new');
  }

  static function infos ($type)
  {
    global $config;

    if (!isset($config->data['OBJECTS'][strtoupper($type)])) {
      throw new NonExistingObjectTypeException('Non-existing type '.$type);
    }

    return $config->data['OBJECTS'][strtoupper($type)];
  }

  static function isOfType ($attrs, $type)
  {
    //TODO : cache ldapFilter objects?
    $infos  = self::infos($type);
    $filter = ldapFilter::parse($infos['filter']);
    return $filter($attrs);
  }
}

?>
