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
 * \brief This class handles the entries list for a management instance
 */
class managementListing
{
  public $pid;

  protected $entries      = [];
  protected $entriesIndex = [];
  protected $base;

  protected $sortDirection  = NULL;
  protected $sortColumn     = NULL;

  protected $baseMode         = TRUE;
  protected $multiSelect      = TRUE;
  protected $bases            = [];
  protected $header           = [];
  protected $objectTypeCount  = [];
  protected $baseSelector;

  /* The management class */
  public $parent;

  protected $columns;

  protected $showFooter;

  /*!
   * \brief Create a listing
   *
   * \param string $parent management instance
   */
  function __construct (management $parent, bool $baseMode = TRUE, bool $multiSelect = TRUE)
  {
    global $config;

    $this->parent       = $parent;
    $this->baseMode     = $baseMode;
    $this->multiSelect  = $multiSelect;

    // Initialize pid
    $this->pid = preg_replace('/[^0-9]/', '', microtime(TRUE));

    $this->setUpBaseSelector();

    // Move footer information
    $this->showFooter = ($config->get_cfg_value('listSummary') == 'TRUE');

    $this->reloadColumns();
  }

  function setUpBaseSelector ()
  {
    global $config, $ui;

    // Set base for filter
    if ($this->baseMode) {
      $this->base = $ui->getCurrentBase();
      $this->refreshBasesList();

      // Instanciate base selector
      $this->baseSelector = new baseSelector($this->bases, $this->base);
    } else {
      $this->base = $config->current['BASE'];
    }
  }

  function getBaseMode (): bool
  {
    return $this->baseMode;
  }

  function getMultiSelect (): bool
  {
    return $this->multiSelect;
  }

  function reloadColumns ()
  {
    $columnInfos = $this->parent->getColumnConfiguration();

    $this->columns  = [];
    $first          = TRUE;
    foreach ($columnInfos as $columnInfo) {
      $column = Column::build($this, $columnInfo[0], $columnInfo[1]);
      if ($first && !empty($columnInfo[1]['attributes'])) {
        $column->setTemplateAttributes(['cn']);
        $first = FALSE;
      }
      $this->columns[] = $column;
    }
  }

  function getColumns (): array
  {
    return $this->columns;
  }

  function renderHeader ()
  {
    $this->header = [];

    // Initialize sort?
    $sortInit = FALSE;
    if (!isset($this->sortDirection)) {
      $this->sortColumn     = 0;
      $this->sortDirection  = [];
      $sortInit             = TRUE;
    }

    foreach ($this->columns as $index => $column) {
      // Initialize everything to one direction
      if ($sortInit) {
        $this->sortDirection[$index] = FALSE;
      }

      $this->header[$index] = [
        'props'     => $column->getHtmlProps(),
        'sortable'  => $column->isSortable(),
        'label'     => $column->getLabel(),
      ];
      if (($index == $this->sortColumn) && $column->isSortable()) {
        $this->header[$index]['sortdirection'] = $this->sortDirection[$index];
      }
    }
  }

