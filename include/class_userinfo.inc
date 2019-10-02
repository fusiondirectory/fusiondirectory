<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2019  FusionDirectory

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
 * \file class_userinfo.inc
 * Source code for the class userinfo
 */

/* Define shadow states */
define('POSIX_ACCOUNT_EXPIRED',           1);
define('POSIX_WARN_ABOUT_EXPIRATION',     2);
define('POSIX_FORCE_PASSWORD_CHANGE',     4);
define('POSIX_DISALLOW_PASSWORD_CHANGE',  8);

/*!
 * \brief Class userinfo
 * This class contains all informations and functions
 * about user
 */

class userinfo
{
  var $dn;
  var $cn;
  var $uid;
  var $sn           = '';
  var $givenName    = '';
  var $gidNumber    = -1;
  var $language     = "";
  var $subtreeACL   = [];
  var $ACL          = [];
  var $groups       = [];
  var $roles        = [];
  var $result_cache = [];
  var $ignoreACL    = FALSE;

  var $ACLperPath   = [];

  /*! \brief LDAP size limit handler */
  protected $sizeLimitHandler;

  /*! \brief Current management base */
  protected $currentBase;

  /*! \brief Password change should be forced */
  protected $forcePasswordChange = FALSE;

  /* get acl's an put them into the userinfo object
     attr subtreeACL (userdn:components, userdn:component1#sub1#sub2,component2,...) */
  function __construct ($userdn)
  {
    global $config;
    $this->dn         = $userdn;
    $this->ignoreACL  = ($config->get_cfg_value('ignoreAcl') == $this->dn);

    $this->loadLDAPInfo();

    /* Initialize ACL_CACHE */
    $this->reset_acl_cache();

    $this->sizeLimitHandler = new ldapSizeLimit();
  }

  /*! \brief Loads user information from LDAP */
  function loadLDAPInfo ()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cat($this->dn, ['cn', 'sn', 'givenName', 'uid', 'gidNumber', 'preferredLanguage']);
    $attrs = $ldap->fetch();

    $this->uid = $attrs['uid'][0];

    if (isset($attrs['cn'][0])) {
      $this->cn = $attrs['cn'][0];
    } elseif (isset($attrs['givenName'][0]) && isset($attrs['sn'][0])) {
      $this->cn = $attrs['givenName'][0].' '.$attrs['sn'][0];
    } else {
      $this->cn = $attrs['uid'][0];
    }
    if (isset($attrs['gidNumber'][0])) {
      $this->gidNumber = $attrs['gidNumber'][0];
    }
    if (isset($attrs['sn'][0])) {
      $this->sn = $attrs['sn'][0];
    }
    if (isset($attrs['givenName'][0])) {
      $this->givenName = $attrs['givenName'][0];
    }

