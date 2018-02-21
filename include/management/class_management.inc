<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018  FusionDirectory

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
 * \brief Management base class
 */
class management
{
  /* Object types we are currently managing */
  public $objectTypes;

  /* managementListing instance which manages the entries */
  protected $listing;

  // The currently used object(s) (e.g. in edit, removal)
  protected $dn      = '';
  protected $dns  = array();

  // The last used object(s).
  protected $last_dn  = '';
  protected $last_dns = array();

  // The opened object.
  protected $tabObject    = NULL;
  protected $dialogObject = NULL;

  // The last opened object.
  protected $last_tabObject     = NULL;
  protected $last_dialogObject  = NULL;

  protected $headline;
  protected $title;
  protected $icon;

  protected $actions = array();

  public $neededAttrs = array();

  // Whether to display a header or not.
  protected $skipHeader = FALSE;

  function __construct($objectTypes = FALSE)
  {
    global $config;

    $plInfos = pluglist::pluginInfos(get_class($this));
    if ($objectTypes === FALSE) {
      $objectTypes  = $plInfos['plManages'];
    }
    $this->headline = $plInfos['plShortName'];
    $this->title    = $plInfos['plTitle'];
    $this->icon     = $plInfos['plIcon'];

    if (!preg_match('/^geticon/', $this->icon)) {
      $this->icon = get_template_path($this->icon);
    }

    $this->objectTypes = $objectTypes;

    $this->listing = new managementListing($this);

    // Build filter
    //~ if (session::global_is_set(get_class($this).'_filter')) {
      //~ $filter = session::global_get(get_class($this).'_filter');
    //~ } else {
      //~ $filter = new filter();
      //~ $filter->setObjectStorage($this->storagePoints);
    //~ }
    //~ $this->setFilter($filter);

    $this->configureActions();

    //~ $this->configureFilter();
    //~ if ($this->baseMode === FALSE) {
      //~ $this->headpage->setBase($config->current['BASE']);
    //~ }
  }

  protected function configureActions()
  {
    // Register default actions
    $createMenu = array();
    foreach ($this->objectTypes as $type) {
      $infos  = objects::infos($type);
      $icon   = 'geticon.php?context=actions&icon=document-new&size=16';
      if (isset($infos['icon'])) {
        $icon = $infos['icon'];
      }
      $createMenu[] = new Action(
        'new_'.$type, $infos['name'], $icon,
        '0', 'newEntry',
        array('c')
      );
    }
    $this->registerAction(
      new ActionSubMenu(
        'new', _('Create'), 'geticon.php?context=actions&icon=document-new&size=16',
        $createMenu
      )
    );

    $this->registerAction(
      new Action(
        'edit', _('Edit'), 'geticon.php?context=actions&icon=document-edit&size=16',
        '+', 'editEntry'
      )
    );
    $this->registerAction(
      new Action(
        'remove', _('Remove'), 'geticon.php?context=actions&icon=edit-delete&size=16',
        '+', 'removeRequested',
        array('d')
      )
    );

    /* Actions from footer are not in any menus and do not need a label */
    $this->registerAction(new HiddenAction('apply',           'applyChanges'));
    $this->registerAction(new HiddenAction('save',            'saveChanges'));
    $this->registerAction(new HiddenAction('cancel',          'cancelEdit'));
    $this->registerAction(new HiddenAction('cancelDelete',    'cancelEdit'));
    $this->registerAction(new HiddenAction('removeConfirmed', 'removeConfirmed'));
  }

  /*!
   *  \brief Register an action to show in the action menu and/or the action column
   */
  function registerAction(Action $action)
  {
    foreach ($action->listActions() as $actionName) {
      $this->actions[$actionName] = $action;
    }
  }

