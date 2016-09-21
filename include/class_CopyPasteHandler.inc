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
 * \file class_CopyPasteHandler.inc
 * Source code for class CopyPasteHandle
 */

/*!
 * \brief This class contains all function to copy and paste
 */
class CopyPasteHandler
{
  var $current;

  /*!
   * \brief This array contains all dns of the currently copied objects
   */
  protected $objectList = array();
  /*!
   * \brief This array contains all remaining objects to paste
   */
  protected $queue = array();

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
  function __construct()
  {
    $this->current  = NULL;
    $this->queue    = array();
  }


  /*!
   * \brief Entry entry to Copy & Paste queue.
   * A Queue entry is represented as follows.
   *  array['method']     - 'copy' or 'cut'
   *  array['dn']         - the dn of the object added to the queue
   *  array['type']       - Object type
   *
   * \param String $dn The dn of the object added to the queue
   *
   * \param String $action Copy or Cut
   *
   * \param String $type the type of the object
   */
  function add_to_queue($dn, $action, $type)
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'add_to_queue');

    if (!in_array($action, array('cut','copy'))) {
      trigger_error(sprintf('Specified action "%s" does not exists for copy & paste.', $action));
      return FALSE;
    }

    $tmp = array();

    $tmp['method']  = $action;
    $tmp['dn']      = $dn;
    $tmp['type']    = $type;

    $infos              = objects::infos($type);
    $tmp['aclCategory'] = $infos['aclCategory'];
    $tmp['mainTab']     = $infos['mainTab'];
    $tmp['parent']      = NULL;

    $this->queue[] = $tmp;
    if ($action == 'copy') {
      $this->objectList[] = $tmp;
    }
    $this->require_update = TRUE;

    return TRUE;
  }


  /*!
   * \brief This removes all objects from queue.
   *    Remove hdd dumps of current entries too.
   *    Remove entries older than 24 hours.
   */
  function cleanup_queue()
  {
    $this->current        = FALSE;
    $this->require_update = TRUE;
    $this->queue          = array();
    $this->objectList     = array();
  }

  /*!
   * \brief This resets the queue to allow pasting again.
   */
  function resetPaste()
  {
    $this->current        = FALSE;
    $this->require_update = TRUE;
    $this->queue          = $this->objectList;
  }

  /*!
   * \brief Check if there are still entries the object queue
   */
  function entries_queued()
  {
    return ((count($this->queue) > 0) || ($this->current != FALSE));
  }

  /*!
   * \brief Paste one entry from LDAP
   */
  protected function load_entry_from_ldap($entry, $base)
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $entry['dn'], 'load_entry_from_ldap');
    if (!isset($entry['tab_class']) && !isset($entry['type'])) {
      return array();
    }

    $entry['object'] = objects::open($entry['dn'], $entry['type']);

    $entry['object']->set_acl_base($base);
    if ($entry['parent'] !== NULL) {
      $entry['object']->parent = $entry['parent'];
    }

    if ($entry['method'] == 'copy') {
      $entry['object']->resetCopyInfos();
    }

    $entry['object']->resetBase();

    return $entry;
  }

  /*!
   * \brief Displays a dialog which allows the user to fix all dependencies of this object.
   *      Create unique names, ids, or what ever
   */
  function execute($base)
  {
    $ui = get_userinfo();

    /* Check which entries can be pasted directly.
     * Create a list of all entries that can be pasted directly.
     */
    if ($this->require_update) {
      $this->clean_objects      = array();
      $this->objects_to_fix     = array();
      $this->disallowed_objects = array();

      /* Put each queued object in one of the above arrays */
      foreach ($this->queue as $key => $entry) {

        /* Update entries on demand */
        if (!isset($entry['object'])) {
          $entry = $this->load_entry_from_ldap($entry, $base);
          $this->queue[$key] = $entry;
        }

        /* Retrieve ACL infos */
        $copy_acl = $ui->is_copyable($entry['dn'], $entry['aclCategory']);
        $cut_acl  = $ui->is_cutable($entry['dn'], $entry['aclCategory'], $entry['mainTab']);

        /* Check permissions */
        if (($entry['method'] == 'copy') && !$copy_acl) {
          $this->disallowed_objects[$key] = $entry;
        } elseif (($entry['method'] == 'cut') && !$cut_acl) {
          $this->disallowed_objects[$key] = $entry;
        } else {
          $this->clean_objects[$key] = $entry;
        }
      }
      if (count($this->disallowed_objects)) {
        $dns = array();
        foreach ($this->disallowed_objects as $entry) {
          $dns[] = $entry['dn'];
        }
        msg_dialog::display(_('Permission'), msgPool::permCreate($dns), INFO_DIALOG);
      }
      $this->require_update = FALSE;
    }

    /* Save objects that can be pasted directly */
    if (count($this->clean_objects)) {
      $this->save_object();
      foreach ($this->clean_objects as $key => $entry) {
        $this->current = $entry;
        $dn     = $this->current['object']->dn;
        $errors = $this->current['object']->save();

        if (empty($errors)) {
          $this->current_saved($dn);
          /* Remove from queue -> avoid saving twice */
          unset($this->queue[$key]);
        } else {
          $this->objects_to_fix[$key] = $entry;
        }
        unset($this->clean_objects[$key]);
      }
      $this->current = FALSE;
    }

    /* Save edited entry and force loading new one */
    if (isset($this->current['object'])) {
      $this->current['object']->save_object();
    }

    /* Save current object if edition is finished */
    if (isset($_POST['edit_finish']) && $this->current) {
      $dn     = $this->current['object']->dn;
      $errors = $this->current['object']->save();

      if (empty($errors)) {
        $this->current_saved($dn);
      } else {
        msg_dialog::displayChecks($errors);
      }
    }

    /* Display a list of all pastable entries */
    if ($this->current || count($this->objects_to_fix)) {
      $this->save_object();
      if (!$this->current) {
        $key = key($this->objects_to_fix);
        if ($key !== NULL) {
          $this->current = $this->objects_to_fix[$key];
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
          $display .= '<p class="plugbottom">'."\n";
          $display .= '<input type="submit" name="edit_finish" style="width:80px" value="'.msgPool::okButton().'"/>'."\n";
          $display .= "&nbsp;\n";
          $display .= '<input type="submit" name="abort_current_cut-copy_operation" value="'.msgPool::cancelButton().'"/>'."\n";
          $display .= '<input type="submit" name="abort_all_cut-copy_operations" value="'._('Cancel all').'"/>'."\n";
          $display .= '</p>';
          return $display;
        }
      }
    }
    return "";
  }

  private function current_saved($dn)
  {
    $this->lastdn   = $dn;
    $this->handleReferences();
    $this->current  = FALSE;
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

    $this->current['object']->getBaseObject()->handleForeignKeys(
      $src_dn,
      $dst_dn,
      ($this->current['method'] == 'cut' ? 'move' : 'copy')
    );
  }

  /*!
   * \brief Generate the paste icon for headpages
   *
   * \return the paste icon for headpages
   */
  function generatePasteIcon()
  {
    $Copy_Paste = "&nbsp;<img class='center' src='images/lists/seperator.png' alt='' height='16' width='1'>&nbsp;";
    if ($this->entries_queued()) {
      $Copy_Paste .= "<input type='image' name='editPaste' class='center'
        src='geticon.php?context=actions&amp;icon=edit-paste&amp;size=16' alt='"._("Paste")."'>&nbsp;";
    } else {
      $Copy_Paste .= "<img class='center' src='geticon.php?context=actions&amp;icon=edit-paste&amp;size=16&amp;disabled=1' alt=\""._("Cannot paste")."\">&nbsp;";
    }
    return $Copy_Paste;
  }
}
?>
