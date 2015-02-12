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
 * \see class_plugin
 */
class acl extends plugin
{
  /* attribute list for save action */
  var $attributes     = array('gosaAclEntry');
  var $objectclasses  = array('gosaAcl');

  /* Helpers */
  var $dialogState      = "head";
  var $gosaAclEntry     = array();
  var $aclType          = "";
  var $aclObject        = "";
  var $aclContents      = array();
  var $aclFilter        = "";
  var $aclMyObjects     = array();
  var $roles            = array();
  var $recipients       = array();
  var $currentIndex     = 0;
  var $wasNewEntry      = FALSE;
  var $savedAclContents = array();
  var $acl_category     = "acl/";

  /*!
   * \brief Acl contructor
   *
   * \param String $config Configuration file for ACL
   *
   * \param String $parent
   *
   * \param String $dn The DN
   */
  function acl (&$config, $dn = NULL, $baseobject = NULL)
  {
    /* Include config object */
    parent::__construct($config, $dn, $baseobject);

    /* Load ACL's */
    $this->gosaAclEntry = array();
    if (isset($this->attrs['gosaAclEntry'])) {
      for ($i = 0; $i < $this->attrs['gosaAclEntry']['count']; $i++) {
        $acl = $this->attrs['gosaAclEntry'][$i];
        $this->gosaAclEntry = array_merge($this->gosaAclEntry, acl::explodeACL($acl));
      }
    }
    ksort($this->gosaAclEntry);

    /* Save parent - we've to know more about it than other plugins... */
    if (($baseobject !== NULL) && isset($baseobject->parent)) {
      $this->parent =& $baseobject->parent;
    }

    $ldap = $config->get_ldap_link();

    /* Roles TODO - use objects::ls?*/
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=gosaRole)', array('cn', 'description','gosaAclTemplate','dn'));
    while ($attrs = $ldap->fetch()) {
      $role_id = $attrs['dn'];

      $this->roles[$role_id]['acls']  = acl::explodeRole($attrs['gosaAclTemplate']);
      $this->roles[$role_id]['cn']    = $attrs['cn'][0];
      if (isset($attrs['description'][0])) {
        $this->roles[$role_id]['description'] = $attrs['description'][0];
      }
    }

    /* Finally - we want to get saved... */
    $this->is_account = TRUE;
  }

  /*!
   *  \brief Function sort an array by elements priority
   *
   *  \param Array $list Array to be sorted
   */
  function sort_by_priority($list)
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
          if (in_array_ics('gosaAccount', $attrs['objectClass'])) {
            $a['U:'.$dn] = $attrs['cn'][0]." [".$attrs['uid'][0]."]";
          } else {
            $a['G:'.$dn] = $attrs['cn'][0];
            if (isset($attrs['description'][0])) {
              $a['G:'.$dn] .= " [".$attrs['description'][0]."]";
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

  /*!
   * \brief Prepare for Copy & Paste
   *
   * \see plugin::PrepareForCopyPaste($source)
   *
   * \param string $source Source to prepare for copy and paste
   */
  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);

    $dn                 = $source['dn'];
    $acl_c              = new acl($this->config, $dn);
    $this->gosaAclEntry = $acl_c->gosaAclEntry;
  }

  /*!
   * \brief Removes object from parent
   */
  function remove_from_parent()
  {
    parent::remove_from_parent();

    /* include global link_info */
    $ldap = $this->config->get_ldap_link();

    $ldap->cd($this->dn);
    $this->cleanup();
    $ldap->modify($this->attrs);

    new log('remove', 'acls/'.get_class($this), $this->dn, array_keys($this->attrs), $ldap->get_error());

    /* Optionally execute a command after we're done */
    $this->handle_post_events('remove');
  }


  /*
   * \brief Return plugin informations for acl handling
   */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('ACL'),
      'plDescription' => _('Manage access control lists'),
      'plSelfModify'  => FALSE,
      'plPriority'    => 0,
      'plCategory'    => array('acl' => array('description'  => _('ACL').'&nbsp;&amp;&nbsp;'._('ACL roles'),
                                                      'objectClass'  => array('gosaAcl','gosaRole'))),
      'plObjectType'  => array(),

      'plProvidedAcls'  => array()
    );
  }


  /*!
   * \brief Remove acls defined for $src
   */
  function remove_acl()
  {
    acl::remove_acl_for($this->dn);
  }


  /*!
   * \brief Remove acls defined for $src
   *
   * \param String $dn The DN
   *
   * FIXME
   */
  static function remove_acl_for($dn)
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search("(&(objectClass=gosaAcl)(gosaAclEntry=*".base64_encode($dn)."*))", array("gosaAclEntry","dn"));
    $new_entries = array();
    while ($attrs = $ldap->fetch()) {
      if (!isset($attrs['gosaAclEntry'])) {
        continue;
      }
      unset($attrs['gosaAclEntry']['count']);

      // Remove entry directly
      foreach ($attrs['gosaAclEntry'] as $entry) {
        $parts        = explode(':', $entry);
        $members      = explode(',', $parts[2]);
        $new_members  = array();
        foreach ($members as $member) {
          if (base64_decode($member) != $dn) {
            $new_members[] = $member;
          } else {
            fusiondirectory_log("modify", "user/acl", $attrs['dn'], array(), sprintf("Removed acl for %s on object %s.", $dn, $attrs['dn']));
          }
        }

        /* We can completely remove the entry if there are no members anymore */
        if (count($new_members)) {
          $parts[2]       = implode(",", $new_members);
          $new_entries[]  = implode(":", $parts);
        }
      }

      // There should be a modification, so write it back
      $ldap->cd($attrs['dn']);
      $new_attrs = array("gosaAclEntry" => $new_entries);
      $ldap->modify($new_attrs);
      if (!$ldap->success()) {
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_MOD, get_class()), ERROR_DIALOG);
      }
    }
  }

  /*!
   * \brief Update the acl membership
   *
   * \param String $src The source
   *
   * \param String $dst The destination
   */
  static function update_acl_membership($src, $dst)
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search("(&(objectClass=gosaAcl)(gosaAclEntry=*".base64_encode($src)."*))", array("gosaAclEntry","dn"));
    while ($attrs = $ldap->fetch()) {
      $acl = new acl($config, $attrs['dn']);
      foreach ($acl->gosaAclEntry as $id => $entry) {
        foreach ($entry['members'] as $m_id => $member) {
          if ($m_id == "U:".$src) {
            unset($acl->gosaAclEntry[$id]['members'][$m_id]);
            $new = "U:".$dst;

            $acl->gosaAclEntry[$id]['members'][$new] = $new;
            fusiondirectory_log("modify", "user/acl", $attrs['dn'], array(), sprintf("Updated acl for user %s on object %s.", $src, $attrs['dn']));
          }
          if ($m_id == "G:".$src) {
            unset($acl->gosaAclEntry[$id]['members'][$m_id]);
            $new = "G:".$dst;

            $acl->gosaAclEntry[$id]['members'][$new] = $new;
            fusiondirectory_log("modify", "group/acl", $attrs['dn'], array(), sprintf("Updated acl for group %s on object %s.", $src, $attrs['dn']));
          }
        }
      }
      $acl->save();
    }
  }
}

?>
