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
 * \file class_userinfo.inc
 * Source code for the class userinfo
 */

/*!
 * \brief Class userinfo
 * This class contains all informations and functions
 * about user
 */

class userinfo
{
  var $dn;
  var $ip;
  var $cn;
  var $uid;
  var $sn           = '';
  var $givenName    = '';
  var $gidNumber    = -1;
  var $language     = "";
  var $subtreeACL   = array();
  var $ACL          = array();
  var $groups       = array();
  var $roles        = array();
  var $result_cache = array();
  var $ignoreACL    = FALSE;

  var $ACLperPath             = array();
  var $ACLperPath_usesFilter  = array();

  /* get acl's an put them into the userinfo object
     attr subtreeACL (userdn:components, userdn:component1#sub1#sub2,component2,...) */
  function __construct($unused, $userdn)
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cat($userdn, array('cn', 'sn', 'givenName', 'uid', 'gidNumber', 'preferredLanguage'));
    $attrs = $ldap->fetch();

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

    $this->dn   = $userdn;
    $this->uid  = $attrs['uid'][0];
    $this->ip   = $_SERVER['REMOTE_ADDR'];

    $this->ignoreACL = ($config->get_cfg_value("ignoreAcl") == $this->dn);

    /* Initialize ACL_CACHE */
    $this->reset_acl_cache();
  }

  /*!
  * \brief Reset acl cache
  */
  public function reset_acl_cache()
  {
    /* Initialize ACL_CACHE */
    session::global_set('ACL_CACHE', array());
  }

  /*!
   * \brief Load an acl
   */
  function loadACL()
  {
    global $config;
    $this->ACL          = array();
    $this->groups       = array();
    $this->roles        = array();
    $this->result_cache = array();
    $this->reset_acl_cache();
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);

    /* Get member groups... */
    $ldap->search('(&(objectClass=groupOfNames)(member='.ldap_escape_f($this->dn).'))', array('dn'));
    while ($attrs = $ldap->fetch()) {
      $this->groups[$attrs['dn']] = $attrs['dn'];
    }

    /* Get member POSIX groups... */
    $ldap->search('(&(objectClass=posixGroup)(memberUid='.ldap_escape_f($this->uid).'))', array('dn'));
    while ($attrs = $ldap->fetch()) {
      $this->groups[$attrs['dn']] = $attrs['dn'];
    }

    /* Get member roles... */
    $ldap->search('(&(objectClass=organizationalRole)(roleOccupant='.ldap_escape_f($this->dn).'))', array('dn'));
    while ($attrs = $ldap->fetch()) {
      $this->roles[$attrs['dn']] = $attrs['dn'];
    }

    /* Crawl through ACLs and move relevant to the tree */
    $ldap->search("(objectClass=gosaACL)", array('dn', 'gosaAclEntry'));
    $aclp = array();
    $aclc = array();
    while ($attrs = $ldap->fetch()) {

      /* Insert links in ACL array */
      $aclp[$attrs['dn']] = substr_count($attrs['dn'], ',');
      $aclc[$attrs['dn']] = array();
      $ol = array();
      for ($i = 0; $i < $attrs['gosaAclEntry']['count']; $i++) {
        $ol = array_merge($ol, acl::explodeAcl($attrs['gosaAclEntry'][$i]));
      }
      $aclc[$attrs['dn']] = $ol;
    }

    /* Resolve roles here */
    foreach ($aclc as $dn => $data) {
      foreach ($data as $prio => $aclc_value) {
        unset($aclc[$dn][$prio]);

        $ldap->cat($aclc_value['acl'], array("gosaAclTemplate"));
        $attrs = $ldap->fetch();

        if (isset($attrs['gosaAclTemplate'])) {
          $roleAcls = acl::explodeRole($attrs['gosaAclTemplate']);
          foreach ($roleAcls as $roleAcl) {
            $aclc[$dn][]  = array(
              'acl'     => $roleAcl,
              'type'    => $aclc_value['type'],
              'members' => $aclc_value['members'],
              'filter'  => $aclc_value['filter']
            );
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
        $interresting = FALSE;

        /* No members? This ACL rule is deactivated ... */
        if (!count($type['members'])) {
          $interresting = FALSE;
        } else {
          /* Inspect members... */
          foreach (array_keys($type['members']) as $grp) {
            /* Some group inside the members that is relevant for us? */
            if (in_array_ics(@LDAP::convert(preg_replace('/^G:/', '', $grp)), $this->groups)) {
              $interresting = TRUE;
            }

            /* Some role inside the members that is relevant for us? */
            if (in_array_ics(@LDAP::convert(preg_replace('/^R:/', '', $grp)), $this->roles)) {
              $interresting = TRUE;
            }

            /* User inside the members? */
            if (mb_strtoupper(preg_replace('/^U:/', '', $grp)) == mb_strtoupper($this->dn)) {
              $interresting = TRUE;
            }

            /* Wildcard? */
            if (preg_match('/^G:\*/',  $grp)) {
              $interresting = TRUE;
            }
          }
        }

        if ($interresting) {
          if (!isset($this->ACL[$dn])) {
            $this->ACL[$dn] = array();
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


      For object located in 'ou=dep1,ou=base' we have to both ACLs,
       for objects in 'ou=base' we only have to apply on ACL.
     */
    $without_self_acl = $all_acl = array();
    foreach ($this->ACL as $dn => $acl) {
      $sdn    = $dn;
      $first  = TRUE; // Run at least once
      while ((strpos($dn, ",") !== FALSE) || $first) {
        $first = FALSE;
        if (isset($this->ACL[$dn])) {
          $all_acl[$sdn][$dn]           = $this->ACL[$dn];
          $without_self_acl[$sdn][$dn]  = $this->ACL[$dn];
          foreach ($without_self_acl[$sdn][$dn] as $acl_id => $acl_set) {

            /* Remember which ACL set has speicial user filter
             */
            if (isset($acl_set['filter']{1})) {
              $this->ACLperPath_usesFilter[$sdn] = TRUE;
            }

            /* Remove all acl entries which are especially for the current user (self acl)
             */
            foreach ($acl_set['acl'] as $object => $object_acls) {
              if (isset($object_acls[0]) && (strpos($object_acls[0], "s") !== FALSE)) {
                unset($without_self_acl[$sdn][$dn][$acl_id]['acl'][$object]);
              }
            }
          }
        }
        $dn = preg_replace("/^[^,]*+,/", "", $dn);
      }
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
  function get_acl_target_objects()
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
   * \param bool $any_acl FALSE
   *
   * \return all the permissions for the dn and category
   */
  function get_category_permissions($dn, $category, $any_acl = FALSE)
  {
    return @$this->get_permissions($dn, $category.'/0', "");
  }


  /*!
   * \brief Check if the given object (dn) is copyable
   *
   * \param string $dn     The object dn
   *
   * \param string $object The acl  category (e.g. user)
   *
   * \param string $class  The acl  class (e.g. user)
   *
   * \return boolean TRUE if the given object is copyable else FALSE
  */
  function is_copyable($dn, $object, $class)
  {
    return preg_match("/r/", $this->has_complete_category_acls($dn, $object));
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
  function is_cutable($dn, $object, $class)
  {
    $remove = preg_match("/d/", $this->get_permissions($dn, $object."/".$class));
    $read   = preg_match("/r/", $this->has_complete_category_acls($dn, $object));
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
  function is_pasteable($dn, $object)
  {
    return preg_match("/w/", $this->has_complete_category_acls($dn, $object));
  }


  /*!
   * \brief Checks if we are allowed to restore a snapshot for the given dn.
   *
   * \param string $dn     The destination dn
   *
   * \param string $object The acl  category (e.g. user)
   *
   * \return boolean TRUE if we are allowed to restore a snapshot.
   */
  function allow_snapshot_restore($dn, $object)
  {
    if (!is_array($object)) {
      $object = array($object);
    }
    $r = $w = TRUE;
    foreach ($object as $category) {
      $w &= preg_match("/w/", $this->has_complete_category_acls($dn, $category));
      $r &= preg_match("/r/", $this->has_complete_category_acls($dn, $category));
    }
    return ($r && $w);
  }


  /*!
   * \brief Checks if we are allowed to create a snapshot of the given dn.
   *
   * \param string $dn     The source dn
   *
   * \param string $object The acl category (e.g. user)
   *
   * \return boolean TRUE if we are allowed to restore a snapshot.
   */
  function allow_snapshot_create($dn, $object)
  {
    if (!is_array($object)) {
      $object = array($object);
    }
    $r = TRUE;
    foreach ($object as $category) {
      $r &= preg_match("/r/", $this->has_complete_category_acls($dn, $category));
    }
    return $r;
  }

  /*!
   * \brief Get the permissions for a specified dn
   *
   * \param string $dn         The object dn
   *
   * \param string $object     The acl category (e.g. user)
   *
   * \param string $attribute  The acl class (e.g. user)
   *
   * \param bool $skip_write   Remove the write acl for this dn
   *
   */
  function get_permissions($dn, $object, $attribute = "", $skip_write = FALSE)
  {
    global $config;
    /* If we are forced to skip ACLs checks for the current user
        then return all permissions.
     */
    if ($this->ignore_acl_for_current_user()) {
      if ($skip_write) {
        return "rcdm";
      }
      return "rwcdm";
    }

    /* Push cache answer? */
    $ACL_CACHE = &session::global_get('ACL_CACHE');
    if (isset($ACL_CACHE["$dn+$object+$attribute"])) {
      $ret = $ACL_CACHE["$dn+$object+$attribute"];
      if ($skip_write) {
        $ret = str_replace(array('w','c','d','m'), '', $ret);
      }
      return $ret;
    }

    /* Detect the set of ACLs we have to check for this object
     */
    $adn = $dn;
    while (!isset($this->ACLperPath[$adn]) && (strpos($adn, ",") !== FALSE)) {
      $adn = preg_replace("/^[^,]*+,/", "", $adn);
    }
    if (isset($this->ACLperPath[$adn])) {
      $ACL = $this->ACLperPath[$adn];
    } else {
      $ACL_CACHE["$dn+$object+$attribute"] = "";
      return "";
    }

    /* If we do not need to respect any user-filter settings
        we can skip the per object ACL checks.
     */
    $orig_dn = $dn;
    if (!isset($this->ACLperPath_usesFilter[$adn])) {
      $dn = $adn;
      if (isset($ACL_CACHE["$dn+$object+$attribute"])) {
        $ret = $ACL_CACHE["$dn+$object+$attribute"];
        if (!isset($ACL_CACHE["$orig_dn+$object+$attribute"])) {
          $ACL_CACHE["$orig_dn+$object+$attribute"] = $ACL_CACHE["$dn+$object+$attribute"];
        }
        if ($skip_write) {
          $ret = str_replace('w', '', $ret);
        }
        return $ret;
      }
    }

    /* Get ldap object, for later filter checks
     */
    $ldap = $config->get_ldap_link();

    $acl = array("r" => "", "w" => "", "c" => "", "d" => "", "m" => "", "a" => "");

    /* Build dn array */
    $path = explode(',', $dn);
    $path = array_reverse($path);

    /* Walk along the path to evaluate the acl */
    $cpath = "";
    foreach ($path as $element) {

      /* Clean potential ACLs for each level */
      if (isset($config->idepartments[$cpath])) {
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
          if (isset($subacl['filter']) && !empty($subacl['filter'])) {
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
          if ($attribute == "" && isset($subacl['acl'][$object])) {
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
          if (strstr($object, "/0")) {
            $ocs = preg_replace("/\/0$/", "", $object);
            if (isset($config->data['CATEGORIES'][$ocs])) {

              /* if $attribute is "", then check every single attribute for this object.
                 if it is 0, then just check the object category ACL.
               */
              if ($attribute == "") {
                foreach ($config->data['CATEGORIES'][$ocs]['classes'] as $oc) {
                  if (isset($subacl['acl'][$ocs.'/'.$oc])) {
                    // Skip ACLs wich are defined for ourselfs only - if not checking against ($ui->dn)
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
    if (isset($config->idepartments[$dn])) {
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
      $ret = str_replace(array('w','c','d','m'), '', $ret);
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
  function get_module_departments($module, $skip_self_acls = FALSE )
  {
    global $config;
    /* If we are forced to skip ACLs checks for the current user
        then return all departments as valid.
     */
    if ($this->ignore_acl_for_current_user()) {
      return array_keys($config->idepartments);
    }

    /* Use cached results if possilbe */
    $ACL_CACHE = &session::global_get('ACL_CACHE');

    if (!is_array($module)) {
      $module = array($module);
    }

    $res = array();
    foreach ($module as $mod) {
      if (isset($ACL_CACHE['MODULE_DEPARTMENTS'][$mod])) {
        $res = array_merge($res, $ACL_CACHE['MODULE_DEPARTMENTS'][$mod]);
        continue;
      }

      $deps = array();

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

          if ($found && !isset($config->idepartments[$dn])) {
            while (!isset($config->idepartments[$dn]) && strpos($dn, ",")) {
              $dn = preg_replace("/^[^,]+,/", "", $dn);
            }
            if (isset($config->idepartments[$dn])) {
              $deps[$dn] = $dn;
            }
          }
        }
      }

      /* For all gosaDepartments */
      foreach ($config->departments as $dn) {
        if (isset($deps[$dn])) continue;
        $acl = "";
        if (strpos($mod, '/')) {
          $acl .= $this->get_permissions($dn, $mod);
        } else {
          $acl .= $this->get_category_permissions($dn, $mod, TRUE);
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
  function mergeACL($acl, $type, $newACL)
  {
    $at = array("subtree" => "s", "one" => "1");

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
  function cleanACL($acl, $reset = FALSE)
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
   * #FIXME This could be logical wrong or could be optimized in the future
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
  function has_complete_category_acls($dn, $category)
  {
    global $config;
    $acl    = "rwcdm";
    $types  = "rwcdm";

    if (!is_string($category)) {
      trigger_error("category must be string");
      $acl = "";
    } else {
      if (!isset($this->result_cache['has_complete_category_acls'][$dn][$category]))   {
        if (isset($config->data['CATEGORIES'][$category])) {
          foreach ($config->data['CATEGORIES'][$category]['classes'] as $oc) {
            /* Skip objectClass '0' (e.g. user/0) get_permissions will ever return '' ??  */
            if ($oc == "0") continue;
            $tmp = $this->get_permissions($dn, $category."/".$oc);
            for ($i = 0, $l = strlen($types); $i < $l; $i++) {
              if (!preg_match("/".$types[$i]."/", $tmp)) {
                $acl = preg_replace("/".$types[$i]."/", "", $acl);
              }
            }
          }
        } else {
          $acl = "";
        }
        $this->result_cache['has_complete_category_acls'][$dn][$category] = $acl;
      } else {
        $acl = $this->result_cache['has_complete_category_acls'][$dn][$category];
      }
    }
    return $acl;
  }


  /*!
   * \brief Ignore acl for the current user
   *
   * \return Returns TRUE if the current user is configured in IGNORE_ACL=".."
   *  in your fusiondirectory.conf FALSE otherwise
   */
  function ignore_acl_for_current_user()
  {
    return $this->ignoreACL;
  }

  /*!
  * \brief Checks the posixAccount status by comparing the shadow attributes.
  *
  * \param object $config   The FusionDirectory configuration object.
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
    // Skip this for the admin account, we do not want to lock him out.
    if ($this->is_user_admin()) {
      return 0;
    }

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->cat($this->dn);
    $attrs    = $ldap->fetch();
    $current  = floor(date("U") / 60 / 60 / 24);

    // Fetch required attributes
    foreach (array('shadowExpire','shadowLastChange','shadowMax','shadowMin',
                'shadowInactive','shadowWarning','sambaKickoffTime') as $attr) {
      $$attr = (isset($attrs[$attr][0])? $attrs[$attr][0] : NULL);
    }

    // Check if the account has reached its kick off limitations.
    // ----------------------------------------------------------
    // Once the accout reaches the kick off limit it has expired.
    if ($sambaKickoffTime !== NULL) {
      if (time() >= $sambaKickoffTime) {
        return POSIX_ACCOUNT_EXPIRED;
      }
    }

    // Check if the account has expired.
    // ---------------------------------
    // An account is locked/expired once its expiration date has reached (shadowExpire).
    // If the optional attribute (shadowInactive) is set, we've to postpone
    //  the account expiration by the amount of days specified in (shadowInactive).
    if (($shadowExpire != NULL) && ($shadowExpire <= $current)) {

      // The account seems to be expired, but we've to check 'shadowInactive' additionally.
      // ShadowInactive specifies an amount of days we've to reprieve the user.
      // It some kind of x days' grace.
      if (($shadowInactive == NULL) || $current > $shadowExpire + $shadowInactive) {

        // Finally we've detect that the account is deactivated.
        return POSIX_ACCOUNT_EXPIRED;
      }
    }

    // The users password is going to expire.
    // --------------------------------------
    // We've to warn the user in the case of an expiring account.
    // An account is going to expire when it reaches its expiration date (shadowExpire).
    // The user has to be warned, if the days left till expiration, match the
    //  configured warning period (shadowWarning)
    // --> shadowWarning: Warn x days before account expiration.
    if (($shadowExpire != NULL) && ($shadowWarning != NULL)) {

      // Check if the account is still active and not already expired.
      if ($shadowExpire >= $current) {

        // Check if we've to warn the user by comparing the remaining
        //  number of days till expiration with the configured amount
        //  of days in shadowWarning.
        if (($shadowExpire - $current) <= $shadowWarning) {
          return POSIX_WARN_ABOUT_EXPIRATION;
        }
      }
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
    if (($shadowLastChange != NULL) && ($shadowMax != NULL)) {
      // Check if we've an outdated password.
      if ($current >= ($shadowLastChange + $shadowMax)) {
        return POSIX_FORCE_PASSWORD_CHANGE;
      }
    }

    // Check if we've to freeze the users password.
    // --------------------------------------------
    // Once a user has changed his password, he cannot change it again
    //  for a given amount of days (shadowMin).
    // We should not allow to change the password within FusionDirectory too.
    if (($shadowLastChange != NULL) && ($shadowMin != NULL)) {
      // Check if we've an outdated password.
      if (($shadowLastChange + $shadowMin) >= $current) {
        return POSIX_DISALLOW_PASSWORD_CHANGE;
      }
    }

    return 0;
  }

  /* \brief Check if a user is a 'user admin'
   */
  function is_user_admin()
  {
    global $config;
    if (empty($this->ACLperPath)) {
      $this->loadACL();
    }
    return ($this->get_permissions($config->current['BASE'], 'user/user') == 'rwcdm');
  }
}
?>
