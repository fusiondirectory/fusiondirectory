<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2018  FusionDirectory

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
 * \file class_managementListing.inc
 * Source code for class managementListing
 */

/*!
 * \brief This class contains all the function needed to sort list
 * go up, go down , back , next. etc...
 */
class entrySortIterator implements Iterator {
  protected $data;

  /*!
   * \brief listingSortIterator constructor
   *
   * \param array $data listingSortIterator data
   *
   * \param $direction Direction
   *
   * \param $attribute Attribute
   *
   * \param string $type
   */
  public function __construct($entries, $column, $direction)
  {
    // Sort for attribute
    if (is_object($column)) {
      uasort(
        $entries,
        function ($ao, $bo) use ($column)
        {
          return $column->compare($ao, $bo);
        }
      );
    }

    // Invert if direction is set
    if ($direction) {
      $this->data = array_reverse($entries, TRUE);
    } else {
      $this->data = $entries;
    }
  }

  /*!
   * \brief Put the array pointer to the first element
   *
   * \return the first element of the array
   */
  function rewind()
  {
    return reset($this->data);
  }

  /*!
   * \brief Get the current data element
   *
   * \return The current element pointed by array pointer
   */
  function current()
  {
    return current($this->data);
  }

  /*!
   * \brief Get the key element
   *
   * \return the key element of the array
   */
  function key()
  {
    return key($this->data);
  }

  /*!
   * \brief Get the next data element
   *
   * \return The next element pointed by array pointer
   */
  function next()
  {
    return next($this->data);
  }

  /*!
   * \brief Check if the data array is valid
   *
   * \return TRUE if the array is valid, return FALSE otherwise
   */
  function valid()
  {
    return (key($this->data) !== NULL);
  }
}

/*!
 * \brief This class handles the entries list for a management instance
 */
class managementListing
{
  protected $entries      = array();
  protected $entriesIndex = array();
  public $multiSelect            = TRUE;
  protected $base;

  protected $sortDirection  = NULL;
  protected $sortColumn     = NULL;
  //~ protected $sortAttribute;
  //~ protected $sortType;

  //~ protected $numColumns;
  protected $baseMode = TRUE;
  protected $bases    = array();
  protected $header   = array();
  //~ protected $colprops = array();
  //~ protected $filters  = array();
  //~ protected $filter   = NULL;
  public $pid;
  //~ protected $departmentTypes  = array();
  //~ var $objectTypes      = array();
  protected $objectTypeCount  = array();
  //~ protected $objectDnMapping  = array();
  //~ protected $snapshotHandler  = NULL;
  protected $baseSelector;
  //~ protected $filterCache = array();

  /* The management class */
  public $parent;

  protected $columns;

  protected $showFooter;

  /*!
   * \brief Create a listing
   *
   * \param string $parent management instance
   */
  function __construct($parent)
  {
    global $config;

    $this->parent = $parent;

    // Initialize pid
    $this->pid = preg_replace('/[^0-9]/', '', microtime(TRUE));

    // Set base for filter
    if ($this->baseMode) {
      $this->base = session::global_get('CurrentMainBase'); // TODO Replace with config or ui var
      if ($this->base == NULL) {
        $this->base = $config->current['BASE'];
      }
      $this->refreshBasesList();
    } else {
      $this->base = $config->current['BASE'];
    }

    // Move footer information
    $this->showFooter = ($config->get_cfg_value('listSummary') == 'TRUE');

    // Instanciate base selector
    $this->baseSelector = new baseSelector($this->bases, $this->base);

    $this->reloadColumns();
  }

  function reloadColumns()
  {
    $columnInfos = $this->parent->getColumnConfiguration();

    $this->columns = array();
    foreach ($columnInfos as $columnInfo) {
      $this->columns[] = Column::build($columnInfo[0], $columnInfo[1]);
    }

    foreach ($this->columns as $column) {
      $column->setParent($this);
    }

    $this->columns[1]->setTemplateAttribute('cn');
  }

