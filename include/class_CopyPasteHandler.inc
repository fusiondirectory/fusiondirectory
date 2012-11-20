<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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
 * \file class_CopyPasteHandler.inc
 * Source code for class CopyPasteHandle
 */

/*!
 * \brief This class contains all function to copy and paste
 */
class CopyPasteHandler {

  var $config;
  var $current;

  /*!
   * \brief This array contains all dns of the currently copyied objects
   */
  var $queue = array();

  /*!
   *  \brief The dn of the last edited object
   */
  var $lastdn = "";

  var $disallowed_objects = array();
  var $objects_to_fix     = array();
  var $clean_objects      = array();
  var $require_update     = FALSE;



  /*!
   * \brief Create CP handler
   *
   * \param string $config
   */
  function CopyPasteHandler(&$config)
  {
    $this->config       = &$config;
    $this->current      = NULL;
    $this->queue        = array();
    $this->setvar_array = array();
  }


  /*!
   * \brief Entry entry to Copy & Paste queue.
   * A Queue entry is represented as follows.
   *  array['file_name']  - Position on hdd
   *  array['method']     - 'copy' or 'cut'
   *  array['dn']         - the dn of the object added to the queue
   *  array['tab_class']  - Tab object that should be used to initialize the new object
   *  array['tab_object'] - Tab object name used to initialize correct object Type like USERTABS
   *
   * \param String $dn The dn of the object added to the queue
   *
   * \param String $action Copy or Cut
   *
   * \param String $tab_class Tab object that should be used to initialize the new object
   *
   * \param String $tab_object Tab object name used to initialize correct object Type like USERTABS
   *
   * \param String $tab_acl_category Tab with acl category
   */
  function add_to_queue($dn, $action, $tab_class, $tab_object, $tab_acl_category)
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, "add_to_queue");
    if (!class_available($tab_class)) {
      trigger_error(sprintf("Specified class object '%s' does not exists.", $tab_class));
      return FALSE;
    }

    if (!isset($this->config->data['TABS'][$tab_object])) {
      trigger_error(sprintf("Specified tab object '%s' does not exists.", $tab_object));
      return FALSE;
    }

    if (!in_array($action, array("cut","copy"))) {
      trigger_error(sprintf("Specified action '%s' does not exists for copy & paste.", $action));
      return FALSE;
    }

    $tmp = array();

    $tmp['method']            = $action;
    $tmp['dn']                = $dn;
    $tmp['tab_class']         = $tab_class;
    $tmp['tab_object']        = $tab_object;
    $tmp['tab_acl_category']  = $tab_acl_category;

    $this->queue[]        = $tmp;
    $this->require_update = TRUE;

    return TRUE;
  }


  /*!
   * \brief This removes all objects from queue.
   * 		Remove hdd dumps of current entries too.
   * 		Remove entries older than 24 hours.
   */
  function cleanup_queue()
  {
    $this->current        = FALSE;
    $this->require_update = TRUE;
    $this->queue          = array();
  }

  /*!
   * \brief Check if there are still entries the object queue
   */
  function entries_queued()
  {
    return ((count($this->queue) > 0) || ($this->current != FALSE));
  }


  /*!
   * \brief Paste one entry from queue
   */
  function load_entry_from_queue($entry, $base)
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $entry['dn'], "load_entry_from_queue");
    if (!isset($entry['tab_class'])) {
      return array();
    }

    $tab_c  = $entry['tab_class'];
    $tab_o  = $entry['tab_object'];
    $tab_a  = $entry['tab_acl_category'];

    $entry['object'] = new $tab_c($this->config, $this->config->data['TABS'][$tab_o], $entry['dn'], $tab_a);
    $entry['object']->set_acl_base($base);

    if ($entry['method'] == "copy") {
      $entry['object']->resetCopyInfos();
    }

    return $entry;
  }

  /*!
   * \brief Displays a dialog which allows the user to fix all dependencies of this object.
   * 	    Create unique names, ids, or what ever
   */
  function execute($base)
  {
    $ui   = get_userinfo();
    $type = $this->current['method'];

    /* Check which entries can be pasted directly.
     * Create a list of all entries that can be pasted directly.
     */
    if ($this->require_update) {
      $this->clean_objects      = array();
      $this->objects_to_fix     = array();
      $this->disallowed_objects = array();

      /* Put each queued object in one of the above arrays
       */
      foreach ($this->queue as $key => $entry) {

        /* Update entries on demand
         */
        if (!isset($entry['object'])) {
          $entry = $this->load_entry_from_queue($entry, $base);
          $this->queue[$key] = $entry;
        }
        $msgs = $entry['object']->check();

        /* To copy an object we require full read access to the object category
         */
        $copy_acl = preg_match("/r/", $ui->has_complete_category_acls($entry['dn'], $entry['tab_acl_category']));

        /* In order to copy an object we require read an delete acls
         */
        $cut_acl  = preg_match("/d/", $ui->has_complete_category_acls($entry['dn'], $entry['tab_acl_category']));
        $cut_acl &= preg_match("/r/", $ui->has_complete_category_acls($entry['dn'], $entry['tab_acl_category']));

        /* Check permissions */
        if ($entry['method'] == "copy" && !$copy_acl) {
          $this->disallowed_objects[$key] = $entry;
        } elseif ($entry['method'] == "cut" && !$cut_acl) {
          $this->disallowed_objects[$key] = $entry;
        } elseif (!count($msgs)) {
          $this->clean_objects[$key]  = $entry;
        } else {
          $this->objects_to_fix[$key] = $entry;
        }
      }
      if (count($this->disallowed_objects)) {
        $dns = array();
        foreach ($this->disallowed_objects as $entry) {
          $dns[] = $entry['dn'];
        }
        msg_dialog::display(_("Permission"), msgPool::permCreate($dns), INFO_DIALOG);
      }
      $this->require_update = FALSE;
    }

    /* Save objects that can be pasted directly
     */
    if (count($this->clean_objects)) {
      $this->save_object();
      foreach ($this->clean_objects as $key => $entry) {

        /* Remove from queue -> avoid saving twice */
        unset($this->queue[$key]);
        unset($this->clean_objects[$key]);

        /* Load next queue entry */
        $this->current  = $entry;
        $this->lastdn   = $this->current['object']->dn;
        $this->current['object']->save();
        $this->handleReferences();
      }
      $this->current = FALSE;
    }

    /* Save edited entry and force loading new one
     */
    if (isset($this->current['object'])) {
      $this->current['object']->save_object();
    }

    if (isset($_POST['edit_finish']) && $this->current) {
      $msgs = $this->current['object']->check();

      /* Load next queue entry */
      if (!count($msgs)) {
        $this->current['object']->save();
        $this->handleReferences();
        $this->lastdn   = $this->current['object']->dn;
        $this->current  = FALSE;
      } else {
        foreach ($msgs as $msg) {
          msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
        }
      }
    }

    /* Display a list of all pastable entries
     */
    if ($this->current || count($this->objects_to_fix)) {
      $this->save_object();
      if (!$this->current) {
        $key = key($this->objects_to_fix);
        if (isset($this->objects_to_fix[$key])) {
          $this->current  = $this->objects_to_fix[$key];
          unset($this->objects_to_fix[$key]);
          unset($this->queue[$key]);
        }
      }
      if ($this->current) {
        $display = $this->current['object']->execute();
        if (isset($this->current['object']->dialog) &&
            (is_object($this->current['object']->dialog) || $this->current['object']->dialog)) {
          return $display;
        } else {
          // Display ok, (apply) and cancel buttons
          $display .= "<p class=\"plugbottom\" style=\"text-align:right\">\n";
          $display .= "<input type=submit name=\"edit_finish\" style=\"width:80px\" value=\"".msgPool::okButton()."\">\n";
          $display .= "&nbsp;\n";
          $display .= '<input type="submit" name="abort_current_cut-copy_operation" value="'.msgPool::cancelButton().'"/>'."\n";
          $display .= '<input type="submit" name="abort_all_cut-copy_operations" value="'._('Cancel all').'"/>'."\n";
          $display .= "</p>";
          return $display;
        }
      }
    }
    return "";
  }


  /*!
   * \brief Get the last endited entry
   *
   * \return the dn of the last edited entry
   */
  function last_entry()
  {
    return $this->lastdn;
  }


  /*!
   * \brief Save new values posted by copy & paste dialog
   */
  function save_object()
  {
    if (isset($_POST['abort_current_cut-copy_operation'])) {
      $this->current = FALSE;
    }

    if (isset($_POST['abort_all_cut-copy_operations'])) {
      $this->cleanup_queue();
      $this->current = FALSE;
    }
  }

  function handleReferences()
  {
    $dst_dn = $this->current['object']->dn;
    $src_dn = $this->current['dn'];

    // Only copy references if required
    if ($this->current['method'] != 'copy') {
      return;
    }

    // Migrate objectgroups
    $ogroups = get_sub_list("(&(objectClass=gosaGroupOfNames)(member=".LDAP::prepare4filter(LDAP::fix($src_dn))."))",
            "ogroups", array(get_ou("ogroupRDN")), $this->config->current['BASE'], array("dn"), GL_SUBSEARCH | GL_NO_ACL_CHECK);

    // Walk through all objectGroups
    foreach ($ogroups as $ogroup) {
      $o_ogroup = new ogroup($this->config, $ogroup['dn']);
      $o_ogroup->member[$dst_dn] = $dst_dn;
      $o_ogroup->save();
    }

    if (class_available('roleGeneric')) {
      // Update roles
      $roles = get_sub_list("(&(objectClass=organizationalRole)(roleOccupant=".LDAP::prepare4filter(LDAP::fix($src_dn))."))",
              "roles", array(get_ou("roleRDN")), $this->config->current['BASE'], array("dn"), GL_SUBSEARCH | GL_NO_ACL_CHECK);

      // Walk through all roles
      foreach ($roles as $role) {
        $role = new roleGeneric($this->config, $role['dn']);
        $role->roleOccupant[] = $dst_dn;
        $role->save();
      }
    }

    // Update groups
    if (isset($this->current['object']->uid) && !empty($this->current['object']->uid)) {

      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->cat($src_dn);
      $attrs = $ldap->fetch();
      if (isset($attrs['uid'][0])) {
        $suid = $attrs['uid'][0];

        $uid = $this->current['object']->uid;
        $groups = get_sub_list("(&(objectClass=posixGroup)(memberUid={$suid}))",
                "groups", array(get_ou("groupRDN")), $this->config->current['BASE'], array("dn"), GL_SUBSEARCH | GL_NO_ACL_CHECK);

        // Walk through all POSIX groups
        foreach ($groups as $group) {
          $o_group = new group($this->config, $group['dn']);
          $o_group->addUser($uid);
          $o_group->save();
        }
      }
    }
  }

  /*!
   * \brief Generate the paste icon for headpages
   *
   * \return the paste icon for headpages
   */
  function generatePasteIcon()
  {
    $Copy_Paste = "&nbsp;<img class='center' src='images/lists/seperator.png' align='middle' alt='' height='16' width='1'>&nbsp;";
    if ($this->entries_queued()) {
      $img = "images/lists/paste.png";

      $Copy_Paste .= "<input type='image' name='editPaste' class='center'
        src='".$img."' alt='"._("Paste")."'>&nbsp;";
    } else {
      $Copy_Paste .= "<img class='center' src='images/lists/paste-grey.png' alt=\""._("Cannot paste")."\">&nbsp;";
    }
    return $Copy_Paste;
  }
}
?>
