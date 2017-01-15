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
    $flag   = (($scope == 'sub') ? GL_SUBSEARCH : 0);
    return static::get_list($parent, $base, $filter, $attributes, $category, $objectStorage, $flag | GL_SIZELIMIT);
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
   * \param flag $flags GL_SUBSEARCH
   */
  static function get_list($parent, $base, $filter, $attributes, $category, $objectStorage, $flags)
  {
    global $config, $ui;

    // Move to arrays for category and objectStorage
    if (!is_array($category)) {
      $category = array($category);
    }

    // Store in base - i.e. is a rdn value empty?
    $storeOnBase = in_array('', $objectStorage);

    $default_method = (($storeOnBase && !($flags & GL_SUBSEARCH)) ? 'ls' : 'search');

    // Initialize search bases
    $bases = array();

    // Get list of sub bases to search on
    if ($storeOnBase) {
      $bases[$base] = "";
    } else {
      foreach ($objectStorage as $oc) {
        $oc = preg_replace('/,$/', '', $oc);
        if (!strpos($oc, ',')) {
          preg_match('/([^=]+)=(.*)$/', $oc, $m);
          if ($flags & GL_SUBSEARCH) {
            $bases[$base][] = $m[1].":dn:=".$m[2];
          } else {
            $bases["$oc,$base"][] = $m[1].":dn:=".$m[2];
          }
        } else {
          // No, there's no \, in pre defined RDN values
          preg_match('/^([^,]+),(.*)$/', $oc, $matches);
          preg_match('/([^=]+)=(.*)$/', $matches[1], $m);
          if ($flags & GL_SUBSEARCH) {
            $bases[$base][] = $m[1].":dn:=".$m[2];
          } else {
            $bases[$matches[2].",$base"][] = $m[1].":dn:=".$m[2];
          }
        }
      }
    }

    // Get LDAP link
    $ldap = $config->get_ldap_link($flags & GL_SIZELIMIT);

    // Do search for every base
    $result = array();
    $limit_exceeded = FALSE;

    // Save initial selected base for comparison purposes
    $initial_base = $base;

    foreach ($bases as $base => $dnFilters) {
      $method = $default_method;

      // Break if the size limit is exceeded
      if ($limit_exceeded) {
        return $result;
      }

      // Switch to new base and search
      if (is_array($dnFilters)) {
        $dnFilter = "(|";
        foreach ($dnFilters as $df) {
          $dnFilter .= "($df)";
        }
        $dnFilter .= ")";
      } else {
        $dnFilter = "";
      }
      $ldap->cd($base);

      // In no-subsearch mode overwrite default method if
      // current base equals the initial base
      if ($base == $initial_base && !($flags & GL_SUBSEARCH)) {
        $method = "ls";
      }

      if ($method == "ls") {
        $ldap->ls("(&$filter$dnFilter)", $base, $attributes);
      } else {
        $ldap->search("(&$filter$dnFilter)", $attributes);
      }

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
        if ($flags & GL_NO_ACL_CHECK) {
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
