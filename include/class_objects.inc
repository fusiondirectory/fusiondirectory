<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016  FusionDirectory

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

class objects
{
  /*!
   * \brief Get list of object of objectTypes from $types in $ou
   *
   * \param array   $types the objectTypes to list
   * \param mixed   $attrs The attributes to fetch.
   * If this is a single value, the resulting associative array will have for each dn the value of this attribute.
   * If this is an array, the keys must be the wanted attributes, and the values can be either 1, '*' or 'raw'
   *  depending if you want a single value or an array of values. 'raw' means untouched LDAP value and is only useful for dns.
   *  Other values are considered to be 1.
   * \param string  $ou the LDAP branch to search in, base will be used if it is NULL
   * \param string  $filter an additional filter to use in the LDAP search.
   *
   * \return The list of objects as an associative array (keys are dns)
   */
  static function ls ($types, $attrs = NULL, $ou = NULL, $filter = '')
  {
    if (!is_array($types)) {
      $types = array($types);
    }

    if ($attrs === NULL) {
      $attrs = array();
      foreach ($types as $type) {
        $infos = static::infos($type);
        $attrs[] = $infos['mainAttr'];
      }
      $attrs = array_unique($attrs);
      if (count($attrs) == 1) {
        $attrs = $attrs[0];
      }
    }

    if (is_array($attrs)) {
      $search_attrs = array_keys($attrs);
    } else {
      $search_attrs = array($attrs);
    }
    try {
      $ldap = static::search($types, $search_attrs, $ou, $filter);
    } catch (NonExistingBranchException $e) {
      return array();
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
   * \brief Get count of objects of objectTypes from $types in $ou
   *
   * \param array   $types the objectTypes to list
   * \param string  $ou the LDAP branch to search in, base will be used if it is NULL
   * \param string  $filter an additional filter to use in the LDAP search.
   *
   * \return The number of objects of type $type in $ou
   */
  static function count ($types, $ou = NULL, $filter = '')
  {
    try {
      $ldap = static::search($types, array('dn'), $ou, $filter);
    } catch (EmptyFilterException $e) {
      return 0;
    } catch (NonExistingBranchException $e) {
      return 0;
    }
    return $ldap->count();
  }

  private static function search ($types, $search_attrs, $ou = NULL, $filter = '')
  {
    global $config;

    if (!is_array($types)) {
      $types = array($types);
    }

    if ($ou === NULL) {
      $ou = $config->current['BASE'];
    }

    $typeFilters = array();
    foreach ($types as $type) {
      $infos = static::infos($type);

      if ($infos['filter'] == '') {
        continue;
      }
      $typeFilters[] = $infos['filter'];
    }
    if (empty($typeFilters)) {
      throw new EmptyFilterException();
    }

    $ldap = $config->get_ldap_link();
    if (!$ldap->dn_exists($ou)) {
      throw new NonExistingBranchException();
    }
    if (empty($filter)) {
      $filter = '(|'.implode($typeFilters).')';
    } else {
      if (!preg_match('/^\(.*\)$/', $filter)) {
        $filter = '('.$filter.')';
      }
      $filter = '(&'.$filter.'(|'.implode($typeFilters).'))';
    }
    $ldap->cd($ou);
    $ldap->search($filter, $search_attrs);
    if (!$ldap->success()) {
      throw new LDAPFailureException($ldap->get_error());
    }
    return $ldap;
  }

  /*!
   * \brief Create the tab object for the given dn
   *
   * \param string  $type the objectType to open
   * \param string  $dn   the dn to open
   *
   * \return The created tab object
   */
  static function open ($dn, $type)
  {

    $infos    = static::infos($type);
    $tabClass = $infos['tabClass'];

    $tabObject = new $tabClass($type, $dn);
    $tabObject->set_acl_base();
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, "Openned as $type object");

    return $tabObject;
  }

  static function link($dn, $type, $subaction = '', $text = NULL, $icon = TRUE)
  {
    global $config;

    $infos = static::infos($type);
    if (!isset($infos['management'])) {
      throw new NoManagementClassException('Asked for link for type "'.$type.'" but it does not have a management class');
    }
    $pInfos = pluglist::pluginInfos($infos['management']);
    $index  = $pInfos['INDEX'];
    $action = 'edit';
    if ($subaction != '') {
      $action .= '_'.$subaction;
    }
    $href   = "main.php?plug=$index&amp;reset=1&amp;act=listing_$action&amp;dn=".urlencode($dn);

    if ($text === NULL) {
      $ldap = $config->get_ldap_link();
      $ldap->cat($dn, array($infos['nameAttr']));
      if ($attrs = $ldap->fetch()) {
        if (isset($attrs[$infos['nameAttr']][0])) {
          $text = $attrs[$infos['nameAttr']][0];
        } else {
          $text = $dn;
        }
      } else {
        throw new NonExistingLdapNodeException('Dn '.$dn.' not found in LDAP');
      }
    } elseif (is_array($text)) {
      $text = $text[$infos['nameAttr']][0];
    }

    if ($icon && isset($infos['icon'])) {
      $text = '<img alt="'.$infos['name'].'" title="'.$dn.'" src="'.htmlentities($infos['icon'], ENT_COMPAT, 'UTF-8').'" class="center"/>&nbsp;'.$text;
    }

    return '<a href="'.$href.'">'.$text.'</a>';
  }

  static function create ($type)
  {
    return static::open('new', $type);
  }

  static function infos ($type)
  {
    global $config;

    if (!isset($config->data['OBJECTS'][strtoupper($type)])) {
      throw new NonExistingObjectTypeException('Non-existing type "'.$type.'"');
    }

    return $config->data['OBJECTS'][strtoupper($type)];
  }

  static function isOfType ($attrs, $type)
  {
    //TODO : cache ldapFilter objects?
    $infos  = static::infos($type);
    $filter = ldapFilter::parse($infos['filter']);
    return $filter($attrs);
  }

  static function types ()
  {
    global $config;
    return array_keys($config->data['OBJECTS']);
  }

  /* !\brief  This method returns a list of all available templates for the given type
   */
  static function getTemplates($type, $requiredPermissions = 'r')
  {
    global $config, $ui;

    $infos = static::infos($type);

    $templates = array();
    $ldap = $config->get_ldap_link();
    foreach ($config->departments as $key => $value) {
      // Search all templates from the current dn.
      $ldap->cd($infos['ou'].$value);
      // We could also adapt object filter to match templates
      $ldap->search('(objectClass=fdTemplate)', array('cn'));
      if ($ldap->count() != 0) {
        while ($attrs = $ldap->fetch()) {
          $dn = $attrs['dn'];
          if ($requiredPermissions != '') {
            if (!preg_match('/'.$requiredPermissions.'/', $ui->get_permissions($dn, $infos['aclCategory'].'/'.'template'))) {
              continue;
            }
          }
          $templates[$dn] = $attrs['cn'][0]." - ".LDAP::fix($key);
        }
      }
    }
    natcasesort($templates);
    reset($templates);
    return $templates;
  }
}

?>
