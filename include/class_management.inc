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
  // The plugin description
  public $plugname      = "unconfigured";
  public $plIcon        = "unconfigured";
  public $plDescription = "unconfigured";
  public $plHeadline    = "unconfigured";

  // The currently used object(s) (e.g. in edit, removal)
  // this is public due to some compatibility problems with class plugin..
  public $dn      = "";
  protected $dns  = array();

  // The last used object(s).
  protected $last_dn  = "";
  protected $last_dns = array();

  // The common places the displayed objects are stored in. (e.g. array("ou=groups,",".."))
  protected $storagePoints = array();

  // The tab definitions to use for the current object.
  // e.g. usertabs
  protected $tabClass     = "";
  // e.g. USERTABS
  protected $tabType      = "";
  // e.g. generic
  protected $aclPlugin    = "";
  // e.g. user
  protected $aclCategory  = "";
  // e.g. users
  protected $objectName   = "";

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

  function  __construct($plugname, $headpage)
  {
    global $config;
    $this->plugname = $plugname;
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
    global $config;
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

    // Pre-render list to init things if a dn is gonna be opened on first load
    if (isset($_REQUEST['dn'])) {
      $this->headpage->setBase($config->current['BASE']);
      $this->headpage->filter->setCurrentScope('sub');
      $this->renderList();
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
      if (!count($bases)) $bases[] = $this->headpage->getBase();

      $this->snapHandler->setSnapshotBases($bases);
    }

    // Display list
    return $this->renderList();
  }

  function renderList()
  {
    $this->headpage->update();
    $display = $this->headpage->render();
    return $this->getHeader().$display;
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
      return "";
    }

    if (in_array_ics('plInfo', get_class_methods(get_class($this)))) {
      $plInfos  = pluglist::pluginInfos(get_class($this));
      $plTitle  = $plInfos['plTitle'];
      $plIcon   = $plInfos['plIcon'];
    } else {
      $plTitle  = $this->plDescription;
      $plIcon   = $this->plIcon;
    }

    if (!preg_match('/^geticon/', $plIcon)) {
      $plIcon = get_template_path($plIcon);
    }
    if (get_object_info() != '') {
      $display = print_header($plIcon, _($plTitle), LDAP::fix(get_object_info()));
    } else {
      $display = print_header($plIcon, _($plTitle));
    }
    return $display;
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
    if (isset($this->tabObject->read_only) && ($this->tabObject->read_only == TRUE)) {
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
   * \brief  Initiates the removal for the given entries
   *           and displays a confirmation dialog.
   *
   * \param  String  $action  The name of the action which was the used as trigger.
   *
   * \param  Array   $target  A list of object dns, which should be affected by this method.
   *
   * \param  Array   $all     A combination of both 'action' and 'target'.
   */
  protected function removeEntryRequested($action = "", $target = array(), $all = array())
  {
    global $ui;
    $disallowed = array();
    $this->dns = array();

    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $target, "Entry removel requested!");

    // Check permissons for each target
    foreach ($target as $dn) {
      $acl = $ui->get_permissions($dn, $this->aclCategory."/".$this->aclPlugin);
      if (preg_match("/d/", $acl)) {
        $this->dns[] = $dn;
      } else {
        $disallowed[] = $dn;
      }
    }
    if (count($disallowed)) {
      msg_dialog::display(_("Permission"), msgPool::permDelete($disallowed), INFO_DIALOG);
    }

    // We've at least one entry to delete.
    if (count($this->dns)) {

      // check locks
      if ($locks = get_locks($this->dns)) {
        return gen_locked_message($locks, $this->dns);
      }

      // Add locks
      $dns_names = array();
      foreach ($this->dns as $dn) {
        $dns_names[] = LDAP::fix($dn);
      }
      add_lock ($this->dns, $ui->dn);

      // Display confirmation dialog.
      $smarty = get_smarty();
      $smarty->assign("info", msgPool::deleteInfo($dns_names, _($this->objectName)));
      $smarty->assign("multiple", TRUE);
      return $smarty->fetch(get_template_path('remove.tpl', TRUE));
    }
  }


  /*!
   * \brief  Object removal was confirmed, now remove the requested entries.
   *
   * \param  String  $action  The name of the action which was the used as trigger.
   *
   * \param  Array   $target  A list of object dns, which should be affected by this method.
   *
   * \param  Array   $all     A combination of both 'action' and 'target'.
   *
   * \param String $altTabClass Empty string.
   *
   * \param String $altTabType Empty string.
   *
   * \param String $altAclCategory Empty string.
   */
  function removeEntryConfirmed($action = "", $target = array(), $all = array(),
      $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    global $ui;
    $tabType      = $this->tabType;
    $tabClass     = $this->tabClass;
    $aclCategory  = $this->aclCategory;

    if (!empty($altTabClass)) $tabClass = $altTabClass;
    if (!empty($altTabType)) $tabType = $altTabType;
    if (!empty($altAclCategory)) $aclCategory = $altAclCategory;

    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $target, "Entry removal confirmed!");

    foreach ($this->dns as $dn) {
      // Check permissions, are we allowed to remove this object?
      $acl = $ui->get_permissions($dn, $aclCategory."/".$this->aclPlugin);
      if (preg_match("/d/", $acl)) {

        // Delete the object
        $this->dn = $dn;
        $this->openTabObject(
          new $tabClass(NULL, $config->data['TABS'][$tabType], $this->dn, $aclCategory),
          $this->dn
        );
        $this->tabObject->delete ();

        // Remove the lock for the current object.
        del_lock($this->dn);
      } else {
        msg_dialog::display(_('Permission error'), msgPool::permDelete(), ERROR_DIALOG);
        logging::log('security', 'group/'.get_class($this), $dn, array(), 'Tried to trick deletion.');
      }
    }

    // Cleanup
    $this->remove_lock();
    $this->closeDialogs();
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
    if (isset($_POST['edit_apply']))  $action['action'] = 'apply';
    if (isset($_POST['edit_finish'])) $action['action'] = 'save';
    if (isset($_POST['edit_cancel'])) $action['action'] = 'cancel';
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
   * \brief  Opens the snapshot creation dialog for the given target.
   *
   * \param  String  $action  The name of the action which was the used as trigger.
   *
   * \param  Array   $target  A list of object dns, which should be affected by this method.
   *
   * \param  Array   $all     A combination of both 'action' and 'target'.
   */
  function createSnapshotDialog($action = '', $target = array(), $all = array())
  {
    global $ui;
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $target, 'Snaptshot creation initiated!');

    if (count($target) == 1) {
      $this->dn = array_pop($target);
      if (empty($this->dn)) {
        return;
      }
      $aclCategory = $this->aclCategory;
      if ($ui->allow_snapshot_create($this->dn, $aclCategory)) {
        $this->dialogObject = new SnapshotCreateDialog($this->dn, $this, $aclCategory);
        $this->dialogObject->set_acl_base($this->dn);
      } else {
        msg_dialog::display(_('Permission'), sprintf(_('You are not allowed to create a snapshot for %s.'), $this->dn),
            ERROR_DIALOG);
      }
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
   * \brief  Displays the "Restore snapshot dialog" for a given target.
   *          If no target is specified, open the restore removed object
   *           dialog.
   * \param  String  $action  The name of the action which was the used as trigger.
   *
   * \param  Array   $target  A list of object dns, which should be affected by this method.
   *
   * \param  Array   $all'     A combination of both 'action' and 'target'.
   */
  function restoreSnapshotDialog($action = '', $target = array(), $all = array())
  {
    global $ui;

    // Set current restore base for snapshot handling.
    if (is_object($this->snapHandler)) {
      $bases = array();
      foreach ($this->storagePoints as $sp) {
        $bases[] = $sp.$this->headpage->getBase();
      }
    }

    // No bases specified? Try base
    if (!count($bases)) $bases[] = $this->headpage->getBase();

    if (!count($target)) {
      // No target, open the restore removed object dialog.
      $this->dn = $this->headpage->getBase();
    } else {
      // Display the restore points for a given object.
      $this->dn = $target[0];
    }
    if (empty($this->dn)) {
      return;
    }

    $aclCategory = $this->aclCategory;
    if ($ui->allow_snapshot_restore($this->dn, $aclCategory)) {
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, 'Snaptshot restoring initiated!');
      $this->snapHandler->setSnapshotBases($bases);
      $this->dialogObject = new SnapshotRestoreDialog($this->dn, $this, !count($target), $aclCategory);
      $this->dialogObject->set_acl_base($this->dn);
    } else {
      msg_dialog::display(_('Permission'), sprintf(_('You are not allowed to restore a snapshot for %s.'), $this->dn),
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
   * \brief  This method intiates the object creation.
   *
   * \param  String  $action  The name of the action which was the used as trigger.
   *
   * \param  Array   $target  A list of object dns, which should be affected by this method.
   *
   * \param  Array   $all     A combination of both 'action' and 'target'.
   *
   * \param String $altTabClass Empty string.
   *
   * \param String $altTabType Empty string.
   *
   * \param String $altAclCategory Empty string.
   */
  function newEntry($action = "", $target = array(), $all = array(), $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    global $config;
    /* To handle mutliple object types overload this method.
     * ...
     *   registerAction('newUser', 'newEntry');
     *   registerAction('newGroup','newEntry');
     * ...
     *
     * function newEntry($action = "", $target= array(), $all=array(), $altTabClass ="", $altTabType = "", $altAclCategory)
     * {
     *   switch($action) {
     *     case 'newUser' : {
     *       mangement::newEntry($action,$target,$all,"usertabs","USERTABS","user");
     *     }
     *     case 'newGroup' : {
     *       mangement::newEntry($action,$target,$all,"grouptabs","GROUPTABS","group");
     *     }
     *   }
     * }
     **/
    $tabType      = $this->tabType;
    $tabClass     = $this->tabClass;
    $aclCategory  = $this->aclCategory;
    if (!empty($altTabClass)) $tabClass = $altTabClass;
    if (!empty($altTabType)) $tabType = $altTabType;
    if (!empty($altAclCategory)) $aclCategory = $altAclCategory;

    // Check locking & lock entry if required
    $this->displayApplyBtn  = FALSE;
    $this->dn               = "new";
    $this->is_single_edit   = FALSE;

    set_object_info($this->dn);

    // Open object.
    if (empty($tabClass) || empty($tabType)) {
      // No tab type defined
    } else {
      if (isset($config->data['TABS'][$tabType])) {
        $this->openTabObject(
          new $tabClass(NULL, $config->data['TABS'][$tabType], $this->dn, $aclCategory),
          $this->headpage->getBase()
        );
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "Create new entry initiated!");
      } else {
        msg_dialog::display(_("Error"), sprintf(_("No tab declaration for '%s' found in your configuration file. Cannot create plugin instance!"), $tabType), ERROR_DIALOG);
      }
    }
  }


  /*!
   * \brief  This method opens an existing object or a list of existing objects to be edited.
   *
   * \param  String  $action  The name of the action which was the used as trigger.
   *
   * \param  Array   $target  A list of object dns, which should be affected by this method.
   *
   * \param  Array   $all     A combination of both 'action' and 'target'.
   *
   * \param String $altTabClass Empty string.
   *
   * \param String $altTabType Empty string.
   *
   * \param String $altAclCategory Empty string.
   */
  function editEntry($action = "", $target = array(), $all = array(), $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    global $config, $ui;
    /* To handle mutliple object types overload this method.
     * ...
     *   registerAction('editUser', 'editEntry');
     *   registerAction('editGroup','editEntry');
     * ...
     *
     * function editEntry($action = "", $target= array(), $all=array(), $altTabClass ="", $altTabType = "", $altAclCategory)
     * {
     *   switch($action) {
     *     case 'editUser' : {
     *       mangement::editEntry($action,$target,$all,"usertabs","USERTABS","user");
     *     }
     *     case 'editGroup' : {
     *       mangement::editEntry($action,$target,$all,"grouptabs","GROUPTABS","group");
     *     }
     *   }
     * }
     **/

    // Do not create a new tabObject while there is already one opened,
    //  the user may have just pressed F5 to reload the page.
    if (is_object($this->tabObject)) {
      return;
    }

    $tabType      = $this->tabType;
    $tabClass     = $this->tabClass;
    $aclCategory  = $this->aclCategory;
    if (!empty($altTabClass)) $tabClass = $altTabClass;
    if (!empty($altTabType)) $tabType = $altTabType;
    if (!empty($altAclCategory)) $aclCategory = $altAclCategory;

    $this->displayApplyBtn = count($target) == 1;

    // Single edit - we only got one object dn.
    if (count($target) == 1) {
      $this->is_single_edit = TRUE;

      // Get the dn of the object and creates lock
      $this->dn = array_pop($target);
      set_object_info($this->dn);
      if ($locks = get_locks($this->dn)) {
        return gen_locked_message($locks, $this->dn, TRUE);
      }
      add_lock ($this->dn, $ui->dn);

      // Open object.
      if (empty($tabClass) || empty($tabType)) {
        trigger_error("We can't edit any object(s). 'tabClass' or 'tabType' is empty!");
      } else {
        $tab = $tabClass;
        $this->openTabObject(
          new $tab(NULL, $config->data['TABS'][$tabType], $this->dn, $aclCategory),
          $this->dn
        );
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "Edit entry initiated!");
      }
    }
  }


  /*!
   * \brief  Save object modifications and closes dialogs (returns to object listing).
   *          - Calls 'tab::check' to validate the given input.
   *          - Calls 'tab::save' to save back object modifications (e.g. to ldap).
   *          - Calls 'management::remove_locks' to remove eventually created locks.
   *          - Calls 'management::closeDialogs' to return to the object listing.
   */
  protected function saveChanges()
  {
    if ($this->tabObject instanceOf simpleTabs) {
      $this->tabObject->save_object();
      $msgs = $this->tabObject->check();
      if (count($msgs)) {
        msg_dialog::displayChecks($msgs);
        return;
      } else {
        $this->tabObject->save();
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dns, 'Entry saved!');
        $this->remove_lock();
        $this->closeDialogs();
      }
    } elseif ($this->dialogObject instanceOf plugin) {
      $this->dialogObject->save_object();
      $msgs = $this->dialogObject->check();
      if (count($msgs)) {
        msg_dialog::displayChecks($msgs);
        return;
      } else {
        $this->dialogObject->save();
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dns, 'Entry saved!');
        $this->remove_lock();
        $this->closeDialogs();
      }
    }
  }


  /*!
   *  \brief  Save object modifications and keep dialogs opened.
   *          - Calls 'ldap::check' to validate the given input.
   *          - Calls 'ldap::save' to save back object modifications (e.g. to ldap).
   */
  protected function applyChanges()
  {
    if ($this->tabObject instanceOf simpleTabs) {
      $this->tabObject->save_object();
      $msgs = $this->tabObject->check();
      if (count($msgs)) {
        msg_dialog::displayChecks($msgs);
        return "";
      } else {
        $this->tabObject->save();
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dns, "Modifications applied!");
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
   * \brief  Editing an object was caneled.
   *          Close dialogs/tabs and remove locks.
   */
  protected function cancelEdit()
  {
    $this->remove_lock();
    $this->closeDialogs();
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
   * \brief  This method is used to queue and process copy&paste actions.
   *         Allows to copy, cut and paste mutliple entries at once.
   *
   * \param String $action The name of the action which was the used as trigger.
   *
   * \param Array $target A list of object dns, which should be affected by this method.
   *
   * \param Array $all A combination of both 'action' and 'target'.
   *
   * \param String $altTabClass Empty string.
   *
   * \param $altTabType Empty string.
   *
   * \param $altAclCategory Empty string.
   *
   * \param $altAclPlugin Empty string.
   */
  function copyPasteHandler($action = "", $target = array(), $all = array(),
      $altTabClass = "", $altTabType = "", $altAclCategory = "", $altAclPlugin = "")
  {
    global $ui;

    // Return without any actions while copy&paste handler is disabled.
    if (!is_object($this->cpHandler)) {
      return FALSE;
    }

    $tabType      = $this->tabType;
    $tabClass     = $this->tabClass;
    $aclCategory  = $this->aclCategory;
    $aclPlugin    = $this->aclPlugin;
    if (!empty($altTabClass) && !is_array($altTabClass)) {
      $tabClass = $altTabClass;
    }
    if (!empty($altTabType) && !is_array($altTabType)) {
      $tabType = $altTabType;
    }
    if (!empty($altAclCategory) && !is_array($altAclCategory)) {
      $aclCategory = $altAclCategory;
    }
    if (!empty($altAclPlugin) && !is_array($altAclPlugin)) {
      $aclPlugin = $altAclPlugin;
    }

    // Save user input
    $this->cpHandler->save_object();

    // Add entries to queue
    if ($action == "copy" || $action == "cut") {
      $this->cpHandler->cleanup_queue();
      foreach ($target as $dn) {
        if (is_array($altTabClass)) {
          $tabClass = (empty($altTabClass[$dn])?$this->tabClass:$altTabClass[$dn]);
        }
        if (is_array($altTabType)) {
          $tabType = $altTabType[$dn];
        }
        if (is_array($altAclCategory)) {
          $aclCategory = (empty($altAclCategory[$dn])?$this->aclCategory:$altAclCategory[$dn]);
        }
        if (is_array($altAclPlugin)) {
          $aclPlugin = $altAclPlugin[$dn];
        }

        if ($action == "copy" && $ui->is_copyable($dn, $aclCategory, $aclPlugin)) {
          $this->cpHandler->add_to_queue($dn, "copy", $tabClass, $tabType, $aclCategory);
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, "Entry copied!");
        }
        if ($action == "cut" && $ui->is_cutable($dn, $aclCategory, $aclPlugin)) {
          $this->cpHandler->add_to_queue($dn, "cut", $tabClass, $tabType, $aclCategory);
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, "Entry cutted!");
        }
      }
    }

    // Initiate pasting
    if ($action == "paste") {
      $this->cpPastingStarted = TRUE;
    }

    // Display any c&p dialogs, eg. object modifications required before pasting.
    if ($this->cpPastingStarted && $this->cpHandler->entries_queued()) {
      $data = $this->cpHandler->execute($this->headpage->getBase());
      if (!empty($data)) {
        return $data;
      }
    }

    // Automatically disable pasting process since there is no entry left to paste.
    if (!$this->cpHandler->entries_queued()) {
      $this->cpPastingStarted = FALSE;
    }
    return "";
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
