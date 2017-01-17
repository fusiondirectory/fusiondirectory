<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016  FusionDirectory

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

class templateDialog
{
  protected $simpleManagement;
  protected $type;
  protected $template = NULL;
  protected $templates;
  protected $target = NULL;

  protected $tabObject;

  protected $post_finish = 'template_continue';
  protected $post_cancel = 'template_cancel';

  function __construct($simpleManagement, $type, $dn = NULL, $target = NULL)
  {
    $this->simpleManagement = $simpleManagement;
    $this->type             = $type;
    $this->templates        = objects::getTemplates($this->type);
    if ($dn !== NULL) {
      if (isset($this->templates[$dn])) {
        $this->template = new template($this->type, $dn);
      } else {
        trigger_error('Unknown template "'.$dn.'"');
      }
    }
    $this->target = $target;
  }

  function save_object ()
  {
    if ($this->target === NULL) {
      if (isset($_POST[$this->post_cancel])) {
        return $this->handle_cancel();
      }
      if ((isset($_POST[$this->post_finish]) || isset($_GET[$this->post_finish])) && is_object($this->template)) {
        $this->template->save_object();
        return $this->handle_finish();
      }
    }

    if (
      isset($_POST['template']) &&
      isset($this->templates[$_POST['template']])
      ) {
      if (is_object($this->template)) {
        trigger_error('redefining template object');
      }
      $this->template = new template($this->type, $_POST['template']);
      /* This method can loop if there are several targets */
      unset($_POST['template']);
    }
    if (is_object($this->template)) {
      if ($this->target !== NULL) {
        $this->simpleManagement->openTabObject($this->template->apply($this->target), $this->template->getBase());
        $this->simpleManagement->handleTemplateApply();
        return FALSE;
      } else {
        $this->template->save_object();
      }
    }

    return TRUE;
  }

  function setNextTarget ($target)
  {
    $this->target = $target;
  }

  function execute ()
  {
    $smarty = get_smarty();
    if (is_object($this->template)) {
      $smarty->assign('template_dialog', $this->template->execute());
    } else {
      $smarty->assign('templates', $this->templates);
    }
    $display = $smarty->fetch(get_template_path('template.tpl'));
    return $display;
  }

  function handle_finish ()
  {
    $this->simpleManagement->closeDialogs();
    $this->simpleManagement->openTabObject($this->template->apply(), $this->template->getBase());
    return FALSE;
  }

  function handle_cancel ()
  {
    $this->simpleManagement->closeDialogs();
    return FALSE;
  }
}

class simpleManagement extends management
{
  /* Attributes that child classes should override */
  protected $objectTypes  = array();

  /* Attributes that child classes can override */
  protected $departmentBrowser      = TRUE;
  protected $departmentRootVisible  = TRUE;
  protected $baseMode               = TRUE;
  protected $multiSelect            = TRUE;

  protected $filterXMLPath    = NULL;
  protected $listXMLPath      = NULL;
  protected $autoFilter       = TRUE;
  protected $autoActions      = TRUE;

  protected $autoFilterAttributes = array('dn', 'cn', 'description');

  protected $headpageClass = "listing";

  protected $skipTemplates = TRUE;

