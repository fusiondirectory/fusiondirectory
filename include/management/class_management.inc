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

    // Build headpage
    //~ $this->headpage = new $this->headpageClass($this->parseXML($this->listXMLPath));
    //~ $this->headpage->setFilter($filter);

    // Register default actions
    $this->registerAction('new',    'newEntry');
    $this->registerAction('edit',   'editEntry');
    $this->registerAction('apply',  'applyChanges');
    $this->registerAction('save',   'saveChanges');

    $this->registerAction('cancel',       'cancelEdit');
    $this->registerAction('cancelDelete', 'cancelEdit');

    $this->registerAction('remove',           'removeEntryRequested');
    $this->registerAction('removeConfirmed',  'removeEntryConfirmed');

    //~ $this->configureHeadpage();
    //~ $this->configureFilter();
    //~ if ($this->baseMode === FALSE) {
      //~ $this->headpage->setBase($config->current['BASE']);
    //~ }
  }

  /*!
   *  \brief  Every click in the list user interface sends an event
   *          here can we connect those events to a method.
   *          eg. see simpleManagement::registerEvent('new','createUser')
   *          When the action/event new is send, the method 'createUser'
   *          will be called.
   */
  function registerAction($action, $target)
  {
    $this->actions[$action] = $target;
  }

  /*!
   * \brief  Detects actions/events send by the ui
   *           and the corresponding targets.
   */
  function detectPostActions()
  {
    //~ if (!is_object($this->headpage)) {
      //~ trigger_error("No valid headpage given....!");
      //~ return array();
    //~ }
    //~ $action = $this->headpage->getAction();
    $action = array('action' => '');
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
   * \brief  Execute this plugin
   *          Handle actions/events, locking, snapshots, dialogs, tabs,...
   */
  function execute()
  {
    // Ensure that html posts and gets are kept even if we see a 'Entry islocked' dialog.
    //~ $vars = array('/^act$/','/^listing/','/^PID$/');
    //~ session::set('LOCK_VARS_TO_USE', $vars);

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
    $smarty->assign('ACTIONS', $this->listing->renderActionMenu());
    $smarty->assign('BASE', $this->listing->renderBase()); // TODO: move here?

    // Assign summary
    $smarty->assign('HEADLINE', $this->headline);

    return $this->getHeader().$smarty->fetch(get_template_path('simple-list.tpl'));
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