    /* Assign user language */
    if (isset($attrs['preferredLanguage'][0])) {
      $this->language = $attrs['preferredLanguage'][0];
    }
  }

  /*!
  * \brief Reset acl cache
  */
  public function reset_acl_cache ()
  {
    /* Initialize ACL_CACHE */
    session::set('ACL_CACHE', []);
  }

  /*!
   * \brief Load an acl
   */
  function loadACL ()
  {
    global $config;
    $this->ACL          = [];
    $this->groups       = [];
    $this->roles        = [];
    $this->result_cache = [];
    $this->reset_acl_cache();
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);

    /* Get member groups... */
    $ldap->search('(&(objectClass=groupOfNames)(member='.ldap_escape_f($this->dn).'))', ['dn']);
    while ($attrs = $ldap->fetch()) {
      $this->groups[$attrs['dn']] = $attrs['dn'];
    }

    /* Get member POSIX groups... */
    $ldap->search('(&(objectClass=posixGroup)(memberUid='.ldap_escape_f($this->uid).'))', ['dn']);
    while ($attrs = $ldap->fetch()) {
      $this->groups[$attrs['dn']] = $attrs['dn'];
    }

    /* Get member roles... */
    $ldap->search('(&(objectClass=organizationalRole)(roleOccupant='.ldap_escape_f($this->dn).'))', ['dn']);
    while ($attrs = $ldap->fetch()) {
      $this->roles[$attrs['dn']] = $attrs['dn'];
    }

    /* Crawl through ACLs and move relevant to the tree */
    $ldap->search("(objectClass=gosaACL)", ['dn', 'gosaAclEntry']);
    $aclp = [];
    $aclc = [];
    while ($attrs = $ldap->fetch()) {

      /* Insert links in ACL array */
      $aclp[$attrs['dn']] = substr_count($attrs['dn'], ',');
      $ol = [];
      for ($i = 0; $i < $attrs['gosaAclEntry']['count']; $i++) {
        $ol = array_merge($ol, acl::explodeAcl($attrs['gosaAclEntry'][$i]));
      }
      $aclc[$attrs['dn']] = $ol;
    }


    /* Resolve roles here */
    foreach ($aclc as $dn => $data) {
      foreach ($data as $prio => $aclc_value) {
        unset($aclc[$dn][$prio]);

        $ldap->cat($aclc_value['acl'], ["gosaAclTemplate"]);
        $attrs = $ldap->fetch();

        if (isset($attrs['gosaAclTemplate'])) {
          $roleAcls = acl::explodeRole($attrs['gosaAclTemplate']);
          foreach ($roleAcls as $roleAcl) {
            $aclc[$dn][]  = [
              'acl'     => $roleAcl,
              'type'    => $aclc_value['type'],
              'members' => $aclc_value['members'],
              'filter'  => $aclc_value['filter']
            ];
          }
        }
      }
    }

    /* ACL's read, sort for tree depth */
    asort($aclp);

    /* Sort in tree order */
    foreach ($aclp as $dn => $acl) {
      /* Check if we need to keep this ACL */
      foreach ($aclc[$dn] as $idx => $type) {
        $interesting = FALSE;

        /* No members? This ACL rule is deactivated ... */
        if (count($type['members'])) {
          /* Inspect members... */
          foreach (array_keys($type['members']) as $grp) {
            /* Some group inside the members that is relevant for us? */
            if (in_array_ics(preg_replace('/^G:/', '', $grp), $this->groups)) {
              $interesting = TRUE;
              break;
            }

            /* Some role inside the members that is relevant for us? */
            if (in_array_ics(preg_replace('/^R:/', '', $grp), $this->roles)) {
              $interesting = TRUE;
              break;
            }

            /* User inside the members? */
            if (mb_strtoupper(preg_replace('/^U:/', '', $grp)) == mb_strtoupper($this->dn)) {
              $interesting = TRUE;
              break;
            }

            /* Wildcard? */
            if (preg_match('/^G:\*/',  $grp)) {
              $interesting = TRUE;
              break;
            }
          }
        }

        if ($interesting) {
          if (!isset($this->ACL[$dn])) {
            $this->ACL[$dn] = [];
          }
          $this->ACL[$dn][$idx] = $type;
        }
      }
    }

    /* Create an array which represent all relevant permissions settings
        per dn.

      The array will look like this:

      .     ['ou=base']        ['ou=base']          = array(ACLs);
      .
      .     ['ou=dep1,ou=base']['ou=dep1,ou=base']  = array(ACLs);
      .                        ['ou=base']          = array(ACLs);


      For objects located in 'ou=dep1,ou=base' we have to apply both ACLs,
       for objects in 'ou=base' we only have to apply one ACL.
     */
    $without_self_acl = $all_acl = [];
    foreach ($this->ACL as $dn => $acl) {
      $sdn = $dn;
      do {
        if (isset($this->ACL[$dn])) {
          $all_acl[$sdn][$dn]           = $this->ACL[$dn];
          $without_self_acl[$sdn][$dn]  = $this->ACL[$dn];
          foreach ($without_self_acl[$sdn][$dn] as $acl_id => $acl_set) {
            /* Remove all acl entries which are especially for the current user (self acl) */
            foreach ($acl_set['acl'] as $object => $object_acls) {
              if (isset($object_acls[0]) && (strpos($object_acls[0], 's') !== FALSE)) {
                unset($without_self_acl[$sdn][$dn][$acl_id]['acl'][$object]);
                if (empty($without_self_acl[$sdn][$dn][$acl_id]['acl'])) {
                  unset($without_self_acl[$sdn][$dn][$acl_id]);
                }
              }
            }
          }
        }
        $dn = preg_replace("/^[^,]*+,/", "", $dn);
      } while (strpos($dn, ',') !== FALSE);
    }
    $this->ACLperPath = $without_self_acl;

    /* Append Self entry */
    $dn = $this->dn;
    while (strpos($dn, ",") && !isset($all_acl[$dn])) {
      $dn = preg_replace("/^[^,]*+,/", "", $dn);
    }
    if (isset($all_acl[$dn])) {
      $this->ACLperPath[$this->dn] = $all_acl[$dn];
    }
  }

  /*!
   * \brief Returns an array containing all target objects we've permissions on
   *
   * \return Return the next id or NULL if failed
   */
  function get_acl_target_objects ()
  {
    return array_keys($this->ACLperPath);
  }

  /*!
   * \brief Get permissions by category
   *
   * \param string $dn Dn from which we want to know permissions.
   *
   * \param string $category Category for which we want the acl eg: server
   *
   * \return all the permissions for the dn and category
   */
  function get_category_permissions ($dn, $category)
  {
    return @$this->get_permissions($dn, $category.'/0', '');
  }


  /*!
   * \brief Check if the given object (dn) is copyable
   *
   * \param string $dn     The object dn
   *
   * \param string $object The acl  category (e.g. user)
   *
   * \return boolean TRUE if the given object is copyable else FALSE
  */
  function is_copyable ($dn, $object): bool
  {
    return (strpos($this->get_complete_category_acls($dn, $object), 'r') !== FALSE);
  }


  /*!
   * \brief Check if the given object (dn) is cutable
   *
   * \param string $dn     The object dn
   *
   * \param string $object The acl  category (e.g. user)
   *
   * \param string $class  The acl  class (e.g. user)
   *
   * \return boolean TRUE if the given object is cutable else FALSE
   */
  function is_cutable ($dn, $object, $class): bool
  {
    $remove = (strpos($this->get_permissions($dn, $object.'/'.$class), 'd') !== FALSE);
    $read   = (strpos($this->get_complete_category_acls($dn, $object), 'r') !== FALSE);
    return ($remove && $read);
  }


  /*!
   * \brief Checks if we are allowed to paste an object to the given destination ($dn)
   *
   * \param string $dn The destination dn
   *
   * \param string $object The acl  category (e.g. user)
   *
   * \return Boolean TRUE if we are allowed to paste an object.
   */
  function is_pasteable ($dn, $object): bool
  {
    return (strpos($this->get_complete_category_acls($dn, $object), 'w') !== FALSE);
  }


  /*!
   * \brief Checks if we are allowed to restore a snapshot for the given dn.
   *
   * \param string $dn     The destination dn
   *
   * \param string $categories The acl  category (e.g. user)
   *
   * \param boolean $deleted Is it a deleted or existing object
   *
   * \return boolean TRUE if we are allowed to restore a snapshot.
   */
  function allow_snapshot_restore ($dn, $categories, $deleted): bool
  {
    $permissions = $this->get_snapshot_permissions($dn, $categories);
    return in_array(($deleted ? 'restore_deleted' : 'restore_over'), $permissions);
  }


  /*!
   * \brief Checks if we are allowed to create a snapshot of the given dn.
   *
   * \param string $dn     The source dn
   *
   * \param string $categories The acl category (e.g. user)
   *
   * \return boolean TRUE if we are allowed to create a snapshot.
   */
  function allow_snapshot_create ($dn, $categories): bool
  {
    $permissions = $this->get_snapshot_permissions($dn, $categories);
    return in_array('c', $permissions);
  }


  /*!
   * \brief Checks if we are allowed to delete a snapshot of the given dn.
   *
   * \param string $dn     The source dn
   *
   * \param string $categories The acl category (e.g. user)
   *
   * \return boolean TRUE if we are allowed to delete a snapshot.
   */
  function allow_snapshot_delete ($dn, $categories): bool
  {
    $permissions = $this->get_snapshot_permissions($dn, $categories);
    return in_array('d', $permissions);
  }

  function get_snapshot_permissions ($dn, $categories)
  {
    if (!is_array($categories)) {
      $categories = [$categories];
    }
    /* Possible permissions for snapshots */
    $objectPermissions    = ['r', 'c', 'd'];
    $attributePermissions = ['restore_over', 'restore_deleted'];
    foreach ($categories as $category) {
      $acl = $this->get_permissions($dn, $category.'/SnapshotHandler');
      foreach ($objectPermissions as $i => $perm) {
        if (strpos($acl, $perm) === FALSE) {
          unset($objectPermissions[$i]);
        }
      }
      foreach ($attributePermissions as $i => $attribute) {
        $acl = $this->get_permissions($dn, $category.'/SnapshotHandler', $attribute);
        if (strpos($acl, 'w') === FALSE) {
          unset($attributePermissions[$i]);
        }
      }
    }
    return array_merge($objectPermissions, $attributePermissions);
  }

  /*!
   * \brief Get the permissions for a specified dn
   *
   * \param string $dn         The object dn
   *
   * \param string $object     The acl category (e.g. user)
   *
   * \param string $attribute
   *
   * \param bool $skip_write   Remove the write acl for this dn
   *
   */
  function get_permissions ($dn, $object, $attribute = "", $skip_write = FALSE)
  {
    global $config;
    /* If we are forced to skip ACLs checks for the current user
        then return all permissions.
     */
    if ($this->ignore_acl_for_current_user()) {
      if ($skip_write) {
        return 'r';
      }
      return 'rwcdm';
    }

    $attribute = static::sanitizeAttributeName($attribute);

    /* Push cache answer? */
    $ACL_CACHE = &session::get_ref('ACL_CACHE');
    if (isset($ACL_CACHE["$dn+$object+$attribute"])) {
      $ret = $ACL_CACHE["$dn+$object+$attribute"];
      if ($skip_write) {
        $ret = str_replace(['w','c','d','m'], '', $ret);
      }
      return $ret;
    }

    /* Detect the set of ACLs we have to check for this object
     */
    $adn = $dn;
    while (!isset($this->ACLperPath[$adn]) && (strpos($adn, ',') !== FALSE)) {
      $adn = preg_replace("/^[^,]*+,/", "", $adn);
    }
    if (isset($this->ACLperPath[$adn])) {
      $ACL = $this->ACLperPath[$adn];
    } else {
      $ACL_CACHE["$dn+$object+$attribute"] = '';
      return '';
    }

    /* If we do not need to respect any user-filter settings
        we can skip the per object ACL checks.
     */
    $orig_dn  = $dn;
    $dn       = $adn;
    if (isset($ACL_CACHE["$dn+$object+$attribute"])) {
      $ret = $ACL_CACHE["$dn+$object+$attribute"];
      if (!isset($ACL_CACHE["$orig_dn+$object+$attribute"])) {
        $ACL_CACHE["$orig_dn+$object+$attribute"] = $ret;
      }
      if ($skip_write) {
        $ret = str_replace(['w','c','d','m'], '', $ret);
      }
      return $ret;
    }

    /* Get ldap object, for later filter checks */
    $ldap = $config->get_ldap_link();

    $acl = ['r' => '', 'w' => '', 'c' => '', 'd' => '', 'm' => '', 'a' => ''];

    /* Build dn array */
    $path = explode(',', $dn);
    $path = array_reverse($path);

    $departmentInfo = $config->getDepartmentInfo();

    /* Walk along the path to evaluate the acl */
    $cpath = '';
    foreach ($path as $element) {

      /* Clean potential ACLs for each level */
      if (isset($departmentInfo[$cpath])) {
        $acl = $this->cleanACL($acl);
      }

      if ($cpath == "") {
        $cpath = $element;
      } else {
        $cpath = $element.','.$cpath;
      }

      if (isset($ACL[$cpath])) {

        /* Inspect this ACL, place the result into ACL */
        foreach ($ACL[$cpath] as $subacl) {

          /* Reset? Just clean the ACL and turn over to the next one... */
          if ($subacl['type'] == 'reset') {
            $acl = $this->cleanACL($acl, TRUE);
            continue;
          }

          /* With user filter */
          if (!empty($subacl['filter'])) {
            $id = $dn."-".$subacl['filter'];
            if (!isset($ACL_CACHE['FILTER'][$id])) {
              $ACL_CACHE['FILTER'][$id] = $ldap->object_match_filter($dn, $subacl['filter']);
            }
            if (!$ACL_CACHE['FILTER'][$id]) {
              continue;
            }
          }

          /* Self ACLs? */
          if (($dn != $this->dn) && isset($subacl['acl'][$object][0]) && (strpos($subacl['acl'][$object][0], "s") !== FALSE)) {
            continue;
          }

          /* If attribute is "", we want to know, if we've *any* permissions here...
              Merge global class ACLs [0] with attributes specific ACLs [attribute].
           */
          if (($attribute == '') && isset($subacl['acl'][$object])) {
            foreach ($subacl['acl'][$object] as $attr => $dummy) {
              $acl = $this->mergeACL($acl, $subacl['type'], $subacl['acl'][$object][$attr]);
            }
            continue;
          }

          /* Per attribute ACL? */
          if (isset($subacl['acl'][$object][$attribute])) {
            $acl = $this->mergeACL($acl, $subacl['type'], $subacl['acl'][$object][$attribute]);
            continue;
          }

          /* Per object ACL? */
          if (isset($subacl['acl'][$object][0])) {
            $acl = $this->mergeACL($acl, $subacl['type'], $subacl['acl'][$object][0]);
            continue;
          }

          /* Global ACL? */
          if (isset($subacl['acl']['all'][0])) {
            $acl = $this->mergeACL($acl, $subacl['type'], $subacl['acl']['all'][0]);
            continue;
          }

          /* Category ACLs    (e.g. $object = "user/0")
           */
          if (strstr($object, '/0')) {
            $ocs = preg_replace("/\/0$/", "", $object);
            if (isset($config->data['CATEGORIES'][$ocs])) {

              /* if $attribute is "", then check every single attribute for this object.
                 if it is 0, then just check the object category ACL.
               */
              if ($attribute == "") {
                foreach ($config->data['CATEGORIES'][$ocs]['classes'] as $oc) {
                  if (isset($subacl['acl'][$ocs.'/'.$oc])) {
                    // Skip ACLs which are defined for ourselfs only - if not checking against ($ui->dn)
                    if (isset($subacl['acl'][$ocs.'/'.$oc][0]) &&
                        ($dn != $this->dn) &&
                        (strpos($subacl['acl'][$ocs.'/'.$oc][0], "s") !== FALSE)) {
                      continue;
                    }

                    foreach ($subacl['acl'][$ocs.'/'.$oc] as $attr => $dummy) {
                      $acl = $this->mergeACL($acl, $subacl['type'], $subacl['acl'][$ocs.'/'.$oc][$attr]);
                    }
                    continue;
                  }
                }
              } else {
                if (isset($subacl['acl'][$ocs.'/'.$oc][0])) {
                  if (($dn != $this->dn) && (strpos($subacl['acl'][$ocs.'/'.$oc][0], "s") !== FALSE)) {
                    continue;
                  }
                  $acl = $this->mergeACL($acl, $subacl['type'], $subacl['acl'][$ocs.'/'.$oc][0]);
                }
              }
            }
            continue;
          }
        }
      }
    }

    /* If the requested ACL is for a container object, then alter
        ACLs by applying cleanACL a last time.
     */
    if (isset($departmentInfo[$dn])) {
      $acl = $this->cleanACL($acl);
    }

    /* Assemble string */
    $ret = "";
    foreach ($acl as $key => $value) {
      if ($value !== "") {
        $ret .= $key;
      }
    }

    $ACL_CACHE["$dn+$object+$attribute"]      = $ret;
    $ACL_CACHE["$orig_dn+$object+$attribute"] = $ret;

    /* Remove write if needed */
    if ($skip_write) {
      $ret = str_replace(['w','c','d','m'], '', $ret);
    }
    return $ret;
  }

  /*!
   * \brief Extract all departments that are accessible
   *
   * Extract all departments that are accessible (direct or 'on the way' to an
   * accessible department)
   *
   * \param string $module The module
   *
   * \param bool $skip_self_acls FALSE
   *
   * \return array Return all accessible departments
   */
  function get_module_departments ($module, $skip_self_acls = FALSE)
  {
    global $config;
    /* If we are forced to skip ACLs checks for the current user
        then return all departments as valid.
     */
    if ($this->ignore_acl_for_current_user()) {
      return array_values($config->getDepartmentList());
    }

    /* Use cached results if possilbe */
    $ACL_CACHE = &session::get_ref('ACL_CACHE');

    if (!is_array($module)) {
      $module = [$module];
    }

    $departmentInfo = $config->getDepartmentInfo();

    $res = [];
    foreach ($module as $mod) {
      if (isset($ACL_CACHE['MODULE_DEPARTMENTS'][$mod])) {
        $res = array_merge($res, $ACL_CACHE['MODULE_DEPARTMENTS'][$mod]);
        continue;
      }

      $deps = [];

      /* Search for per object ACLs */
      foreach ($this->ACL as $dn => $infos) {
        foreach ($infos as $info) {
          $found = FALSE;
          foreach ($info['acl'] as $cat => $data) {
            /* Skip self acls? */
            if ($skip_self_acls && isset($data['0']) && (strpos($data['0'], "s") !== FALSE)) {
              continue;
            }
            if (preg_match("/^".preg_quote($mod, '/')."/", $cat)) {
              $found = TRUE;
              break;
            }
          }

          if ($found && !isset($departmentInfo[$dn])) {
            while (!isset($departmentInfo[$dn]) && strpos($dn, ',')) {
              $dn = preg_replace("/^[^,]+,/", "", $dn);
            }
            if (isset($departmentInfo[$dn])) {
              $deps[$dn] = $dn;
            }
          }
        }
      }

      /* For all departments */
      $departments = $config->getDepartmentList();
      foreach ($departments as $dn) {
        if (isset($deps[$dn])) {
          continue;
        }
        $acl = '';
        if (strpos($mod, '/')) {
          $acl .= $this->get_permissions($dn, $mod);
        } else {
          $acl .= $this->get_category_permissions($dn, $mod);
        }
        if (!empty($acl)) {
          $deps[$dn] = $dn;
        }
      }

      $ACL_CACHE['MODULE_DEPARTMENTS'][$mod] = $deps;
      $res = array_merge($res, $deps);
    }

    return array_values($res);
  }

  /*!
   * \brief Merge acls
   *
   * \param $acl The ACL
   *
   * \param $type The type
   *
   * \param $newACL The new ACL
   */
  function mergeACL ($acl, $type, $newACL)
  {
    $at = ["subtree" => "s", "one" => "1"];

    if ((strpos($newACL, 'w') !== FALSE) && (strpos($newACL, 'r') === FALSE)) {
      $newACL .= "r";
    }

    /* Ignore invalid characters */
    $newACL = preg_replace('/[^rwcdm]/', '', $newACL);

    foreach (str_split($newACL) as $char) {
      /* Skip "self" ACLs without combination of rwcdm, they have no effect.
         -self flag without read/write/create/...
       */
      if (empty($char)) {
        continue;
      }

      /* Skip subtree entries */
      if ($acl[$char] == 's') {
        continue;
      }

      if ($type == "base" && $acl[$char] != 1) {
        $acl[$char] = 0;
      } else {
        $acl[$char] = $at[$type];
      }
    }

    return $acl;
  }

  /*!
   * \brief Clean acls
   *
   * \param $acl ACL to be cleaned
   *
   * \param boolean $reset FALSE
   */
  function cleanACL ($acl, $reset = FALSE)
  {
    foreach ($acl as $key => $value) {
      /* Continue, if value is empty or subtree */
      if (($value == "") || ($value == "s")) {
        continue;
      }

      /* Reset removes everything but 'p' */
      if ($reset && $value != 'p') {
        $acl[$key] = "";
        continue;
      }

      /* Decrease tree level */
      if (is_int($value)) {
        if ($value) {
          $acl[$key]--;
        } else {
          $acl[$key] = "";
        }
      }
    }

    return $acl;
  }

  /*!
   * \brief Return combined acls for a given category
   *
   * Return combined acls for a given category.
   * All acls will be combined like boolean AND
   * As example ('rwcdm' + 'rcd' + 'wrm'= 'r')
   *
   * Results will be cached in $this->result_cache.
   * $this->result_cache will be resetted if load_acls is called.
   *
   * \param string $dn The DN
   *
   * \param string $category The category
   *
   * \return string return acl combined with boolean AND
   */
  function get_complete_category_acls ($dn, $category)
  {
    global $config;

    if (!is_string($category)) {
      trigger_error('category must be string');
      return '';
    } else {
      if (isset($this->result_cache['get_complete_category_acls'][$dn][$category])) {
        return $this->result_cache['get_complete_category_acls'][$dn][$category];
      }
      $acl = 'rwcdm';
      if (isset($config->data['CATEGORIES'][$category])) {
        foreach ($config->data['CATEGORIES'][$category]['classes'] as $oc) {
          if ($oc == '0') {
            /* Skip objectClass '0' (e.g. user/0) */
            continue;
          }
          $tmp = $this->get_permissions($dn, $category.'/'.$oc);
          $types = $acl;
          for ($i = 0, $l = strlen($types); $i < $l; $i++) {
            if (strpos($tmp, $types[$i]) === FALSE) {
              $acl = str_replace($types[$i], '', $acl);
            }
          }
        }
      } else {
        $acl = '';
      }
      $this->result_cache['get_complete_category_acls'][$dn][$category] = $acl;
      return $acl;
    }
  }


  /*!
   * \brief Ignore acl for the current user
   *
   * \return Returns TRUE if the current user is configured in IGNORE_ACL=".."
   *  in your fusiondirectory.conf FALSE otherwise
   */
  function ignore_acl_for_current_user ()
  {
    return $this->ignoreACL;
  }

  /*!
  * \brief Checks the posixAccount status by comparing the shadow attributes.
  *
  * \return const
  *                  POSIX_ACCOUNT_EXPIRED           - If the account is expired.
  *                  POSIX_WARN_ABOUT_EXPIRATION     - If the account is going to expire.
  *                  POSIX_FORCE_PASSWORD_CHANGE     - The password has to be changed.
  *                  POSIX_DISALLOW_PASSWORD_CHANGE  - The password cannot be changed right now.
  *
  *
  *
  *      shadowLastChange
  *      |
  *      |---- shadowMin --->    |       <-- shadowMax --
  *      |                       |       |
  *      |------- shadowWarning ->       |
  *                                      |-- shadowInactive --> DEACTIVATED
  *                                      |
  *                                      EXPIRED
  *
  */
  function expired_status ()
  {
    global $config;

    if ($this->forcePasswordChange) {
      return POSIX_FORCE_PASSWORD_CHANGE;
    }

    // Skip this for the admin account, we do not want to lock him out.
    if ($this->is_user_admin()) {
      return 0;
    }

    $ldap = $config->get_ldap_link();

    if (class_available('ppolicyAccount')) {
      try {
        list($policy, $attrs) = user::fetchPpolicy($this->dn);
        if (
          isset($policy['pwdExpireWarning'][0]) &&
          isset($policy['pwdMaxAge'][0]) &&
          isset($attrs['pwdChangedTime'][0])
        ) {
          $now                      = new DateTime('now', timezone::utc());
          $pwdExpireWarningSeconds  = intval($policy['pwdExpireWarning'][0]);
          $maxAge                   = $policy['pwdMaxAge'][0];
          /* Build expiration date from pwdChangedTime and max age */
          $expDate = LdapGeneralizedTime::fromString($attrs['pwdChangedTime'][0]);
          $expDate->setTimezone(timezone::utc());
          $expDate->add(new DateInterval('PT'.$maxAge.'S'));
          if ($expDate->getTimeStamp() < ($now->getTimeStamp() + $pwdExpireWarningSeconds)) {
            return POSIX_WARN_ABOUT_EXPIRATION;
          }
        }
      } catch (NonExistingLdapNodeException $e) {
        /* ppolicy not found in the LDAP */
      }
    }

    if ($config->get_cfg_value('handleExpiredAccounts') != 'TRUE') {
      return 0;
    }

    $ldap->cd($config->current['BASE']);
    $ldap->cat($this->dn);
    $attrs    = $ldap->fetch();
    $current  = floor(date("U") / 60 / 60 / 24);

    // Fetch required attributes
    foreach (['shadowExpire','shadowLastChange','shadowMax','shadowMin',
                'shadowInactive','shadowWarning','sambaKickoffTime'] as $attr) {
      $$attr = (isset($attrs[$attr][0]) ? $attrs[$attr][0] : NULL);
    }

    // Check if the account has reached its kick off limitations.
    // ----------------------------------------------------------
    // Once the accout reaches the kick off limit it has expired.
    if (($sambaKickoffTime !== NULL) && (time() >= $sambaKickoffTime)) {
      return POSIX_ACCOUNT_EXPIRED;
    }

    // Check if the account has expired.
    // ---------------------------------
    // An account is locked/expired once its expiration date was reached (shadowExpire).
    // If the optional attribute (shadowInactive) is set, we've to postpone
    //  the account expiration by the amount of days specified in (shadowInactive).
    // ShadowInactive specifies an amount of days we've to reprieve the user.
    // It some kind of x days' grace.
    if (($shadowExpire != NULL) && ($shadowExpire <= $current)
      && (($shadowInactive == NULL) || ($current > $shadowExpire + $shadowInactive))) {
      return POSIX_ACCOUNT_EXPIRED;
    }

    // The users password is going to expire.
    // --------------------------------------
    // We've to warn the user in the case of an expiring account.
    // An account is going to expire when it reaches its expiration date (shadowExpire).
    // The user has to be warned, if the days left till expiration, match the
    //  configured warning period (shadowWarning)
    // --> shadowWarning: Warn x days before account expiration.
    // Check if the account is still active and not already expired.
    // Check if we've to warn the user by comparing the remaining
    //  number of days till expiration with the configured amount of days in shadowWarning.
    if (($shadowExpire != NULL) && ($shadowWarning != NULL)
      && ($shadowExpire >= $current) && ($shadowExpire <= $current + $shadowWarning)) {
      return POSIX_WARN_ABOUT_EXPIRATION;
    }

    // -- I guess this is the correct detection, isn't it?
    if (($shadowLastChange != NULL) && ($shadowWarning != NULL) && ($shadowMax != NULL)) {
      $daysRemaining = ($shadowLastChange + $shadowMax) - $current;
      if ($daysRemaining > 0 && $daysRemaining <= $shadowWarning) {
        return POSIX_WARN_ABOUT_EXPIRATION;
      }
    }

    // Check if we've to force the user to change his password.
    // --------------------------------------------------------
    // A password change is enforced when the password is older than
    //  the configured amount of days (shadowMax).
    // The age of the current password (shadowLastChange) plus the maximum
    //  amount amount of days (shadowMax) has to be smaller than the
    //  current timestamp.
    // Check if we've an outdated password.
    if (($shadowLastChange != NULL) && ($shadowMax != NULL)
      && ($current >= $shadowLastChange + $shadowMax)) {
      return POSIX_FORCE_PASSWORD_CHANGE;
    }

    // Check if we've to freeze the users password.
    // --------------------------------------------
    // Once a user has changed his password, he cannot change it again
    //  for a given amount of days (shadowMin).
    // We should not allow to change the password within FusionDirectory too.
    // Check if we've an outdated password.
    if (($shadowLastChange != NULL) && ($shadowMin != NULL)
      && ($shadowLastChange + $shadowMin >= $current)) {
      return POSIX_DISALLOW_PASSWORD_CHANGE;
    }

    return 0;
  }

  /* \brief Check if a user is a 'user admin'
   */
  function is_user_admin ()
  {
    global $config;
    if (empty($this->ACLperPath)) {
      $this->loadACL();
    }
    return ($this->get_permissions($config->current['BASE'], 'user/user') == 'rwcdm');
  }

  /* \brief Test if a plugin is blacklisted for this user (does not show up in the menu)
   */
  function isBlacklisted ($plugin)
  {
    global $config;
    $blacklist = $config->get_cfg_value('PluginsMenuBlacklist', []);
    foreach ($blacklist as $item) {
      list ($group, $p) = explode('|', $item, 2);
      if (($plugin == $p) && (in_array($group, $this->groups) || in_array($group, $this->roles))) {
        return TRUE;
      }
    }

    return FALSE;
  }

  /* \brief Search which ACL category should be used for this attribute and this object type, if any
   *
   * \return The ACL category, or FALSE if not found, or TRUE if acl check should be bypassed
   */
  function getAttributeCategory ($type, $attribute)
  {
    global $config;

    if (in_array_ics($attribute, ['objectClass', 'dn'])) {
      return TRUE;
    }

    $attribute = static::sanitizeAttributeName($attribute);

    if (is_array($type)) {
      /* Used for recursion through subtabs */
      $prefix = '';
      $tabs   = $type;
    } else {
      /* Usual workflow */
      $infos  = objects::infos($type);
      $prefix = $infos['aclCategory'].'/';
      $tabs   = $config->data['TABS'][$infos['tabGroup']];
    }
    foreach ($tabs as $tab) {
      $acls = pluglist::pluginInfos($tab['CLASS'])['plProvidedAcls'];
      if (isset($acls[$attribute])) {
        return $prefix.$tab['CLASS'];
      }
      if (isset($tab['SUBTABS'])) {
        $acl = $this->getAttributeCategory($config->data['TABS'][$tab['SUBTABS']], $attribute);
        if ($acl !== FALSE) {
          return $prefix.$acl;
        }
      }
    }
    return FALSE;
  }

  function getSizeLimitHandler ()
  {
    return $this->sizeLimitHandler;
  }

  /* \brief Returns the base this user is stored in
   */
  function getBase ()
  {
    return get_base_from_people($this->dn);
  }

  /* \brief Returns the current base the user went to in management classes
   */
  function getCurrentBase ()
  {
    if (!empty($this->currentBase)) {
      return $this->currentBase;
    } else {
      return $this->getBase();
    }
  }

  /* \brief Sets the current base the user went to in management classes
   */
  function setCurrentBase ($base)
  {
    $this->currentBase = $base;
  }

  /* \brief Get ACL name or HTML id from attribute name
   */
  public static function sanitizeAttributeName ($name)
  {
    return preg_replace('/[\/\-,.#:;]/', '_', $name);
  }

  /*!
   * \brief Get user from LDAP directory
   *
   * Search the user by login or other fields authorized by the configuration
   *
   * \param string $username The username or email to check
   *
   * \return userinfo instance on SUCCESS, FALSE if not found, string error on error
   */
  public static function getLdapUser (string $username)
  {
    global $config;

    /* look through the entire ldap */
    $ldap = $config->get_ldap_link();
    if (!$ldap->success()) {
      msg_dialog::display(_('LDAP error'),
          msgPool::ldaperror($ldap->get_error(FALSE), '', LDAP_AUTH),
          FATAL_ERROR_DIALOG);
      exit();
    }

    $allowed_attributes = ['uid','mail'];
    $verify_attr = [];
    $tmp = explode(',', $config->get_cfg_value('loginAttribute'));
    foreach ($tmp as $attr) {
      if (in_array($attr, $allowed_attributes)) {
        $verify_attr[] = $attr;
      }
    }

    if (count($verify_attr) == 0) {
      $verify_attr = ['uid'];
    }
    $tmp    = $verify_attr;
    $tmp[]  = 'uid';
    $filter = '';
    foreach ($verify_attr as $attr) {
      $filter .= '('.$attr.'='.$username.')';
    }
    $filter = '(&(|'.$filter.')(objectClass=inetOrgPerson))';
    $ldap->cd($config->current['BASE']);
    $ldap->search($filter, $tmp);

    /* get results, only a count of 1 is valid */
    if ($ldap->count() == 0) {
      /* user not found */
      return FALSE;
    } elseif ($ldap->count() != 1) {
      /* found more than one matching id */
      return _('Login (uid) is not unique inside the LDAP tree. Please contact your administrator.');
    }

    /* LDAP schema is not case sensitive. Perform additional check. */
    $attrs = $ldap->fetch();
    $success = FALSE;
    foreach ($verify_attr as $attr) {
      if (isset($attrs[$attr][0]) && $attrs[$attr][0] == $username) {
        $success = TRUE;
      }
    }
    $ldap->disconnect();
    if (!$success) {
      return FALSE;
    }

    return new userinfo($attrs['dn']);
  }

  /*!
   * \brief Verify user login against LDAP directory
   *
   * Checks if the specified username is in the LDAP and verifies if the
   * password is correct by binding to the LDAP with the given credentials.
   *
   * \param string $username The username to check
   *
   * \param string $password The password to check
   *
   * \return TRUE on SUCCESS, NULL or FALSE on error
   */
  public static function loginUser (string $username, string $password): userinfo
  {
    global $config;

    $ui = static::getLdapUser($username);

    if ($ui === FALSE) {
      throw new LoginFailureException(ldap::invalidCredentialsError());
    } elseif (is_string($ui)) {
      msg_dialog::display(_('Internal error'), $ui, FATAL_ERROR_DIALOG);
      exit();
    }

    /* password check, bind as user with supplied password  */
    $ldapObj = new LDAP($ui->dn, $password, $config->current['SERVER'],
      isset($config->current['LDAPFOLLOWREFERRALS']) && ($config->current['LDAPFOLLOWREFERRALS'] == 'TRUE'),
      isset($config->current['LDAPTLS']) && ($config->current['LDAPTLS'] == 'TRUE')
    );
    $ldap = new ldapMultiplexer($ldapObj);
    if (!$ldap->success()) {
      if ($ldap->get_error(FALSE) == 'changeAfterReset') {
        $ui->forcePasswordChange = TRUE;
      } else {
        throw new LoginFailureException($ldap->get_error(FALSE));
      }
    }

    if (class_available('ppolicyAccount') && !function_exists('ldap_bind_ext')) {
      $ldap->cd($config->current['BASE']);
      $ldap->search('(objectClass=*)', [], 'one');
      if (!$ldap->success()) {
        $ui->forcePasswordChange = TRUE;
      }
    }

    /* Username is set, load subtreeACL's now */
    $ui->loadACL();

    return $ui;
  }
}
