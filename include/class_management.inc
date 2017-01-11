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
 * \file class_management.inc
 * Source code for the class management
 */

/*!
 * \brief This class contains all the function needed to manage various
 * functions like new,save, edit, apply, cut/copy/paste , snapshot etc...
 */
class management
{
  // The currently used object(s) (e.g. in edit, removal)
  // $dn is public due to some compatibility problems with class plugin..
  public $dn      = '';
  protected $dns  = array();

  // The last used object(s).
  protected $last_dn  = '';
  protected $last_dns = array();

  // The common places the displayed objects are stored in. (e.g. array("ou=groups,",".."))
  protected $storagePoints = array();

  // The opened object.
  protected $tabObject    = NULL;
  protected $dialogObject = NULL;

  // The last opened object.
  protected $last_tabObject     = NULL;
  protected $last_dialogObject  = NULL;

  // Whether to display the apply button or not
  protected $displayApplyBtn = FALSE;

  // Whether to display a header or not.
  protected $skipHeader = FALSE;

  // Whether to display a footer or not.
  protected $skipFooter = FALSE;

  protected $skipCpHandler    = FALSE;
  protected $skipSnapHandler  = FALSE;

  // Copy&Paste handler
  protected $cpHandler = NULL;

  // Indicates that we want to paste objects right now.
  protected $cpPastingStarted = FALSE;

  // The Snapshot handler class.
  protected $snapHandler = NULL;

  // The listing handlers
  protected $headpage = NULL;
  protected $filter   = NULL;

  // A list of configured actions/events
  protected $actions = array();

  // Attributes managed by this plugin, can be used in post events;
  public $attributes = array();

  // Some management classes are used in tab groups and needs this set to FALSE.
  var $is_template    = FALSE;

  function  __construct($headpage)
  {
    global $config;
    $this->headpage = $headpage;

    // Add copy&paste and snapshot handler.
    if (!$this->skipCpHandler) {
      $this->cpHandler = new CopyPasteHandler();
      $this->headpage->setCopyPasteHandler($this->cpHandler);
    }
    if (!$this->skipSnapHandler && ($config->get_cfg_value("enableSnapshots") == "TRUE")) {
      $this->snapHandler = new SnapshotHandler();
      $this->headpage->setSnapshotHandler($this->snapHandler);
    }

    // Register default actions
    $this->registerAction("new",    "newEntry");
    $this->registerAction("edit",   "editEntry");
    $this->registerAction("apply",  "applyChanges");
    $this->registerAction("save",   "saveChanges");

    $this->registerAction("cancel",       "cancelEdit");
    $this->registerAction("cancelDelete", "cancelEdit");

    $this->registerAction("remove",           "removeEntryRequested");
    $this->registerAction("removeConfirmed",  "removeEntryConfirmed");

    $this->registerAction("copy",   "copyPasteHandler");
    $this->registerAction("cut",    "copyPasteHandler");
    $this->registerAction("paste",  "copyPasteHandler");

    if ($config->get_cfg_value('enableSnapshots') == 'TRUE') {
      $this->registerAction('snapshot', 'createSnapshotDialog');
      $this->registerAction('restore',  'restoreSnapshotDialog');
    }
  }

  /*!
   * \brief  Execute this plugin
   *          Handle actions/events, locking, snapshots, dialogs, tabs,...
   */
  function execute()
  {
    // Ensure that html posts and gets are kept even if we see a 'Entry islocked' dialog.
    $vars = array('/^act$/','/^listing/','/^PID$/','/^FILTER_PID$/');
    session::set('LOCK_VARS_TO_USE', $vars);

    /* Display the copy & paste dialog, if it is currently open */
    $ret = $this->copyPasteHandler("", array());
    if ($ret) {
      return $this->getHeader().$ret;
    }

    // Update filter
    if ($this->filter) {
      $this->filter->update();
      session::global_set(get_class($this)."_filter", $this->filter);
      session::set('autocomplete', $this->filter);
      if (!$this->filter->isValid()) {
        msg_dialog::display(_("Filter error"), _("The filter is incomplete!"), ERROR_DIALOG);
      }
    }

    // Handle actions (POSTs and GETs)
    $str = $this->handleActions($this->detectPostActions());
    if ($str) {
      return $this->getHeader().$str;
    }

    // Open single dialog objects
    if (is_object($this->dialogObject)) {
      if (method_exists($this->dialogObject, 'save_object')) {
        $this->dialogObject->save_object();
      }
      if (method_exists($this->dialogObject, 'execute')) {
        $display = $this->dialogObject->execute();
        $display .= $this->_getTabFooter();
        return $this->getHeader().$display;
      }
    }

    // Display tab object.
    if ($this->tabObject instanceOf simpleTabs) {
      $this->tabObject->save_object();
      $display = $this->tabObject->execute();
      $display .= $this->_getTabFooter();
      return $this->getHeader().$display;
    }

    // Set current restore base for snapshot handling.
    if (is_object($this->snapHandler)) {
      $bases = array();
      foreach ($this->storagePoints as $sp) {
        $bases[] = $sp.$this->headpage->getBase();
      }

      // No bases specified? Try base
      if (!count($bases)) {
        $bases[] = $this->headpage->getBase();
      }

      $this->snapHandler->setSnapshotBases($bases);
    }

    // Display list
    return $this->renderList();
  }