  function __construct()
  {
    global $config;

    if ($this->filterXMLPath === NULL) {
      $this->filterXMLPath = get_template_path('simple-filter.xml', TRUE, dirname(__FILE__));
    }
    if ($this->listXMLPath === NULL) {
      $this->listXMLPath = get_template_path('simple-list.xml', TRUE, dirname(__FILE__));
    }

    foreach ($this->objectTypes as &$object) {
      $object = strtoupper($object);
    }
    unset($object);

    $this->storagePoints = array();
    foreach ($this->objectTypes as $key => $object) {
      try {
        $i = objects::infos($object);
      } catch (NonExistingObjectTypeException $e) {
        /* Remove objectTypes which are not existing */
        unset($this->objectTypes[$key]);
        continue;
      }
      if ($i['ou'] !== NULL) {
        $this->storagePoints[] = $i['ou'];
      }
    }
    $this->storagePoints = array_unique($this->storagePoints);
    if (count($this->storagePoints) == 0) {
      $this->storagePoints[] = "";
    }

    // Build filter
    if (session::global_is_set(get_class($this).'_filter')) {
      $filter = session::global_get(get_class($this).'_filter');
    } else {
      $filter = new filter($this->filterXMLPath);
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new $this->headpageClass($this->parseXML($this->listXMLPath));
    $headpage->setFilter($filter);

    parent::__construct($headpage);

    $this->configureHeadpage();
    $this->configureFilter();
    if ($this->baseMode === FALSE) {
      $this->headpage->setBase($config->current['BASE']);
    }
  }

  /* Build the action menu, fill the object definition, etc, based on objectTypes info */
  function configureHeadpage ()
  {
    if ($this->autoActions) {
      if ($this->skipTemplates) {
        $this->headpage->xmlData['actionmenu']['action'][0]['action'] = array();
      } else {
        $this->headpage->xmlData['actionmenu']['action'][0]['action'] = array(
          array(
            'type'    => 'sub',
            'image'   => 'geticon.php?context=actions&icon=document-new&size=16',
            'label'   => _('Template'),
            'action'  => array(),
          ),
          array(
            'type'    => 'sub',
            'image'   => 'geticon.php?context=actions&icon=document-new&size=16',
            'label'   => _('From template'),
            'action'  => array(),
          ),
        );
      }
    }
    $this->configureHeadline();
    foreach ($this->objectTypes as $object) {
      $i = objects::infos($object);

      if (!isset($i['icon'])) {
        trigger_error('Missing icon for type '.$object);
        $i['icon'] = '';
      }

      $this->headpage->objectTypes[$object] =
        array(
          'label'       => $i['name'],
          'category'    => $i['aclCategory'],
          'class'       => $i['mainTab'],
          'image'       => $i['icon'],
          'filter'      => ldapFilter::parse($i['filter']),
          'nameAttr'    => $i['nameAttr'],
        );
      if (!$this->skipTemplates) {
        $this->headpage->objectTypes['template_'.$object] =
          array(
            'label'       => sprintf(_('%s template'), $i['name']),
            'category'    => $i['aclCategory'],
            'class'       => 'template',
            'image'       => $i['icon'],//FIXME
            'filter'      =>
              new ldapFilter(
                '&',
                array(
                  new ldapFilterLeaf('objectClass', '=', 'fdTemplate'),
                  fdTemplateFilter(ldapFilter::parse($i['filter'])),
                )
              ),
            'nameAttr'    => $i['nameAttr'],
          );
      }
      $this->headpage->categories[] = $i['aclCategory'];
      if ($this->autoActions) {
        $this->registerAction('new_'.$object, 'newEntry');
        $icon = 'geticon.php?context=actions&amp;icon=document-new&amp;size=16';
        if (isset($i['icon'])) {
          $icon = $i['icon'];
        } else {
          $infos = pluglist::pluginInfos($i['mainTab']);
          if (isset($infos['plIcon'])) {
            $icon = $infos['plIcon'];
          }
        }
        $this->headpage->xmlData['actionmenu']['action'][0]['action'][] = array(
          'name'  => 'new_'.$object,
          'type'  => 'entry',
          'image' => $icon,
          'label' => $i['name'],
          'acl'   => $i['aclCategory'].'/'.$i['mainTab'].'[c]',
        );
        if (!$this->skipTemplates) {
          $this->registerAction('new_template_'.$object, 'newEntryTemplate');
          $this->registerAction('template_apply_'.$object, 'newEntryFromTemplate');
          $this->headpage->xmlData['actionmenu']['action'][0]['action'][0]['action'][] = array(
            'name'  => 'new_template_'.$object,
            'type'  => 'entry',
            'image' => $icon,
            'label' => $i['name'],
            'acl'   => $i['aclCategory'].'/'.$i['mainTab'].'[c]',
          );
          $this->headpage->xmlData['actionmenu']['action'][0]['action'][1]['action'][] = array(
            'name'  => 'template_apply_'.$object,
            'type'  => 'entry',
            'image' => $icon,
            'label' => $i['name'],
            'acl'   => $i['aclCategory'].'/'.$i['mainTab'].'[c]',
          );
        }
      }
    }
    if ($this->autoActions && !$this->skipTemplates) {
      $this->registerAction('template_apply', 'newEntryFromTemplate');
    }
    $this->headpage->refreshBasesList();
  }

  function configureHeadline ()
  {
    $pInfos = pluglist::pluginInfos(get_class($this));
    $this->headpage->headline = $pInfos['plShortName'];
  }

  function configureFilterCategory ()
  {
    $pInfos = pluglist::pluginInfos(get_class($this));
    $cat = NULL;
    if (isset($pInfos['plCategory'])) {
      $cat = key($pInfos['plCategory']);
      if (is_numeric($cat)) {
        $cat = $pInfos['plCategory'][$cat];
      }
    } elseif (isset($pInfos['plObjectType'])) {
      $ot = key($pInfos['plObjectType']);
      if (is_numeric($ot)) {
        $ot = $pInfos['plObjectType'][$ot];
      }
      $infos = objects::infos($ot);
      $cat = $infos['aclCategory'];
    }
    if ($cat === NULL) {
      trigger_error('Could not find a category for '.get_class($this));
    }
    $this->filter->category = $cat;
  }

  /* Build the filter(s) based on objectTypes info */
  function configureFilter ()
  {
    if ($this->autoFilter) {
      $this->configureFilterCategory();
      $attributes = $this->autoFilterAttributes;
      $filter     = '(|';
      foreach ($this->objectTypes as $object) {
        $tag    = 'FILTER'.$object;
        $filter .= '$'.$tag;
        $i      = objects::infos($object);
        $this->filter->elements[$tag] = array(
          'type'    => 'checkbox',
          'tag'     => $tag,
          'default' => TRUE,
          'unset'   => array(),
          'set'     => preg_replace('/\$/', '\\\$', $i['filter'])
        );
        $this->filter->elementValues[$tag] = TRUE;

        $attributes[] = $i['mainAttr'];
        $attributes[] = $i['nameAttr'];
      }
      if (!$this->skipTemplates) {
        $tag    = 'FILTERTEMPLATE';
        $filter .= '$'.$tag;
        $this->filter->elements[$tag] = array(
          'type'    => 'checkbox',
          'tag'     => $tag,
          'default' => TRUE,
          'unset'   => array(),
          'set'     => '(objectClass=fdTemplate)'
        );
        $this->filter->elementValues[$tag] = TRUE;
        $attributes[] = 'fdTemplateField';
      }
      $filter .= ')';
      $attributes = array_values(array_unique($attributes));
      $this->filter->query[0]['filter']       = '(&'.$filter.'$NAME)';
      $this->filter->query[0]['attribute']    = $attributes;
      $this->filter->elements['NAME']['set']  = '(|('.join('=*$*)(', $attributes).'=*$*))';
      $this->filter->elements['NAME']['autocomplete']['filter']     = '(&'.$filter.'(|('.join('=*$NAME*)(', $attributes).'=*$NAME*))'.')';
      $this->filter->elements['NAME']['autocomplete']['attribute']  = $attributes;
      uasort($this->filter->elements, 'strlenSort');
      $this->filter->elements = array_reverse($this->filter->elements);
    }
  }

  function parseXML ($file)
  {
    $contents = file_get_contents($file);
    $data     = xml::xml2array($contents, 1);
    foreach (array('departmentBrowser','departmentRootVisible','baseMode','multiSelect') as $var) {
      $data['list']['definition']["$var"] = ($this->$var ? 'true' : 'false');
    }
    return $data;
  }

  function getType($dn)
  {
    return $this->getHeadpage()->getType($dn);
  }

  function renderList()
  {
    global $config;
    $smarty = get_smarty();
    $filters = array();
    foreach ($this->objectTypes as $object) {
      $filters[] = array(
        'id'    => 'FILTER'.$object,
        'label' => sprintf(_('Show %s'), $config->data['OBJECTS'][$object]['name'])
      );
    }
    if (!$this->skipTemplates) {
      $filters[] = array(
        'id'    => 'FILTERTEMPLATE',
        'label' => sprintf(_('Show %s'), _('Template'))
      );
    }
    $smarty->assign('objectFilters', $filters);
    $this->headpage->update();
    $display = $this->headpage->render();
    return $this->getHeader().$display;
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
    $type   = strtoupper(preg_replace('/^new_/', '', $action));

    $this->displayApplyBtn  = FALSE;
    $this->dn               = 'new';
    $this->is_single_edit   = FALSE;

    set_object_info($this->dn);

    // Open object
    $this->openTabObject(objects::open($this->dn, $type), $this->headpage->getBase());
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "Create new entry initiated!");
  }

