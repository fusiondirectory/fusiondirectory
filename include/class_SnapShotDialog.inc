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

/*!
 * \file class_SnapShotDialog.inc
 * Source code for SnapShotDialog
 */

/*!
 * \brief This class contains all the function needed to make dialogs
 * for the snapshot functionality
 *
 * \see plugin
 */
class SnapShotDialog extends plugin
{
  var $config;
  var $attributes= array("CurrentDescription");
  var $CurrentDescription= "";
  var $parent= NULL;
  var $display_restore_dialog= false;         /* Defines the restore mode */
  var $display_all_removed_objects= false;    /* Specifies which objects will be listed, all
                                                 snapshots for a single entry or all snapshots
                                                 of already deleted objects  */
  var $dialog= true;
  var $del_dn= "";
  var $ui;
  var $acl;
  var $dns = array();
  var $snap_shot_bases = array();
  var $last_list = array();

  /*!
   * \brief Create a SnapShotDialog
   *
   * \param string $config
   *
   * \param string $dn The DN
   *
   * \param string $parent
   */
  function SnapShotDialog(&$config, $dn, &$parent)
  {
    plugin::plugin($config,$dn);
    $this->parent   = &$parent;
    $this->ui       = get_userinfo();
  }

  /*! \brief Test if snapshoting is enabled
   *
   * Test weither snapshotting is enabled or not. There will also be some errors posted,
   * if the configuration failed
   *
   * \return TRUE if snapshots are enabled, and FALSE if it is disabled
   */
  function snapshotEnabled()
  {
    return $this->config->snapshotEnabled();
  }

  /*!
   * \brief Get all deleted snapshots
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

  /*
   * \brief Return available snapshots for the given base
   *
   * \param string $dn The DN
   *
   * \param string boolean $raw FALSE
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
   * \brief Remove a snapshot
   *
   * \param string $dn The DN
   */
  function remove_snapshot($dn)
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->rmdir_recursive($dn);
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $dn));
    }
  }


  /*!
   * \brief Set a snapshot base
   *
   * Show deleted snapshots from these bases
   *
   * \param string $bases
   */
  function set_snapshot_bases($bases)
  {
    $this->snap_shot_bases = $bases;
  }


  /*!
   * \brief Display snapshot dialog
   */
  function execute()
  {
    plugin::execute();
    $smarty = get_smarty();

    $ui = get_userinfo();
    $matches = array();
    foreach($_POST as $name => $value) {
      if (preg_match("/^RemoveSnapshot_(.*)_[xy]$/",$name,$matches)) {
        $entry = intval($matches[1]);

        if (isset($this->last_list[$entry])) {
          $this->del_dn = $this->last_list[$entry]['dn'];
          $smarty= get_smarty();
          $smarty->assign("intro", sprintf(_("You're about to delete the snapshot '%s'."), LDAP::fix($this->del_dn)));
          return $smarty->fetch (get_template_path('remove.tpl'));
        }

        break;
      }
    }

    /* Remove snapshot */
    if (isset($_POST['delete_confirm']) && !empty($this->del_dn)) {
      $this->remove_snapshot($this->del_dn);
      $this->del_dn = "";
    }

    /* We must restore a snapshot */

    if ($this->display_restore_dialog) {

      /* Should we only display all snapshots of already deleted objects
                or the snapshots for the given object dn */
      $res = array();
      $tmp = array();
      if ($this->display_all_removed_objects) {
        if (count($this->snap_shot_bases)) {
          foreach($this->snap_shot_bases as $dn) {
            $tmp = array_merge($tmp,$this->getAllDeletedSnapshots($dn,true));
          }
        } else {
          $tmp = $this->getAllDeletedSnapshots($this->snap_shot_bases,true);
        }
      } else {
        $tmp = $this->Available_SnapsShots($this->dn,true);
      }

      $DivListSnapShots = new divSelectBox("SnapShotRestore");
      $DivListSnapShots->SetHeight(180);

      $list_of_elements = array();

      /* Walk through all entries and setup the display text */
      foreach($tmp as $key => $entry){

        /* Check permissions */
        $TimeStamp = $entry['gosaSnapshotTimestamp'][0];
        $list_of_elements[$TimeStamp] = $entry;
      }

      /* Sort generated list */
      krsort($list_of_elements);

      /* Add Elements to divlist */
      $this->last_list = array_values($list_of_elements);
      foreach($this->last_list as $key => $entry){
        $actions= "<input type='image' src='images/lists/restore.png' name='RestoreSnapshot_$key'
          class='center' title='"._("Restore snapshot")."'>&nbsp;";
        $actions.= "<input type='image' src='images/lists/trash.png' name='RemoveSnapshot_$key'
          class='center' title='"._("Remove snapshot")."'>&nbsp;";

        $time_stamp   = date(_("Y-m-d, H:i:s"),preg_replace("/\-.*$/","",$entry['gosaSnapshotTimestamp'][0]));
        $display_data   = $entry['description'][0];

        if($this->display_all_removed_objects){
          $display_data.= " - ".$entry['gosaSnapshotDN'][0];
        }

        $field0 = array("html"=> $time_stamp , "attach"=> "style='vertical-align:top;width:120px;'");
        $field1 = array("html"=> htmlentities (utf8_decode(LDAP::fix($display_data))), "attach"=> "");
        $field2 = array("html"=> $actions ,
                "attach"=> "style='border-right:0px;vertical-align:top;width:40px;text-align:right;'");
        $DivListSnapShots->AddEntry(array($field0,$field1,$field2));
      }

      $smarty->assign("SnapShotDivlist",$DivListSnapShots->DrawList());
      $smarty->assign("CountSnapShots",count($list_of_elements));
    }

    $smarty->assign("restore_deleted",$this->display_all_removed_objects);
    $smarty->assign("RestoreMode",$this->display_restore_dialog);
    $smarty->assign("CurrentDate",date(_("Y-m-d, H:i:s")));
    $smarty->assign("CurrentDN",LDAP::fix($this->dn));
    $smarty->assign("CurrentDescription",$this->CurrentDescription);
    return($smarty->fetch(get_template_path("snapshotdialog.tpl")));
  }

  /*!
   * \brief Check the snapshot dialog
   */
  function check()
  {
    $message = plugin::check();
    if(!$this->display_restore_dialog){
      if(empty($this->CurrentDescription)){
        $message[]= msgPool::invalid(_("Description"));
      }
    }
    return($message);
  }

  /*!
   * \brief Save an object
   */
  function save_object()
  {
    plugin::save_object();
    foreach($this->attributes as $name){
      if(isset($_POST[$name])){
        $this->$name = stripslashes($_POST[$name]);
      }
    }
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