  function getHeadpage()
  {
    return $this->headpage;
  }

  function getFilter()
  {
    return $this->filter;
  }

  /*!
   * \brief  Generates the plugin header which is displayed whenever a tab object is
   *           opened.
   */
  protected function getHeader()
  {
    if ($this->skipHeader) {
      return '';
    }

    $plInfos  = pluglist::pluginInfos(get_class($this));
    $plTitle  = $plInfos['plTitle'];
    $plIcon   = $plInfos['plIcon'];

    if (!preg_match('/^geticon/', $plIcon)) {
      $plIcon = get_template_path($plIcon);
    }
    return print_header($plIcon, $plTitle, LDAP::fix(get_object_info()));
  }


  /*!
   * \brief  Generates the footer which is used whenever a tab object is
   *           displayed.
   */
  protected function _getTabFooter()
  {
    // Do not display tab footer for non tab objects
    if (!($this->tabObject instanceOf simpleTabs)) {
      return "";
    }

    // Check if there is a dialog opened - We don't need any buttons in this case.
    if ($this->tabObject->dialogOpened()) {
      return '';
    }

    // Skip footer if requested;
    if ($this->skipFooter) {
      return "";
    }

    // In case an of locked entry, we may have opened a read-only tab.
    $str = "";
    if ($this->tabObject->readOnly()) {
      $str .= '<p class="plugbottom">'."\n".
        '<input type="submit" name="edit_cancel" value="'.msgPool::cancelButton().'">'."\n".
        '</p>';
      return $str;
    } else {
      // Display ok, (apply) and cancel buttons
      $str .= '<p class="plugbottom">'."\n";
      $str .= '<input type="submit" name="edit_finish" style="width:80px" value="'.msgPool::okButton().'"/>'."\n";
      $str .= "&nbsp;\n";
      if ($this->displayApplyBtn) {
        $str .= '<input type="submit" name="edit_apply" value="'.msgPool::applyButton().'"/>'."\n";
        $str .= "&nbsp;\n";
      }
      $str .= '<input type="submit" name="edit_cancel" value="'.msgPool::cancelButton().'"/>'."\n";
      $str .= '</p>';
    }
    return $str;
  }

  /*!
   * \brief  Detects actions/events send by the ui
   *           and the corresponding targets.
   */
  function detectPostActions()
  {
    if (!is_object($this->headpage)) {
      trigger_error("No valid headpage given....!");
      return array();
    }
    $action = $this->headpage->getAction();
    if (isset($_POST['edit_cancel'])) {
      $action['action'] = 'cancel';
    } elseif (isset($_POST['edit_finish'])) {
      $action['action'] = 'save';
    } elseif (isset($_POST['edit_apply'])) {
      $action['action'] = 'apply';
    }
    if (!$this->is_modal_dialog()) {
      if (isset($_POST['delete_confirmed'])) {
        $action['action'] = 'removeConfirmed';
      }
      if (isset($_POST['delete_cancel'])) {
        $action['action'] = 'cancelDelete';
      }
    }

    return $action;
  }

  /*!
   *  \brief  Calls the registered method for a given action/event.
   */
  function handleActions($action)
  {
    // Start action
    if (isset($this->actions[$action['action']])) {
      $func = $this->actions[$action['action']];
      if (!isset($action['targets'])) {
        $action['targets'] = array();
      }
      return $this->$func($action['action'], $action['targets'], $action);
    }
  }