  /*!
   * \brief  Detects actions/events send by the ui
   *           and the corresponding targets.
   */
  function detectPostActions()
  {
    // TODO: do not check all actions in dialog/tab mode
    if (!is_object($this->listing)) {
      throw new Exception('No valid listing object');
    }
    $action = $this->listing->getAction();
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
  function handleAction(array $action)
  {
    // Start action
    if (isset($this->actions[$action['action']])) {
      if (!isset($action['targets'])) {
        $action['targets'] = array();
      }
      return $this->actions[$action['action']]->execute($this, $action);
    }
  }

  protected function handleSubAction(array $action)
  {
    if (preg_match('/^tab_/', $action['subaction'])) {
      $tab = preg_replace('/^tab_/', '', $action['subaction']);
      if (isset($this->tabObject->by_object[$tab])) {
        $this->tabObject->current = $tab;
      } else {
        trigger_error('Unknown tab: '.$tab);
      }
      return TRUE;
    }
    return FALSE;
  }

  /*!
   * \brief  Execute this plugin
   *          Handle actions/events, locking, snapshots, dialogs, tabs,...
   */
  function execute()
  {
    // Ensure that html posts and gets are kept even if we see a 'Entry islocked' dialog.
    //~ $vars = array('/^act$/','/^listing/','/^PID$/');
    //~ session::set('LOCK_VARS_TO_USE', $vars);

    // Handle actions (POSTs and GETs)
    $action = $this->detectPostActions();
    if (!empty($action)) {
      $str = $this->handleAction($action);
      if ($str) {
        return $this->getHeader().$str;
      }
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

    // Update filter
    //~ if ($this->filter) {
      //~ $this->filter->update();
      //~ session::global_set(get_class($this).'_filter', $this->filter);
      //~ session::set('autocomplete', $this->filter);
      //~ if (!$this->filter->isValid()) {
        //~ msg_dialog::display(_('Filter error'), _('The filter is incomplete!'), ERROR_DIALOG);
      //~ }
    //~ }

    // Update list
    $this->listing->update();

    // Display list
    return $this->renderList();
  }

  function renderList()
  {
    global $config;

    //~ $smarty = get_smarty();
    //~ $filters = array();
    //~ foreach ($this->objectTypes as $object) {
      //~ $filters[] = array(
        //~ 'id'    => 'FILTER'.$object,
        //~ 'label' => sprintf(_('Show %s'), $config->data['OBJECTS'][$object]['name'])
      //~ );
    //~ }
    //~ if (!static::$skipTemplates) {
      //~ $filters[] = array(
        //~ 'id'    => 'FILTERTEMPLATE',
        //~ 'label' => sprintf(_('Show %s'), _('Template'))
      //~ );
    //~ }
    //~ $smarty->assign('objectFilters', $filters);
    //~ $this->headpage->update();
    //~ $display = $this->headpage->render();
    //~ return $this->getHeader().$display;

    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    $smarty->assign('FILTER', '');
    //~ $smarty->assign("FILTER", $this->filter->render());
    $smarty->assign('SIZELIMIT',    print_sizelimit_warning());
    $smarty->assign('LIST',         $this->listing->render());
    $smarty->assign('MULTISELECT',  $this->listing->multiSelect);

    // Assign navigation elements
    $nav = $this->listing->renderNavigation(); // TODO: move here?
    foreach ($nav as $key => $html) {
      $smarty->assign($key, $html);
    }

    //~ // Assign action menu / base
    $smarty->assign('ACTIONS', $this->renderActionMenu());
    $smarty->assign('BASE', $this->listing->renderBase()); // TODO: move here?

    // Assign summary
    $smarty->assign('HEADLINE', $this->headline);

    return $this->getHeader().$smarty->fetch(get_template_path('simple-list.tpl'));
  }

  function renderActionMenu()
  {
    // Load shortcut
    $result   = '<input type="hidden" name="act" id="actionmenu" value=""/>'."\n".
                '<div style="display:none"><input type="submit" name="exec_act" id="exec_act" value=""/></div>'."\n".
                '<ul class="level1" id="root"><li><a href="#">'._('Actions').
                '&nbsp;<img class="center optional" src="images/down-arrow.png" alt="down arrow"/></a>'."\n";
    $result   .= '<ul class="level2">'."\n";
    foreach ($this->actions as $action) {
      // TODO: ACL handling
      // Build ul/li list
      $result .= $action->renderMenuItems();
    }

    return '<div id="pulldown">'.$result.'</ul></li></ul></div>'."\n";
  }

  function renderActionColumn(ListingEntry $entry)
  {
    // Go thru all actions
    $result = '';
    foreach ($this->actions as $action) {
      $result .= $action->renderColumnIcons($entry);
    }

    return $result;
  }

  function getActionRowClasses(ListingEntry $entry)
  {
    $classes = array();
    foreach ($this->actions as $action) {
      // TODO optimize
      $classes = array_merge($classes, $action->getRowClasses($entry));
    }

    return $classes;
  }

  /*!
   * \brief  Removes ldap object locks created by this class.
   *         Whenever an object is edited, we create locks to avoid
   *         concurrent modifications.
   *         This locks will automatically removed here.
   */
  function remove_lock()
  {
    if (!empty($this->dn) && $this->dn != 'new') {
      del_lock($this->dn);
    }
    if (count($this->dns)) {
      del_lock($this->dns);
    }
  }

  function is_modal_dialog()
  {
    return (is_object($this->tabObject) || is_object($this->dialogObject));
  }

  /*!
   * \brief  Generates the plugin header which is displayed whenever a tab object is opened.
   */
  protected function getHeader()
  {
    if ($this->skipHeader) {
      return '';
    }

    return print_header($this->icon, $this->title, get_object_info());
  }

  function openTabObject($object, $base)
  {
    $this->tabObject = $object;
    $this->tabObject->set_acl_base($base);
    $this->tabObject->parent = &$this;
  }

  /*!
   * \brief  This method closes dialogs
   *          and cleans up the cached object info and the ui.
   */
  public function closeDialogs()
  {
    $this->last_dn  = $this->dn;
    $this->dn       = '';
    $this->last_dns = $this->dns;
    $this->dns      = array();

    $this->last_tabObject     = $this->tabObject;
    $this->tabObject          = NULL;
    $this->last_dialogObject  = $this->dialogObject;
    $this->dialogObject       = NULL;

    set_object_info();
  }

  /*!
   * \brief  Generates the footer which is used whenever a tab object is displayed.
   */
  protected function _getTabFooter()
  {
    // Do not display tab footer for non tab objects
    if (!($this->tabObject instanceOf simpleTabs)) {
      return '';
    }

    // Check if there is a dialog opened - We don't need any buttons in this case.
    if ($this->tabObject->dialogOpened()) {
      return '';
    }

    // In case an of locked entry, we may have opened a read-only tab.
    $str = '';
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
      if ($this->dn != 'new') {
        $str .= '<input type="submit" name="edit_apply" value="'.msgPool::applyButton().'"/>'."\n";
        $str .= "&nbsp;\n";
      }
      $str .= '<input type="submit" name="edit_cancel" value="'.msgPool::cancelButton().'"/>'."\n";
      $str .= '</p>';
    }
    return $str;
  }

  /* Action handlers */

  /*!
   * \brief  This method intiates the object creation.
   *
   * \param  array  $action A combination of both 'action' and 'target':
   *                        action: The name of the action which was the used as trigger.
   *                        target: A list of object dns, which should be affected by this method.
   */
  function newEntry(array $action)
  {
    $type = preg_replace('/^new_/', '', $action['action']);

    $this->dn = 'new';

    set_object_info($this->dn);

    // Open object
    $this->openTabObject(objects::create($type), $this->listing->getBase());
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, 'Create entry initiated');
  }