  function getColumns()
  {
    return $this->columns;
  }

  /*!
   * \brief Set a SnapshotHandler
   *
   * \param $handler The handler
   *
   * \see  SnapshotHandler
   */
  function setSnapshotHandler($handler)
  {
    if ($handler->enabled()) {
      $this->snapshotHandler = $handler;
    } else {
      $this->snapshotHandler = NULL;
    }
  }

  /*!
   * \brief Set a filter
   *
   * \param string $filter
   *
   * \see filter
   */
  function setFilter($filter)
  {
    $this->filter     = $filter;
    $filter->headpage = $this;
    if ($this->departmentBrowser) {
      $this->departments = $this->getDepartments();
    }
    $this->filter->setBase($this->base);
  }

  function renderHeader()
  {
    $this->header = array();

    // Initialize sort?
    $sortInit = FALSE;
    if (!$this->sortDirection) {
      $this->sortColumn = 0;
      $this->sortDirection = array();
      $sortInit = TRUE;
    }

    foreach ($this->columns as $index => $column) {
      // Initialize everything to one direction
      if ($sortInit) {
        $this->sortDirection[$index] = FALSE;
      }

      $this->header[$index] = array(
        'props'     => $column->getHtmlProps(),
        'sortable'  => $column->isSortable(),
        'label'     => $column->getLabel(),
      );
      if (($index == $this->sortColumn) && $column->isSortable()) {
        $this->header[$index]['sortdirection'] = $this->sortDirection[$index];
      }
    }
  }

  /*!
   * \brief Render
   */
  function render()
  {
    // Check for exeeded sizelimit
    if (($message = check_sizelimit()) != '') {
      return $message;
    }

    $this->renderHeader();

    $smarty = get_smarty();
    $smarty->assign('PID',          $this->pid);
    $smarty->assign('PLUG',         $_GET['plug']);
    $smarty->assign('multiSelect',  $this->multiSelect);
    $smarty->assign('showFooter',   $this->showFooter);
    $smarty->assign('headers',      $this->header);

    $smarty->assign('columnCount', (count($this->columns) + ($this->multiSelect ? 1 : 0)));

    // Complete list by sorting entries and appending them to the output
    $entryIterator = $this->getIterator();

    $rows = array();
    foreach ($entryIterator as $entry) {
      $row = array(
        'cells'   => array(),
        'classes' => array(),
      );
      foreach ($this->columns as $column) {
        $row['cells'][] = array(
          'props'   => $column->getHtmlCellProps(),
          'render'  => $column->renderCell($entry)
        );
        $row['classes'] = array_merge($row['classes'], $column->getRowClasses($entry));
      }

      $row['index']   = $entry->row;

      $rows[] = $row;
    }
    $smarty->assign('rows', $rows);

    // Add the footer if requested
    if ($this->showFooter) {
      $types = array();
      foreach ($this->parent->objectTypes as $type) {
        if (isset($this->objectTypeCount[$type])) {
          $infos = objects::infos($type);
          $types[] = array(
            'name'      => $infos['name'],
            'icon'      => $infos['icon'],
            'count'     => $this->objectTypeCount[$type],
          );
        }
        if (isset($this->objectTypeCount['template_'.$type])) {
          $infos = objects::infos($type);
          $types[] = array(
            'name'      => sprintf(_('%s template'), $infos['name']),
            'icon'      => 'geticon.php?context=devices&icon=template&size=16',
            'count'     => $this->objectTypeCount['template_'.$type],
          );
        }
      }
      $smarty->assign('objectCounts', $types);
    }

    return $smarty->fetch(get_template_path('management/list.tpl'));
  }

  function getIterator()
  {
    return new entrySortIterator($this->entries, $this->columns[$this->sortColumn], $this->sortDirection[$this->sortColumn]);
  }