  function newEntryTemplate($action = "", $target = array(), $all = array())
  {
    if ($this->skipTemplates) {
      return;
    }
    $action = preg_replace('/^new_template_/', 'new_', $action);
    $this->newEntry($action, $target, $all);
    $this->tabObject->setTemplateMode('template_cn');
  }

  function newEntryFromTemplate($action, $target = array())
  {
    if ($this->skipTemplates) {
      return;
    }
    if (isset ($target[0])) {
      $dn = $target[0];
    } else {
      $dn = NULL;
    }
    if ($action == 'template_apply') {
      if ($dn === NULL) {
        return;
      }
      $type = $this->getType($dn);
      $type = preg_replace('/^template_/', '', $type);
    } else {
      $type = preg_replace('/^template_apply_/', '', $action);
    }
    $this->dialogObject = new templateDialog($this, $type, $dn);
  }

  function applyTemplateToEntry($action, $target = array())
  {
    global $ui;
    if ($this->skipTemplates) {
      return;
    }
    if (empty($target)) {
      return;
    }
    $type       = preg_replace('/^template_apply_to_/', '', $action);
    $this->dns  = $target;

    // check locks
    if ($locks = get_locks($this->dns)) {
      return gen_locked_message($locks, $this->dns);
    }

    // Add locks
    add_lock ($this->dns, $ui->dn);

    $this->dn           = array_shift($this->dns);
    $this->dialogObject = new templateDialog($this, $type, NULL, $this->dn);
  }

