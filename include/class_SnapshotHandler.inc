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
  var $isEnabled= false;
  var $snapshotBases= array();


  /*!
   * \brief Create handler
   *
   * \param ŝtring $config
   */
  function SnapshotHandler(&$config)
  {
    $this->config = &$config;
    $config = $this->config;

    if($config->get_cfg_value("enableSnapshots") == "TRUE"){

      /* Check if the snapshot_base is defined */
      if ($config->get_cfg_value("snapshotBase") == ""){

        /* Send message if not done already */
        if(!session::is_set("snapshotFailMessageSend")){
          session::set("snapshotFailMessageSend",TRUE);
          msg_dialog::display(_("Configuration error"),
              sprintf(_("The snapshot functionality is enabled, but the required variable '%s' is not set."),
                "snapshotBase"), ERROR_DIALOG);
        }
        return;
      }

      /* Check if the snapshot_base is defined */
      if (!is_callable("gzcompress")){

        /* Send message if not done already */
        if(!session::is_set("snapshotFailMessageSend")){
          session::set("snapshotFailMessageSend",TRUE);
          msg_dialog::display(_("Configuration error"),
              sprintf(_("The snapshot functionality is enabled, but the required compression module is missing. Please install '%s'."),"php5-zip / php5-gzip"), ERROR_DIALOG);
        }
        return;
      }

      /* check if there are special server configurations for snapshots */
      if ($config->get_cfg_value("snapshotURI") != ""){

        /* check if all required vars are available to create a new ldap connection */
        $missing = "";
        foreach(array("snapshotURI","snapshotAdminDn","snapshotAdminPassword","snapshotBase") as $var){
          if($config->get_cfg_value($var) == ""){
            $missing .= $var." ";

            /* Send message if not done already */
            if(!session::is_set("snapshotFailMessageSend")){
              session::set("snapshotFailMessageSend",TRUE);
              msg_dialog::display(_("Configuration error"),
                  sprintf(_("The snapshot functionality is enabled, but the required variable '%s' is not set."),
                    $missing), ERROR_DIALOG);
            }
            return;
          }
        }
      }
      $this->isEnabled= true;
      return;
    }
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
    $this->snapshotBases= $bases;
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
  function get_snapshot_link()
  {
    $snapshotLdap= null;

    /* check if there are special server configurations for snapshots */
    if($this->config->get_cfg_value("snapshotURI") != ""){
      $server= $this->config->get_cfg_value("snapshotURI");
      $user= $this->config->get_cfg_value("snapshotAdminDn");
      $password= $this->config->get_credentials($this->config->get_cfg_value("snapshotAdminPassword"));
      $snapshotLdap= new ldapMultiplexer(new LDAP($user,$password, $server));
    }

    /* Prepare bases */
    $this->snapshotLdapBase= $this->config->get_cfg_value("snapshotBase");
    $snapshotLdap->cd($this->snapshotLdapBase);
    if (!$snapshotLdap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($snapshotLdap->get_error(), $this->snapshotLdapBase, "", get_class()));
    }

    return $snapshotLdap;
  }

  /*!
   * \brief Get the deleted snapshots
   *
   * \param string $objectBase
   *
   * \param boolean $raw false
   */
  function getDeletedSnapshots($objectBase, $raw= false)
  {
    // Skip if not enabled
    if(!$this->enabled()){
      return(array());
    }

    // Load user info
    $ui= get_userinfo();

    /* Create an additional ldap object which
       points to our ldap snapshot server */
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $snapshotLdap= $this->get_snapshot_link();
    if (!$snapshotLdap) {
      $snapshotLdap= $ldap;
    }

    // Initialize base
    $base= preg_replace("/".preg_quote($this->config->current['BASE'], '/')."$/",
                        "", $objectBase).$this->snapshotLdapBase;

    /* Fetch all objects and check if they do not exist anymore */
    $objects= array();
    $snapshotLdap->cd($base);
    $snapshotLdap->ls("(objectClass=gosaSnapshotObject)", $base,
                      array("gosaSnapshotType", "gosaSnapshotTimestamp", "gosaSnapshotDN", "description"));
    while($entry = $snapshotLdap->fetch()){

      $chk =  str_replace($base,"",$entry['dn']);
      if(preg_match("/,ou=/",$chk)) continue;

      if(!isset($entry['description'][0])){
        $entry['description'][0]  = "";
      }
      $objects[] = $entry;
    }

    /* Check if entry still exists */
    foreach($objects as $key => $entry){
      $ldap->cat($entry['gosaSnapshotDN'][0]);
      if($ldap->count()){
        unset($objects[$key]);
      }
    }

    /* Format result as requested */
    if($raw) {
      return($objects);
    }else{
      $tmp = array();
      foreach($objects as $key => $entry){
        $tmp[base64_encode($entry['dn'])] = $entry['description'][0];
      }
    }
    return($tmp);
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
   * \param string $raw false
   */
  function getSnapshots($dn, $raw= false)
  {
    // Empty if disabled
    if(!$this->enabled()){
      return(array());
    }

    /* Create an additional ldap object which
       points to our ldap snapshot server */
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);

    // Load snapshot LDAP connection
    $snapshotLdap= $this->get_snapshot_link();
    if (!$snapshotLdap) {
      $snapshotLdap= $ldap;
    }

    $objectBase= preg_replace("/^[^,]*./","",$dn);

    // Initialize base
    $base= preg_replace("/".preg_quote($this->config->current['BASE'], '/')."$/",
                        "", $objectBase).$this->snapshotLdapBase;

    /* Fetch all objects with  gosaSnapshotDN=$dn */
    $snapshotLdap->cd($base);
    $snapshotLdap->ls("(&(objectClass=gosaSnapshotObject)(gosaSnapshotDN=".$dn."))",$base,
        array("gosaSnapshotType","gosaSnapshotTimestamp","gosaSnapshotDN","description"));

    /* Put results into a list and add description if missing */
    $objects= array();
    while($entry = $snapshotLdap->fetch()){
      if(!isset($entry['description'][0])){
        $entry['description'][0]  = "";
      }
      $objects[] = $entry;
    }

    /* Return the raw array, or format the result */
    if($raw){
      return($objects);
    }else{
      $tmp = array();
      foreach($objects as $entry){
        $tmp[base64_encode($entry['dn'])] = $entry['description'][0];
      }
    }
    return($tmp);
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
    if (is_array($dn)) {
      $dns  = $dn;
      $dn   = $dns[0];
    } else {
      $dns = array($dn);
    }

    /* Check if snapshot functionality is enabled */
    if (!$this->snapshotEnabled()) {
      return;
    }

    /* Get configuration from fusiondirectory.conf */
    $config = $this->config;

    /* Create lokal ldap connection */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);

    /* check if there are special server configurations for snapshots */
    if ($config->get_cfg_value("snapshotURI") == "") {

      /* Source and destination server are both the same, just copy source to dest obj */
      $ldap_to      = $ldap;
      $snapldapbase = $this->config->current['BASE'];

    } else {
      $server         = $config->get_cfg_value("snapshotURI");
      $user           = $config->get_cfg_value("snapshotAdminDn");
      $password       = $this->config->get_credentials($config->get_cfg_value("snapshotAdminPassword"));
      $snapldapbase   = $config->get_cfg_value("snapshotBase");

      $ldap_to        = new ldapMultiplexer(new LDAP($user, $password, $server));
      $ldap_to -> cd($snapldapbase);

      if (!$ldap_to->success()) {
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap_to->get_error(), $snapldapbase, "", get_class()));
      }

    }

    /* check if the dn exists */
    if ($ldap->dn_exists($dn)) {

      /* Extract seconds & mysecs, they are used as entry index */
      list($usec, $sec) = explode(" ", microtime());

      /* Collect some infos */
      $base           = $this->config->current['BASE'];
      $snap_base      = $config->get_cfg_value("snapshotBase");
      $base_of_object = preg_replace ('/^[^,]+,/i', '', $dn);
      $new_base       = preg_replace("/".preg_quote($base, '/')."$/", "", $base_of_object).$snap_base;

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
      $ldap_to->cd($snapldapbase);
      $ldap_to->create_missing_trees($snapldapbase);
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
    if(!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn));
    }
    $this->dn = $old_dn;
  }

 /*!
  * \brief Check if the snapshot is enabled
  *
  * \return true if snapshots are enabled, and false if it is disalbed
  *   There will also be some errors psoted, if the configuration failed
  */
  function snapshotEnabled()
  {
    return $this->config->snapshotEnabled();
  }


  /*!
   * \brief Get the available snapshots
   *
   * \return available snapshots for the given base
   */
  function Available_SnapsShots($dn,$raw = false)
  {
    if(!$this->snapshotEnabled()) return(array());

    /* Create an additional ldap object which
       points to our ldap snapshot server */
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $cfg= &$this->config->current;

    /* check if there are special server configurations for snapshots */
    if($this->config->get_cfg_value("snapshotURI") == ""){
      $ldap_to      = $ldap;
    }else{
      $server         = $this->config->get_cfg_value("snapshotURI");
      $user           = $this->config->get_cfg_value("snapshotAdminDn");
      $password       = $this->config->get_credentials($this->config->get_cfg_value("snapshotAdminPassword"));
      $snapldapbase   = $this->config->get_cfg_value("snapshotBase");
      $ldap_to        = new ldapMultiplexer(new LDAP($user,$password, $server));
      $ldap_to -> cd($snapldapbase);
      if (!$ldap_to->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap_to->get_error(), $snapldapbase, "", get_class()));
      }
    }
    /* Prepare bases and some other infos */
    $base           = $this->config->current['BASE'];
    $snap_base      = $this->config->get_cfg_value("snapshotBase");
    $base_of_object = preg_replace ('/^[^,]+,/i', '', $dn);
    $new_base       = preg_replace("/".preg_quote($base, '/')."$/","",$base_of_object).$snap_base;
    $tmp            = array();

    /* Fetch all objects with  gosaSnapshotDN=$dn */
    $ldap_to->cd($new_base);
    $ldap_to->ls("(&(objectClass=gosaSnapshotObject)(gosaSnapshotDN=".$dn."))",$new_base,
        array("gosaSnapshotType","gosaSnapshotTimestamp","gosaSnapshotDN","description"));

    /* Put results into a list and add description if missing */
    while($entry = $ldap_to->fetch()){
      if(!isset($entry['description'][0])){
        $entry['description'][0]  = "";
      }
      $tmp[] = $entry;
    }

    /* Return the raw array, or format the result */
    if($raw){
      return($tmp);
    }else{
      $tmp2 = array();
      foreach($tmp as $entry){
        $tmp2[base64_encode($entry['dn'])] = $entry['description'][0];
      }
    }
    return($tmp2);
  }

  /*!
   * \brief Get all deleted snapshots
   *
   * \param string $base_of_object
   *
   * \param boolean $raw false
   */
  function getAllDeletedSnapshots($base_of_object,$raw = false)
  {
    if(!$this->snapshotEnabled()) return(array());

    /* Create an additional ldap object which
       points to our ldap snapshot server */
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $cfg= &$this->config->current;

    /* check if there are special server configurations for snapshots */
    if($this->config->get_cfg_value("snapshotURI") == ""){
      $ldap_to      = $ldap;
    }else{
      $server         = $this->config->get_cfg_value("snapshotURI");
      $user           = $this->config->get_cfg_value("snapshotAdminDn");
      $password       = $this->config->get_credentials($this->config->get_cfg_value("snapshotAdminPassword"));
      $snapldapbase   = $this->config->get_cfg_value("snapshotBase");
      $ldap_to        = new ldapMultiplexer(new LDAP($user,$password, $server));
      $ldap_to -> cd($snapldapbase);
      if (!$ldap_to->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap_to->get_error(), $snapldapbase, "", get_class()));
      }
    }

    /* Prepare bases */
    $base           = $this->config->current['BASE'];
    $snap_base      = $this->config->get_cfg_value("snapshotBase");
    $new_base       = preg_replace("/".preg_quote($base, '/')."$/","",$base_of_object).$snap_base;
  /* Fetch all objects and check if they do not exist anymore */
    $ui = get_userinfo();
    $tmp = array();
    $ldap_to->cd($new_base);
    $ldap_to->ls("(objectClass=gosaSnapshotObject)",$new_base,array("gosaSnapshotType","gosaSnapshotTimestamp","gosaSnapshotDN","description"));
    while($entry = $ldap_to->fetch()){

      $chk =  str_replace($new_base,"",$entry['dn']);
      if(preg_match("/,ou=/",$chk)) continue;

      if(!isset($entry['description'][0])){
        $entry['description'][0]  = "";
      }
      $tmp[] = $entry;
    }

    /* Check if entry still exists */
    foreach($tmp as $key => $entry){
      $ldap->cat($entry['gosaSnapshotDN'][0]);
      if($ldap->count()){
        unset($tmp[$key]);
      }
    }

    /* Format result as requested */
    if($raw) {
      return($tmp);
    }else{
      $tmp2 = array();
      foreach($tmp as $key => $entry){
        $tmp2[base64_encode($entry['dn'])] = $entry['description'][0];
      }
    }
    return($tmp2);
  }


  /*!
   * \brief Restore selected snapshot
   *
   * \param string $dn The DN
   */
  function restore_snapshot($dn)
  {
    if(!$this->snapshotEnabled()) return(array());

    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $cfg= &$this->config->current;

    /* check if there are special server configurations for snapshots */
    if($this->config->get_cfg_value("snapshotURI") == ""){
      $ldap_to      = $ldap;
    }else{
      $server         = $this->config->get_cfg_value("snapshotURI");
      $user           = $this->config->get_cfg_value("snapshotAdminDn");
      $password       = $this->config->get_credentials($this->config->get_cfg_value("snapshotAdminPassword"));
      $snapldapbase   = $this->config->get_cfg_value("snapshotBase");
      $ldap_to        = new ldapMultiplexer(new LDAP($user,$password, $server));
      $ldap_to -> cd($snapldapbase);
      if (!$ldap_to->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap_to->get_error(), $snapldapbase, "", get_class()));
      }
    }

    /* Get the snapshot */
    $ldap_to->cat($dn);
    $restoreObject = $ldap_to->fetch();

    /* Prepare import string */
    $data  = gzuncompress($ldap_to->get_attribute($dn,'gosaSnapshotData'));

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
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
