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
 * \file class_filterLDAP.inc
 * Source code for class filterLDAP
 */

 /*!
  * \brief This class contains all the function to manage LDAP filter
  *
  * \see filter
  */
class filterLDAP
{
  /*!
   * \brief Query
   *
   * \param string $parent
   *
   * \param string $base
   *
   * \param string $scope
   *
   * \param string $filter
   *
   * \param array $attributes
   *
   * \param string $category
   *
   * \param array $objectStorage
   */
  static function query($parent, $base, $scope, $filter, $attributes, $category, $objectStorage = array(""))
  {
    return static::get_list($parent, $base, $filter, $attributes, $category, $objectStorage, (($scope == 'sub') ? 'subtree' : 'one'), TRUE);
  }

  /*!
   * \brief Get list
   *
   * \param string $parent
   *
   * \param string $base
   *
   * \param string $filter
   *
   * \param array $attributes
   *
   * \param string $category
   *
   * \param array $objectStorage
   *
   * \param string $scope subtree or one
   *
   * \param boolean $sizelimit
   *
   * \param boolean $checkAcl
   */
  static function get_list($parent, $base, $filter, $attributes, $category, $objectStorage, $scope, $sizelimit = TRUE, $skipAcl = FALSE)
  {
    global $config, $ui;

    // Move to arrays for category and objectStorage
    if (!is_array($category)) {
      $category = array($category);
    }

    // Initialize search bases
    $bases = array();

    foreach ($objectStorage as $oc) {
      $oc = preg_replace('/,$/', '', $oc);
      if ($scope == 'one') {
        if (empty($oc)) {
          $bases[$base] = TRUE;
        } else {
          $bases[$oc.','.$base] = TRUE;
        }
      } else {
        if (empty($oc)) {
          /* If we search on the base as well because of an empty storage string,
           * it makes no sense to do dn filters */
          $bases[$base] = TRUE;
          break;
        }
        $parts    = ldap_explode_dn($oc, 0);
        unset($parts['count']);
        $dnFilter = array();
        foreach ($parts as $part) {
          preg_match('/([^=]+)=(.*)$/', $part, $m);
          $dnFilter[] = '('.$m[1].':dn:='.$m[2].')';
        }
        if (count($dnFilter) > 1) {
          $bases[$base][] = '(&'.implode('', $dnFilter).')';
        } else {
          $bases[$base][] = $dnFilter[0];
        }
      }
    }

    // Get LDAP link
    $ldap = $config->get_ldap_link($sizelimit);

    // Do search for every base
    $result = array();
    $limit_exceeded = FALSE;

    foreach ($bases as $base => $dnFilters) {
      // Break if the size limit is exceeded
      if ($limit_exceeded) {
        return $result;
      }

      // Switch to new base and search
      if (is_array($dnFilters)) {
        $dnFilter = '(|'.implode('', $dnFilters).')';
      } else {
        $dnFilter = '';
      }
      $ldap->cd($base);

      $ldap->search('(&'.$filter.$dnFilter.')', $attributes, $scope);

      // Check for size limit exceeded messages for GUI feedback
      if (preg_match("/size limit/i", $ldap->get_error())) {
        session::set('limit_exceeded', TRUE);
        $limit_exceeded = TRUE;
      }

      /* Crawl through result entries and perform the migration to the
         result array */
      while ($attrs = $ldap->fetch()) {
        $dn = $attrs['dn'];

        /* Skip ACL checks if we are forced to skip those checks */
        if ($skipAcl) {
          $result[] = $attrs;
        } else {
          // Check entry permission
          $obj = $parent->headpage->getObjectTypeInfos($dn, $attrs);
          if (isset($obj['category'])) {
            $o = $obj['category'].'/'.$obj['class'];
            if (strpos($ui->get_permissions($dn, $o), 'r') !== FALSE) {
              $result[] = $attrs;
            }
          } else {
            @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, "Invalid objectType given!");
          }
        }
      }
    }

    return $result;
  }
}
?>