  function handleTemplateApply ($cancel = FALSE)
  {
    if ($this->skipTemplates) {
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

  /*!
   * \brief  Save object modifications and closes dialogs (returns to object listing).
   *          - Calls 'tab::check' to validate the given input.
   *          - Calls 'tab::save' to save back object modifications (e.g. to ldap).
   *          - Calls 'management::remove_locks' to remove eventually created locks.
   *          - Calls 'management::closeDialogs' to return to the object listing.
   */
  protected function saveChanges()
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
   * \brief  Editing an object was caneled.
   *          Close dialogs/tabs and remove locks.
   */
  protected function cancelEdit()
  {
    if (($this->tabObject instanceOf simpleTabs) && ($this->dialogObject instanceOf templateDialog)) {
      $this->handleTemplateApply(TRUE);
      return;
    }
    $this->remove_lock();
    $this->closeDialogs();
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
    global $ui;
    // Do not create a new tabObject while there is already one opened,
    //  the user may have just pressed F5 to reload the page.
    if (is_object($this->tabObject)) {
      return;
    }

    $this->displayApplyBtn = (count($target) == 1);

    // Single edit - we only got one object dn.
    if (count($target) == 1) {
      $type = $this->getType($target[0]);
      if (preg_match('/^template_/', $type) && !$this->skipTemplates) {
        $type = preg_replace('/^template_/', '', $type);
      }

      $this->is_single_edit = TRUE;

      // Get the dn of the object and creates lock
      $this->dn = array_pop($target);
      set_object_info($this->dn);
      if ($locks = get_locks($this->dn)) {
        return gen_locked_message($locks, $this->dn, TRUE);
      }
      add_lock ($this->dn, $ui->dn);

      // Open object
      $this->openTabObject(objects::open($this->dn, $type), $this->dn);
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "Edit entry initiated!");
      if (isset($all['subaction'])) {
        if ($this->handleSubAction($all) == FALSE) {
          trigger_error('Was not able to handle subaction: '.$all['subaction']);
        }
      }
    }
  }

  function handleSubAction($all)
  {
    if (preg_match('/^tab_/', $all['subaction'])) {
      $tab = preg_replace('/^tab_/', '', $all['subaction']);
      if (isset($this->tabObject->by_object[$tab])) {
        $this->tabObject->current = $tab;
      } else {
        trigger_error('Unknown tab: '.$tab);
      }
      return TRUE;
    }
    return FALSE;
  }

  /*! \brief    Queue selected objects to be removed.
   *            Checks ACLs, Locks and ask for confirmation.
   */
  protected function removeEntryRequested($action = "", $target = array(), $all = array())
  {
    global $ui;
    $disallowed = array();
    $this->dns  = array();

    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $target, "Entry removal requested!");

    // Check permissons for each target
    foreach ($target as $dn) {
      $type = $this->getType($dn);
      if (preg_match('/^template_/', $type) && !$this->skipTemplates) {
        $type = preg_replace('/^template_/', '', $type);
      }
      try {
        $info = objects::infos($type);
        $acl  = $ui->get_permissions($dn, $info['aclCategory'].'/'.$info['mainTab']);
        if (preg_match('/d/', $acl)) {
          $this->dns[] = $dn;
        } else {
          $disallowed[] = $dn;
        }
      } catch (NonExistingObjectTypeException $e) {
        trigger_error("Unknown object type received :".$e->getMessage());
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
        $type   = $this->getType($dn);
        if (preg_match('/^template_/', $type) && !$this->skipTemplates) {
          $type = preg_replace('/^template_/', '', $type);
          $info = objects::infos($type);
          $info['mainAttr'] = 'cn';
        } else {
          $info = objects::infos($type);
        }
        $entry  = $this->getHeadpage()->getEntry($dn);
        $objects[] = array(
          'name'  => $entry[$info['mainAttr']][0],
          'dn'    => LDAP::fix($dn),
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

  function removeEntryConfirmed($action = '', $target = array(), $all = array(),
                                $altTabClass = '', $altTabType = '', $altAclCategory = '')
  {
    global $ui;
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $target, "Entry removal confirmed!");

    foreach ($this->dns as $dn) {
      $type = $this->getType($dn);
      if (empty($type)) {
        continue;
      }
      if (preg_match('/^template_/', $type) && !$this->skipTemplates) {
        $type = preg_replace('/^template_/', '', $type);
      }
      $infos = objects::infos($type);
      // Check permissions, are we allowed to remove this object?
      $acl = $ui->get_permissions($dn, $infos['aclCategory']."/".$infos['mainTab']);
      if (preg_match("/d/", $acl)) {
        // Delete the object
        $this->dn = $dn;
        $this->openTabObject(objects::open($this->dn, $type), $this->dn);
        $this->tabObject->delete();

        // Remove the lock for the current object.
        del_lock($this->dn);
      } else {
        msg_dialog::display(_('Permission error'), msgPool::permDelete(), ERROR_DIALOG);
        logging::log('security', 'simpleManagement/'.get_class($this), $dn, array(), 'Tried to trick deletion.');
      }
    }

    // Cleanup
    $this->remove_lock();
    $this->closeDialogs();
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
    global $config, $ui;
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $target, 'Snaptshot creation initiated!');

    if (count($target) == 1) {
      $this->dn = array_pop($target);
      if (empty($this->dn)) {
        return;
      }
      $aclCategory = $config->data['OBJECTS'][$this->getType($this->dn)]['aclCategory'];
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
    global $config, $ui;
    // Set current restore base for snapshot handling.
    if (is_object($this->snapHandler)) {
      $bases = array();
      foreach ($this->storagePoints as $sp) {
        $bases[] = $sp.$this->headpage->getBase();
      }
    }

    // No bases specified? Try base
    if (!count($bases)) {
      $bases[] = $this->headpage->getBase();
    }

    if (!count($target)) {
      // No target, open the restore removed object dialog.
      $this->dn     = $this->headpage->getBase();
      $aclCategory  = $this->headpage->categories;
    } else {
      // Display the restore points for a given object.
      $this->dn = $target[0];
      if (empty($this->dn)) {
        return;
      }
      $aclCategory = $config->data['OBJECTS'][$this->getType($this->dn)]['aclCategory'];
    }

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

  /*! \brief  This method is used to queue and process copy&paste actions.
   *          Allows to copy, cut and paste mutliple entries at once.
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function copyPasteHandler($action = "", $target = array(), $all = array(),
      $altTabClass = "", $altTabType = "", $altAclCategory = "", $altAclPlugin = "")
  {
    global $ui;
    // Return without any actions while copy&paste handler is disabled.
    if (!is_object($this->cpHandler)) {
      return FALSE;
    }

    // Save user input
    $this->cpHandler->save_object();

    // Add entries to queue
    if (($action == 'copy') || ($action == 'cut')) {
      $this->cpHandler->cleanup_queue();
      foreach ($target as $dn) {
        $type   = $this->getType($dn);
        if (preg_match('/^template_/', $type) && !$this->skipTemplates) {
          $type = preg_replace('/^template_/', '', $type);
        }
        $infos  = objects::infos($type);

        if (($action == 'copy') && $ui->is_copyable($dn, $infos['aclCategory'])) {
          $this->cpHandler->add_to_queue($dn, 'copy', $type);
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Entry copied!');
        }
        if (($action == 'cut') && $ui->is_cutable($dn, $infos['aclCategory'], $infos['mainTab'])) {
          $this->cpHandler->add_to_queue($dn, 'cut', $type);
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, 'Entry cut!');
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
      $this->cpHandler->resetPaste();
    }
    return "";
  }

  static function mainInc ($classname)
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
        $managementObject = new $classname();
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
?>
