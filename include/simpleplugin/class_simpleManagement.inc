<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013  FusionDirectory

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


class simpleManagement extends management
{
  /* Management attributes we override */
  protected $tabClass     = 'simpleTabs';
  protected $tabType      = '';
  protected $aclCategory  = '';
  protected $aclPlugin    = '';

  /* Attributes that child classes should override */
  protected $objectName   = ''; // FIXME : should not be needed anymore
  protected $objectTypes  = array();

  /* Attributes that child classes can override */
  protected $departmentBrowser      = TRUE;
  protected $departmentRootVisible  = TRUE;
  protected $baseMode               = TRUE;

  protected $filterXMLPath  = NULL;
  protected $listXMLPath    = NULL;
  protected $autoFilter     = TRUE;

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
    foreach ($this->objectTypes as $object) {
      $ou = $this->config->data['OBJECTS'][$object]['ou'];
      if ($ou !== NULL) {
        $this->storagePoints[] = $ou;
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
    $headpage = new listing($this->listXMLPath);
    $headpage->departmentBrowser      = $this->departmentBrowser;
    $headpage->departmentRootVisible  = $this->departmentRootVisible;
    $headpage->baseMode               = $this->baseMode;
    $headpage->setFilter($filter);

    parent::__construct($config, $ui, 'unused', $headpage);

    $this->configureFilterAndHeadpage();
  }

  /* Build the action menu, fill the object definition, etc, based on objectTypes info */
  function configureFilterAndHeadpage ()
  {
    $this->headpage->xmlData['actionmenu']['action'][0]['action'] = array();
    $pInfos = pluglist::pluginInfos(get_class($this));
    $this->headpage->headline = $pInfos['plShortName'];
    $filter = '(|';
    foreach ($this->objectTypes as $object) {
      $i = $this->config->data['OBJECTS'][$object];

      $this->headpage->objectTypes[$object] =
        array(
          'label'       => $i['name'],
          'objectClass' => preg_replace('/^.*objectClass=([^)]+)(\).*)?$/', '\\1', $i['filter']),
          'category'    => $i['aclCategory'],
          'class'       => $i['mainTab'],
          'image'       => $i['icon']
        );
      $this->headpage->categories[] = $i['aclCategory'];
      $this->registerAction('new_'.$object, 'newEntry');
      $icon = 'images/lists/new.png';
      if (isset($i['icon'])) {
        $icon = $i['icon'];
      } else {
        $infos = pluglist::pluginInfos($i['mainTab']);
        if (isset($infos['plIcon'])) {
          $icon = $infos['plIcon'];
        }
      }
      $this->headpage->xmlData['actionmenu']['action'][0]['action'][] = array(
        'name' => 'new_'.$object,
        'type' => 'entry',
        'image' => $icon,
        'label' => $i['name'],
      );
      $filter .= '$FILTER'.$object;
    }
    $filter .= ')';
    if ($this->autoFilter) {
      $this->filter->category = key($pInfos['plCategory']);
      $this->filter->query[0]['filter']    = '(&'.$filter.'$NAME)';
      $this->filter->query[0]['attribute'] = array('dn', 'cn', 'description');
      $this->filter->element['NAME']['set'] = '(cn=*$*)';
      $this->filter->element['NAME']['autocomplete']['filter'] = '(&'.$filter.'(cn=*$NAME*))';
      foreach ($this->objectTypes as $object) {
        $tag = 'FILTER'.$object;
        $this->filter->elements[$tag] = array(
          'type'    => 'checkbox',
          'tag'     => $tag,
          'default' => TRUE,
          'unset'   => array(),
          'set'     => $this->config->data['OBJECTS'][$object]['filter']
        );
        $this->filter->elementValues[$tag] = TRUE;
      }
      uasort($this->filter->elements, 'strlenSort');
      $this->filter->elements = array_reverse($this->filter->elements);
    }
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

  // Overridden new handler - We've different types of departments to create!
  function newEntry($action = '', $target = array(), $all = array(), $altTabClass = '',
                    $altTabType = '', $altAclCategory = '')
  {
    $type   = strtoupper(preg_replace('/^new_/', '', $action));
    $infos  = $this->config->data['OBJECTS'][$type];

    return management::newEntry($action, $target, $all, $infos['tabClass'],
                                $infos['tabGroup'], $infos['aclCategory']);
  }

  function editEntry( $action = '', $target = array(), $all = array(), $altTabClass = '',
                      $altTabType = '', $altAclCategory = '')
  {
    $type   = $this->getType($target[0]);
    $infos  = $this->config->data['OBJECTS'][$type];

    $str = management::editEntry( $action, $target, $all, $infos['tabClass'],
                                $infos['tabGroup'], $infos['aclCategory']);
    if (preg_match('/^edit_/', $action)) {
      $tab = preg_replace('/^edit_/', '', $action);
      if (isset($this->tabObject->by_object[$tab])) {
        $this->tabObject->current = $tab;
      } else {
        trigger_error('Unknown tab: '.$tab);
      }
    }
    if (!empty($str)) {
      return $str;
    }
  }

  /*! \brief    Queue selected objects to be removed.
   *            Checks ACLs, Locks and ask for confirmation.
   */
  protected function removeEntryRequested($action = "", $target = array(), $all = array())
  {
    $disallowed = array();
    $this->dns = array();

    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $target, "Entry removal requested!");

    // Check permissons for each target
    $tInfo = $this->config->data['OBJECTS'];
    foreach ($target as $dn) {
      $type = $this->getType($dn);
      if (!isset($tInfo[$type])) {
        trigger_error("Unknown object type received '$type'!");
      } else {
        $info = $tInfo[$type];
        $acl = $this->ui->get_permissions($dn, $info['aclCategory'].'/'.$info['mainTab']);
        if (preg_match('/d/', $acl)) {
          $this->dns[] = $dn;
        } else {
          $disallowed[] = $dn;
        }
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
      $dns_names = array();
      foreach ($this->dns as $dn) {
        $dns_names[] = LDAP::fix($dn);
      }
      add_lock ($this->dns, $this->ui->dn);

      // Display confirmation dialog.
      $smarty = get_smarty();
      $smarty->assign('info', msgPool::deleteInfo($dns_names, _($this->objectName)));
      $smarty->assign('multiple', TRUE);
      return $smarty->fetch(get_template_path('simple-remove.tpl'));
    }
  }

  function removeEntryConfirmed($action = '', $target = array(), $all = array(),
                                $altTabClass = '', $altTabType = '', $altAclCategory = '')
  {
    $targets = array();
    foreach ($this->dns as $dn) {
      $type = $this->getType($dn);
      if (empty($type)) {
        continue;
      }
      $targets[$type][] = $dn;
    }

    $tInfo = $this->config->data['OBJECTS'];
    foreach ($targets as $type => $dns) {
      $this->dns = $dns;
      management::removeEntryConfirmed($action, $target, $all, $tInfo[$type]['tabClass'],
                                  $tInfo[$type]['tabGroup'], $tInfo[$type]['aclCategory']);
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
  function copyPasteHandler($action = '', $target = array(), $all = array(),
      $altTabClass = '', $altTabType = '', $altAclCategory = '', $altAclPlugin = '')
  {
    $tInfo = $this->config->data['OBJECTS'];

    $tabClass     = array();
    $tabType      = array();
    $aclCategory  = array();
    $aclPlugin    = array();

    $headpage = $this->getHeadpage();

    foreach ($target as $dn) {
      $type   = $this->getType($dn);
      $entry  = $headpage->getEntry($dn);

      $tabClass[$dn]    = $tInfo[$type]['tabClass'];
      $tabType[$dn]     = $tInfo[$type]['tabGroup'];
      $aclCategory[$dn] = $tInfo[$type]['aclCategory'];
      $aclPlugin[$dn]   = $tInfo[$type]['mainTab'];
    }

    return parent::copyPasteHandler($action, $target, $all, $tabClass, $tabType, $aclCategory, $aclPlugin);
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
