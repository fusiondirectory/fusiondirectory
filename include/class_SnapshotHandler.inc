<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

/*
 * \file class_SnapshotHandler
 * Source code for class SnapshotHandler
 */

/*!
 * \brief This class contains all the function needed to handle
 * the snapshot functionality
 */
class SnapshotHandler {
  var $config;
  var $isEnabled      = FALSE;
  var $snapshotBases  = array();

  /*!
   * \brief Create handler
   *
   * \param config $config
   */
  function __construct(&$config)
  {
    $this->config     = $config;
    $this->isEnabled  = $this->config->snapshotEnabled();
  }

  /*!
   * \brief Check if the snapshot is enable
   *
   * \return boolean TRUE if is enable, return FALSE otherwise
   */
  function enabled()
  {
    return $this->isEnabled;
  }

  /*!
   * \brief Set a new snapshot bases
   *
   * \param array $bases
   */
  function setSnapshotBases($bases)
  {
    $this->snapshotBases = $bases;
  }

  /*!
   * \brief Get the current snapshot bases
   *
   * \return array $bases
   */
  function getSnapshotBases()
  {
    return $this->snapshotBases;
  }

  /*!
   * \brief Get a snapshot LDAP Multiplexer
   */
  protected function get_snapshot_link($default = NULL)
  {
    $snapshotLdap = $default;

    /* check if there are special server configurations for snapshots */
    if ($this->config->get_cfg_value("snapshotURI") != "") {
      $server       = $this->config->get_cfg_value("snapshotURI");
      $user         = $this->config->get_cfg_value("snapshotAdminDn");
      $password     = $this->config->get_credentials($this->config->get_cfg_value("snapshotAdminPassword"));
      $snapshotLdap = new ldapMultiplexer(new LDAP($user, $password, $server));
    }

    /* Prepare bases */
    $this->snapshotLdapBase = $this->config->get_cfg_value("snapshotBase");
    $snapshotLdap->cd($this->snapshotLdapBase);
    if (!$snapshotLdap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($snapshotLdap->get_error(), $this->snapshotLdapBase, "", get_class()));
    }