  /*!
   * \brief Render
   */
  function render (): string
  {
    global $ui;

    // Check for exeeded sizelimit
    if (($message = $ui->getSizeLimitHandler()->check()) != '') {
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

    $rows = [];
    foreach ($entryIterator as $entry) {
      $row = [
        'cells'   => [],
        'classes' => [],
      ];
      foreach ($this->columns as $column) {
        $row['cells'][] = [
          'props'   => $column->getHtmlCellProps(),
          'render'  => $column->renderCell($entry)
        ];
        $column->fillRowClasses($row['classes'], $entry);
      }

      $row['index']   = $entry->row;

      $rows[] = $row;
    }
    $smarty->assign('rows', $rows);

    // Add the footer if requested
    if ($this->showFooter) {
      $types = [];
      foreach ($this->parent->objectTypes as $type) {
        if (isset($this->objectTypeCount[$type])) {
          $infos = objects::infos($type);
          $types[] = [
            'name'      => $infos['name'],
            'icon'      => $infos['icon'],
            'count'     => $this->objectTypeCount[$type],
          ];
        }
        if (isset($this->objectTypeCount['template_'.$type])) {
          $infos = objects::infos($type);
          $types[] = [
            'name'      => sprintf(_('%s template'), $infos['name']),
            'icon'      => 'geticon.php?context=devices&icon=template&size=16',
            'count'     => $this->objectTypeCount['template_'.$type],
          ];
        }
      }
      $smarty->assign('objectCounts', $types);
    }

    return $smarty->fetch(get_template_path('management/list.tpl'));
  }

  function getIterator (): Iterator
  {
    return new EntrySortIterator($this->entries, $this->columns[$this->sortColumn] ?? NULL, $this->sortDirection[$this->sortColumn] ?? FALSE);
  }

  /*!
   * \brief Updates base and sorting according to POST and GET infos
   */
  function updateBase ()
  {
    global $ui;

    // Take care of base selector
    if ($this->baseMode) {
      $this->baseSelector->update();

      // Check if a wrong base was supplied
      if (!$this->baseSelector->checkLastBaseUpdate()) {
        msg_dialog::display(_('Error'), msgPool::check_base(), ERROR_DIALOG);
      }

      // Save base
      $this->base = $this->baseSelector->getBase();
      $ui->setCurrentBase($this->base);
    }

    // Do not do anything if this is not our PID
    if ($this->baseMode || !(isset($_REQUEST['PID']) && ($_REQUEST['PID'] != $this->pid))) {
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
          $parentBase = preg_replace('/^[^,]+,/', '', $this->base);
          $this->tryAndSetBase($parentBase);
        } elseif ($action == 'HOME') {
          $ui = get_userinfo();
          $this->tryAndSetBase($ui->getBase());
        }
      }
    }
  }

  /*!
   * \brief Update a listing
   */
  function update (string $dn = NULL)
  {
    if ($dn === NULL) {
      $this->updateBase();
    }

    // Update filter
    $this->parent->filter->update($this->base);

    // Update filter and refresh entries
    $attrs = $this->parent->neededAttrs;
    foreach ($this->columns as $column) {
      $column->fillNeededAttributes($attrs);
    }
    if ($dn !== NULL) {
      $this->parent->filter->setScope('base');
      list($this->entries, $this->objectTypeCount) = $this->parent->filter->query($attrs, $dn);
      $this->parent->filter->setScope('one');
    } else {
      list($this->entries, $this->objectTypeCount) = $this->parent->filter->query($attrs, $this->base);
    }
    /* Store the order of the entries to access them by index later */
    $this->entriesIndex = array_keys($this->entries);
  }

  /*!
   * \brief Set a new base valor
   *
   * \param string $base
   */
  function setBase ($base)
  {
    global $ui;
    $this->base = $base;
    if ($this->baseMode) {
      $this->baseSelector->setBase($this->base);
      $ui->setCurrentBase($this->base);
    }
  }

  function tryAndSetBase ($base)
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
  function getBase (): string
  {
    return $this->base;
  }

  function renderBase (): string
  {
    if (!$this->baseMode) {
      return '';
    }

    return $this->baseSelector->render();
  }

  function renderNavigation (bool $skipConfiguration = FALSE): array
  {
    global $ui;

    if ($this->baseMode) {
      $enableBack = TRUE;
      $enableRoot = TRUE;
      $enableHome = TRUE;

      /* Check if base = first available base */
      $deps = array_keys($this->bases);

      if (!count($deps) || $deps[0] == $this->base) {
        $enableBack = FALSE;
        $enableRoot = FALSE;
      }

      /* Check if we are in users home  department */
      if (!count($deps) || ($this->base == $ui->getBase()) || !in_array_ics($ui->getBase(), $deps)) {
        $enableHome = FALSE;
      }

      $actions = [
        [
          'id'      => 'ROOT',
          'desc'    => _('Go to root department'),
          'name'    => _('Root'),
          'icon'    => 'geticon.php?context=actions&icon=go-first&size=16',
          'enabled' => $enableRoot,
          'class'   => '',
        ],
        [
          'id'      => 'BACK',
          'desc'    => _('Go up one department'),
          'name'    => _('Up'),
          'icon'    => 'geticon.php?context=actions&icon=go-up&size=16',
          'enabled' => $enableBack,
          'class'   => '',
        ],
        [
          'id'      => 'HOME',
          'desc'    => _('Go to user\'s department'),
          'name'    => _('Home'),
          'icon'    => 'geticon.php?context=actions&icon=go-home&size=16',
          'enabled' => $enableHome,
          'class'   => '',
        ],
      ];
    } else {
      $actions = [];
    }

    $actions[] = [
      'id'      => 'REFRESH',
      'desc'    => _('Reload list'),
      'name'    => _('Reload'),
      'icon'    => 'geticon.php?context=actions&icon=view-refresh&size=16',
      'enabled' => TRUE,
      'class'   => 'optional',
    ];

    if (!$skipConfiguration) {
      $actions[] = [
        'id'      => 'listing_configure',
        'desc'    => _('Configure this management list'),
        'name'    => _('Configure'),
        'icon'    => 'geticon.php?context=categories&icon=settings&size=16',
        'enabled' => TRUE,
        'class'   => '',
      ];
    }

    return $actions;
  }

  /*!
   * \brief Get action
   */
  function getAction (): array
  {
    global $config;

    $result = ['targets' => [], 'action' => '', 'subaction' => NULL];

    // Do not do anything if this is not our PID, or there's even no PID available...
    if (!isset($_REQUEST['dn']) && (!isset($_REQUEST['PID']) || $_REQUEST['PID'] != $this->pid)) {
      return $result;
    }

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
        /* Pre-render list to init things if a dn is gonna be opened on first load */
        $dn         = urldecode($_REQUEST['dn']);
        $action     = $m[1];
        $this->focusDn($dn);
        $this->render();

        $result['action']     = $action;
        $result['targets'][]  = $dn;
        // Make sure no other management class intercept the same dn
        unset($_REQUEST['dn']);
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
          $result['targets']  = [$this->entriesIndex[$target]];
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

    if (preg_match('/^([a-zA-Z\.]+)_([a-zA-Z_\.]+)$/', $result['action'], $m)) {
      $result['action']     = $m[1];
      $result['subaction']  = $m[2];
    }
    return $result;
  }

  /*!
   * \brief Set base close to this dn and load only him
   */
  function focusDn (string $dn)
  {
    /* Detect the longer base valid for this dn */
    $longerBase = '';
    foreach (array_keys($this->bases) as $base) {
      if (preg_match('/'.preg_quote($base, '/').'$/i', $dn)
        && (strlen($base) > strlen($longerBase))) {
        $longerBase = $base;
      }
    }
    $this->setBase($longerBase);
    $this->update($dn);
  }

  /*!
   * \brief Refresh the bases list
   */
  function refreshBasesList ()
  {
    global $config;
    $ui = get_userinfo();

    // Fill internal bases list
    $this->bases = [];

    $categories = [];
    foreach ($this->parent->objectTypes as $otype) {
      $i = objects::infos($otype);
      $categories[$i['aclCategory']] = $i['aclCategory'];
    }

    $deps = $ui->get_module_departments(array_values($categories));
    $departmentTree = $config->getDepartmentTree();
    foreach ($departmentTree as $key => $dep) {
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

  /*!
   * \brief Get entry
   *
   * \param string $dn The DN
   */
  function getEntry (string $dn)
  {
    if (isset($this->entries[$dn])) {
      return $this->entries[$dn];
    }
    return NULL;
  }

  function fillSearchedAttributes (string $type, array &$attrs)
  {
    global $ui;

    $searchedAttributes = [];
    foreach ($this->columns as $column) {
      $column->fillSearchedAttributes($searchedAttributes);
    }

    $searchedAttributes = array_unique($searchedAttributes);

    foreach ($searchedAttributes as $attr) {
      if (!isset($attrs[$attr])) {
        $category = $ui->getAttributeCategory($type, $attr);
        if ($category !== FALSE) {
          $attrs[$attr] = $category;
        }
      }
    }
  }
}
