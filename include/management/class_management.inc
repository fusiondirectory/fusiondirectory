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
  protected $currentDn  = '';
  protected $currentDns = [];

  // The last used object(s).
  protected $previousDn   = '';
  protected $previousDns  = [];

  // The opened object.
  protected $tabObject    = NULL;
  protected $dialogObject = NULL;

  // The last opened object.
  protected $last_tabObject     = NULL;
  protected $last_dialogObject  = NULL;

  public $headline;
  public $title;
  public $icon;

  protected $actions        = [];
  protected $actionHandlers = [];

  protected $exporters = [];

  public $neededAttrs = [];

  public static $skipTemplates = TRUE;

  /* Default columns */
  public static $columns = [
    ['ObjectTypeColumn', []],
    ['LinkColumn',       ['attributes' => 'nameAttr',    'label' => 'Name']],
    ['LinkColumn',       ['attributes' => 'description', 'label' => 'Description']],
    ['ActionsColumn',    ['label' => 'Actions']],
  ];

  // Whether to display a header or not.
  protected $skipHeader = FALSE;

  function __construct ($objectTypes = FALSE)
  {
    global $config, $class_mapping;

    if ($objectTypes === FALSE) {
      $plInfos = pluglist::pluginInfos(get_class($this));
      $objectTypes  = $plInfos['plManages'];
    }

    if (!preg_match('/^geticon/', $this->icon)) {
      $this->icon = get_template_path($this->icon);
    }

    /* Ignore non existing objectTypes. This happens when an optional plugin is missing. */
    foreach ($objectTypes as $key => $type) {
      try {
        objects::infos($type);
        $objectTypes[$key] = strtoupper($type);
      } catch (NonExistingObjectTypeException $e) {
        unset($objectTypes[$key]);
      }
    }

    $this->objectTypes = array_values($objectTypes);

    $this->setUpHeadline();
    $this->setUpListing();
    $this->setUpFilter();

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
        $info = call_user_func([$class, 'getInfo']);
        if ($info != NULL) {
          $this->exporters = array_merge($this->exporters, $info);
        }
      }
    }

    $this->configureActions();
  }

  protected function setUpListing ()
  {
    $this->listing  = new managementListing($this);
  }

  protected function setUpFilter ()
  {
    $this->filter   = new managementFilter($this);
  }

  protected function setUpHeadline ()
  {
    $plInfos = pluglist::pluginInfos(get_class($this));

    $this->headline = $plInfos['plShortName'];
    $this->title    = $plInfos['plTitle'];
    $this->icon     = $plInfos['plIcon'];
  }

  protected function configureActions ()
  {
    global $config;

    // Register default actions
    $createMenu = [];

    if (!static::$skipTemplates) {
      $templateMenu     = [];
      $fromTemplateMenu = [];
    }

    foreach ($this->objectTypes as $type) {
      $infos  = objects::infos($type);
      $img    = 'geticon.php?context=actions&icon=document-new&size=16';
      if (isset($infos['icon'])) {
        $img = $infos['icon'];
      }
      $createMenu[] = new Action(
        'new_'.$type, $infos['name'], $img,
        '0', 'newEntry',
        [$infos['aclCategory'].'/'.$infos['mainTab'].'/c']
      );
      if (!static::$skipTemplates) {
        $templateMenu[] = new Action(
          'new_template_'.$type, $infos['name'], $img,
          '0', 'newEntryTemplate',
          [$infos['aclCategory'].'/template/c']
        );
        $fromTemplateMenu[] = new Action(
          'template_apply_'.$type, $infos['name'], $img,
          '0', 'newEntryFromTemplate',
          [$infos['aclCategory'].'/template/r', $infos['aclCategory'].'/'.$infos['mainTab'].'/c']
        );
      }
    }

    if (!static::$skipTemplates) {
      $createMenu =
        array_merge(
          [
            new SubMenuAction(
              'template', _('Template'), 'geticon.php?context=devices&icon=template&size=16',
              $templateMenu
            ),
            new SubMenuAction(
              'fromtemplate', _('From template'), 'geticon.php?context=actions&icon=document-new&size=16',
              $fromTemplateMenu
            ),
          ],
          $createMenu
        );
    }

    $this->registerAction(
      new SubMenuAction(
        'new', _('Create'), 'geticon.php?context=actions&icon=document-new&size=16',
        $createMenu
      )
    );

    // Add export actions
    $exportMenu = [];
    foreach ($this->exporters as $action => $exporter) {
      $exportMenu[] = new Action(
        $action, $exporter['label'], $exporter['image'],
        '0', 'export'
      );
    }
    $this->registerAction(
      new SubMenuAction(
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
    $this->actions['edit']->setSeparator(TRUE);

    if (!$this->skipCpHandler) {
      $this->registerAction(
        new Action(
          'cut', _('Cut'), 'geticon.php?context=actions&icon=edit-cut&size=16',
          '+', 'copyPasteHandler',
          ['dr']
        )
      );
      $this->registerAction(
        new Action(
          'copy', _('Copy'), 'geticon.php?context=actions&icon=edit-copy&size=16',
          '+', 'copyPasteHandler',
          ['r']
        )
      );
      $this->registerAction(
        new Action(
          'paste', _('Paste'), 'geticon.php?context=actions&icon=edit-paste&size=16',
          '0', 'copyPasteHandler',
          ['w']
        )
      );
      $this->actions['paste']->setEnableFunction([$this, 'enablePaste']);
    }

    if (!static::$skipTemplates) {
      $this->registerAction(
        new Action(
          'template_apply_to', _('Apply template'), 'geticon.php?context=actions&icon=tools-wizard&size=16',
          '+', 'applyTemplateToEntry',
          ['/template/r', 'c'],
          TRUE,
          FALSE
        )
      );
    }

    $this->registerAction(
      new Action(
        'remove', _('Remove'), 'geticon.php?context=actions&icon=edit-delete&size=16',
        '+', 'removeRequested',
        ['d']
      )
    );

    if (!static::$skipSnapshots && ($config->get_cfg_value('enableSnapshots') == 'TRUE')) {
      $this->registerAction(
        new Action(
          'snapshot', _('Create snapshot'), 'geticon.php?context=actions&icon=snapshot&size=16',
          '1', 'createSnapshotDialog',
          ['/snapshot/c']
        )
      );
      $this->registerAction(
        new Action(
          'restore', _('Restore snapshot'), 'geticon.php?context=actions&icon=document-restore&size=16',
          '*', 'restoreSnapshotDialog',
          ['w', '/snapshot/r']
        )
      );
      $this->actions['snapshot']->setSeparator(TRUE);
      $this->actions['restore']->setEnableFunction([$this, 'enableSnapshotRestore']);
    }

    if (!static::$skipTemplates) {
      $this->registerAction(
        new Action(
          'template_apply', _('Create an object from this template'), 'geticon.php?context=actions&icon=document-new&size=16',
          '1', 'newEntryFromTemplate',
          ['/template/r', 'c'],
          FALSE,
          TRUE,
          ['template']
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
  function registerAction (Action $action)
  {
    $action->setParent($this);
    $this->actions[$action->getName()] = $action;
    foreach ($action->listActions() as $actionName) {
      $this->actionHandlers[$actionName] = $action;
    }
  }

  public function getColumnConfiguration (): array
  {
    global $config;

    if (!isset($this->columnConfiguration)) {
      // LDAP configuration
      $this->columnConfiguration = $config->getManagementConfig(get_class($this));
    }

    if (!isset($this->columnConfiguration)) {
      // Default configuration
      $this->columnConfiguration = static::$columns;
    }

    // Session configuration
    return $this->columnConfiguration;
  }

  public function setColumnConfiguration ($columns)
  {
    $this->columnConfiguration = $columns;
    $this->listing->reloadColumns();
  }

  /*!
   * \brief  Detects actions/events send by the ui
   *           and the corresponding targets.
   */
  function detectPostActions (): array
  {
    if (!is_object($this->listing)) {
      throw new FusionDirectoryException('No valid listing object');
    }
    $action = ['targets' => [], 'action' => '', 'subaction' => NULL];
    if ($this->is_modal_dialog()) {
      if (isset($_POST['edit_cancel'])) {
        $action['action'] = 'cancel';
      } elseif (isset($_POST['edit_finish'])) {
        $action['action'] = 'save';
      } elseif (isset($_POST['edit_apply'])) {
        $action['action'] = 'apply';
      }
    } else {
      if (isset($_POST['delete_confirmed'])) {
        $action['action'] = 'removeConfirmed';
      } elseif (isset($_POST['delete_cancel'])) {
        $action['action'] = 'cancelDelete';
      } else {
        $action = $this->listing->getAction();
      }
    }

    return $action;
  }

  /*!
   *  \brief  Calls the registered method for a given action/event.
   */
  function handleAction (array $action)
  {
    // Start action
    if (isset($action['subaction']) && isset($this->actionHandlers[$action['action'].'_'.$action['subaction']])) {
      return $this->actionHandlers[$action['action'].'_'.$action['subaction']]->execute($this, $action);
    } elseif (isset($this->actionHandlers[$action['action']])) {
      return $this->actionHandlers[$action['action']]->execute($this, $action);
    }
  }

  protected function handleSubAction (array $action): bool
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
  function execute (): string
  {
    // Ensure that html posts and gets are kept even if we see a 'Entry islocked' dialog.
    $vars = ['/^act$/','/^listing/','/^PID$/'];
    session::set('LOCK_VARS_TO_USE', $vars);

    /* Display the copy & paste dialog, if it is currently open */
    $ret = $this->copyPasteHandler();
    if ($ret) {
      return $this->getHeader().$ret;
    }

    // Handle actions (POSTs and GETs)
    $action = $this->detectPostActions();
    if (!empty($action['action'])) {
      @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action, 'Action');
      try {
        $str = $this->handleAction($action);
        if (!empty($str)) {
          return $this->getHeader().$str;
        }
      } catch (FusionDirectoryException $e) {
        msg_dialog::display(_('Error'), $e->getMessage(), ERROR_DIALOG);
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

    // Update list
    $this->listing->update();

    // Init snapshot list for renderSnapshotActions
    if (is_object($this->snapHandler)) {
      $this->snapHandler->initSnapshotCache($this->listing->getBase());
    }

    // Display list
    return $this->renderList();
  }

  function renderList (): string
  {
    global $config, $ui;

    // Rendering things using smarty themselves first
    $listRender   = $this->listing->render();
    $filterRender = $this->renderFilter();
    $actionMenu   = $this->renderActionMenu();

    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    $smarty->assign('LIST',         $listRender);
    $smarty->assign('FILTER',       $filterRender);
    $smarty->assign('ACTIONS',      $actionMenu);
    $smarty->assign('SIZELIMIT',    $ui->getSizeLimitHandler()->renderWarning());
    $smarty->assign('NAVIGATION',   $this->listing->renderNavigation());
    $smarty->assign('BASE',         $this->listing->renderBase());
    $smarty->assign('HEADLINE',     $this->headline);

    return $this->getHeader().$smarty->fetch(get_template_path('management/management.tpl'));
  }

  protected function renderFilter (): string
  {
    return $this->filter->render();
  }

  protected function renderActionMenu (): string
  {
    $menuActions = [];
    foreach ($this->actions as $action) {
      // Build ul/li list
      $action->fillMenuItems($menuActions);
    }

    if (empty($menuActions)) {
      return '';
    }

    $smarty = get_smarty();
    $smarty->assign('actions', $menuActions);
    return $smarty->fetch(get_template_path('management/actionmenu.tpl'));
  }

  function renderActionColumn (ListingEntry $entry): string
  {
    // Go thru all actions
    $result = '';
    foreach ($this->actions as $action) {
      $result .= $action->renderColumnIcons($entry);
    }

    return $result;
  }

  function fillActionRowClasses (&$classes, ListingEntry $entry)
  {
    foreach ($this->actions as $action) {
      $action->fillRowClasses($classes, $entry);
    }
  }

  /*!
   * \brief  Removes ldap object locks created by this class.
   *         Whenever an object is edited, we create locks to avoid
   *         concurrent modifications.
   *         This locks will automatically removed here.
   */
  function remove_lock ()
  {
    if (!empty($this->currentDn) && ($this->currentDn != 'new')) {
      del_lock($this->currentDn);
    }
    if (count($this->currentDns)) {
      del_lock($this->currentDns);
    }
  }

  function is_modal_dialog (): bool
  {
    return (is_object($this->tabObject) || is_object($this->dialogObject));
  }

  /*!
   * \brief  Generates the plugin header which is displayed whenever a tab object is opened.
   */
  protected function getHeader (): string
  {
    if ($this->skipHeader) {
      return '';
    }

    return print_header($this->icon, $this->title, get_object_info());
  }

  function openTabObject ($object, $base)
  {
    $this->tabObject = $object;
    $this->tabObject->set_acl_base($base);
    $this->tabObject->parent = &$this;
  }

  /*!
   * \brief  This method closes dialogs
   *          and cleans up the cached object info and the ui.
   */
  public function closeDialogs ()
  {
    $this->previousDn   = $this->currentDn;
    $this->currentDn    = '';
    $this->previousDns  = $this->currentDns;
    $this->currentDns   = [];

    $this->last_tabObject     = $this->tabObject;
    $this->tabObject          = NULL;
    $this->last_dialogObject  = $this->dialogObject;
    $this->dialogObject       = NULL;

    set_object_info();
  }

  protected function listAclCategories (): array
  {
    $cat = [];
    foreach ($this->objectTypes as $type) {
      $infos  = objects::infos($type);
      $cat[]  = $infos['aclCategory'];
    }
    return array_unique($cat);
  }

  /*!
   * \brief  Generates the footer which is used whenever a tab object is displayed.
   */
  protected function _getTabFooter (): string
  {
    // Do not display tab footer for non tab objects
    if (!($this->tabObject instanceOf simpleTabs)) {
      return '';
    }

    // Check if there is a dialog opened - We don't need any buttons in this case.
    if ($this->tabObject->dialogOpened()) {
      return '';
    }

    $smarty = get_smarty();
    $smarty->assign('readOnly', $this->tabObject->readOnly());
    $smarty->assign('showApply', ($this->currentDn != 'new'));
    return $smarty->fetch(get_template_path('management/tabfooter.tpl'));
  }

  function handleTemplateApply ($cancel = FALSE)
  {
    if (static::$skipTemplates) {
      return;
    }
    if ($cancel) {
      $msgs = [];
    } else {
      $msgs = $this->tabObject->save();
    }
    if (count($msgs)) {
      msg_dialog::displayChecks($msgs);
      return;
    } else {
      if (!$cancel) {
        @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDn, 'Template applied!');
      }
      del_lock($this->currentDn);
      if (empty($this->currentDns)) {
        $this->closeDialogs();
      } else {
        $this->last_tabObject = $this->tabObject;
        $this->tabObject      = NULL;
        $this->currentDn      = array_shift($this->currentDns);
        $this->dialogObject->setNextTarget($this->currentDn);
        $this->dialogObject->save_object();
      }
    }
  }

  function enablePaste ($action, ListingEntry $entry = NULL): bool
  {
    if ($entry === NULL) {
      return $this->cpHandler->entries_queued();
    } else {
      return FALSE;
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
  function newEntry (array $action)
  {
    $type = $action['subaction'];

    $this->currentDn = 'new';

    set_object_info($this->currentDn);

    // Open object
    $this->openTabObject(objects::create($type), $this->listing->getBase());
    @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDn, 'Create entry initiated');
  }

  function newEntryTemplate (array $action)
  {
    if (static::$skipTemplates) {
      return;
    }
    $action['subaction'] = preg_replace('/^template_/', '', $action['subaction']);
    $this->newEntry($action);
    $this->tabObject->setTemplateMode('template_cn');
  }

  function newEntryFromTemplate (array $action)
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

  function applyTemplateToEntry (array $action)
  {
    global $ui;
    if (static::$skipTemplates) {
      return;
    }
    if (empty($action['targets'])) {
      return;
    }
    $this->currentDns = $action['targets'];

    // check locks
    if ($locks = get_locks($this->currentDns)) {
      return gen_locked_message($locks, $this->currentDns);
    }

    // Add locks
    add_lock($this->currentDns, $ui->dn);

    // Detect type and check that all targets share the same type
    $type = NULL;

    foreach ($this->currentDns as $dn) {
      $entry = $this->listing->getEntry($dn);
      if ($entry === NULL) {
        trigger_error('Could not find '.$dn.', action canceled');
        $this->currentDns = [];
        return;
      }

      if ($entry->isTemplate()) {
        msg_dialog::display(_('Error'), _('Applying a template to a template is not possible'), ERROR_DIALOG);
        $this->currentDns = [];
        return;
      }

      if (!isset($type)) {
        $type = $entry->type;
      } elseif ($entry->type != $type) {
        msg_dialog::display(_('Error'), _('All selected entries need to share the same type to be able to apply a template to them'), ERROR_DIALOG);
        $this->currentDns = [];
        return;
      }
    }

    $this->currentDn = array_shift($this->currentDns);

    $this->dialogObject = new templateDialog($this, $type, NULL, $this->currentDn);
  }


  /*!
   * \brief  This method opens an existing object to be edited.
   *
   * \param  array  $action A combination of both 'action' and 'target':
   *                        action: The name of the action which was the used as trigger.
   *                        target: A list of object dns, which should be affected by this method.
   */
  function editEntry (array $action)
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
    $this->currentDn = $target;
    set_object_info($this->currentDn);
    if ($locks = get_locks($this->currentDn, TRUE)) {
      return gen_locked_message($locks, $this->currentDn, TRUE);
    }
    add_lock($this->currentDn, $ui->dn);

    // Open object
    $this->openTabObject(objects::open($this->currentDn, $entry->getTemplatedType()), $this->currentDn);
    @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDn, 'Edit entry initiated');
    if (isset($action['subaction'])
      && ($this->handleSubAction($action) === FALSE)) {
      trigger_error('Was not able to handle subaction: '.$action['subaction']);
    }
  }

  /*!
   * \brief  Editing an object was caneled.
   *          Close dialogs/tabs and remove locks.
   */
  function cancelEdit ()
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
   *          - Calls 'management::closeDialogs' to return to the object listing.
   */
  function saveChanges ()
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
        @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDns, 'Entry saved');
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
        $errors = $this->dialogObject->save();
        if (count($errors)) {
          msg_dialog::displayChecks($errors);
          return;
        } else {
          @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDns, 'Dialog saved');
          $this->remove_lock();
          $this->closeDialogs();
        }
      }
    }
  }

  /*!
   *  \brief  Save object modifications and keep dialogs opened
   */
  function applyChanges ()
  {
    if ($this->tabObject instanceOf simpleTabs) {
      $this->tabObject->save_object();
      $msgs = $this->tabObject->save();
      if (count($msgs)) {
        msg_dialog::displayChecks($msgs);
      } else {
        @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDns, 'Modifications applied');
        $this->tabObject->re_init();
        /* Avoid applying the POST a second time */
        $_POST = [];
      }
    }
  }

  /*! \brief  Queue selected objects to be removed.
   *          Checks ACLs, Locks and ask for confirmation.
   */
  function removeRequested (array $action)
  {
    global $ui;
    $disallowed       = [];
    $this->currentDns = [];

    @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action['targets'], 'Entry deletion requested');

    // Check permissons for each target
    foreach ($action['targets'] as $dn) {
      $entry = $this->listing->getEntry($dn);
      try {
        if ($entry->checkAcl('d')) {
          $this->currentDns[] = $dn;
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
    if (count($this->currentDns)) {
      // Check locks
      if ($locks = get_locks($this->currentDns)) {
        return gen_locked_message($locks, $this->currentDns);
      }

      // Add locks
      add_lock($this->currentDns, $ui->dn);

      $objects = [];
      foreach ($this->currentDns as $dn) {
        $entry  = $this->listing->getEntry($dn);
        $infos  = objects::infos($entry->getTemplatedType());
        if ($entry->isTemplate()) {
          $infos['nameAttr'] = 'cn';
        }
        $objects[] = [
          'name'  => $entry[$infos['nameAttr']],
          'dn'    => $dn,
          'icon'  => $infos['icon'],
          'type'  => $infos['name']
        ];
      }

      return $this->removeConfirmationDialog($objects);
    }
  }

  /*! \brief Display confirmation dialog
   */
  protected function removeConfirmationDialog (array $objects)
  {
    $smarty = get_smarty();
    $smarty->assign('objects', $objects);
    $smarty->assign('multiple', TRUE);
    return $smarty->fetch(get_template_path('simple-remove.tpl'));
  }

  /*! \brief  Deletion was confirmed, delete the objects queued.
   *          Checks ACLs just in case.
   */
  function removeConfirmed (array $action)
  {
    global $ui;
    @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDns, 'Entry deletion confirmed');

    foreach ($this->currentDns as $dn) {
      $entry = $this->listing->getEntry($dn);
      if (empty($entry)) {
        continue;
      }
      if ($entry->checkAcl('d')) {
        // Delete the object
        $this->currentDn = $dn;
        $this->openTabObject(objects::open($this->currentDn, $entry->getTemplatedType()), $this->currentDn);
        $this->tabObject->delete();

        // Remove the lock for the current object.
        del_lock($this->currentDn);
      } else {
        msg_dialog::display(_('Permission error'), msgPool::permDelete($dn), ERROR_DIALOG);
        logging::log('security', 'management/'.get_class($this), $dn, [], 'Tried to trick deletion.');
      }
    }

    // Cleanup
    $this->remove_lock();
    $this->closeDialogs();
  }

  function configureDialog (array $action)
  {
    $this->dialogObject = new ManagementConfigurationDialog($this);
  }

  /*! \brief  This method is used to queue and process copy&paste actions.
   *          Allows to copy, cut and paste mutliple entries at once.
   */
  function copyPasteHandler (array $action = ['action' => ''])
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
          @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Entry copied!');
        }
        if (($action['action'] == 'cut') && $entry->checkAcl('rd')) {
          $this->cpHandler->add_to_queue($dn, 'cut', $entry->getTemplatedType());
          @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Entry cut!');
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
  function createSnapshotDialog (array $action)
  {
    global $config, $ui;
    @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action['targets'], 'Snaptshot creation initiated!');

    $this->currentDn = array_pop($action['targets']);
    if (empty($this->currentDn)) {
      return;
    }
    $entry = $this->listing->getEntry($this->currentDn);
    if ($entry->snapshotCreationAllowed()) {
      $this->dialogObject = new SnapshotCreateDialog($this->currentDn, $this, '');
      $this->dialogObject->set_acl_base($this->currentDn);
    } else {
      msg_dialog::display(_('Permission'), sprintf(_('You are not allowed to create a snapshot for %s.'), $this->currentDn),
          ERROR_DIALOG);
    }
  }

  /*!
   * \brief  Displays the "Restore snapshot dialog" for a given target.
   *          If no target is specified, open the restore removed object dialog.
   */
  function restoreSnapshotDialog (array $action)
  {
    global $config, $ui;

    if (empty($action['targets'])) {
      // No target, open the restore removed object dialog.
      $this->currentDn  = $this->listing->getBase();
      $aclCategory      = $this->listAclCategories();
    } else {
      // Display the restore points for a given object.
      $this->currentDn = $action['targets'][0];
      if (empty($this->currentDn)) {
        return;
      }
      $aclCategory = objects::infos($this->listing->getEntry($this->currentDn)->getTemplatedType())['aclCategory'];
    }

    if ($ui->allow_snapshot_restore($this->currentDn, $aclCategory, empty($action['targets']))) {
      @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDn, 'Snaptshot restoring initiated!');
      $this->dialogObject = new SnapshotRestoreDialog($this->currentDn, $this, empty($action['targets']), $aclCategory);
    } else {
      msg_dialog::display(_('Permission'), sprintf(_('You are not allowed to restore a snapshot for %s.'), $this->currentDn),
          ERROR_DIALOG);
    }
  }


  function export (array $action)
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

  function getSnapshotBases (): array
  {
    $bases = [];
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
  function getAllDeletedSnapshots (): array
  {
    $bases = $this->getSnapshotBases();
    $tmp = [];
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
  function getAvailableSnapsShots (string $dn): array
  {
    return $this->snapHandler->getAvailableSnapsShots($dn);
  }

  /*
   * \brief Whether snapshot restore action should be enabled for an entry
   */
  function enableSnapshotRestore ($action, ListingEntry $entry = NULL): bool
  {
    if ($entry !== NULL) {
      /* For entries */
      return $this->snapHandler->hasSnapshots($entry->dn);
    } else {
      /* For action menu */
      return $this->snapHandler->hasDeletedSnapshots($this->getSnapshotBases());
    }
  }

  /*!
   * \brief  Creates a new snapshot entry
   */
  function createSnapshot (string $dn, string $description)
  {
    global $ui;
    if ($this->currentDn !== $dn) {
      trigger_error('There was a problem with the snapshot workflow');
      return;
    }
    if (!empty($dn) && $ui->allow_snapshot_create($dn, $this->dialogObject->aclCategory)) {
      $this->snapHandler->createSnapshot($dn, $description);
      @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Snaptshot created!');
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
  function restoreSnapshot (string $dn)
  {
    global $ui;
    if (!empty($dn) && $ui->allow_snapshot_restore($dn, $this->dialogObject->aclCategory, $this->dialogObject->global)) {
      $this->snapHandler->restoreSnapshot($dn);
      @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Snaptshot restored');
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
  function removeSnapshot (string $dn)
  {
    global $ui;
    if (!empty($dn) && $ui->allow_snapshot_delete($dn, $this->dialogObject->aclCategory)) {
      $this->snapHandler->removeSnapshot($dn);
      @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Snaptshot deleted');
    } else {
      msg_dialog::display(_('Permission'), sprintf(_('You are not allowed to delete a snapshot for %s.'), $dn),
          ERROR_DIALOG);
    }
  }


  static function mainInc ($classname = NULL, $objectTypes = FALSE)
  {
    global $remove_lock, $cleanup, $display;

    if ($classname === NULL) {
      $classname = get_called_class();
    }

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