  /*!
   * \brief  Creates a new snapshot entry
   */
  function createSnapshot($dn, $description)
  {
    global $ui;
    if ($this->dn !== $dn) {
      trigger_error('There was a problem with the snapshot workflow');
      return;
    }
    if (!empty($dn) && $ui->allow_snapshot_create($dn, $this->dialogObject->aclCategory)) {
      $this->snapHandler->createSnapshot($dn, $description);
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Snaptshot created!');
    } else {
      msg_dialog::display(_('Permission'), sprintf(_('You are not allowed to restore a snapshot for %s.'), $dn),
          ERROR_DIALOG);
    }
  }

  /*!
   * \brief  Restores a snapshot object.
   *
   * \param  String  $dn  The DN of the snapshot
   */
  function restoreSnapshot($dn)
  {
    global $ui;
    if (!empty($dn) && $ui->allow_snapshot_restore($dn, $this->dialogObject->aclCategory)) {
      $this->snapHandler->restoreSnapshot($dn);
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Snaptshot restored');
      $this->closeDialogs();
    } else {
      msg_dialog::display(_('Permission'), sprintf(_('You are not allowed to restore a snapshot for %s.'), $dn),
          ERROR_DIALOG);
    }
  }

  /*!
   * \brief Get all deleted snapshots
   *
   * \param string $base The base
   *
   * \param string boolean $raw
   */
  function getAllDeletedSnapshots($raw = FALSE)
  {
    $tmp = array();
    $snapshotBases = $this->snapHandler->getSnapshotBases();
    if (is_array($snapshotBases)) {
      foreach ($snapshotBases as $base) {
        $tmp = array_merge($tmp, $this->snapHandler->getAllDeletedSnapshots($base, $raw));
      }
    } else {
      $tmp = $this->snapHandler->getAllDeletedSnapshots($snapshotBases, $raw);
    }
    return $tmp;
  }

  /*
   * \brief Return available snapshots for the given base
   *
   * \param string $dn The DN
   *
   * \param string boolean $raw
   */
  function getAvailableSnapsShots($dn, $raw = FALSE)
  {
    return $this->snapHandler->getAvailableSnapsShots($dn, $raw);
  }

  /*!
   * \brief Delete a snapshot
   *
   * \param string $dn DN of the snapshot
   */
  function removeSnapshot($dn)
  {
    $this->snapHandler->removeSnapshot($dn);
  }

  /*!
   *  \brief  Save object modifications and keep dialogs opened
   */
  protected function applyChanges()
  {
    if ($this->tabObject instanceOf simpleTabs) {
      $this->tabObject->save_object();
      $msgs = $this->tabObject->save();
      if (count($msgs)) {
        msg_dialog::displayChecks($msgs);
      } else {
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dns, 'Modifications applied!');
        $this->tabObject->re_init();
      }
    }
  }

  /*!
   * \brief  This method closes dialogs
   *          and cleans up the cached object info and the ui.
   */
  public function closeDialogs()
  {
    $this->last_dn  = $this->dn;
    $this->dn       = "";
    $this->last_dns = $this->dns;
    $this->dns      = array();

    $this->last_tabObject     = $this->tabObject;
    $this->tabObject          = NULL;
    $this->last_dialogObject  = $this->dialogObject;
    $this->dialogObject       = NULL;

    $this->skipFooter   = FALSE;
    set_object_info();
  }

  /*!
   *  \brief  Every click in the list user interface sends an event
   *          here can we connect those events to a method.
   *          eg. see management::registerEvent('new','createUser')
   *          When the action/event new is send, the method 'createUser'
   *          will be called.
   */
  function registerAction($action, $target)
  {
    $this->actions[$action] = $target;
  }

  /*!
   * \brief  Removes ldap object locks created by this class.
   *         Whenever an object is edited, we create locks to avoid
   *         concurrent modifications.
   *         This locks will automatically removed here.
   */
  function remove_lock()
  {
    if (!empty($this->dn) && $this->dn != "new") {
      del_lock($this->dn);
    }
    if (count($this->dns)) {
      del_lock($this->dns);
    }
  }

  /*!
   * \brief Set a new filter
   *
   * \param object $filter The new filter
   */
  function setFilter($filter)
  {
    $this->filter = $filter;
  }

  function is_modal_dialog()
  {
    return (is_object($this->tabObject) || is_object($this->dialogObject));
  }

  function openTabObject($object, $base)
  {
    $this->tabObject = $object;
    $this->tabObject->set_acl_base($base);
    $this->tabObject->parent = &$this;
  }
}
?>
