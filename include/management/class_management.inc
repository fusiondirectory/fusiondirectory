<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2020  FusionDirectory

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
class management implements FusionDirectoryDialog
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
  /**
   * @var ?simpleTabs
   */
  protected $tabObject    = NULL;
  protected $dialogObject = NULL;

  // The last opened object.
  protected $last_tabObject     = NULL;
  protected $last_dialogObject  = NULL;

  protected $renderCache;

  public $headline;
  public $title;
  public $icon;

  protected $actions        = [];
  protected $actionHandlers = [];

  protected $exporters = [];

  public $neededAttrs = [];

  public static $skipTemplates = TRUE;

  /* Disable and hide configuration system */
  protected $skipConfiguration = FALSE;

  protected $columnConfiguration;

  /* Default columns */
  public static $columns = [
    ['ObjectTypeColumn', []],
    ['LinkColumn',       ['attributes' => 'nameAttr',    'label' => 'Name']],
    ['LinkColumn',       ['attributes' => 'description', 'label' => 'Description']],
    ['ActionsColumn',    ['label' => 'Actions']],
  ];

  function __construct (
    $objectTypes = FALSE,
    array $filterElementDefinitions = [
      ['TabFilterElement',  []],
    ]
  )
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
    $this->setUpFilter($filterElementDefinitions);

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

  protected function setUpFilter (array $filterElementDefinitions)
  {
    $this->filter   = new managementFilter($this, NULL, $filterElementDefinitions);
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

    if (class_available('archivedObject')) {
      $action = archivedObject::getManagementAction($this->objectTypes, 'archiveRequested');
      if ($action !== NULL) {
        $this->registerAction($action);
        $this->registerAction(new HiddenAction('archiveConfirmed',  'archiveConfirmed'));
        $this->registerAction(new HiddenAction('archiveCancel',     'cancelEdit'));
      }
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
          ['/SnapshotHandler/c']
        )
      );
      $this->registerAction(
        new Action(
          'restore', _('Restore snapshot'), 'geticon.php?context=actions&icon=document-restore&size=16',
          '*', 'restoreSnapshotDialog',
          ['w', '/SnapshotHandler/r']
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
    if (!$this->skipConfiguration) {
      $this->registerAction(new HiddenAction('configure',       'configureDialog'));
    }
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
    if ($this->showTabFooter()) {
      if (isset($_POST['edit_cancel'])) {
        $action['action'] = 'cancel';
      } elseif (isset($_POST['edit_finish'])) {
        $action['action'] = 'save';
      } elseif (isset($_POST['edit_apply'])) {
        $action['action'] = 'apply';
      }
    } elseif (!$this->dialogOpened()) {
      if (isset($_POST['delete_confirmed'])) {
        $action['action'] = 'removeConfirmed';
      } elseif (isset($_POST['delete_cancel'])) {
        $action['action'] = 'cancelDelete';
      } elseif (isset($_POST['archive_confirmed'])) {
        $action['action'] = 'archiveConfirmed';
      } elseif (isset($_POST['archive_cancel'])) {
        $action['action'] = 'archiveCancel';
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

  /* For management we have to render directly in readPost in some cases */
  public function readPost ()
  {
    $this->renderCache = $this->execute();
  }

  public function update (): bool
  {
    if ($this->renderCache === NULL) {
      if (!$this->dialogOpened()) {
        // Update list
        $this->listing->update();

        // Init snapshot list for renderSnapshotActions
        if (is_object($this->snapHandler)) {
          $this->snapHandler->initSnapshotCache($this->listing->getBase());
        }
      }
    }
    return TRUE;
  }

  public function render (): string
  {
    if ($this->renderCache === NULL) {
      if ($this->tabObject instanceOf simpleTabs) {
        /* Display tab object */
        $display = $this->tabObject->render();
        $display .= $this->getTabFooter();
        $this->renderCache = $this->getHeader().$display;
      } elseif (is_object($this->dialogObject)) {
        /* Display dialog object */
        $display = $this->dialogObject->render();
        $display .= $this->getTabFooter();
        $this->renderCache = $this->getHeader().$display;
      } else {
        /* Display list */
        $this->renderCache = $this->renderList();
      }
    }
    return $this->renderCache;
  }

  /*!
   * \brief  Execute this plugin
   *          Handle actions/events, locking, snapshots, dialogs, tabs,...
   */
  protected function execute ()
  {
    // Ensure that html posts and gets are kept even if we see a 'Entry islocked' dialog.
    session::set('LOCK_VARS_TO_USE', ['/^act$/','/^listing/','/^PID$/']);

    /* Display the copy & paste dialog, if it is currently open */
    $ret = $this->copyPasteHandler();
    if ($ret) {
      return $this->getHeader().$ret;
    }

    // Handle actions (POSTs and GETs)
    $action = $this->detectPostActions();
    if (!empty($action['action'])) {
      logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action, 'Action');
      try {
        $str = $this->handleAction($action);
        if (!empty($str)) {
          return $this->getHeader().$str;
        }
      } catch (FusionDirectoryException $e) {
        $error = new FusionDirectoryError(htmlescape($e->getMessage()), 0, $e);
        $error->display();
      }
    }

    /* Save tab or dialog object */
    if ($this->tabObject instanceOf simpleTabs) {
      $this->tabObject->readPost();
      $this->tabObject->update();
    } elseif (is_object($this->dialogObject)) {
      try {
        $this->dialogObject->readPost();
        if (is_object($this->dialogObject)) {
          /* Check again as readPost might close it */
          if (!$this->dialogObject->update()) {
            $this->closeDialogs();
          }
        }
      } catch (FusionDirectoryException $e) {
        $error = new FusionDirectoryError(htmlescape($e->getMessage()), 0, $e);
        $error->display();
        $this->closeDialogs();
      }
    }

    return NULL;
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
    $smarty->assign('NAVIGATION',   $this->listing->renderNavigation($this->skipConfiguration));
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
  public function removeLocks ()
  {
    if (!empty($this->currentDn) && ($this->currentDn != 'new')) {
      Lock::deleteByObject($this->currentDn);
    }
    if (count($this->currentDns)) {
      Lock::deleteByObject($this->currentDns);
    }
  }

  function dialogOpened (): bool
  {
    return (is_object($this->tabObject) || is_object($this->dialogObject));
  }

  /*!
   * \brief Sets smarty headline and returns the plugin header which is displayed whenever a tab object is opened.
   */
  protected function getHeader (): string
  {
    global $smarty;

    $smarty->assign('headline', $this->title);
    $smarty->assign('headline_image', $this->icon);

    if (is_object($this->tabObject) && ($this->currentDn != '')) {
      return '<div class="pluginfo">'.$this->currentDn."</div>\n";
    }
    return '';
  }

  function openTabObject ($object)
  {
    $this->tabObject          = $object;
    $this->tabObject->parent  = &$this;
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
   * \brief Whether footer buttons should appear
   */
  protected function showTabFooter (): bool
  {
    // Do not display tab footer for non tab objects
    if (!($this->tabObject instanceOf simpleTabs)) {
      return FALSE;
    }

    // Check if there is a dialog opened - We don't need any buttons in this case.
    if ($this->tabObject->dialogOpened()) {
      return FALSE;
    }

    return TRUE;
  }

  /*!
   * \brief  Generates the footer which is used whenever a tab object is displayed.
   */
  protected function getTabFooter (): string
  {
    // Do not display tab footer for non tab objects
    if (!$this->showTabFooter()) {
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
        logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDn, 'Template applied!');
      }
      Lock::deleteByObject($this->currentDn);
      if (empty($this->currentDns)) {
        $this->closeDialogs();
      } else {
        $this->last_tabObject = $this->tabObject;
        $this->tabObject      = NULL;
        $this->currentDn      = array_shift($this->currentDns);
        $this->dialogObject->setNextTarget($this->currentDn);
        $this->dialogObject->readPost();
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

    // Open object
    $this->openTabObject(objects::create($type));
    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDn, 'Create entry initiated');
  }

  function newEntryTemplate (array $action)
  {
    if (static::$skipTemplates) {
      return;
    }
    $type = preg_replace('/^template_/', '', $action['subaction']);

    $this->currentDn = 'new';

    // Open object
    $this->openTabObject(objects::createTemplate($type));
    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDn, 'Create template entry initiated');
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
    if ($locks = Lock::get($this->currentDns)) {
      return Lock::genLockedMessage($locks, FALSE, _('Apply anyway'));
    }

    // Add locks
    Lock::add($this->currentDns);

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
        $error = new FusionDirectoryError(htmlescape(_('Applying a template to a template is not possible')));
        $error->display();
        $this->currentDns = [];
        return;
      }

      if (!isset($type)) {
        $type = $entry->type;
      } elseif ($entry->type != $type) {
        $error = new FusionDirectoryError(htmlescape(_('All selected entries need to share the same type to be able to apply a template to them')));
        $error->display();
        $this->currentDns = [];
        return;
      }
    }

    $this->currentDn = array_shift($this->currentDns);

    $this->dialogObject = new templateDialog($this, $type, NULL, $this->currentDn);
  }

  /*! \brief  Queue selected objects to be archived.
   *          Checks Locks and ask for confirmation.
   */
  public function archiveRequested (array $action)
  {
    global $ui;

    if (empty($action['targets'])) {
      return;
    }
    $this->currentDns = $action['targets'];

    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action['targets'], 'Entry archive requested');

    // Check locks
    if ($locks = Lock::get($this->currentDns)) {
      return Lock::genLockedMessage($locks, FALSE, _('Archive anyway'));
    }

    // Add locks
    Lock::add($this->currentDns);

    $objects = [];
    foreach ($this->currentDns as $dn) {
      $entry  = $this->listing->getEntry($dn);
      if ($entry->isTemplate()) {
        $error = new FusionDirectoryError(htmlescape(_('Archiving a template is not possible')));
        $error->display();
        $this->removeLocks();
        $this->currentDns = [];
        return;
      }
      $infos  = objects::infos($entry->getTemplatedType());
      $objects[] = [
        'name'  => $entry[$infos['nameAttr']][0],
        'dn'    => $dn,
        'icon'  => $infos['icon'],
        'type'  => $infos['name']
      ];
    }

    $smarty = get_smarty();
    $smarty->assign('objects', $objects);
    return $smarty->fetch(get_template_path('simple-archive.tpl'));
  }

  public function archiveConfirmed (array $action)
  {
    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDns, 'Archiving');

    $success = 0;
    foreach ($this->currentDns as $dn) {
      $entry = $this->listing->getEntry($dn);

      $errors = archivedObject::archiveObject($entry->type, $dn);
      if (empty($errors)) {
        $success++;
      } else {
        msg_dialog::displayChecks($errors);
      }
      Lock::deleteByObject($dn);
    }

    if ($success > 0) {
      msg_dialog::display(
        _('Archive success'),
        htmlescape(sprintf(_('%d entries were successfully archived'), $success)),
        INFO_DIALOG
      );
    }

    $this->currentDns = [];
  }

  /*!
   * \brief  This method opens an existing object to be edited.
   *
   * \param  array  $action A combination of both 'action' and 'targets':
   *                        action: The name of the action which was the used as trigger.
   *                        targets: A list of object dns, which should be affected by this method.
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
    if ($locks = Lock::get($this->currentDn, TRUE)) {
      return Lock::genLockedMessage($locks, TRUE);
    }
    Lock::add($this->currentDn);

    // Open object
    $this->openTabObject(objects::open($this->currentDn, $entry->getTemplatedType()));
    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDn, 'Edit entry initiated');
    if (isset($action['subaction'])
      && ($this->handleSubAction($action) === FALSE)) {
      trigger_error('Was not able to handle subaction: '.$action['subaction']);
    }
  }

  /*!
   * \brief  Editing an object was canceled.
   *          Close dialogs/tabs and remove locks.
   */
  function cancelEdit ()
  {
    if (($this->tabObject instanceOf simpleTabs) && ($this->dialogObject instanceOf templateDialog)) {
      $this->handleTemplateApply(TRUE);
      return;
    }
    $this->removeLocks();
    $this->closeDialogs();
  }

  /*!
   * \brief  Save object modifications and closes dialogs (returns to object listing).
   *          - Calls 'simpleTabs::save' to save back object modifications (e.g. to ldap).
   *          - Calls 'management::closeDialogs' to return to the object listing.
   */
  function saveChanges ()
  {
    if ($this->tabObject instanceOf simpleTabs) {
      $this->tabObject->readPost();
      $this->tabObject->update();
      if ($this->dialogObject instanceOf templateDialog) {
        $this->handleTemplateApply();
      } else {
        $msgs = $this->tabObject->save();
        if (count($msgs)) {
          msg_dialog::displayChecks($msgs);
        } else {
          logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDns, 'Entry saved');
          $this->removeLocks();
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
      $this->tabObject->readPost();
      $this->tabObject->update();
      $msgs = $this->tabObject->save();
      if (count($msgs)) {
        msg_dialog::displayChecks($msgs);
      } else {
        logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDns, 'Modifications applied');
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

    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action['targets'], 'Entry deletion requested');

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
      $error = new FusionDirectoryPermissionError(msgPool::permDelete($disallowed));
      $error->display();
    }

    // We've at least one entry to delete.
    if (count($this->currentDns)) {
      // Check locks
      if ($locks = Lock::get($this->currentDns)) {
        return Lock::genLockedMessage($locks, FALSE, _('Delete anyway'));
      }

      // Add locks
      Lock::add($this->currentDns);

      $objects = [];
      foreach ($this->currentDns as $dn) {
        $entry  = $this->listing->getEntry($dn);
        $infos  = objects::infos($entry->getTemplatedType());
        if ($entry->isTemplate()) {
          $infos['nameAttr'] = 'cn';
        }
        $objects[] = [
          'name'  => $entry[$infos['nameAttr']][0],
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
    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDns, 'Entry deletion confirmed');

    foreach ($this->currentDns as $dn) {
      $entry = $this->listing->getEntry($dn);
      if (empty($entry)) {
        continue;
      }
      if ($entry->checkAcl('d')) {
        // Delete the object
        $this->currentDn = $dn;
        $this->openTabObject(objects::open($this->currentDn, $entry->getTemplatedType()));
        $errors = $this->tabObject->delete();
        msg_dialog::displayChecks($errors);

        // Remove the lock for the current object.
        Lock::deleteByObject($this->currentDn);
      } else {
        $error = new FusionDirectoryPermissionError(msgPool::permDelete($dn));
        $error->display();
        logging::log('security', 'management/'.get_class($this), $dn, [], 'Tried to trick deletion.');
      }
    }

    // Cleanup
    $this->removeLocks();
    $this->closeDialogs();
  }

  function configureDialog (array $action)
  {
    if (!$this->skipConfiguration) {
      $this->dialogObject = new ManagementConfigurationDialog($this);
    }
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
    $this->cpHandler->readPost();

    // Add entries to queue
    if (($action['action'] == 'copy') || ($action['action'] == 'cut')) {
      $this->cpHandler->cleanup_queue();
      foreach ($action['targets'] as $dn) {
        $entry = $this->listing->getEntry($dn);
        if (($action['action'] == 'copy') && $entry->checkAcl('r')) {
          $this->cpHandler->add_to_queue($dn, 'copy', $entry->getTemplatedType());
          logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Entry copied!');
        }
        if (($action['action'] == 'cut') && $entry->checkAcl('rd')) {
          $this->cpHandler->add_to_queue($dn, 'cut', $entry->getTemplatedType());
          logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Entry cut!');
        }
      }
    }

    // Initiate pasting
    if ($action['action'] == 'paste') {
      $this->cpPastingStarted = TRUE;
    }

    // Display any c&p dialogs, eg. object modifications required before pasting.
    if ($this->cpPastingStarted && $this->cpHandler->entries_queued()) {
      $this->cpHandler->update();
      $data = $this->cpHandler->render();
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
    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action['targets'], 'Snapshot creation initiated!');

    $this->currentDn = array_pop($action['targets']);
    if (empty($this->currentDn)) {
      return;
    }
    $entry = $this->listing->getEntry($this->currentDn);
    if ($entry->snapshotCreationAllowed()) {
      $this->dialogObject = new SnapshotCreateDialog($this->currentDn, $this, '');
    } else {
      $error = new FusionDirectoryError(
        htmlescape(sprintf(
          _('You are not allowed to create a snapshot for %s.'),
          $this->currentDn
        ))
      );
      $error->display();
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
      $aclCategories    = $this->listAclCategories();
    } else {
      // Display the restore points for a given object.
      $this->currentDn = $action['targets'][0];
      if (empty($this->currentDn)) {
        return;
      }
      $aclCategories = [objects::infos($this->listing->getEntry($this->currentDn)->getTemplatedType())['aclCategory']];
    }

    if ($ui->allow_snapshot_restore($this->currentDn, $aclCategories, empty($action['targets']))) {
      logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDn, 'Snapshot restoring initiated!');
      $this->dialogObject = new SnapshotRestoreDialog($this->currentDn, $this, empty($action['targets']), $aclCategories);
    } else {
      $error = new FusionDirectoryError(
        htmlescape(sprintf(
          _('You are not allowed to restore a snapshot for %s.'),
          $this->currentDn
        ))
      );
      $error->display();
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
    if (empty($dn) || ($this->currentDn !== $dn)) {
      trigger_error('There was a problem with the snapshot workflow');
      return;
    }
    $entry = $this->listing->getEntry($dn);
    if ($entry->snapshotCreationAllowed()) {
      $this->snapHandler->createSnapshot($dn, $description, $entry->type);
      logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Snapshot created!');
    } else {
      $error = new FusionDirectoryPermissionError(htmlescape(sprintf(_('You are not allowed to restore a snapshot for %s.'), $dn)));
      $error->display();
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
      $dn = $this->snapHandler->restoreSnapshot($dn);
      logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Snapshot restored');
      $this->closeDialogs();
      if ($dn !== FALSE) {
        $this->listing->focusDn($dn);
        $entry = $this->listing->getEntry($dn);
        $this->currentDn = $dn;
        Lock::add($this->currentDn);

        // Open object
        $this->openTabObject(objects::open($this->currentDn, $entry->getTemplatedType()));
        $this->saveChanges();
      }
    } else {
      $error = new FusionDirectoryPermissionError(htmlescape(sprintf(_('You are not allowed to restore a snapshot for %s.'), $dn)));
      $error->display();
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
      logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Snapshot deleted');
    } else {
      $error = new FusionDirectoryPermissionError(htmlescape(sprintf(_('You are not allowed to delete a snapshot for %s.'), $dn)));
      $error->display();
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
      $macl->removeLocks();
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
      $managementObject->readPost();
      $managementObject->update();
      $display = $managementObject->render();

      /* Store the object in the session */
      session::set($classname, $managementObject);
    }
  }
}
