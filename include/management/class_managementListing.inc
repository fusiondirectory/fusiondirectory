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
  //~ protected $header   = array();
  //~ protected $colprops = array();
  //~ protected $filters  = array();
  //~ protected $filter   = NULL;
  public $pid;
  //~ protected $departmentTypes  = array();
  //~ var $objectTypes      = array();
  protected $objectTypeCount  = array();
  //~ protected $objectDnMapping  = array();
  //~ protected $copyPasteHandler = NULL;
  //~ protected $snapshotHandler  = NULL;
  //~ var $exporter       = array();
  //~ var $exportColumns  = array();
  protected $height         = 0;
  protected $baseSelector;
  //~ protected $filterCache = array();

  /* The management class */
  public $parent;

  protected $columns;

  protected $showFooter;

  /*!
   * \brief Create a listing
   *
   * \param string $data either a filename or an array representation of the XML
   */
  function __construct($parent)
  {
    global $config;

    $this->parent = $parent;

    // Initialize pid
    $this->pid = preg_replace('/[^0-9]/', '', microtime(TRUE));

    // Set base for filter
    if ($this->baseMode) {
      $this->base = session::global_get('CurrentMainBase'); // Replace with config or ui var
      if ($this->base == NULL) {
        $this->base = $config->current['BASE'];
      }
      $this->refreshBasesList();
    } else {
      $this->base = $config->current['BASE'];
    }

    // Move footer information
    $this->showFooter = ($config->get_cfg_value('listSummary') == 'TRUE');

    // Register build in filters
    //~ $this->registerElementFilter("departmentType",  "listing::filterDepartmentType");
    //~ $this->registerElementFilter("departmentLink",  "listing::filterDepartmentLink");
    //~ $this->registerElementFilter("link",            "listing::filterLink");
    //~ $this->registerElementFilter("nameLink",        "listing::filterNameLink");

    // Load exporters
    //~ foreach (array_keys($class_mapping) as $class) {
      //~ if (preg_match('/Exporter$/', $class)) {
        //~ $info = call_user_func(array($class, "getInfo"));
        //~ if ($info != NULL) {
          //~ $this->exporter = array_merge($this->exporter, $info);
        //~ }
      //~ }
    //~ }

    // Instanciate base selector
    $this->baseSelector = new baseSelector($this->bases, $this->base);

    $this->columns = array(
      new ObjectTypeColumn(),
      new LinkColumn('sn',        'Last name'),
      new LinkColumn('givenName', 'First name'),
      new Column('uid',           'Login'),
      new PropertiesColumn(NULL,  'Properties'),
      new ActionsColumn(NULL,     'Actions'),
    );

    foreach ($this->columns as $column) {
      $column->setParent($this);
    }

    $this->columns[1]->setTemplateAttribute('cn');
  }

  /*!
   * \brief Set a CopyPasteHandler
   *
   * \param $handler The handler
   *
   * \see CopyPasteHandler
   */
  function setCopyPasteHandler($handler)
  {
    $this->copyPasteHandler = $handler;
  }

  /*!
   * \brief Set the height
   *
   * \param integer $height
   */
  function setHeight($height)
  {
    $this->height = $height;
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
    $this->plainHeader = array();

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

      $sorter = '';
      if (($index == $this->sortColumn) && $column->isSortable()) {
        $sorter = '&nbsp;<img title="'.($this->sortDirection[$index] ? _('Up') : _('Down')).'" src="geticon.php?context=actions&amp;size=16&amp;icon=view-sort-'.($this->sortDirection[$index] ? 'descending' : 'ascending').'" alt="'.($this->sortDirection[$index] ? _('Sort up') : _('Sort down')).'"/>';
      }

      $link = 'href="?plug='.$_GET['plug'].'&amp;PID='.$this->pid.'&amp;act=SORT_'.$index.'"';
      if ($column->isSortable()) {
        $this->header[$index] = '<th '.$column->getHtmlProps().'><a '.$link.'>'.$column->getLabel().$sorter.'</a></th>';
      } else {
        $this->header[$index] = '<th '.$column->getHtmlProps().'>'.$column->getLabel().'</th>';
      }
      $this->plainHeader[]  = $column->getLabel();
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

    // Initialize list
    $result = '<input type="hidden" value="'.$this->pid.'" name="PID"/>'."\n";
    $height = 450;
    if ($this->height != 0) {
      $result .= '<input type="hidden" value="'.$this->height.'" id="d_height"/>'."\n";
      $height = $this->height;
    }

    $result .= '<div style="width:100%;">'."\n";
    $result .= '<table class="listingTable management">'."\n";

    // Build list header
    $result .= '<thead><tr>'."\n";
    if ($this->multiSelect) {
      $result .= '<th class="checkbox"><input type="checkbox" id="select_all" name="select_all" title="'._('Select all').'" onClick=\'toggle_all_("listing_selected_[0-9]*$","select_all");\' /></th>'."\n";
    }
    foreach ($this->header as $header) {
      $result .= $header;
    }
    $result .= '</tr></thead>'."\n";

    // Build list body
    $result .= '<tbody>'."\n";

    // No results? Just take an empty colspanned row
    if (count($this->entries) == 0) {
      $result .= '<tr><td colspan="'.(count($this->columns) + ($this->multiSelect ? 1 : 0)).'" class="filler">&nbsp;</td></tr>'."\n";
    }

    // Fill with contents, sort as configured
    $row = 0;
    foreach ($this->entries as $dn => &$entry) {
      $trow       = '';
      $rowclasses = array();

      // Render multi select if needed
      if ($this->multiSelect) {
        $trow .= '<td class="checkbox"><input type="checkbox" id="listing_selected_'.$row.'" name="listing_selected_'.$row.'"/></td>'."\n";
      }

      foreach ($this->columns as $index => $column) {
        $renderedCell = $column->renderCell($entry);
        $trow .= '<td '.$column->getHtmlCellProps().'>'.$renderedCell.'</td>'."\n";
        $rowclasses = array_merge($rowclasses, $column->getRowClasses($entry));
      }

      // Save rendered entry
      $entry['render'] = '<tr class="'.implode(' ', $rowclasses).'">'.$trow."</tr>\n";
      $row++;
    }
    unset($entry);

    // Complete list by sorting entries for _sort$index and appending them to the output
    $entryIterator = new entrySortIterator($this->entries, $this->columns[$this->sortColumn], $this->sortDirection[$this->sortColumn]);
    foreach ($entryIterator as $row => $entry) {
      $result .= $entry['render'];
    }

    // Close list body
    $result .= "</tbody></table></div>";

    // Add the footer if requested
    if ($this->showFooter) {
      $result .= '<div class="nlistFooter">';

      foreach ($this->parent->objectTypes as $type) {
        if (isset($this->objectTypeCount[$type])) {
          $infos = objects::infos($type);
          $result .= '<img src="'.htmlentities($infos['icon'], ENT_COMPAT, 'UTF-8').'" title="'.$infos['name'].'" alt="'.$infos['name'].'"/>&nbsp;'.$this->objectTypeCount[$type]."&nbsp;&nbsp;&nbsp;&nbsp;";
        }
      }

      $result .= '</div>';
    }

    return $result;
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
    if ($refresh || !(isset($_REQUEST['PID']) && $_REQUEST['PID'] != $this->pid)) {

      // Filter POST with "act" attributes -> posted from action menu
      if (isset($_POST['exec_act']) && $_POST['act'] != '') {
        if (preg_match('/^export.*$/', $_POST['act']) && isset($this->exporter[$_POST['act']])) {
          $exporter = $this->exporter[$_POST['act']];
          $userinfo = ", "._("created by")." ".$ui->cn." - ".strftime('%A, %d. %B %Y, %H:%M:%S');
          $entryIterator = new listingSortIterator($this->entries, $this->sortDirection[$this->sortColumn], "_sort".$this->sortColumn, $this->sortType);
          $sortedEntries = array();
          foreach ($entryIterator as $entry) {
            $sortedEntries[] = $entry;
          }
          $instance = new $exporter['class']($this->headline.$userinfo, $this->plainHeader, $sortedEntries, $this->exportColumns);
          $type = call_user_func(array($exporter['class'], "getInfo"));
          $type = $type[$_POST['act']];
          send_binary_content($instance->query(), $type['filename'], $type = $type['mime']);
        }
      }

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
    //~ $this->filter->setBase($this->base);
    //~ $this->entries = $this->filter->query();
    $attrs = $this->parent->neededAttrs;
    foreach ($this->columns as $column) {
      $column->fillNeededAttributes($attrs);
    }
    $this->objectTypeCount  = array();
    $this->entries          = array();
    $row                    = 0;
    foreach ($this->parent->objectTypes as $type) {
      // TODO: How do we handle collisions?
      $infos      = objects::infos($type);
      $base       = (empty($infos['ou']) ? $this->base : $infos['ou'].$this->base);
      $attrsAsked = $attrs;
      if (!empty($infos['mainAttr']) && !isset($attrsAsked[$infos['mainAttr']])) {
        /* Ask for mainAttr */
        $attrsAsked[$infos['mainAttr']] = '1';
      }
      if (!empty($infos['nameAttr']) && !isset($attrsAsked[$infos['nameAttr']])) {
        /* Ask for nameAttr */
        $attrsAsked[$infos['nameAttr']] = '1';
      }

      $entries = objects::ls($type, $attrsAsked, $base, '', TRUE, 'one');

      $this->objectTypeCount[$type] = count($entries);
      foreach ($entries as $dn => $entry) {
        $this->entries[$dn] = new ListingEntry($this, $type, $dn, $entry, $row++);
      }

      if (!$this->parent::$skipTemplates) {
        $entries = objects::ls($type, $attrsAsked, 'ou=templates,'.$base, '', TRUE, 'one', TRUE);

        $this->objectTypeCount['template_'.$type] = count($entries);
        foreach ($entries as $dn => $entry) {
          $this->entries[$dn] = new ListingEntry($this, 'template_'.$type, $dn, $entry, $row++);
        }
      }
    }
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

    /* Draw root button */
    if ($enableRoot) {
      $result['ROOT'] = '<input type="image" src="geticon.php?context=actions&amp;icon=go-first&amp;size=16"'.
                       ' name="ROOT" title="'._('Go to root department').'" alt="'._('Root').'"/>';
    } else {
      $result['ROOT'] = '<img src="geticon.php?context=actions&amp;icon=go-first&amp;size=16&amp;disabled=1" alt="'._('Root').'"/>';
    }

    /* Draw back button */
    if ($enableBack) {
      $result['BACK'] = '<input type="image" src="geticon.php?context=actions&amp;icon=go-up&amp;size=16"'.
                       ' name="BACK" title="'._('Go up one department').'" alt="'._('Up').'"/>';
    } else {
      $result['BACK'] = '<img src="geticon.php?context=actions&amp;icon=go-up&amp;size=16&amp;disabled=1" alt="'._('Up').'"/>';
    }

    /* Draw home button */
    if ($enableHome) {
      $result['HOME'] = '<input type="image" src="geticon.php?context=actions&amp;icon=go-home&amp;size=16"'.
                        ' name="HOME" title="'._('Go to user\'s department').'" alt="'._('Home').'"/>';
    } else {
      $result['HOME'] = '<img src="geticon.php?context=actions&amp;icon=go-home&amp;size=16&amp;disabled=1" alt="'._('Home').'"/>';
    }

    /* Draw reload button, this button is enabled everytime */
    $result['RELOAD'] = '<input type="image" class="optional" src="geticon.php?context=actions&amp;icon=view-refresh&amp;size=16"'.
                       ' name="REFRESH" title="'._('Reload list').'" alt="'._('Submit').'"/>';

    return $result;
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
      if (!preg_match('/^export.*$/', $_POST['act'])) {
        $result['action'] = validate($_POST['act']);
      }
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


  function renderCopyPasteMenu($separator, $copy = TRUE, $cut = TRUE)
  {
    // We can only provide information if we've got a copypaste handler
    // instance
    if (!is_object($this->copyPasteHandler)) {
      return '';
    }

    // Presets
    $result = "";
    $read   = FALSE;
    $paste  = FALSE;
    $ui     = get_userinfo();

    // Switch flags to on if there's at least one category which allows read/paste
    foreach ($this->categories as $category) {
      $read   = $read || (strpos($ui->get_category_permissions($this->base, $category), 'r') !== FALSE);
      $paste  = $paste || ($ui->is_pasteable($this->base, $category) == 1);
    }

    // Draw entries that allow copy and cut
    if ($read) {
      // Copy entry
      if ($copy) {
        $result .= $this->renderActionMenuActionLink($separator, 'copy', _('Copy'), 'geticon.php?context=actions&icon=edit-copy&size=16');
        $separator = '';
      }

      // Cut entry
      if ($cut) {
        $result .= $this->renderActionMenuActionLink($separator, 'cut', _('Cut'), 'geticon.php?context=actions&icon=edit-cut&size=16');
        $separator = '';
      }
    }

    // Draw entries that allow pasting entries
    if ($paste) {
      if ($this->copyPasteHandler->entries_queued()) {
        $result .= $this->renderActionMenuActionLink($separator, 'paste', _('Paste'), 'geticon.php?context=actions&icon=edit-paste&size=16');
      } else {
        $result .= "<li$separator>".'<a href="#"><img src="geticon.php?context=actions&amp;icon=edit-paste&amp;size=16&amp;disabled=1" alt="paste" class="center">&nbsp;'._('Paste').'</a></li>';
      }
    }

    return $result;
  }


  function renderCopyPasteActions($row, $dn, $category, $class, $copy = TRUE, $cut = TRUE)
  {
    // We can only provide information if we've got a copypaste handler
    // instance
    if (!is_object($this->copyPasteHandler)) {
      return '';
    }

    // Presets
    $ui = get_userinfo();
    $result = "";

    // Render cut entries
    if ($cut) {
      if ($ui->is_cutable($dn, $category, $class)) {
        $result .= '<input class="center" type="image"'.
                    ' src="geticon.php?context=actions&amp;icon=edit-cut&amp;size=16"'.
                    ' alt="'._('Cut').'" name="listing_cut_'.$row.'" title="'._('Cut this entry').'"'.
                    '/>';
      } else {
        $result .= '<img src="images/empty.png" alt=" " class="center optional"/>';
      }
    }

    // Render copy entries
    if ($copy) {
      if ($ui->is_copyable($dn, $category)) {
        $result .= '<input class="center" type="image"'.
                    ' src="geticon.php?context=actions&amp;icon=edit-copy&amp;size=16"'.
                    ' alt="'._('Copy').'" name="listing_copy_'.$row.'" title="'._('Copy this entry').'"'.
                    '/>';
      } else {
        $result .= '<img src="images/empty.png" alt=" " class="center optional"/>';
      }
    }

    return $result;
  }

  function renderExporterMenu($separator)
  {
    // Presets
    $result = "";

    // Draw entries
    $result .= "<li$separator id='actionmenu_exportList'><a href='#'><img class='center' src='geticon.php?context=actions&amp;icon=document-export&amp;size=16' alt='export'>&nbsp;"._("Export list")."&nbsp;<img src='images/forward-arrow.png' alt='arrow'></a><ul class='level3'>";

    // Export CVS as build in exporter
    foreach ($this->exporter as $action => $exporter) {
      $result .= $this->renderActionMenuActionLink('', $action, $exporter['label'], $exporter['image']);
    }

    // Finalize list
    $result .= "</ul></li>";

    return $result;
  }


  function renderSnapshotActions($row, $dn, $category)
  {
    /* We can only provide information if we've got a snapshot handler instance */
    if (!is_object($this->snapshotHandler)) {
      return '';
    }

    // Presets
    $result = '';
    $ui = get_userinfo();

    if ($ui->allow_snapshot_restore($dn, $category, FALSE)) {
      /* Draw restore button */

      if ($this->snapshotHandler->hasSnapshots($dn)) {
        /* We have snapshots for this dn */
        $result .= '<input class="center" type="image"'.
                    ' src="geticon.php?context=actions&amp;icon=document-restore&amp;size=16"'.
                    ' alt="'._('Restore snapshot').'" name="listing_restore_'.$row.'"'.
                    ' title="'._('Restore snapshot').'"/>';
      } else {
        $result .= '<img class="center"'.
                    ' src="geticon.php?context=actions&amp;icon=document-restore&amp;size=16&amp;disabled=1"'.
                    ' alt="restore"/>';
      }
    }

    if ($ui->allow_snapshot_create($dn, $category)) {
      /* Draw snapshot button */
      $result .= '<input class="center" type="image"'.
                  ' src="geticon.php?context=actions&amp;icon=snapshot&amp;size=16"'.
                  ' alt="'._('Create snapshot').'" name="listing_snapshot_'.$row.'"'.
                  ' title="'._('Create a new snapshot from this object').'"/>';
    } else {
      $result .= '<img src="images/empty.png" alt=" " class="center optional"/>';
    }

    return $result;
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