  /*!
   * \brief Update a listing
   */
  function update()
  {
    $ui = get_userinfo();

    // Take care of base selector
    if ($this->baseMode) {
      $this->baseSelector->update();
      // Check if a wrong base was supplied
      if (!$this->baseSelector->checkLastBaseUpdate()) {
         msg_dialog::display(_("Error"), msgPool::check_base(), ERROR_DIALOG);
      }
    }

    // Save base
    $refresh = FALSE;
    if ($this->baseMode) {
      $this->base = $this->baseSelector->getBase();
      session::global_set("CurrentMainBase", $this->base);
      $refresh = TRUE;
    }

    // Do not do anything if this is not our PID
    if ($refresh || !(isset($_REQUEST['PID']) && ($_REQUEST['PID'] != $this->pid))) {

      // Filter GET with "act" attributes
      if (isset($_GET['act'])) {
        $key = validate($_GET['act']);
        if (preg_match('/^SORT_([0-9]+)$/', $key, $match)) {
          // Switch to new column or invert search order?
          $column = $match[1];
          if ($this->sortColumn != $column) {
            $this->sortColumn = $column;
          } else {
            $this->sortDirection[$column] = !$this->sortDirection[$column];
          }

          // Allow header to update itself according to the new sort settings
          $this->renderHeader();
        }
      }

      if ($this->baseMode) {
        // Override base if we got signals from the navigation elements
        $action = '';
        foreach (array_keys($_POST) as $key) {
          if (preg_match('/^(ROOT|BACK|HOME)_x$/', $key, $match)) {
            $action = $match[1];
            break;
          }
        }

        // Navigation handling
        if ($action == 'ROOT') {
          $this->setBase(key($this->bases));
        } elseif ($action == 'BACK') {
          $base = preg_replace('/^[^,]+,/', '', $this->base);
          $this->tryAndSetBase($base);
        } elseif ($action == 'HOME') {
          $this->tryAndSetBase(get_base_from_people($ui->dn));
        }
      }
    }

    // Update filter and refresh entries
    $attrs = $this->parent->neededAttrs;
    foreach ($this->columns as $column) {
      $column->fillNeededAttributes($attrs);
    }
    list($this->entries, $this->objectTypeCount) = $this->parent->filter->query($attrs, $this->base);
    /* Store the order of the entries to access them by index later */
    $this->entriesIndex = array_keys($this->entries);
  }

  /*!
   * \brief Set a new base valor
   *
   * \param string $base
   */
  function setBase($base)
  {
    $this->base = $base;
    if ($this->baseMode) {
      $this->baseSelector->setBase($this->base);
      session::global_set('CurrentMainBase', $this->base);
    }
  }

  function tryAndSetBase($base)
  {
    if (isset($this->bases[$base])) {
      $this->setBase($base);
    }
  }

  /*!
   * \brief Accessor of the base
   *
   * \return the base
   */
  function getBase()
  {
    return $this->base;
  }

  function renderBase()
  {
    if (!$this->baseMode) {
      return;
    }

    return $this->baseSelector->render();
  }

  function renderNavigation()
  {
    $result = array();
    $enableBack = TRUE;
    $enableRoot = TRUE;
    $enableHome = TRUE;

    $ui = get_userinfo();

    /* Check if base = first available base */
    $deps = array_keys($this->bases);

    if (!count($deps) || $deps[0] == $this->base) {
      $enableBack = FALSE;
      $enableRoot = FALSE;
    }

    /* Check if we are in users home  department */
    if (!count($deps) || ($this->base == get_base_from_people($ui->dn)) || !in_array_ics(get_base_from_people($ui->dn), $deps)) {
      $enableHome = FALSE;
    }

    $actions = array(
      array(
        'id'      => 'ROOT',
        'desc'    => _('Go to root department'),
        'name'    => _('Root'),
        'icon'    => 'geticon.php?context=actions&icon=go-first&size=16',
        'enabled' => $enableRoot,
        'class'   => '',
      ),
      array(
        'id'      => 'BACK',
        'desc'    => _('Go up one department'),
        'name'    => _('Up'),
        'icon'    => 'geticon.php?context=actions&icon=go-up&size=16',
        'enabled' => $enableBack,
        'class'   => '',
      ),
      array(
        'id'      => 'HOME',
        'desc'    => _('Go to user\'s department'),
        'name'    => _('Home'),
        'icon'    => 'geticon.php?context=actions&icon=go-home&size=16',
        'enabled' => $enableHome,
        'class'   => '',
      ),
      array(
        'id'      => 'REFRESH',
        'desc'    => _('Reload list'),
        'name'    => _('Reload'),
        'icon'    => 'geticon.php?context=actions&icon=view-refresh&size=16',
        'enabled' => TRUE,
        'class'   => 'optional',
      ),
      array(
        'id'      => 'listing_configure',
        'desc'    => _('Configure this management list'),
        'name'    => _('Configure'),
        'icon'    => 'geticon.php?context=categories&icon=settings&size=16',
        'enabled' => TRUE,
        'class'   => '',
      ),
    );

    // TODO Remove root/back/home if not base mode
    return $actions;
  }