  /*!
   * \brief  This method opens an existing object to be edited.
   *
   * \param  array  $action A combination of both 'action' and 'target':
   *                        action: The name of the action which was the used as trigger.
   *                        target: A list of object dns, which should be affected by this method.
   */
  function editEntry(array $action)
  {
    global $ui;

    // Do not create a new tabObject while there is already one opened,
    //  the user may have just pressed F5 to reload the page.
    if (is_object($this->tabObject)) {
      return;
    }

    $target = array_pop($action['targets']);

    $type = $this->listing->getType($target);
    if ($type === NULL) {
      trigger_error('Could not find type for '.$target.', open canceled');
      return;
    }
    //~ if (preg_match('/^template_/', $type) && !static::$skipTemplates) {
      //~ $type = preg_replace('/^template_/', '', $type);
    //~ }

    // Get the dn of the object and create lock
    $this->dn = $target;
    set_object_info($this->dn);
    if ($locks = get_locks($this->dn, TRUE)) {
      return gen_locked_message($locks, $this->dn, TRUE);
    }
    add_lock ($this->dn, $ui->dn);

    // Open object
    $this->openTabObject(objects::open($this->dn, $type), $this->dn);
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, 'Edit entry initiated');
    if (isset($action['subaction'])) {
      if ($this->handleSubAction($action) == FALSE) {
        trigger_error('Was not able to handle subaction: '.$action['subaction']);
      }
    }
  }

  /*!
   * \brief  Editing an object was caneled.
   *          Close dialogs/tabs and remove locks.
   */
  function cancelEdit()
  {
    //~ if (($this->tabObject instanceOf simpleTabs) && ($this->dialogObject instanceOf templateDialog)) {
      //~ $this->handleTemplateApply(TRUE);
      //~ return;
    //~ }
    $this->remove_lock();
    $this->closeDialogs();
  }

  /*!
   * \brief  Save object modifications and closes dialogs (returns to object listing).
   *          - Calls 'simpleTabs::save' to save back object modifications (e.g. to ldap).
   *          - Calls 'simpleManagement::closeDialogs' to return to the object listing.
   */
  function saveChanges()
  {
    //~ if (($this->tabObject instanceOf simpleTabs) && ($this->dialogObject instanceOf templateDialog)) {
      //~ $this->tabObject->save_object();
      //~ $this->handleTemplateApply();
      //~ return;
    //~ }
    if ($this->tabObject instanceOf simpleTabs) {
      $this->tabObject->save_object();
      $msgs = $this->tabObject->save();
      if (count($msgs)) {
        msg_dialog::displayChecks($msgs);
        return;
      } else {
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dns, 'Entry saved');
        $this->remove_lock();
        $this->closeDialogs();
      }
    } elseif ($this->dialogObject instanceOf simplePlugin) {
      $this->dialogObject->save_object();
      $msgs = $this->dialogObject->check();
      if (count($msgs)) {
        msg_dialog::displayChecks($msgs);
        return;
      } else {
        $this->dialogObject->save();
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dns, 'Dialog saved');
        $this->remove_lock();
        $this->closeDialogs();
      }
    }
  }

  /*!
   *  \brief  Save object modifications and keep dialogs opened
   */
  function applyChanges()
  {
    if ($this->tabObject instanceOf simpleTabs) {
      $this->tabObject->save_object();
      $msgs = $this->tabObject->save();
      if (count($msgs)) {
        msg_dialog::displayChecks($msgs);
      } else {
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dns, 'Modifications applied');
        $this->tabObject->re_init();
      }
    }
  }

  /*! \brief  Queue selected objects to be removed.
   *          Checks ACLs, Locks and ask for confirmation.
   */
  function removeRequested(array $action)
  {
    global $ui;
    $disallowed = array();
    $this->dns  = array();

    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action['targets'], 'Entry deletion requested');

    // Check permissons for each target
    foreach ($action['targets'] as $dn) {
      $type = $this->listing->getType($dn);
      //~ if (preg_match('/^template_/', $type) && !static::$skipTemplates) {
        //~ $type = preg_replace('/^template_/', '', $type);
      //~ }
      try {
        $info = objects::infos($type);
        $acl  = $ui->get_permissions($dn, $info['aclCategory'].'/'.$info['mainTab']);
        if (preg_match('/d/', $acl)) {
          $this->dns[] = $dn;
        } else {
          $disallowed[] = $dn;
        }
      } catch (NonExistingObjectTypeException $e) {
        trigger_error('Unknown object type received :'.$e->getMessage());
      }
    }
    if (count($disallowed)) {
      msg_dialog::display(_('Permission'), msgPool::permDelete($disallowed), INFO_DIALOG);
    }

    // We've at least one entry to delete.
    if (count($this->dns)) {
      // check locks
      if ($locks = get_locks($this->dns)) {
        return gen_locked_message($locks, $this->dns);
      }

      // Add locks
      $objects = array();
      foreach ($this->dns as $dn) {
        $entry  = $this->listing->getEntry($dn);
        $type   = $entry->type;
        if (preg_match('/^template_/', $type) && !static::$skipTemplates) {
          $type = preg_replace('/^template_/', '', $type);
          $info = objects::infos($type);
          $info['nameAttr'] = 'cn';
        } else {
          $info = objects::infos($type);
        }
        $objects[] = array(
          'name'  => $entry[$info['nameAttr']],
          'dn'    => $dn,
          'icon'  => $info['icon'],
          'type'  => $info['name']
        );
      }
      add_lock ($this->dns, $ui->dn);

      // Display confirmation dialog.
      $smarty = get_smarty();
      $smarty->assign('objects', $objects);
      $smarty->assign('multiple', TRUE);
      return $smarty->fetch(get_template_path('simple-remove.tpl'));
    }
  }

  /*! \brief  Deletion was confirmed, delete the objects queued.
   *          Checks ACLs just in case.
   */
  function removeConfirmed(array $action)
  {
    global $ui;
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dns, 'Entry deletion confirmed');

    foreach ($this->dns as $dn) {
      $type = $this->listing->getType($dn);
      if (empty($type)) {
        continue;
      }
      //~ if (preg_match('/^template_/', $type) && !static::$skipTemplates) {
        //~ $type = preg_replace('/^template_/', '', $type);
      //~ }
      $infos = objects::infos($type);
      // Check permissions, are we allowed to remove this object?
      $acl = $ui->get_permissions($dn, $infos['aclCategory'].'/'.$infos['mainTab']);
      if (preg_match('/d/', $acl)) {
        // Delete the object
        $this->dn = $dn;
        $this->openTabObject(objects::open($this->dn, $type), $this->dn);
        $this->tabObject->delete();

        // Remove the lock for the current object.
        del_lock($this->dn);
      } else {
        msg_dialog::display(_('Permission error'), msgPool::permDelete($dn), ERROR_DIALOG);
        logging::log('security', 'simpleManagement/'.get_class($this), $dn, array(), 'Tried to trick deletion.');
      }
    }

    // Cleanup
    $this->remove_lock();
    $this->closeDialogs();
  }

  /* End of action handlers */

  static function mainInc ($classname, $objectTypes = FALSE)
  {
    global $remove_lock, $cleanup, $display;

    /* Remove locks */
    if ($remove_lock && session::is_set($classname)) {
      $macl = session::get($classname);
      $macl->remove_lock();
    }

    if ($cleanup) {
      /* Clean up */
      session::un_set($classname);
    } else {
      if (!session::is_set($classname) || (isset($_GET['reset']) && $_GET['reset'] == 1)) {
        /* Create the object if missing or reset requested */
        $managementObject = new $classname($objectTypes);
      } else {
        /* Retrieve the object from session */
        $managementObject = session::get($classname);
      }
      /* Execute and display */
      $display = $managementObject->execute();

      /* Store the object in the session */
      session::set($classname, $managementObject);
    }
  }


}
