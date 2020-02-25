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

/*!
 * \brief Class for handling objects and their types
 *
 * Allows to list, open, create and delete objects
 */
class objects
{
  /*!
   * \brief Get list of object of objectTypes from $types in $ou
   *
   * \param array   $types the objectTypes to list
   * \param mixed   $attrs The attributes to fetch.
   * If this is a single value, the resulting associative array will have for each dn the value of this attribute.
   * If this is an array, the keys must be the wanted attributes, and the values can be either 1, '*', 'b64' or 'raw'
   *  depending if you want a single value or an array of values. 'raw' means untouched LDAP value and is only useful for dns.
   *  Other values are considered to be 1. 'b64' means an array of base64 encoded values and is mainly useful through webservice for binary attributes.
   * \param string  $ou the LDAP branch to search in, base will be used if it is NULL
   * \param string  $filter an additional filter to use in the LDAP search.
   * \param boolean $checkAcl should ACL be ignored or checked? Defaults to FALSE.
   * \param string  $scope scope, defaults to subtree. When using one, be careful what you put in $ou.
   *
   * \return The list of objects as an associative array (keys are dns)
   */
  static function ls ($types, $attrs = NULL, $ou = NULL, $filter = '', $checkAcl = FALSE, $scope = 'subtree', $templateSearch = FALSE, $sizeLimit = FALSE)
  {
    global $ui, $config;

    if ($ou === NULL) {
      $ou = $config->current['BASE'];
    }

    if (!is_array($types)) {
      $types = [$types];
    }

    if ($checkAcl) {
      if (count($types) > 1) {
        throw new FusionDirectoryException('Cannot evaluate ACL for several types');
      }
      $infos  = static::infos(reset($types));
      $acl    = $infos['aclCategory'].'/'.$infos['mainTab'];
      $tplAcl = $infos['aclCategory'].'/template';
    }

    $attrsAcls = [];
    if ($attrs === NULL) {
      if ($templateSearch) {
        $attrs = 'cn';
      } else {
        $attrs = [];
        foreach ($types as $type) {
          $infos = static::infos($type);
          if ($infos['mainAttr']) {
            $attrs[] = $infos['mainAttr'];
          }
        }
        $attrs = array_unique($attrs);
        if (count($attrs) == 1) {
          $attrs = $attrs[0];
        } elseif (count($attrs) == 0) {
          $attrs = ['dn' => 'raw'];
        }
      }
    } elseif ($checkAcl) {
      if (is_array($attrs)) {
        $search_attrs = array_keys($attrs);
      } else {
        $search_attrs = [$attrs];
      }
      foreach ($search_attrs as $search_attr) {
        $category = $ui->getAttributeCategory($types[0], $search_attr);
        if ($category === FALSE) {
          throw new FusionDirectoryException('Could not find ACL for attribute "'.$search_attr.'" for type "'.$types[0].'"');
        }
        if ($category === TRUE) {
          continue;
        }
        if (strpos($ui->get_permissions($ou, $category, $search_attr), 'r') === FALSE) {
          $attrsAcls[$search_attr] = [$category, $search_attr];
        }
      }
    }

    if (is_array($attrs)) {
      $search_attrs = array_keys($attrs);
    } else {
      $search_attrs = [$attrs];
    }
    if ($templateSearch) {
      $search_attrs[] = 'fdTemplateField';
      $search_attrs[] = 'cn';
    }

    try {
      $ldap = static::search($types, $search_attrs, $ou, $filter, $checkAcl, $scope, $templateSearch, $partialFilterAcls, $sizeLimit);
    } catch (NonExistingBranchException $e) {
      return [];
    }
    $result = [];
    while ($fetched_attrs = $ldap->fetch()) {
      $key = $fetched_attrs['dn'];
      if ($checkAcl) {
        if (strpos($ui->get_permissions($key, $acl), 'r') === FALSE) {
          continue;
        }
        foreach ($partialFilterAcls as $partialFilterAcl) {
          if (strpos($ui->get_permissions($key, $partialFilterAcl[0], $partialFilterAcl[1]), 'r') === FALSE) {
            continue 2;
          }
        }
      }
      if (is_array($attrs)) {
        $result[$key] = [];
        foreach ($attrs as $attr => $mode) {
          if (isset($fetched_attrs[$attr])) {
            if (isset($attrsAcls[$attr]) &&
                (strpos($ui->get_permissions($key, $attrsAcls[$attr][0], $attrsAcls[$attr][1]), 'r') === FALSE)) {
              continue;
            }
            switch ($mode) {
              case '*':
                unset($fetched_attrs[$attr]['count']);
              case 'raw':
                $result[$key][$attr] = $fetched_attrs[$attr];
                break;
              case 'b64':
                unset($fetched_attrs[$attr]['count']);
                $result[$key][$attr] = array_map('base64_encode', $fetched_attrs[$attr]);
                break;
              case 1:
              default:
                $result[$key][$attr] = $fetched_attrs[$attr][0];
            }
          }
        }
        if ($templateSearch) {
          if (
              isset($fetched_attrs['cn']) &&
              (!$checkAcl || (strpos($ui->get_permissions($key, $tplAcl, 'template_cn'), 'r') !== FALSE))
            ) {
            $result[$key]['cn'] = $fetched_attrs['cn'][0];
          }
          $result[$key]['fdTemplateField'] = [];
          foreach ($fetched_attrs['fdTemplateField'] as $templateField) {
            $attr = explode(':', $templateField, 2)[0];
            if (isset($attrs[$attr])) {
              if (isset($attrsAcls[$attr]) &&
                  (strpos($ui->get_permissions($key, $attrsAcls[$attr][0], $attrsAcls[$attr][1]), 'r') === FALSE)) {
                continue;
              }
              $result[$key]['fdTemplateField'][] = $templateField;
            }
          }
          if (empty($result[$key]['fdTemplateField'])) {
            unset($result[$key]['fdTemplateField']);
          }
        }
        if (count($result[$key]) === 0) {
          unset($result[$key]);
        }
      } elseif ($templateSearch) {
        if ($attrs == 'cn') {
          if (
              isset($fetched_attrs['cn']) &&
              (!$checkAcl || (strpos($ui->get_permissions($key, $tplAcl, 'template_cn'), 'r') !== FALSE))
            ) {
            $result[$key] = $fetched_attrs['cn'][0];
          }
        } else {
          if (isset($attrsAcls[$attrs]) &&
              (strpos($ui->get_permissions($key, $attrsAcls[$attrs][0], $attrsAcls[$attrs][1]), 'r') === FALSE)) {
            continue;
          }
          foreach ($fetched_attrs['fdTemplateField'] as $templateField) {
            list($attr, $value) = explode(':', $templateField, 2);
            if ($attrs == $attr) {
              $result[$key] = $value;
              break;
            }
          }
        }
      } elseif (isset($fetched_attrs[$attrs])) {
        if (isset($attrsAcls[$attrs]) &&
            (strpos($ui->get_permissions($key, $attrsAcls[$attrs][0], $attrsAcls[$attrs][1]), 'r') === FALSE)) {
          continue;
        }
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
   * \param boolean $checkAcl Should ACL be checked on the filtered attributes.
   *
   * \return The number of objects of type $type in $ou
   */
  static function count ($types, $ou = NULL, $filter = '', $checkAcl = FALSE, $templateSearch = FALSE)
  {
    try {
      $ldap = static::search($types, ['dn'], $ou, $filter, $checkAcl, 'subtree', $templateSearch, $partialFilterAcls);
      if (!empty($partialFilterAcls)) {
        throw new FusionDirectoryException('Not enough rights to use "'.$partialFilterAcls[0][1].'" in filter');
      }
    } catch (EmptyFilterException $e) {
      return 0;
    } catch (NonExistingBranchException $e) {
      return 0;
    }
    return $ldap->count();
  }

  private static function search ($types, $search_attrs, $ou = NULL, $filter = '', $checkAcl = FALSE, $scope = 'subtree', $templateSearch = FALSE, &$partialFilterAcls = [], $sizeLimit = FALSE)
  {
    global $config, $ui;

    $partialFilterAcls = [];

    if (!is_array($types)) {
      $types = [$types];
    }

    if ($ou === NULL) {
      $ou = $config->current['BASE'];
    }

    $typeFilters = [];
    foreach ($types as $type) {
      $infos = static::infos($type);

      if ($infos['filter'] == '') {
        if ($infos['filterRDN'] == '') {
          continue;
        } else {
          $typeFilters[] = $infos['filterRDN'];
        }
      } elseif ($infos['filterRDN'] == '') {
        $typeFilters[] = $infos['filter'];
      } else {
        $typeFilters[] = '(&'.$infos['filter'].$infos['filterRDN'].')';
      }
    }
    if (empty($typeFilters)) {
      throw new EmptyFilterException();
    }

    $ldap = $config->get_ldap_link($sizeLimit);
    if (!$ldap->dn_exists($ou)) {
      throw new NonExistingBranchException();
    }
    if (empty($filter)) {
      $filter = '(|'.implode($typeFilters).')';
    } else {
      if ($checkAcl) {
        if (count($types) > 1) {
          throw new FusionDirectoryException('Cannot evaluate ACL for several types');
        }
        $filterObject     = ldapFilter::parse($filter);
        $filterAttributes = $filterObject->listUsedAttributes();
        foreach ($filterAttributes as $acl) {
          $category = $ui->getAttributeCategory($types[0], $acl);
          if ($category === FALSE) {
            throw new FusionDirectoryException('Could not find ACL for attribute "'.$acl.'" for type "'.$types[0].'"');
          }
          if ($category === TRUE) {
            continue;
          }
          if (strpos($ui->get_permissions($ou, $category, $acl), 'r') === FALSE) {
            $partialFilterAcls[] = [$category, $acl];
          }
        }
      }
      if (!preg_match('/^\(.*\)$/', $filter)) {
        $filter = '('.$filter.')';
      }
      $filter = '(&'.$filter.'(|'.implode($typeFilters).'))';
    }
    if ($templateSearch) {
      $templateFilterObject = new ldapFilter(
        '&',
        [
          new ldapFilterLeaf('objectClass', '=', 'fdTemplate'),
          fdTemplateFilter(ldapFilter::parse($filter)),
        ]
      );
      $filter = "$templateFilterObject";
    }
    $ldap->cd($ou);
    $ldap->search($filter, $search_attrs, $scope);
    if (!$ldap->success()) {
      if ($sizeLimit && $ldap->hitSizeLimit()) {
        // Check for size limit exceeded messages for GUI feedback
        $ui->getSizeLimitHandler()->setLimitExceeded();
      } else {
        throw new LDAPFailureException($ldap->get_error());
      }
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
    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, "Openned as $type object");

    return $tabObject;
  }

  static function link ($dn, $type, $subaction = '', $text = NULL, $icon = TRUE, $link = TRUE)
  {
    global $config;

    $infos = static::infos($type);
    if ($link) {
      if (!isset($infos['management'])) {
        throw new NoManagementClassException('Asked for link for type "'.$type.'" but it does not have a management class');
      }
      $pInfos = pluglist::pluginInfos($infos['management']);
      $index  = $pInfos['INDEX'];
      $action = 'edit';
      if ($subaction != '') {
        $action .= '_'.$subaction;
      }
      $href = "main.php?plug=$index&amp;reset=1&amp;act=listing_$action&amp;dn=".urlencode($dn);
    }

    if ($text === NULL) {
      $ldap = $config->get_ldap_link();
      $ldap->cat($dn, [$infos['nameAttr']]);
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

    $text = htmlentities($text, ENT_COMPAT, 'UTF-8');

    if ($icon && isset($infos['icon'])) {
      $text = '<img alt="'.$infos['name'].'" title="'.$dn.'" src="'.htmlentities($infos['icon'], ENT_COMPAT, 'UTF-8').'" class="center"/>&nbsp;'.$text;
    }

    if ($link) {
      $text = '<a href="'.$href.'">'.$text.'</a>';
    }

    return $text;
  }

  static function create ($type)
  {
    return static::open('new', $type);
  }

  static function createTemplate ($type)
  {
    $infos    = static::infos($type);
    $tabClass = $infos['tabClass'];

    /* Pass fake attrs object to force template mode */
    $attrsObject  = new stdClass();
    $attrsObject->attrs = [];
    $attrsObject->is_template = TRUE;

    $tabObject = new $tabClass($type, 'new', $attrsObject);
    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $type, 'Create template of type');

    return $tabObject;
  }

  static function &infos ($type)
  {
    global $config;

    if (!isset($config->data['OBJECTS'][strtoupper($type)])) {
      throw new NonExistingObjectTypeException('Non-existing type "'.$type.'"');
    }

    $infos =& $config->data['OBJECTS'][strtoupper($type)];

    if (!isset($infos['filterRDN'])) {
      if (empty($infos['ou'])) {
        $infos['filterRDN'] = '';
      } else {
        $parts = ldap_explode_dn(preg_replace('/,$/', '', $infos['ou']), 0);
        unset($parts['count']);
        $dnFilter = [];
        foreach ($parts as $part) {
          preg_match('/([^=]+)=(.*)$/', $part, $m);
          $dnFilter[] = '('.$m[1].':dn:='.$m[2].')';
        }
        if (count($dnFilter) > 1) {
          $infos['filterRDN'] = '(&'.implode('', $dnFilter).')';
        } else {
          $infos['filterRDN'] = $dnFilter[0];
        }
      }
    }

    return $infos;
  }

  static function isOfType ($attrs, $type)
  {
    $filter = static::getFilterObject($type);
    return $filter($attrs);
  }

  /* This method allows to cache parsed filter in filterObject key in objectTypes */
  static function getFilterObject ($type)
  {
    global $config;

    $infos =& static::infos($type);

    if (!isset($infos['filterObject'])) {
      $infos['filterObject'] = ldapFilter::parse($infos['filter']);
    }
    return $infos['filterObject'];
  }

  /* This method allows to cache searched attributes list in objectTypes */
  static function getSearchedAttributes ($type)
  {
    global $config;

    $infos =& static::infos($type);

    if (!isset($infos['searchAttributes'])) {
      $searchAttrs = [];
      if (!empty($infos['mainAttr'])) {
        $searchAttrs[$infos['mainAttr']] = $infos['aclCategory'].'/'.$infos['mainTab'];
      }
      if (!empty($infos['nameAttr'])) {
        $searchAttrs[$infos['nameAttr']] = $infos['aclCategory'].'/'.$infos['mainTab'];
      }
      foreach ($config->data['TABS'][$infos['tabGroup']] as $tab) {
        if (!plugin_available($tab['CLASS'])) {
          continue;
        }
        $plInfos = pluglist::pluginInfos($tab['CLASS']);
        if (isset($plInfos['plSearchAttrs'])) {
          foreach ($plInfos['plSearchAttrs'] as $attr) {
            $searchAttrs[$attr] = $infos['aclCategory'].'/'.$tab['CLASS'];
          }
        }
      }
      $infos['searchAttributes'] = $searchAttrs;
    }

    return $infos['searchAttributes'];
  }

  static function types ()
  {
    global $config;
    return array_keys($config->data['OBJECTS']);
  }

  /* !\brief  This method returns a list of all available templates for the given type
   */
  static function getTemplates ($type, $requiredPermissions = 'r', $filter = '')
  {
    global $config, $ui;

    $infos = static::infos($type);

    $templates    = [];
    $departments  = $config->getDepartmentList();
    foreach ($departments as $key => $value) {
      // Search all templates from the current dn.
      try {
        $ldap = static::search($type, ['cn'], $infos['ou'].$value, $filter, FALSE, 'subtree', TRUE);
      } catch (NonExistingBranchException $e) {
        continue;
      }
      if ($ldap->count() != 0) {
        while ($attrs = $ldap->fetch()) {
          $dn = $attrs['dn'];
          if (($requiredPermissions != '')
            && !preg_match('/'.$requiredPermissions.'/', $ui->get_permissions($dn, $infos['aclCategory'].'/'.'template'))) {
            continue;
          }
          $templates[$dn] = $attrs['cn'][0].' - '.$key;
        }
      }
    }
    natcasesort($templates);
    reset($templates);
    return $templates;
  }
}
