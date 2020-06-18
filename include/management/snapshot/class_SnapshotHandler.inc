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

/*
 * \file class_SnapshotHandler
 * Source code for class SnapshotHandler
 */

/*!
 * \brief This class contains all the function needed to handle
 * the snapshot functionality
 */
class SnapshotHandler
{
  protected $enabled;

  static function plInfo ()
  {
    return [
      'plShortName'   => _('Snapshot'),
      'plDescription' => _('Snapshot handler'),
      /* Categories for snapshots are computed in config class */
      'plCategory'    => [],

      'plProvidedAcls' => [
        'restore_over'    => _('Restore over an existing object'),
        'restore_deleted' => _('Restore a deleted object'),
      ]
    ];
  }

  /*!
   * \brief Create handler
   */
  function __construct ()
  {
    global $config;
    $this->enabled = $config->snapshotEnabled();
    if ($this->enabled) {
      /* Prepare base */
      $this->snapshotRDN = $config->get_cfg_value('snapshotBase');
      $ldap = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);
      try {
        $ldap->create_missing_trees($this->snapshotRDN);
      } catch (FusionDirectoryError $error) {
        $error->display();
      }
    }
  }

  /*!
   * \brief Check if the snapshot is enable
   *
   * \return boolean TRUE if is enable, return FALSE otherwise
   */
  function enabled ()
  {
    return $this->enabled;
  }

  /* \brief Get the snapshot dn of an object dn
   */
  protected function snapshot_dn ($dn)
  {
    global $config;
    return preg_replace("/".preg_quote($config->current['BASE'], '/')."$/", "", $dn)
            .$this->snapshotRDN;
  }

  /*!
   * \brief Check if there are deleted snapshots
   */
  function hasDeletedSnapshots ($bases)
  {
    foreach ($bases as $base) {
      if (count($this->getAllDeletedSnapshots($base)) > 0) {
        return TRUE;
      }
    }
    return FALSE;
  }

  /*!
   * \brief Cache Snapshot information for all objects in $base
   */
  function initSnapshotCache ($base)
  {
    global $config;
    if (!$this->enabled()) {
      return;
    }

    $ldap = $config->get_ldap_link();

    // Initialize base
    $base = $this->snapshot_dn($base);

    /* Fetch all objects with */
    $ldap->cd($base);
    $ldap->search('(&(objectClass=gosaSnapshotObject)(gosaSnapshotDN=*))', ['gosaSnapshotDN']);

    /* Store for which object we have snapshots */
    $this->snapshotsCache = [];
    while ($entry = $ldap->fetch()) {
      $this->snapshotsCache[$entry['gosaSnapshotDN'][0]] = TRUE;
    }
  }

  /*!
   * \brief Check if the DN has snapshots
   *
   * \return the numbers of snapshots
   */
  function hasSnapshots ($dn)
  {
    return isset($this->snapshotsCache[$dn]);
  }

  /*!
   * \brief Get snapshots
   *
   * \param string $dn The DN
   *
   * \param string $raw FALSE
   */
  function getSnapshots ($dn, $raw = FALSE)
  {
    global $config;
    if (!$this->enabled()) {
      return [];
    }

    $ldap = $config->get_ldap_link();

    $objectBase = preg_replace("/^[^,]*./", "", $dn);

    // Initialize base
    $base = $this->snapshot_dn($objectBase);

    /* Fetch all objects with  gosaSnapshotDN=$dn */
    $ldap->cd($base);
    $ldap->search(
      '(&(objectClass=gosaSnapshotObject)(gosaSnapshotDN='.ldap_escape_f($dn).'))',
      ['gosaSnapshotTimestamp','gosaSnapshotDN','description'],
      'one'
    );

    /* Put results into a list and add description if missing */
    $objects = [];
    while ($entry = $ldap->fetch(TRUE)) {
      if (!isset($entry['description'][0])) {
        $entry['description'][0]  = "";
      }
      $objects[] = $entry;
    }

    /* Return the raw array, or format the result */
    if ($raw) {
      return $objects;
    } else {
      $tmp = [];
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
   * \param string $description Snapshot description
   *
   * \param string $objectType Type of snapshotted object
   */
  function createSnapshot ($dn, string $description, string $objectType)
  {
    global $config;
    if (!$this->enabled()) {
      logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Snapshot are disabled but tried to create snapshot');
      return;
    }

    if (is_array($dn)) {
      $dns  = $dn;
      $dn   = $dns[0];
    } else {
      $dns = [$dn];
    }

    $ldap = $config->get_ldap_link();

    /* check if the dn exists */
    if (!$ldap->dn_exists($dn)) {
      logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Tried to snapshot non-existing dn');
      return;
    }

    /* Extract seconds & mysecs, they are used as entry index */
    list($usec, $sec) = explode(" ", microtime());

    /* Collect some infos */
    $base_of_object = preg_replace('/^[^,]+,/i', '', $dn);
    $new_base       = $this->snapshot_dn($base_of_object);
    /* Create object */
    $data = '';
    foreach ($dns as $tmp_dn) {
      $data .= $ldap->generateLdif($tmp_dn, '(!(objectClass=gosaDepartment))', 'sub');
      if (!$ldap->success()) {
        msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $tmp_dn, '', get_class()), LDAP_ERROR);
      }
    }

    $target = [];

    $target['objectClass']          = ['top', 'gosaSnapshotObject'];
    $target['gosaSnapshotData']     = gzcompress($data, 6);
    $target['gosaSnapshotDN']       = $dn;
    $target['description']          = $description;
    $target['fdSnapshotObjectType'] = $objectType;

    /* Insert the new snapshot
       But we have to check first, if the given gosaSnapshotTimestamp
       is already used, in this case we should increment this value till there is
       an unused value. */
    do {
      $target['gosaSnapshotTimestamp']  = str_replace('.', '', $sec.'-'.$usec);
      $new_dn                           = 'gosaSnapshotTimestamp='.$target['gosaSnapshotTimestamp'].','.$new_base;
      $ldap->cat($new_dn);
      $usec++;
    } while ($ldap->count());

    /* Insert this new snapshot */
    $ldap->cd($this->snapshotRDN);
    try {
      $ldap->create_missing_trees($this->snapshotRDN);
      $ldap->create_missing_trees($new_base);
    } catch (FusionDirectoryError $error) {
      $error->display();
    }
    $ldap->cd($new_dn);
    $ldap->add($target);

    if (!$ldap->success()) {
      msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $new_base, '', get_class()), LDAP_ERROR);
    }
    logging::log('snapshot', 'create', $new_dn, array_keys($target), $ldap->get_error());
  }

  /*!
   * \brief Remove a snapshot
   *
   * \param string $dn The DN
   */
  function removeSnapshot ($dn)
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->rmdir_recursive($dn);
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $dn), LDAP_ERROR);
    }
    logging::log('snapshot', 'delete', $dn, [], $ldap->get_error());
  }

  /*!
   * \brief Get the available snapshots
   *
   * \return available snapshots for the given base
   */
  function getAvailableSnapsShots ($dn)
  {
    global $config;
    if (!$this->enabled()) {
      return [];
    }

    $ldap = $config->get_ldap_link();

    /* Prepare bases and some other infos */
    $base_of_object = preg_replace('/^[^,]+,/i', '', $dn);
    $new_base       = $this->snapshot_dn($base_of_object);
    $tmp            = [];

    /* Fetch all objects with  gosaSnapshotDN=$dn */
    $ldap->cd($new_base);
    $ldap->search(
      '(&(objectClass=gosaSnapshotObject)(gosaSnapshotDN='.ldap_escape_f($dn).'))',
      ['gosaSnapshotTimestamp','gosaSnapshotDN','description','fdSnapshotObjectType'],
      'one'
    );

    /* Put results into a list and add description if missing */
    while ($entry = $ldap->fetch(TRUE)) {
      if (!isset($entry['description'][0])) {
        $entry['description'][0]  = "";
      }
      $tmp[] = $entry;
    }

    return $tmp;
  }

  /*!
   * \brief Get all deleted snapshots
   *
   * \param string $base_of_object
   */
  function getAllDeletedSnapshots ($base_of_object)
  {
    global $config;
    if (!$this->enabled()) {
      return [];
    }

    $ldap = $config->get_ldap_link();

    /* Prepare bases */
    $new_base       = $this->snapshot_dn($base_of_object);
    /* Fetch all objects and check if they do not exist anymore */
    $tmp  = [];
    $ldap->cd($new_base);
    $ldap->search(
      '(objectClass=gosaSnapshotObject)',
      ['gosaSnapshotTimestamp','gosaSnapshotDN','description','fdSnapshotObjectType'],
      'one'
    );
    while ($entry = $ldap->fetch(TRUE)) {
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
      if ($ldap->dn_exists($entry['gosaSnapshotDN'][0])) {
        unset($tmp[$key]);
      }
    }

    return $tmp;
  }


  /*!
   * \brief Restore selected snapshot
   *
   * \param string $dn The DN
   */
  function restoreSnapshot ($dn)
  {
    global $config;
    if (!$this->enabled()) {
      logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Snapshot are disabled but tried to restore snapshot');
      return FALSE;
    }

    $ldap = $config->get_ldap_link();

    /* Get the snapshot */
    $ldap->cat($dn, ['gosaSnapshotData','gosaSnapshotDN'], '(gosaSnapshotData=*)');
    if ($attrs = $ldap->fetch()) {
      /* Prepare import string */
      $data = gzuncompress($attrs['gosaSnapshotData'][0]);
      if ($data === FALSE) {
        msg_dialog::display(_('Error'), _('There was a problem uncompressing snapshot data'), ERROR_DIALOG);
        return FALSE;
      }
    } else {
      msg_dialog::display(_('Error'), _('Snapshot data could not be fetched'), ERROR_DIALOG);
      return FALSE;
    }

    /* Import the given data */
    try {
      $ldap->import_complete_ldif($data, FALSE, FALSE);
      logging::log('snapshot', 'restore', $dn, [], $ldap->get_error());
      if (!$ldap->success()) {
        msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $dn, '', get_class()), LDAP_ERROR);
        return FALSE;
      }
      return $attrs['gosaSnapshotDN'][0];
    } catch (LDIFImportException $e) {
      msg_dialog::display(_('LDAP error'), $e->getMessage(), ERROR_DIALOG);
      logging::log('snapshot', 'restore', $dn, [], $e->getMessage());
      return FALSE;
    }
  }
}