  /*!
   * \brief Get action
   */
  function getAction()
  {
    global $config;

    // Do not do anything if this is not our PID, or there's even no PID available...
    if (!isset($_REQUEST['dn']) && (!isset($_REQUEST['PID']) || $_REQUEST['PID'] != $this->pid)) {
      return;
    }

    $result = array('targets' => array(), 'action' => '', 'subaction' => NULL);

    // Filter GET with "act" attributes
    if (isset($_GET['act'])) {
      $key = validate($_GET['act']);
      if (preg_match('/^listing_([a-zA-Z_]+)_([0-9]+)$/', $key, $m)) {
        $action = $m[1];
        $target = $m[2];
        if (isset($this->entriesIndex[$target])) {
          $result['action']     = $action;
          $result['targets'][]  = $this->entriesIndex[$target];
        }
      } elseif (isset($_REQUEST['dn']) && preg_match('/^listing_([a-zA-Z_]+)$/', $key, $m)) {
        // TODO fix this
        /* Pre-render list to init things if a dn is gonna be opened on first load */
        $dn         = urldecode($_REQUEST['dn']);
        $action     = $m[1];
        /* Detect the longer base valid for this dn */
        $longerBase = '';
        foreach (array_keys($this->bases) as $base) {
          if (preg_match('/'.preg_quote($base, '/').'$/i', $dn)) {
            if (strlen($base) > strlen($longerBase)) {
              $longerBase = $base;
            }
          }
        }
        $this->setBase($longerBase);
        /* Search for its main attr value */
        if (isset($this->filter->elementValues['NAME']) && preg_match('/^[^=]+=([^,]+)/', $dn, $m)) {
          $this->filter->elementValues['NAME'] = $m[1];
        }
        $this->filter->setCurrentScope('one');
        /* Bypass size limit just to be sure */
        $oldsizelimit = session::global_get('size_limit');
        session::global_set('size_limit', 0);
        $this->update();
        $this->render();
        session::global_set('size_limit', $oldsizelimit);
        $this->filter->elementValues['NAME'] = '';

        $result['action']     = $action;
        $result['targets'][]  = $dn;
        // Make sure no other management class intercept the same dn
        unset($_REQUEST['dn']);
      }

      // Drop targets if empty
      if (count($result['targets']) == 0) {
        unset($result['targets']);
      }
      if (preg_match('/^([a-zA-Z]+)_([a-zA-Z_]+)$/', $result['action'], $m)) {
        $result['action']     = $m[1];
        $result['subaction']  = $m[2];
      }
      return $result;
    }

    // Filter POST with "listing_" attributes
    foreach (array_keys($_POST) as $key) {

      // Capture selections
      if (preg_match('/^listing_selected_[0-9]+$/', $key)) {
        $target = preg_replace('/^listing_selected_([0-9]+)$/', '$1', $key);
        if (isset($this->entriesIndex[$target])) {
          $result['targets'][] = $this->entriesIndex[$target];
        }
        continue;
      }

      // Capture action with target - this is a one shot
      if (preg_match('/^listing_[a-zA-Z_]+_[0-9]+(|_x)$/', $key)) {
        $target = preg_replace('/^listing_[a-zA-Z_]+_([0-9]+)(|_x)$/', '$1', $key);
        if (isset($this->entriesIndex[$target])) {
          $result['action']   = preg_replace('/^listing_([a-zA-Z_]+)_[0-9]+(|_x)$/', '$1', $key);
          $result['targets']  = array($this->entriesIndex[$target]);
        }
        break;
      }

      // Capture action without target
      if (preg_match('/^listing_[a-zA-Z_]+(|_x)$/', $key)) {
        $result['action'] = preg_replace('/^listing_([a-zA-Z_]+)(|_x)$/', '$1', $key);
        continue;
      }
    }

    // Filter POST with "act" attributes -> posted from action menu
    if (isset($_POST['act']) && ($_POST['act'] != '')) {
      $result['action'] = validate($_POST['act']);
    }

    // Drop targets if empty
    if (count($result['targets']) == 0) {
      unset($result['targets']);
    }
    if (preg_match('/^([a-zA-Z]+)_([a-zA-Z_]+)/', $result['action'], $m)) {
      $result['action']     = $m[1];
      $result['subaction']  = $m[2];
    }
    return $result;
  }