    return $snapshotLdap;
  }

  /* \brief Get an LDAP link and a snapshotlink
   * */
  protected function get_ldap_links()
  {
    /* Create an additional ldap object which
       points to our ldap snapshot server */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    return array($ldap, $this->get_snapshot_link($ldap));
  }

  /* \brief Get the snapshot dn of an object dn
   */
  protected function snapshot_dn($dn)
  {
    return preg_replace("/".preg_quote($this->config->current['BASE'], '/')."$/", "", $dn)
            .$this->snapshotLdapBase;
  }

  /*!
   * \brief Get the deleted snapshots
   *
   * \param string $objectBase
   *
   * \param boolean $raw FALSE
   */
  function getDeletedSnapshots($objectBase, $raw = FALSE)
  {
    if (!$this->enabled()) {
      return array();
    }

    // Load user info
    $ui = get_userinfo();

    list($ldap, $snapshotLdap) = $this->get_ldap_links();

    // Initialize base
    $base = $this->snapshot_dn($objectBase);

    /* Fetch all objects and check if they do not exist anymore */
    $objects = array();
    $snapshotLdap->cd($base);
    $snapshotLdap->ls("(objectClass=gosaSnapshotObject)", $base,
                      array("gosaSnapshotType", "gosaSnapshotTimestamp", "gosaSnapshotDN", "description"));
    while ($entry = $snapshotLdap->fetch()) {

      $chk = str_replace($base, "", $entry['dn']);
      if (preg_match("/,ou=/", $chk)) {
        continue;
      }

      if (!isset($entry['description'][0])) {
        $entry['description'][0]  = "";
      }
      $objects[] = $entry;
    }

    /* Check if entry still exists */
    foreach ($objects as $key => $entry) {
      $ldap->cat($entry['gosaSnapshotDN'][0]);
      if ($ldap->count()) {
        unset($objects[$key]);
      }
    }

    /* Format result as requested */
    if ($raw) {
      return $objects;
    } else {
      $tmp = array();
      foreach ($objects as $key => $entry) {
        $tmp[base64_encode($entry['dn'])] = $entry['description'][0];
      }
    }
    return $tmp;
  }

  /*!
   * \brief Check if the DN has snapshots
   *
   * \return the numbers of snapshots
   */
  function hasSnapshots($dn)
  {
    return (count($this->getSnapshots($dn)) > 0);
  }

  /*!
   * \brief Get snapshots
   *
   * \param string $dn The DN
   *
   * \param string $raw FALSE
   */
  function getSnapshots($dn, $raw = FALSE)
  {
    if (!$this->enabled()) {
      return array();
    }

    list(, $snapshotLdap) = $this->get_ldap_links();

    $objectBase = preg_replace("/^[^,]*./", "", $dn);

    // Initialize base
    $base = $this->snapshot_dn($objectBase);

    /* Fetch all objects with  gosaSnapshotDN=$dn */
    $snapshotLdap->cd($base);
    $snapshotLdap->ls("(&(objectClass=gosaSnapshotObject)(gosaSnapshotDN=".$dn."))", $base,
        array("gosaSnapshotType","gosaSnapshotTimestamp","gosaSnapshotDN","description"));

    /* Put results into a list and add description if missing */
    $objects = array();
    while ($entry = $snapshotLdap->fetch()) {
      if (!isset($entry['description'][0])) {
        $entry['description'][0]  = "";
      }
      $objects[] = $entry;
    }

    /* Return the raw array, or format the result */
    if ($raw) {
      return $objects;
    } else {
      $tmp = array();
      foreach ($objects as $entry) {
        $tmp[base64_encode($entry['dn'])] = $entry['description'][0];
      }
    }
    return $tmp;
  }


  /*!
   * \brief Create a snapshot of the current object
   *
   * \param string $dn The DN
   *
   * \param array $description Snapshot description
   */
  function create_snapshot($dn, $description = array())
  {
    if (!$this->enabled()) {
      return;
    }

    if (is_array($dn)) {
      $dns  = $dn;
      $dn   = $dns[0];
    } else {
      $dns = array($dn);
    }

    /* Get configuration from fusiondirectory.conf */
    $config = $this->config;

    list($ldap, $ldap_to) = $this->get_ldap_links();

    /* check if the dn exists */
    if ($ldap->dn_exists($dn)) {

      /* Extract seconds & mysecs, they are used as entry index */
      list($usec, $sec) = explode(" ", microtime());

      /* Collect some infos */
      $base_of_object = preg_replace ('/^[^,]+,/i', '', $dn);
      $new_base       = $this->snapshot_dn($base_of_object);
      /* Create object */
      $data = '';
      foreach ($dns as $tmp_dn) {
        $data .= $ldap->generateLdif(LDAP::fix($tmp_dn),
              '(&(!(objectClass=gosaDepartment))(!(objectClass=FAIclass)))', array(), 'sub');
      }

      $newName  = str_replace(".", "", $sec."-".$usec);
      $target   = array();

      $target['objectClass']            = array("top", "gosaSnapshotObject");
      $target['gosaSnapshotData']       = gzcompress($data, 6);
      $target['gosaSnapshotType']       = "snapshot";
      $target['gosaSnapshotDN']         = $dn;
      $target['description']            = $description;
      $target['gosaSnapshotTimestamp']  = $newName;

      /* Insert the new snapshot
         But we have to check first, if the given gosaSnapshotTimestamp
         is already used, in this case we should increment this value till there is
         an unused value. */
      $new_dn                           = "gosaSnapshotTimestamp=".$newName.",".$new_base;
      $ldap_to->cat($new_dn);
      while ($ldap_to->count()) {
        $ldap_to->cat($new_dn);
        $newName = str_replace(".", "", $sec."-".(++$usec));
        $new_dn                           = "gosaSnapshotTimestamp=".$newName.",".$new_base;
        $target['gosaSnapshotTimestamp']  = $newName;
      }
      /* Inset this new snapshot */
      $ldap_to->cd($this->snapshotLdapBase);
      $ldap_to->create_missing_trees($this->snapshotLdapBase);
      $ldap_to->create_missing_trees($new_base);
      $ldap_to->cd($new_dn);
      $ldap_to->add($target);
      if (!$ldap_to->success()) {
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap_to->get_error(), $new_dn, LDAP_ADD, get_class()));
      }

      if (!$ldap->success()) {
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $new_base, "", get_class()));
      }

    }
  }

  /*!
   * \brief Remove a snapshot
   *
   * \param string $dn The DN
   */
  function remove_snapshot($dn)
  {
    $ui       = get_userinfo();
    $old_dn   = $this->dn;
    $this->dn = $dn;
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->rmdir_recursive($this->dn);
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn));
    }
    $this->dn = $old_dn;
  }

  /*!
   * \brief Get the available snapshots
   *
   * \return available snapshots for the given base
   */
  function Available_SnapsShots($dn, $raw = FALSE)
  {
    if (!$this->enabled()) {
      return array();
    }

    list (, $ldap_to) = $this->get_ldap_links();

    /* Prepare bases and some other infos */
    $base_of_object = preg_replace ('/^[^,]+,/i', '', $dn);
    $new_base       = $this->snapshot_dn($base_of_object);
    $tmp            = array();

    /* Fetch all objects with  gosaSnapshotDN=$dn */
    $ldap_to->cd($new_base);
    $ldap_to->ls("(&(objectClass=gosaSnapshotObject)(gosaSnapshotDN=".$dn."))", $new_base,
        array("gosaSnapshotType","gosaSnapshotTimestamp","gosaSnapshotDN","description"));

    /* Put results into a list and add description if missing */
    while ($entry = $ldap_to->fetch()) {
      if (!isset($entry['description'][0])) {
        $entry['description'][0]  = "";
      }
      $tmp[] = $entry;
    }

    /* Return the raw array, or format the result */
    if ($raw) {
      return $tmp;
    } else {
      $tmp2 = array();
      foreach ($tmp as $entry) {
        $tmp2[base64_encode($entry['dn'])] = $entry['description'][0];
      }
    }
    return $tmp2;
  }

  /*!
   * \brief Get all deleted snapshots
   *
   * \param string $base_of_object
   *
   * \param boolean $raw FALSE
   */
  function getAllDeletedSnapshots($base_of_object, $raw = FALSE)
  {
    if (!$this->enabled()) {
      return array();
    }

    list ($ldap, $ldap_to) = $this->get_ldap_links();

    /* Prepare bases */
    $new_base       = $this->snapshot_dn($base_of_object);
    /* Fetch all objects and check if they do not exist anymore */
    $ui   = get_userinfo();
    $tmp  = array();
    $ldap_to->cd($new_base);
    $ldap_to->ls("(objectClass=gosaSnapshotObject)", $new_base, array("gosaSnapshotType","gosaSnapshotTimestamp","gosaSnapshotDN","description"));
    while ($entry = $ldap_to->fetch()) {

      $chk = str_replace($new_base, "", $entry['dn']);
      if (preg_match("/,ou=/", $chk)) {
        continue;
      }

      if (!isset($entry['description'][0])) {
        $entry['description'][0]  = "";
      }
      $tmp[] = $entry;
    }

    /* Check if entry still exists */
    foreach ($tmp as $key => $entry) {
      $ldap->cat($entry['gosaSnapshotDN'][0]);
      if ($ldap->count()) {
        unset($tmp[$key]);
      }
    }

    /* Format result as requested */
    if ($raw) {
      return $tmp;
    } else {
      $tmp2 = array();
      foreach ($tmp as $key => $entry) {
        $tmp2[base64_encode($entry['dn'])] = $entry['description'][0];
      }
    }
    return $tmp2;
  }


  /*!
   * \brief Restore selected snapshot
   *
   * \param string $dn The DN
   */
  function restore_snapshot($dn)
  {
    if (!$this->enabled()) {
      return array();
    }

    list ($ldap, $ldap_to) = $this->get_ldap_links();

    /* Get the snapshot */
    $ldap_to->cat($dn);
    $restoreObject = $ldap_to->fetch();

    /* Prepare import string */
    $data = gzuncompress($ldap_to->get_attribute($dn, 'gosaSnapshotData'));

    /* Import the given data */
    try {
      $ldap->import_complete_ldif($data, FALSE, FALSE);
    } catch(Exception $e) {
      msg_dialog::display(_('LDAP error'), $e->getMessage(), ERROR_DIALOG);
    }
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $dn, "", get_class()));
    }
  }
}
?>
