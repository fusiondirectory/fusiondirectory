<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2020  FusionDirectory

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
  var $groups       = [];
  var $roles        = [];

  /*! \brief LDAP attributes of this user at login */
  protected $cachedAttrs  = [];

  protected $result_cache = [];
  protected $ignoreACL    = FALSE;
  protected $ACL          = [];
  protected $ACLperPath   = [];

  /*! \brief LDAP size limit handler */
  protected $sizeLimitHandler;

  /*! \brief Current management base */
  protected $currentBase;

  /*! \brief Password change should be forced */
  protected $forcePasswordChange = FALSE;

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
    $ldap->cat($this->dn, ['*']);
    $attrs = $ldap->fetch(TRUE);

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

    $this->cachedAttrs = $attrs;
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
    global $config, $plist;

    $this->ACL          = [];
    $this->groups       = [];
    $this->roles        = [];
    $this->result_cache = [];
    $this->reset_acl_cache();
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $targetFilterLimit  = $config->get_cfg_value('AclTargetFilterLimit', 100);

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
    $ldap->search('(objectClass=gosaACL)', ['dn', 'gosaAclEntry']);
    $ACLsContent = [];
    while ($attrs = $ldap->fetch()) {

      /* Insert links in ACL array */
      $mergedAcls = [];
      for ($i = 0; $i < $attrs['gosaAclEntry']['count']; $i++) {
        $mergedAcls = array_merge($mergedAcls, acl::explodeACL($attrs['gosaAclEntry'][$i]));
      }
      $ACLsContent[$attrs['dn']] = $mergedAcls;
    }

    $ACLsContentResolved = [];

    /* Resolve roles here */
    foreach ($ACLsContent as $dn => $ACLRules) {
      foreach ($ACLRules as $ACLRule) {
        $ldap->cat($ACLRule['acl'], ['gosaAclTemplate']);
        $attrs = $ldap->fetch();

        if (!isset($attrs['gosaAclTemplate'])) {
          continue;
        }

        $interesting = FALSE;

        /* Inspect members... */
        foreach (array_keys($ACLRule['members']) as $member) {
          /* Wildcard? */
          if ($member === 'G:*') {
            $interesting = TRUE;
            break;
          }

          list($memberType, $memberDn) = explode(':', $member, 2);
          switch ($memberType) {
            case 'G':
              if (in_array_ics($memberDn, $this->groups)) {
                $interesting = TRUE;
                break 2;
              }
              break;
            case 'R':
              if (in_array_ics($memberDn, $this->roles)) {
                $interesting = TRUE;
                break 2;
              }
              break;
            case 'U':
              if (mb_strtolower($memberDn) === mb_strtolower($this->dn)) {
                $interesting = TRUE;
                break 2;
              }
              break;
            default:
              throw FusionDirectoryException('Unknown ACL member type '.$memberType);
          }
        }

        if (!$interesting) {
          continue;
        }

        if (!empty($ACLRule['userfilter']) && !$ldap->object_match_filter($this->dn, $ACLRule['userfilter'])) {
          /* We do not match the user filter */
          continue;
        }

        if (!empty($ACLRule['targetfilter'])) {
          $ldap->cd($dn);
          $ldap->set_size_limit($targetFilterLimit);
          $targetFilter = templateHandling::parseString($ACLRule['targetfilter'], $this->cachedAttrs, 'ldap_escape_f');
          $ldap->search($targetFilter, ['dn']);
          if ($ldap->hitSizeLimit()) {
            msg_dialog::display(
              _('Error'),
              sprintf(
                _('An ACL assignment for the connected user matched more than than the %d objects limit. This user will not have the ACL rights he should.'),
                $targetFilterLimit
              ),
              ERROR_DIALOG
            );
          }
          $targetDns = [];
          while ($targetAttrs = $ldap->fetch()) {
            $targetDns[] = $targetAttrs['dn'];
          }
          $ldap->set_size_limit(0);
        } else {
          $targetDns = [$dn];
        }

        $roleAcls = acl::explodeRole($attrs['gosaAclTemplate']);
        foreach ($roleAcls as $roleAcl) {
          foreach ($targetDns as $targetDn) {
            $ACLsContentResolved[$targetDn][]  = [
              'acl'           => $roleAcl,
              'type'          => $ACLRule['type'],
              'members'       => $ACLRule['members'],
            ];
          }
        }
      }
    }

    /* Sort by tree depth */
    uksort(
      $ACLsContentResolved,
      function ($dn1, $dn2)
      {
        return substr_count($dn1, ',') <=> substr_count($dn2, ',');
      }
    );

    /* Insert in $this->ACL */
    foreach ($ACLsContentResolved as $dn => $ACLRules) {
      foreach ($ACLRules as $idx => $ACLRule) {
        if (!isset($this->ACL[$dn])) {
          $this->ACL[$dn] = [];
        }
        $this->ACL[$dn][$idx] = $ACLRule;
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
    $all_acl = [];
    foreach ($this->ACL as $dn => $acl) {
      $all_acl[$dn][$dn] = $acl;
      $sdn = $dn;
      while (strpos($dn, ',') !== FALSE) {
        $dn = preg_replace('/^[^,]*+,/', '', $dn);
        if (isset($this->ACL[$dn])) {
          $all_acl[$sdn][$dn] = array_filter(
            $this->ACL[$dn],
            function ($ACLInfos)
            {
              return ($ACLInfos['type'] === 'subtree');
            }
          );
        }
      }
    }
    $this->ACLperPath = $all_acl;

    /* Append Self entry */
    $dn = $this->dn;
    while (strpos($dn, ',') && !isset($all_acl[$dn])) {
      $dn = preg_replace('/^[^,]*+,/', '', $dn);
    }
    if (isset($all_acl[$dn])) {
      $this->ACLperPath[$this->dn] = $all_acl[$dn];
      if ($dn !== $this->dn) {
        $this->ACLperPath[$this->dn][$dn] = array_filter(
          $this->ACLperPath[$this->dn][$dn],
          function ($ACLInfos)
          {
            return ($ACLInfos['type'] === 'subtree');
          }
        );
      }
    }

    /* Reset plist menu and ACL cache if needed */
    if (is_object($plist)) {
      $plist->resetCache();
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
    return $this->get_permissions($dn, $category.'/0', '');
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
  function get_permissions ($dn, $object, $attribute = '', $skip_write = FALSE)
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

    /* Detect the set of ACLs we have to check for this object */
    $parentACLdn = $dn;
    while (!isset($this->ACLperPath[$parentACLdn]) && (strpos($parentACLdn, ',') !== FALSE)) {
      $parentACLdn = preg_replace('/^[^,]*+,/', '', $parentACLdn);
    }
    if (!isset($this->ACLperPath[$parentACLdn])) {
      $ACL_CACHE["$dn+$object+$attribute"] = '';
      return '';
    }

    if (($parentACLdn !== $dn) && isset($ACL_CACHE["sub:$parentACLdn+$object+$attribute"])) {
      /* Load parent subtree ACLs from cache */
      $permissions = $ACL_CACHE["sub:$parentACLdn+$object+$attribute"];
    } else {
      $permissions = new ACLPermissions();

      /* Merge relevent permissions from parent ACLs */
      foreach ($this->ACLperPath[$parentACLdn] as $parentdn => $ACLs) {
        /* Inspect this ACL, place the result into permissions */
        foreach ($ACLs as $subacl) {
          if ($permissions->isFull()) {
            /* Stop merging if we have all rights already */
            break 2;
          }

          if (($dn != $this->dn) && isset($subacl['acl'][$object][0]) && ($subacl['acl'][$object][0]->isSelf())) {
            /* Self ACL */
            continue;
          }

          if (($subacl['type'] === 'base') && ($parentdn !== $dn)) {
            /* Base assignment on another dn */
            continue;
          }

          /* Special global ACL */
          if (isset($subacl['acl']['all'][0])) {
            $permissions->merge($subacl['acl']['all'][0]);
          }

          /* Category ACLs (e.g. $object = "user/0") */
          if (strstr($object, '/0')) {
            $ocs = preg_replace("/\/0$/", '', $object);
            if (isset($config->data['CATEGORIES'][$ocs]) && ($attribute == '')) {
              foreach ($config->data['CATEGORIES'][$ocs]['classes'] as $oc) {
                if (isset($subacl['acl'][$ocs.'/'.$oc])) {
                  if (($dn != $this->dn) &&
                      isset($subacl['acl'][$ocs.'/'.$oc][0]) &&
                      ($subacl['acl'][$ocs.'/'.$oc][0]->isSelf())) {
                    /* Skip self ACL */
                    continue;
                  }

                  foreach ($subacl['acl'][$ocs.'/'.$oc] as $anyPermissions) {
                    $permissions->merge($anyPermissions);
                  }
                }
              }
            }
            continue;
          }

          /* If attribute is "", we want to know, if we've *any* permissions here...
              Merge global class ACLs [0] with attributes specific ACLs [attribute].
           */
          if (($attribute == '') && isset($subacl['acl'][$object])) {
            foreach ($subacl['acl'][$object] as $anyPermissions) {
              $permissions->merge($anyPermissions);
            }
            continue;
          }

          /* Per attribute ACL? */
          if (isset($subacl['acl'][$object][$attribute])) {
            $permissions->merge($subacl['acl'][$object][$attribute]);
          }

          /* Per object ACL? */
          if (isset($subacl['acl'][$object][0])) {
            $permissions->merge($subacl['acl'][$object][0]);
          }
        }
      }
    }

    if ($parentACLdn !== $dn) {
      $ACL_CACHE["sub:$parentACLdn+$object+$attribute"] = $permissions;
    }
    $ACL_CACHE["$dn+$object+$attribute"] = $permissions;

    /* Remove write if needed */
    return $permissions->toString($skip_write);
  }

  /*!
   * \brief Extract all departments that are accessible
   *
   * Extract all departments that are accessible (direct or 'on the way' to an
   * accessible department)
   *
   * \param string|array $module The module
   *
   * \param bool $skip_self_acls FALSE
   *
   * \return array Return all accessible departments
   */
  function get_module_departments ($module, bool $skip_self_acls = FALSE): array
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
            if ($skip_self_acls && isset($data[0]) && $data[0]->isSelf()) {
              continue;
            }
            if (preg_match('/^'.preg_quote($mod, '/').'/', $cat) || ($cat === 'all')) {
              /* $cat starts with $mod (example: cat=user/user and mod=user) or cat is 'all' */
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
      $tabs   = $infos['tabClass']::getPotentialTabList($type, $infos);
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

    /* Username is set, load ACLs now */
    $ui->loadACL();

    return $ui;
  }
}
