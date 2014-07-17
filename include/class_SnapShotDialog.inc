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
  var $attributes                   = array("CurrentDescription");
  var $CurrentDescription           = "";

  var $display_restore_dialog       = FALSE;
  /* Defines the restore mode */

  var $display_all_removed_objects  = FALSE;
  /* Specifies which objects will be listed,
   * all snapshots for a single entry or
   * all snapshots of already deleted objects  */

  var $del_dn     = "";
  var $last_list  = array();

  var $snapHandler;

  /*!
   * \brief Create a SnapShotDialog
   *
   * \param string $config
   *
   * \param string $dn The DN
   *
   * \param string $parent
   */
  function __construct(&$config, $dn, &$parent)
  {
    parent::__construct($config, $dn);
    $this->snapHandler = new SnapshotHandler($this->config);
  }

  /*!
   * \brief Get all deleted snapshots
   */
  function getAllDeletedSnapshots($base_of_object, $raw = FALSE)
  {
    return $this->snapHandler->getAllDeletedSnapshots($base_of_object, $raw);
  }

  /*
   * \brief Return available snapshots for the given base
   *
   * \param string $dn The DN
   *
   * \param string boolean $raw FALSE
   */
  function Available_SnapsShots($dn, $raw = FALSE)
  {
    return $this->snapHandler->Available_SnapsShots($dn, $raw);
  }

  /*!
   * \brief Remove a snapshot
   *
   * \param string $dn The DN
   */
  function remove_snapshot($dn)
  {
    $this->snapHandler->remove_snapshot($dn);
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
    $this->snapHandler->setSnapshotBases($bases);
  }


  /*!
   * \brief Display snapshot dialog
   */
  function execute()
  {
    parent::execute();
    $smarty = get_smarty();

    $matches = array();
    foreach ($_POST as $name => $value) {
      if (preg_match("/^RemoveSnapshot_(.*)_[xy]$/", $name, $matches)) {
        $entry = intval($matches[1]);

        if (isset($this->last_list[$entry])) {
          $this->del_dn = $this->last_list[$entry]['dn'];

          $smarty = get_smarty();
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
      $tmp = array();
      if ($this->display_all_removed_objects) {
        $snap_shot_bases = $this->snapHandler->getSnapshotBases();
        if (count($snap_shot_bases)) {
          foreach ($snap_shot_bases as $dn) {
            $tmp = array_merge($tmp, $this->getAllDeletedSnapshots($dn, TRUE));
          }
        } else {
          $tmp = $this->getAllDeletedSnapshots($snap_shot_bases, TRUE);
        }
      } else {
        $tmp = $this->Available_SnapsShots($this->dn, TRUE);
      }

      $DivListSnapShots = new divSelectBox("SnapShotRestore");
      $DivListSnapShots->SetHeight(180);

      $list_of_elements = array();

      /* Walk through all entries and setup the display text */
      foreach ($tmp as $entry) {
        $list_of_elements[$entry['gosaSnapshotTimestamp'][0]] = $entry;
      }

      /* Sort generated list */
      krsort($list_of_elements);

      /* Add Elements to divselectbox */
      $this->last_list = array_values($list_of_elements);
      foreach ($this->last_list as $key => $entry) {
        $actions = "<input type='image' src='geticon.php?context=actions&icon=document-restore&size=16' name='RestoreSnapshot_$key'
          class='center' title='"._("Restore snapshot")."'>&nbsp;";
        $actions .= "<input type='image' src='geticon.php?context=actions&icon=edit-delete&size=16' name='RemoveSnapshot_$key'
          class='center' title='"._("Remove snapshot")."'>&nbsp;";

        $time_stamp     = date(_("Y-m-d, H:i:s"), preg_replace("/\-.*$/", "", $entry['gosaSnapshotTimestamp'][0]));
        $display_data   = $entry['description'][0];

        if ($this->display_all_removed_objects) {
          $display_data .= " - ".$entry['gosaSnapshotDN'][0];
        }

        $field0 = array("html" => $time_stamp, "attach" => "style='vertical-align:top;width:120px;'");
        $field1 = array("html" => htmlentities(utf8_decode(LDAP::fix($display_data))), "attach" => "");
        $field2 = array("html" => $actions,
                "attach" => "style='border-right:0px;vertical-align:top;width:40px;text-align:right;'");
        $DivListSnapShots->AddEntry(array($field0,$field1,$field2));
      }

      $smarty->assign("SnapShotDivlist", $DivListSnapShots->DrawList());
      $smarty->assign("CountSnapShots", count($list_of_elements));
    }

    $smarty->assign("restore_deleted", $this->display_all_removed_objects);
    $smarty->assign("RestoreMode",  $this->display_restore_dialog);
    $smarty->assign("CurrentDate", date(_("Y-m-d, H:i:s")));
    $smarty->assign("CurrentDN", LDAP::fix($this->dn));
    $smarty->assign("CurrentDescription", $this->CurrentDescription);
    return $smarty->fetch(get_template_path("snapshotdialog.tpl"));
  }

  /*!
   * \brief Check the snapshot dialog
   */
  function check()
  {
    $message = plugin::check();
    if (!$this->display_restore_dialog) {
      if (empty($this->CurrentDescription)) {
        $message[] = msgPool::required(_("Description"));
      }
    }
    return $message;
  }

  /*!
   * \brief Save an object
   */
  function save_object()
  {
    plugin::save_object();
    foreach ($this->attributes as $name) {
      if (isset($_POST[$name])) {
        $this->$name = stripslashes($_POST[$name]);
      }
    }
  }
}

?>
