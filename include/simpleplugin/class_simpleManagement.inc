<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2014  FusionDirectory

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

  protected $tabObject;

  protected $post_finish = 'template_continue';
  protected $post_cancel = 'template_cancel';

  function __construct($simpleManagement, $type)
  {
    $this->simpleManagement = $simpleManagement;
    $this->type             = $type;
    $this->templates        = objects::getTemplates($this->type);
  }

  function save_object ()
  {
    if (isset($_POST[$this->post_cancel])) {
      return $this->handle_cancel();
    }
    if ((isset($_POST[$this->post_finish]) || isset($_GET[$this->post_finish])) && is_object($this->template)) {
      $this->template->save_object();
      return $this->handle_finish();
    }

    if (
      isset($_POST['template']) &&
      isset($this->templates[$_POST['template']])
      ) {
      if (is_object($this->template)) {
        trigger_error('redefining template object');
      }
      $this->template = new template($this->type, $_POST['template']);
    }
    if (is_object($this->template)) {
      $this->template->save_object();
    }

    return TRUE;
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
  /* Management attributes we override */
  protected $tabClass     = 'simpleTabs';
  protected $tabType      = '';
  protected $aclCategory  = '';
  protected $aclPlugin    = '';

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

  function __construct($config, $ui)
  {
    $this->config = $config;
    $this->ui     = $ui;

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

    parent::__construct($config, $ui, 'unused', $headpage);

    $this->configureHeadpage();
    $this->configureFilter();
    if ($this->departmentBrowser === FALSE) {
      $this->headpage->setBase($this->config->current['BASE']);
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
            'image'   => 'geticon.php?context=actions&amp;icon=document-new&amp;size=16',
            'label'   => _('Template'),
            'action'  => array(),
          ),
          array(
            'type'    => 'sub',
            'image'   => 'geticon.php?context=actions&amp;icon=document-new&amp;size=16',
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
            'class'       => 'fdTemplate',
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
        $icon = 'geticon.php?context=actions&icon=document-new&size=16';
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
    $this->headpage->registerElementFilter('filterRowLink', 'simpleManagement::filterRowLink');
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
          'set'     => $i['filter']
        );
        $this->filter->elementValues[$tag] = TRUE;

        $attributes[] = $i['mainAttr'];
        $attributes[] = $i['nameAttr'];
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
    $smarty = get_smarty();
    $filters = array();
    foreach ($this->objectTypes as $object) {
      $filters[] = array(
        'id'    => 'FILTER'.$object,
        'label' => sprintf(_('Show %s'), $this->config->data['OBJECTS'][$object]['name'])
      );
    }
    $smarty->assign('objectFilters', $filters);
    return parent::renderList();
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
    $this->is_new           = TRUE;
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

  function newEntryFromTemplate($action)
  {
    if ($this->skipTemplates) {
      return;
    }
    $type = preg_replace('/^template_apply_/', '', $action);
    $this->dialogObject = new templateDialog($this, $type);
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

      $this->is_new         = FALSE;
      $this->is_single_edit = TRUE;

      // Get the dn of the object and creates lock
      $this->dn = array_pop($target);
      set_object_info($this->dn);
      $user = get_lock($this->dn);
      if ($user != "") {
        return gen_locked_message($user, $this->dn, TRUE);
      }
      add_lock ($this->dn, $this->ui->dn);

      // Open object
      $this->openTabObject(objects::open($this->dn, $type), $this->dn);
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "Edit entry initiated!");
      if (isset($all['subaction'])) {
        $tab = preg_replace('/^tab_/', '', $all['subaction']);
        if (isset($this->tabObject->by_object[$tab])) {
          $this->tabObject->current = $tab;
        } else {
          trigger_error('Unknown tab: '.$tab);
        }
      }
    }
  }

  /*! \brief    Queue selected objects to be removed.
   *            Checks ACLs, Locks and ask for confirmation.
   */
  protected function removeEntryRequested($action = "", $target = array(), $all = array())
  {
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
        $acl  = $this->ui->get_permissions($dn, $info['aclCategory'].'/'.$info['mainTab']);
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
      if ($user = get_multiple_locks($this->dns)) {
        return gen_locked_message($user, $this->dns);
      }

      // Add locks
      $objects = array();
      foreach ($this->dns as $dn) {
        $type   = $this->getType($dn);
        if (preg_match('/^template_/', $type) && !$this->skipTemplates) {
          $type = preg_replace('/^template_/', '', $type);
        }
        $info   = objects::infos($type);
        $entry  = $this->getHeadpage()->getEntry($dn);
        $objects[] = array(
          'name'  => $entry[$info['mainAttr']][0],
          'dn'    => LDAP::fix($dn),
          'icon'  => $info['icon'],
          'type'  => $info['name']
        );
      }
      add_lock ($this->dns, $this->ui->dn);

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
      $acl = $this->ui->get_permissions($dn, $infos['aclCategory']."/".$infos['mainTab']);
      if (preg_match("/d/", $acl)) {
        // Delete the object
        $this->dn = $dn;
        $this->openTabObject(objects::open($this->dn, $type), $this->dn);
        $this->tabObject->delete();

        // Remove the lock for the current object.
        del_lock($this->dn);
      } else {
        msg_dialog::display(_("Permission error"), msgPool::permDelete(), ERROR_DIALOG);
        new log("security", "simpleManagement/".get_class($this), $dn, array(), "Tried to trick deletion.");
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
  function createSnapshotDialog($action = "", $target = array(), $all = array())
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $target, "Snaptshot creation initiated!");

    foreach ($target as $dn) {
      $aclCategory = $this->config->data['OBJECTS'][$this->getType($dn)]['aclCategory'];
      if (!empty($dn) && $this->ui->allow_snapshot_create($dn, $aclCategory)) {
        $this->dialogObject                 = new SnapShotDialog($this->config, $dn, $this);
        $this->dialogObject->aclCategories  = array($aclCategory);
        $this->dialogObject->parent         = &$this;
      } else {
        msg_dialog::display(_("Permission"), sprintf(_("You are not allowed to create a snapshot for %s."), $dn),
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
  function restoreSnapshotDialog($action = "", $target = array(), $all = array())
  {
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

    // No target, open the restore removed object dialog.
    if (!count($target)) {
      $entry = $this->headpage->getBase();
      $aclCategory = $this->config->data['OBJECTS'][$this->getType($entry)]['aclCategory'];
      if (!empty($entry) && $this->ui->allow_snapshot_restore($entry, $aclCategory)) {
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $entry, "Snaptshot restoring initiated!");
        $this->dialogObject = new SnapShotDialog($this->config, $entry, $this);
        $this->dialogObject->set_snapshot_bases($bases);
        $this->dialogObject->display_all_removed_objects  = TRUE;
        $this->dialogObject->display_restore_dialog       = TRUE;
        $this->dialogObject->parent                       = &$this;
        $this->dialogObject->aclCategory                  = $aclCategory;
      } else {
        msg_dialog::display(_("Permission"), sprintf(_("You are not allowed to restore a snapshot for %s."), $entry),
            ERROR_DIALOG);
      }
    } else {
      // Display the restore points for a given object.
      $entry = array_pop($target);
      $aclCategory = $this->config->data['OBJECTS'][$this->getType($entry)]['aclCategory'];
      if (!empty($entry) && $this->ui->allow_snapshot_restore($entry, $aclCategory)) {
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $entry, "Snaptshot restoring initiated!");
        $this->dialogObject = new SnapShotDialog($this->config, $entry, $this);
        $this->dialogObject->set_snapshot_bases($bases);
        $this->dialogObject->display_restore_dialog = TRUE;
        $this->dialogObject->parent                 = &$this;
        $this->dialogObject->aclCategory            = $aclCategory;
      } else {
        msg_dialog::display(_("Permission"), sprintf(_("You are not allowed to restore a snapshot for %s."), $entry),
            ERROR_DIALOG);
      }
    }
  }

  /*!
   * \brief  Restores a snapshot object.
   *         The dn of the snapshot entry has to be given as ['target'] parameter.
   *
   * \param  String  $action  The name of the action which was the used as trigger.
   *
   * \param  Array   $target  A list of object dns, which should be affected by this method.
   *
   * \param  Array   $all     A combination of both 'action' and 'target'.
   */
  function restoreSnapshot($action = "", $target = array(), $all = array())
  {
    $entry = array_pop($target);
    $aclCategory = $this->dialogObject->aclCategory;
    if (!empty($entry) && $this->ui->allow_snapshot_restore($entry, $aclCategory)) {
      $this->snapHandler->restore_snapshot($entry);
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "Snaptshot restored!");
      $this->closeDialogs();
    } else {
      msg_dialog::display(_("Permission"), sprintf(_("You are not allowed to restore a snapshot for %s."), $entry),
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
    // Return without any actions while copy&paste handler is disabled.
    if (!is_object($this->cpHandler)) {
      return FALSE;
    }

    // Save user input
    $this->cpHandler->save_object();

    // Add entries to queue
    if ($action == "copy" || $action == "cut") {
      $this->cpHandler->cleanup_queue();
      foreach ($target as $dn) {
        $type   = $this->getType($dn);
        $infos  = objects::infos($type);

        if ($action == "copy" && $this->ui->is_copyable($dn, $infos['aclCategory'], $infos['mainTab'])) {
          $this->cpHandler->add_to_queue($dn, "copy", $type);
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $dn, "Entry copied!");
        }
        if ($action == "cut" && $this->ui->is_cutable($dn, $infos['aclCategory'], $infos['mainTab'])) {
          $this->cpHandler->add_to_queue($dn, "cut", $type);
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

  static function filterRowLink()
  {
    $pid    = func_get_arg(0);
    $row    = func_get_arg(1);
    $dn     = func_get_arg(2);
    $trans  = func_get_arg(3);
    unset($trans['count']);
    $trans  = join("<br/>\n", $trans);
    if ($trans != "") {
      return '<a href="?plug='.$_GET['plug'].'&amp;PID='.$pid.'&amp;act=listing_edit_'.$row.'" title="'.$dn.'">'.$trans.'</a>';
    }

    return '';
  }

  static function mainInc ($classname)
  {
    global $remove_lock, $cleanup, $display, $config, $ui;

    /* Remove locks */
    if ($remove_lock) {
      if (session::is_set($classname)) {
          $macl = session::get($classname);
          $macl->remove_lock();
      }
    }

    if ($cleanup) {
      /* Clean up */
      session::un_set($classname);
    } else {
      if (!session::is_set($classname) || (isset($_GET['reset']) && $_GET['reset'] == 1)) {
        /* Create the object if missing or reset requested */
        $managementObject = new $classname($config, $ui);
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