  /*!
   * \brief Refresh the bases list
   */
  function refreshBasesList()
  {
    global $config;
    $ui = get_userinfo();

    // Fill internal bases list
    $this->bases = array();

    $categories = array();
    foreach ($this->parent->objectTypes as $otype) {
      $i = objects::infos($otype);
      $categories[$i['aclCategory']] = $i['aclCategory'];
    }

    $deps = $ui->get_module_departments(array_values($categories));
    foreach ($config->idepartments as $key => $dep) {
      if (in_array_ics($key, $deps)) {
        $this->bases[$key] = $dep;
      }
    }

    if (!empty($this->bases) && !isset($this->bases[$this->base])) {
      $this->base = key($this->bases);
    }

    // Populate base selector if already present
    if ($this->baseSelector && $this->baseMode) {
      $this->baseSelector->setBases($this->bases);
      $this->baseSelector->setBase($this->base);
      $this->baseSelector->update(TRUE);
    }
  }

  /*! \brief Get the departments */
  function getDepartments()
  {
    $departments = array();
    $ui = get_userinfo();

    // Get list of supported department types
    $types = departmentManagement::getDepartmentTypes();

    // Load departments allowed by ACL
    $validDepartments = array_keys($this->bases);

    /* Fetch departments andtheir informations */
    foreach ($types as $type) {
      $i    = objects::infos($type);
      $deps = objects::ls(
        $type,
        array(
          'dn'            => 'raw',
          'objectClass'   => 'raw',
          'description'   => 'raw',
          $i['mainAttr']  => 'raw'
        ),
        $this->base,
        '',
        FALSE,
        'one'
      );

      // Analyze list of departments
      foreach ($deps as $department) {
        if (!in_array($department['dn'], $validDepartments)) {
          continue;
        }

        /* php-ldap like indexes are needed for renderCell */
        $count = 0;
        foreach ($department as $key => $values) {
          if ($key != 'dn') {
            $department[$count++] = $key;
          }
        }
        $department['count'] = $count;

        // Add the attribute where we use for sorting
        $department['sort-attribute'] = $i['mainAttr'];

        // Move to the result list
        $departments[] = $department;
      }
    }

    return $departments;
  }

  function getEntry($dn)
  {
    if (isset($this->entries[$dn])) {
      return $this->entries[$dn];
    }
    return NULL;
  }

  /*!
   * \brief Get type
   *
   * \param string $dn The DN
   */
  function getType($dn)
  {
    if (isset($this->entries[$dn])) {
      return $this->entries[$dn]->type;
    }
    return NULL;
  }
}

?>