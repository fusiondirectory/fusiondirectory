<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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
 * \file class_acl.inc
 * Source code for Class ACL
 */

/*!
 * \brief This class contains all the function needed to manage acl
 */
class acl
{
  static function plInfo()
  {
    return array(
      'plShortName'   => _('ACL'),
      'plDescription' => _('Manage access control lists'),
      'plCategory'    => array(
        'acl' => array(
          'description'  => _('ACL').'&nbsp;&amp;&nbsp;'._('ACL roles'),
          'objectClass'  => array('gosaAcl','gosaRole')
        )
      ),
      'plObjectType'  => array(),

      'plProvidedAcls'  => array()
    );
  }

  /*!
   *  \brief Function sort an array by elements priority
   *
   *  \param Array $list Array to be sorted
   */
  static function sort_by_priority($list)
  {
    uksort($list,
      function ($a, $b)
      {
        $infos_a = pluglist::pluginInfos(preg_replace('|^[^/]*/|', '', $a));
        $infos_b = pluglist::pluginInfos(preg_replace('|^[^/]*/|', '', $b));
        $pa = (isset($infos_a['plPriority'])?$infos_a['plPriority']:0);
        $pb = (isset($infos_b['plPriority'])?$infos_b['plPriority']:0);
        if ($pa == $pb) {
          return 0;
        }
        return ($pa < $pb ? -1 : 1);
      }
    );

    return $list;
  }

  /*!
   * \brief Explode a role
   *
   * \param string $acl ACL to be exploded
   */
  static function explodeRole($role)
  {
    if (!is_array($role)) {
      $role = array($role);
    }
    unset($role['count']);
    $result = array();
    foreach ($role as $aclTemplate) {
      $list = explode(':', $aclTemplate, 2);
      $result[$list[0]] = self::extractACL($list[1]);
    }
    ksort($result);
    return $result;
  }

  /*!
   * \brief Explode an acl
   *
   * \param string $acl ACL to be exploded
   */
  static function explodeACL($acl)
  {
    $list = explode(':', $acl);
    if (count($list) == 5) {
      list($index, $type,$role,$members,$filter) = $list;
      $filter = base64_decode($filter);
    } else {
      $filter = "";
      list($index, $type,$role,$members) = $list;
    }

    $a = array( $index => array(
      'type'    => $type,
      'filter'  => $filter,
      'members' => acl::extractMembers($members),
      'acl'     => base64_decode($role),
    ));

    /* Handle unknown types */
    if (!in_array($type, array('subtree', 'base'))) {
      msg_dialog::display(_("Internal error"), sprintf(_("Unkown ACL type '%s'!\nYou might need to run \"fusiondirectory-setup --migrate-acls\" to migrate your acls to the new format."), $type), ERROR_DIALOG);
      $a = array();
    }
    return $a;
  }

  /*!
   * \brief Extract members of an acl
   *
   * \param $acl The acl to be extracted members part
   *
   * \return an array with members
   */
  static function extractMembers($ms)
  {
    global $config;
    $a = array();

    /* Seperate by ',' and place it in an array */
    $ma = explode(',', $ms);

    /* Decode dn's, fill with informations from LDAP */
    $ldap = $config->get_ldap_link();
    foreach ($ma as $memberdn) {
      // Check for wildcard here
      $dn = base64_decode($memberdn);
      if ($dn != "*") {
        $ldap->cat($dn, array('cn', 'objectClass', 'description', 'uid'));

        /* Found entry... */
        if ($ldap->count()) {
          $attrs = $ldap->fetch();
          if (in_array_ics('inetOrgPerson', $attrs['objectClass'])) {
            $a['U:'.$dn] = $attrs['cn'][0].' ['.$attrs['uid'][0].']';
          } elseif (in_array_ics('organizationalRole', $attrs['objectClass'])) {
            $a['R:'.$dn] = $attrs['cn'][0];
            if (isset($attrs['description'][0])) {
              $a['R:'.$dn] .= ' ['.$attrs['description'][0].']';
            }
          } else {
            $a['G:'.$dn] = $attrs['cn'][0];
            if (isset($attrs['description'][0])) {
              $a['G:'.$dn] .= ' ['.$attrs['description'][0].']';
            }
          }
          /* ... or not */
        } else {
          $a['U:'.$dn] = sprintf(_("Unknown entry '%s'!"), $dn);
        }
      } else {
        $a['G:*'] = sprintf(_("All users"));
      }
    }

    return $a;
  }

  /*!
   * \brief Extract an acl
   *
   * \param string $acl The acl to be extracted
   */
  static function extractACL($acl)
  {
    /* Rip acl off the string, seperate by ',' and place it in an array */
    $as = preg_replace('/^[^:]+:[^:]+:[^:]*:([^:]*).*$/', '\1', $acl);
    $aa = explode(',', $as);
    $a  = array();

    /* Dis-assemble single ACLs */
    foreach ($aa as $sacl) {

      /* Dis-assemble field ACLs */
      $ao       = explode('#', $sacl);
      $gobject  = "";
      foreach ($ao as $idx => $ssacl) {

        /* First is department with global acl */
        $object = preg_replace('/^([^;]+);.*$/', '\1', $ssacl);
        $gacl   = preg_replace('/^[^;]+;(.*)$/', '\1', $ssacl);
        if ($idx == 0) {
          /* Create hash for this object */
          $gobject      = $object;
          $a[$gobject]  = array();

          /* Append ACL if set */
          if ($gacl != "") {
            $a[$gobject] = array($gacl);
          }
        } else {
          /* All other entries get appended... */
          list($field, $facl)   = explode(';', $ssacl);
          $a[$gobject][$field]  = $facl;
        }

      }
    }

    return $a;
  }
}
?>
