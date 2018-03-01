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
  public $listing;

  /* managementFilter instance which manages the filters */
  public $filter;

  /* Copy&Paste */
  protected $cpHandler        = NULL;
  protected $cpPastingStarted = FALSE;
  protected $skipCpHandler    = FALSE;

  /* Snapshots */
  protected $snapHandler        = NULL;
  public static $skipSnapshots  = FALSE;

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

  public $headline;
  public $title;
  public $icon;

  protected $actions        = array();
  protected $actionHandlers = array();

  protected $exporters = array();

  public $neededAttrs = array();

  public static $skipTemplates = TRUE;

  /* Default columns */
  public static $columns = array(
    array('ObjectTypeColumn', array()),
    array('LinkColumn',       array('attribute' => 'cn',          'label' => 'Name')),
    array('LinkColumn',       array('attribute' => 'description', 'label' => 'Description')),
    array('ActionsColumn',    array('label' => 'Actions')),
  );

  // Whether to display a header or not.
  protected $skipHeader = FALSE;

  function __construct($objectTypes = FALSE)
  {
    global $config, $class_mapping;

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

    $this->listing  = new managementListing($this);
    $this->filter   = new managementFilter($this);

    // Add copy&paste and snapshot handler.
    if (!$this->skipCpHandler) {
      $this->cpHandler = new CopyPasteHandler();
    }
    if (!static::$skipSnapshots && ($config->get_cfg_value('enableSnapshots') == 'TRUE')) {
      $this->snapHandler = new SnapshotHandler();
    }

    // Load exporters
    foreach (array_keys($class_mapping) as $class) {
      if (preg_match('/Exporter$/', $class)) {
        $info = call_user_func(array($class, 'getInfo'));
        if ($info != NULL) {
          $this->exporters = array_merge($this->exporters, $info);
        }
      }
    }

    $this->configureActions();
  }

  protected function configureActions()
  {
    global $config;

    // Register default actions
    $createMenu = array();

    if (!static::$skipTemplates) {
      $templateMenu     = array();
      $fromTemplateMenu = array();
    }

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
      if (!static::$skipTemplates) {
        // TODO fix ACL
        $templateMenu[] = new Action(
          'new_template_'.$type, $infos['name'], $icon,
          '0', 'newEntryTemplate',
          array('c')
        );
        $fromTemplateMenu[] = new Action(
          'template_apply_'.$type, $infos['name'], $icon,
          '?', 'newEntryFromTemplate',
          array('c')
        );
      }
    }

    if (!static::$skipTemplates) {
      $createMenu =
        array_merge(
          array(
            new ActionSubMenu(
              'template', _('Template'), 'geticon.php?context=devices&icon=template&size=16',
              $templateMenu
            ),
            new ActionSubMenu(
              'fromtemplate', _('From template'), 'geticon.php?context=actions&icon=document-new&size=16',
              $fromTemplateMenu
            ),
          ),
          $createMenu
        );
    }

    $this->registerAction(
      new ActionSubMenu(
        'new', _('Create'), 'geticon.php?context=actions&icon=document-new&size=16',
        $createMenu
      )
    );

    // Add export actions
    $exportMenu = array();
    foreach ($this->exporters as $action => $exporter) {
      $exportMenu[] = new Action(
        $action, $exporter['label'], $exporter['image'],
        '0', 'export'
      );
    }
    $this->registerAction(
      new ActionSubMenu(
        'export', _('Export list'), 'geticon.php?context=actions&icon=document-export&size=16',
        $exportMenu
      )
    );

    $this->registerAction(
      new Action(
        'edit', _('Edit'), 'geticon.php?context=actions&icon=document-edit&size=16',
        '+', 'editEntry'
      )
    );

    if (!$this->skipCpHandler) {
      $this->registerAction(
        new Action(
          'cut', _('Cut'), 'geticon.php?context=actions&icon=edit-cut&size=16',
          '+', 'copyPasteHandler',
          array('dr')
        )
      );
      $this->registerAction(
        new Action(
          'copy', _('Copy'), 'geticon.php?context=actions&icon=edit-copy&size=16',
          '+', 'copyPasteHandler',
          array('r')
        )
      );
      // TODO: disable paste when there is nothing to paste
      $this->registerAction(
        new Action(
          'paste', _('Paste'), 'geticon.php?context=actions&icon=edit-paste&size=16',
          '0', 'copyPasteHandler',
          array('w')
        )
      );
    }

    $this->registerAction(
      new Action(
        'remove', _('Remove'), 'geticon.php?context=actions&icon=edit-delete&size=16',
        '+', 'removeRequested',
        array('d')
      )
    );

    if (!self::$skipSnapshots && ($config->get_cfg_value('enableSnapshots') == 'TRUE')) {
      $this->registerAction(
        new Action(
          'snapshot', _('Create snapshot'), 'geticon.php?context=actions&icon=snapshot&size=16',
          '1', 'createSnapshotDialog'
        )
      );
      $this->registerAction(
        new Action(
          'restore', _('Restore snapshot'), 'geticon.php?context=actions&icon=document-restore&size=16',
          '*', 'restoreSnapshotDialog',
          array('w')
        )
      );
    }

    /* Actions from footer are not in any menus and do not need a label */
    $this->registerAction(new HiddenAction('apply',           'applyChanges'));
    $this->registerAction(new HiddenAction('save',            'saveChanges'));
    $this->registerAction(new HiddenAction('cancel',          'cancelEdit'));
    $this->registerAction(new HiddenAction('cancelDelete',    'cancelEdit'));
    $this->registerAction(new HiddenAction('removeConfirmed', 'removeConfirmed'));
    $this->registerAction(new HiddenAction('configure',       'configureDialog'));
  }

  /*!
   *  \brief Register an action to show in the action menu and/or the action column
   */
  function registerAction(Action $action)
  {
    $this->actions[$action->getName()] = $action;
    foreach ($action->listActions() as $actionName) {
      $this->actionHandlers[$actionName] = $action;
    }
  }

  public function getColumnConfiguration()
  {
    if (!isset($this->columnConfiguration)) {
      // Default configuration
      $this->columnConfiguration = static::$columns;
    }
    // Session configuration
    return $this->columnConfiguration;
    // TODO: LDAP configuration
  }

  public function setColumnConfiguration($columns)
  {
    $this->columnConfiguration = $columns;
    $this->listing->reloadColumns();
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
    if (!isset($action['targets'])) {
      $action['targets'] = array();
    }
    if (isset($action['subaction']) && isset($this->actionHandlers[$action['action'].'_'.$action['subaction']])) {
      return $this->actionHandlers[$action['action'].'_'.$action['subaction']]->execute($this, $action);
    } elseif (isset($this->actionHandlers[$action['action']])) {
      return $this->actionHandlers[$action['action']]->execute($this, $action);
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
    $vars = array('/^act$/','/^listing/','/^PID$/');
    session::set('LOCK_VARS_TO_USE', $vars);

    /* Display the copy & paste dialog, if it is currently open */
    $ret = $this->copyPasteHandler();
    if ($ret) {
      return $this->getHeader().$ret;
    }

    // Handle actions (POSTs and GETs)
    $action = $this->detectPostActions();
    if (!empty($action['action'])) {
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action, 'Action');
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
    if ($this->filter) {
      $this->filter->update();
    }

    // Update list
    $this->listing->update();

    // Display list
    return $this->renderList();
  }

  function renderList()
  {
    global $config;

    $list     = $this->listing->render();
    $filter   = $this->filter->render();
    $actions  = $this->renderActionMenu();

    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    $smarty->assign('FILTER',       $filter);
    $smarty->assign('LIST',         $list);
    $smarty->assign('SIZELIMIT',    print_sizelimit_warning());
    $smarty->assign('MULTISELECT',  $this->listing->multiSelect);

    // Assign navigation elements
    $smarty->assign('NAVIGATION', $this->listing->renderNavigation()); // TODO: move here?

    //~ // Assign action menu / base
    $smarty->assign('ACTIONS', $actions);
    $smarty->assign('BASE', $this->listing->renderBase()); // TODO: move here?

    // Assign summary
    $smarty->assign('HEADLINE', $this->headline);

    return $this->getHeader().$smarty->fetch(get_template_path('management/management.tpl'));
  }

  function renderActionMenu()
  {
    $menuActions = array();
    foreach ($this->actions as $action) {
      // TODO: ACL handling
      // Build ul/li list
      $action->fillMenuItems($menuActions);
    }
    $smarty = get_smarty();
    $smarty->assign('actions', $menuActions);
    return $smarty->fetch(get_template_path('management/actionmenu.tpl'));
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

  protected function listAclCategories()
  {
    $cat = array();
    foreach ($this->objectTypes as $type) {
      $infos  = objects::infos($type);
      $cat[]  = $infos['aclCategory'];
    }
    return array_unique($cat);
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

    // TODO move to smarty template
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
      $str .= '<input type="submit" name="edit_finish" value="'.msgPool::okButton().'"/>'."\n";
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

  function handleTemplateApply ($cancel = FALSE)
  {
    if (static::$skipTemplates) {
      return;
    }
    if ($cancel) {
      $msgs = array();
    } else {
      $msgs = $this->tabObject->save();
    }
    if (count($msgs)) {
      msg_dialog::displayChecks($msgs);
      return;
    } else {
      if (!$cancel) {
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, 'Template applied!');
      }
      del_lock($this->dn);
      if (empty($this->dns)) {
        $this->closeDialogs();
      } else {
        $this->last_tabObject = $this->tabObject;
        $this->tabObject      = NULL;
        $this->dn             = array_shift($this->dns);
        $this->dialogObject->setNextTarget($this->dn);
        $this->dialogObject->save_object();
      }
    }
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
    $type = $action['subaction'];

    $this->dn = 'new';

    set_object_info($this->dn);

    // Open object
    $this->openTabObject(objects::create($type), $this->listing->getBase());
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, 'Create entry initiated');
  }

  function newEntryTemplate(array $action)
  {
    if (static::$skipTemplates) {
      return;
    }
    $action['subaction'] = preg_replace('/^template_/', '', $action['subaction']);
    $this->newEntry($action);
    $this->tabObject->setTemplateMode('template_cn');
  }

  function newEntryFromTemplate(array $action)
  {
    if (static::$skipTemplates) {
      return;
    }
    if (isset($action['targets'][0])) {
      $dn = $action['targets'][0];
    } else {
      $dn = NULL;
    }
    if ($action['subaction'] == 'apply') {
      if ($dn === NULL) {
        return;
      }
      $type = $this->listing->getEntry($dn)->getTemplatedType();
    } else {
      $type = preg_replace('/^apply_/', '', $action['subaction']);
    }
    $this->dialogObject = new templateDialog($this, $type, $dn);
  }

  function applyTemplateToEntry(array $action)
  {
    global $ui;
    if (static::$skipTemplates) {
      return;
    }
    if (empty($action['targets'])) {
      return;
    }
    $type       = preg_replace('/^template_apply_to_/', '', $action['action']);
    $this->dns  = $action['targets'];

    // check locks
    if ($locks = get_locks($this->dns)) {
      return gen_locked_message($locks, $this->dns);
    }

    // Add locks
    add_lock ($this->dns, $ui->dn);

    $this->dn           = array_shift($this->dns);
    $this->dialogObject = new templateDialog($this, $type, NULL, $this->dn);
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

    $entry = $this->listing->getEntry($target);
    if ($entry === NULL) {
      trigger_error('Could not find '.$target.', open canceled');
      return;
    }

    // Get the dn of the object and create lock
    $this->dn = $target;
    set_object_info($this->dn);
    if ($locks = get_locks($this->dn, TRUE)) {
      return gen_locked_message($locks, $this->dn, TRUE);
    }
    add_lock ($this->dn, $ui->dn);

    // Open object
    $this->openTabObject(objects::open($this->dn, $entry->getTemplatedType()), $this->dn);
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
    if (($this->tabObject instanceOf simpleTabs) && ($this->dialogObject instanceOf templateDialog)) {
      $this->handleTemplateApply(TRUE);
      return;
    }
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
    if (($this->tabObject instanceOf simpleTabs) && ($this->dialogObject instanceOf templateDialog)) {
      $this->tabObject->save_object();
      $this->handleTemplateApply();
      return;
    }
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
      $entry = $this->listing->getEntry($dn);
      try {
        if ($entry->checkAcl('d')) {
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
        $infos  = objects::infos($entry->getTemplatedType());
        if ($entry->isTemplate()) {
          $infos['nameAttr'] = 'cn';
        }
        $objects[] = array(
          'name'  => $entry[$infos['nameAttr']],
          'dn'    => $dn,
          'icon'  => $infos['icon'],
          'type'  => $infos['name']
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
      $entry = $this->listing->getEntry($dn);
      if (empty($entry)) {
        continue;
      }
      if ($entry->checkAcl('d')) {
        // Delete the object
        $this->dn = $dn;
        $this->openTabObject(objects::open($this->dn, $entry->getTemplatedType()), $this->dn);
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

  function configureDialog(array $action)
  {
    $this->dialogObject = new ManagementConfigurationDialog($this);
  }

  /*! \brief  This method is used to queue and process copy&paste actions.
   *          Allows to copy, cut and paste mutliple entries at once.
   */
  function copyPasteHandler(array $action = array('action' => ''))
  {
    global $ui;

    // Exit if copy&paste handler is disabled.
    if (!is_object($this->cpHandler)) {
      return FALSE;
    }

    // Save user input
    $this->cpHandler->save_object();

    // Add entries to queue
    if (($action['action'] == 'copy') || ($action['action'] == 'cut')) {
      $this->cpHandler->cleanup_queue();
      foreach ($action['targets'] as $dn) {
        $entry = $this->listing->getEntry($dn);
        if (($action['action'] == 'copy') && $entry->checkAcl('r')) {
          $this->cpHandler->add_to_queue($dn, 'copy', $entry->getTemplatedType());
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Entry copied!');
        }
        if (($action['action'] == 'cut') && $entry->checkAcl('rd')) {
          $this->cpHandler->add_to_queue($dn, 'cut', $entry->getTemplatedType());
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Entry cut!');
        }
      }
    }

    // Initiate pasting
    if ($action['action'] == 'paste') {
      $this->cpPastingStarted = TRUE;
    }

    // Display any c&p dialogs, eg. object modifications required before pasting.
    if ($this->cpPastingStarted && $this->cpHandler->entries_queued()) {
      $data = $this->cpHandler->execute($this->listing->getBase());
      if (!empty($data)) {
        return $data;
      }
    }

    // Automatically disable pasting process since there is no entry left to paste.
    if (!$this->cpHandler->entries_queued()) {
      $this->cpPastingStarted = FALSE;
      $this->cpHandler->resetPaste();
    }

    return '';
  }

  /*!
   * \brief  Opens the snapshot creation dialog for the given target.
   */
  function createSnapshotDialog(array $action)
  {
    global $config, $ui;
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action['targets'], 'Snaptshot creation initiated!');

    $this->dn = array_pop($action['targets']);
    if (empty($this->dn)) {
      return;
    }
    $entry = $this->listing->getEntry($this->dn);
    if ($entry->snapshotCreationAllowed()) {
      $this->dialogObject = new SnapshotCreateDialog($this->dn, $this, '');
      $this->dialogObject->set_acl_base($this->dn);
    } else {
      msg_dialog::display(_('Permission'), sprintf(_('You are not allowed to create a snapshot for %s.'), $this->dn),
          ERROR_DIALOG);
    }
  }

  /*!
   * \brief  Displays the "Restore snapshot dialog" for a given target.
   *          If no target is specified, open the restore removed object dialog.
   */
  function restoreSnapshotDialog(array $action)
  {
    global $config, $ui;

    if (!count($action['targets'])) {
      // No target, open the restore removed object dialog.
      $this->dn     = $this->listing->getBase();
      $aclCategory  = $this->listAclCategories();
    } else {
      // Display the restore points for a given object.
      $this->dn = $action['targets'][0];
      if (empty($this->dn)) {
        return;
      }
      $aclCategory = objects::infos($this->getEntry($this->dn)->getTemplatedType())['aclCategory'];
    }

    if ($ui->allow_snapshot_restore($this->dn, $aclCategory, !count($action['targets']))) {
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, 'Snaptshot restoring initiated!');
      $this->dialogObject = new SnapshotRestoreDialog($this->dn, $this, !count($action['targets']), $aclCategory);
    } else {
      msg_dialog::display(_('Permission'), sprintf(_('You are not allowed to restore a snapshot for %s.'), $this->dn),
          ERROR_DIALOG);
    }
  }


  function export(array $action)
  {
    if (!isset($this->exporters[$action['action']])) {
      trigger_error('Unknown exporter '.$action['action']);
      return;
    }
    $exporter = $this->exporters[$action['action']];
    $file     = $exporter['class']::export($this->listing);
    send_binary_content($file, $exporter['filename'], $exporter['mime']);
  }

  /* End of action handlers */

  /* Methods related to Snapshots */

  function getSnapshotBases()
  {
    $bases = array();
    foreach ($this->objectTypes as $type) {
      $infos  = objects::infos($type);
      $bases[] = $infos['ou'].$this->listing->getBase();
    }

    // No bases specified? Try base
    if (!count($bases)) {
      $bases[] = $this->listing->getBase();
    }

    return array_unique($bases);
  }

  /*!
   * \brief Get all deleted snapshots
   */
  function getAllDeletedSnapshots()
  {
    $bases = $this->getSnapshotBases();
    $tmp = array();
    foreach ($bases as $base) {
      $tmp = array_merge($tmp, $this->snapHandler->getAllDeletedSnapshots($base));
    }
    return $tmp;
  }

  /*
   * \brief Return available snapshots for the given base
   *
   * \param string $dn The DN
   */
  function getAvailableSnapsShots($dn)
  {
    return $this->snapHandler->getAvailableSnapsShots($dn);
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
    if (!empty($dn) && $ui->allow_snapshot_restore($dn, $this->dialogObject->aclCategory, $this->dialogObject->global)) {
      $this->snapHandler->restoreSnapshot($dn);
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Snaptshot restored');
      $this->closeDialogs();
    } else {
      msg_dialog::display(_('Permission'), sprintf(_('You are not allowed to restore a snapshot for %s.'), $dn),
          ERROR_DIALOG);
    }
  }

  /*!
   * \brief Delete a snapshot
   *
   * \param string $dn DN of the snapshot
   */
  function removeSnapshot($dn)
  {
    global $ui;
    if (!empty($dn) && $ui->allow_snapshot_delete($dn, $this->dialogObject->aclCategory)) {
      $this->snapHandler->removeSnapshot($dn);
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Snaptshot deleted');
    } else {
      msg_dialog::display(_('Permission'), sprintf(_('You are not allowed to delete a snapshot for %s.'), $dn),
          ERROR_DIALOG);
    }
  }


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
