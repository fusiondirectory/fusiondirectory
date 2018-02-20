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
  protected $entriesTypes = array();
  //~ var $departments            = array();
  //~ var $departmentBrowser      = FALSE;
  //~ var $departmentRootVisible  = FALSE;
  public $multiSelect            = TRUE;
  //~ var $template;
  //~ var $headline;
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
  protected $pid;
  //~ protected $departmentTypes  = array();
  //~ var $objectTypes      = array();
  protected $objectTypeCount  = array();
  //~ protected $objectDnMapping  = array();
  //~ protected $copyPasteHandler = NULL;
  //~ protected $snapshotHandler  = NULL;
  //~ var $exporter       = array();
  //~ var $exportColumns  = array();
  protected $height         = 0;
  protected $scrollPosition = 0;
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
    //~ $this->registerElementFilter("objectType",      "listing::filterObjectType");
    //~ $this->registerElementFilter("link",            "listing::filterLink");
    //~ $this->registerElementFilter("nameLink",        "listing::filterNameLink");
    //~ $this->registerElementFilter("actions",         "listing::filterActions");

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
      new StringColumn('uid',       'Login'),
      new PropertiesColumn(NULL,       'Properties'),
      new ActionsColumn(NULL,       'Actions'),
    );

    foreach ($this->columns as $column) {
      $column->setParent($this);
    }
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

  /*!
   * \brief Save element from a filter
   *
   * \param string $name
   *
   * \param string $call
   */
  function registerElementFilter($name, $call)
  {
    if (!isset($this->filters[$name])) {
      $this->filters[$name] = $call;
      return TRUE;
    }

    return FALSE;
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
        $sorter = '&nbsp;<img class="center" title="'.($this->sortDirection[$index] ? _('Up') : _('Down')).'" src="geticon.php?context=actions&amp;size=16&amp;icon=view-sort-'.($this->sortDirection[$index] ? 'descending' : 'ascending').'" alt="'.($this->sortDirection[$index] ? _('Sort up') : _('Sort down')).'"/>';
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
    $result .= '<input type="hidden" name="position_'.$this->pid.'" id="position_'.$this->pid.'"/>'."\n";
    $height = 450;
    if ($this->height != 0) {
      $result .= '<input type="hidden" value="'.$this->height.'" id="d_height"/>'."\n";
      $height = $this->height;
    }

    $result .= '<div id="d_scrollbody" style="width:100%;">'."\n";
    $result .= '<table style="width:100%;" id="t_scrolltable" class="listingTable">'."\n";

    // Build list header
    $result .= '<thead><tr>'."\n";
    if ($this->multiSelect) {
      $width = '24px';
      $result .= '<th style="text-align:center;padding:0;width:'.$width.';"><input type="checkbox" id="select_all" name="select_all" title="'._('Select all').'" onClick=\'toggle_all_("listing_selected_[0-9]*$","select_all");\' /></th>'."\n";
    }
    foreach ($this->header as $header) {
      $result .= $header;
    }
    $result .= '</tr></thead>'."\n";

    // Build list body
    $result .= '<tbody id="t_nscrollbody">'."\n";

    // No results? Just take an empty colspanned row
    if (count($this->entries) == 0) {
      $result .= '<tr><td colspan="'.(count($this->columns) + ($this->multiSelect ? 1 : 0)).'" style="height:100%;width:100%;">&nbsp;</td></tr>'."\n";
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
        $renderedCell = $column->renderCell($this->pid, $row, $dn, $this->entriesTypes[$dn], $entry);
        $trow .= '<td '.$column->getHtmlCellProps().'>'.$renderedCell.'</td>'."\n";
        $rowclasses = array_merge($rowclasses, $column->getRowClasses($this->pid, $row, $dn, $this->entriesTypes[$dn], $entry));
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
          $result .= '<img class="center" src="'.htmlentities($infos['icon'], ENT_COMPAT, 'UTF-8').'" title="'.$infos['name'].'" alt="'.$infos['name'].'"/>&nbsp;'.$this->objectTypeCount[$type]."&nbsp;&nbsp;&nbsp;&nbsp;";
        }
      }

      $result .= '</div>';
    }

    // Add scroll positioner
    $result .= '<script type="text/javascript">';
    $result .= '$("t_nscrollbody").scrollTop= '.$this->scrollPosition.';';
    $result .= 'var box = $("t_nscrollbody").onscroll= function() {$("position_'.$this->pid.'").value= this.scrollTop;}';
    $result .= '</script>';

    return $result;

    //~ $smarty = get_smarty();
    //~ $smarty->assign("usePrototype", "true");
    //~ $smarty->assign("FILTER", $this->filter->render());
    //~ $smarty->assign("SIZELIMIT", print_sizelimit_warning());
    //~ $smarty->assign("LIST", $result);
    //~ $smarty->assign("MULTISELECT", $this->multiSelect);

    //~ // Assign navigation elements
    //~ $nav = $this->renderNavigation();
    //~ foreach ($nav as $key => $html) {
      //~ $smarty->assign($key, $html);
    //~ }

    //~ // Assign action menu / base
    //~ $smarty->assign("ACTIONS", $this->renderActionMenu());
    //~ $smarty->assign("BASE", $this->renderBase());

    //~ // Assign summary
    //~ $smarty->assign("HEADLINE", $this->headline);

    //~ // Try to load template from plugin the folder first...
    //~ $file = get_template_path($this->xmlData['definition']['template'], TRUE);

    //~ // ... if this fails, try to load the file from the theme folder.
    //~ if (!file_exists($file)) {
      //~ $file = get_template_path($this->xmlData['definition']['template']);
    //~ }

    //~ return $smarty->fetch($file);
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

      // Save position if set
      if (isset($_POST['position_'.$this->pid]) && is_numeric($_POST['position_'.$this->pid])) {
        $this->scrollPosition = $_POST['position_'.$this->pid];
      }

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
    $this->entriesTypes     = array();
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
      $this->entries                = array_merge($this->entries, $entries);
      $this->entriesTypes           = array_merge($this->entriesTypes, array_fill_keys(array_keys($this->entries), $type));
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

  /*!
   * \brief Parse a layout
   *
   * \param string $layout
   */
  function parseLayout($layout)
  {
    $result = array();
    $layout = preg_replace("/^\|/", "", $layout);
    $layout = preg_replace("/\|$/", "", $layout);
    $cols   = explode("|", $layout);

    foreach ($cols as $index => $cfg) {
      if ($cfg != "") {
        $res      = "";
        $classes  = "";
        $components = explode(';', $cfg);
        foreach ($components as $part) {
          switch ($part) {
            case 'r':
              $res .= 'text-align:right;';
              break;
            case 'l':
              $res .= 'text-align:left;';
              break;
            case 'c':
              $res .= 'text-align:center;';
              break;
            case 'o':
              $classes .= 'optional ';
              break;
            default:
              if (preg_match('/^[0-9]+(|px|%)(-d)?$/', $part)) {
                if (!preg_match('/-d$/', $part)) {
                  /* d suffix means dynamic, ie no fixed width */
                  $res .= "width:$part;";
                } else {
                  /* Remove the -d suffix */
                  $part = preg_replace('/-d$/', '', $part);
                }
                $res .= "min-width:$part;";
              }
          }
        }

        $result[$index] = " style='$res'";
        if ($classes != "") {
          $result[$index] .= " class='$classes'";
        }
      } else {
        $result[$index] = "";
      }
    }

    // Save number of columns for later use
    $this->numColumns = count($cols);

    return $result;
  }

  function renderBase()
  {
    if (!$this->baseMode) {
      return;
    }

    return $this->baseSelector->render();
  }


  function processElementFilter($type, $index, $data, $cfg, $row)
  {
    if (isset($this->filterCache[$type.$index])) {
      $filters = $this->filterCache[$type.$index];
    } else {
      preg_match_all('/%{filter:([^(]+)\((.*)\)}/', $data, $matches, PREG_SET_ORDER | PREG_OFFSET_CAPTURE);

      $filters = array();
      foreach ($matches as $match) {
        $cl     = '';
        $method = '';
        if (!preg_match('/^(.*)::(.*)$/', $match[1][0], $m)) {
          if (!isset($this->filters[$match[1][0]]) || !preg_match('/^(.*)::(.*)$/', $this->filters[$match[1][0]], $m)) {
            trigger_error('Unknown filter '.$match[1][0]);
            continue;
          }
        }
        $cl     = $m[1];
        $method = $m[2];

        // Prepare params for function call
        preg_match_all('/"[^"]+"|[^,]+/', $match[2][0], $parts);

        $filters[$match[0][0]] = array($cl, $method, $parts[0], $match[0][1]);
      }

      $this->filterCache[$type.$index] = $filters;
    }

    foreach ($filters as $filterstring => $filter) {
      list ($cl, $method, $parts, $offset) = $filter;
      $params = array();
      foreach ($parts as $param) {
        switch ($param) {
          case 'row':
            $params[] = $row;
            break;
          case 'pid':
            $params[] = $this->pid;
            break;
          case 'base':
            $params[] = $this->getBase();
            break;
          case 'entry':
            $params[] = $cfg;
            break;
          case 'objectType':
            $params[] = $this->getObjectType($cfg['dn'], $cfg);
            break;
          case 'dn':
            $params[] = $cfg['dn'];
            break;
          default:
            if (preg_match('/^"(.*)"$/', $param, $m)) {
              // Fixie with "" is passed directly
              $params[] = $m[1];
            } elseif (isset($cfg[$param])) {
              // LDAP variables get replaced by their objects
              $values = $cfg[$param];
              if (is_array($values)) {
                unset($values['count']);
              }
              $params[] = $values;
            } else {
              $params[] = '';
            }
            break;
        }
      }

      // Replace information
      if ($cl == 'listing') {
        // Non static call
        $data = substr_replace($data, call_user_func_array(array($this, $method), $params), $offset, strlen($filterstring));
      } else {
        // Static call
        $data = substr_replace($data, call_user_func_array(array($cl, $method), $params), $offset, strlen($filterstring));
      }
    }

    return $data;
  }

  /*!
   * \brief Get the object type
   *
   * \param string $dn dn of the object
   * \param string $attrs LDAP attributes of the object
   */
  function getObjectType($dn, $attrs)
  {
    return $this->genericGetType($dn, $attrs, $this->objectTypes);
  }

  /*!
   * \brief Get the department type
   *
   * \param string $dn dn of the object
   * \param string $attrs LDAP attributes of the object
   */
  function getDepartmentType($dn, $attrs)
  {
    return $this->genericGetType($dn, $attrs, $this->departmentTypes);
  }

  /*!
   * \brief Get the object or department type
   */
  protected function genericGetType($dn, $attrs, $types)
  {
    if (isset($this->objectDnMapping[$dn])) {
      return $this->objectDnMapping[$dn];
    }
    $classes = $attrs['objectClass'];
    // Walk thru types and see if there's something matching
    foreach ($types as $type => $objectType) {
      if (isset($objectType['filter'])) {
        if ($objectType['filter']($attrs)) {
          $this->objectDnMapping[$dn] = $type;
          return $this->objectDnMapping[$dn];
        } else {
          continue;
        }
      }
      $ocs = $objectType['objectClass'];
      if (!is_array($ocs)) {
        $ocs = array($ocs);
      }

      $found = TRUE;
      foreach ($ocs as $oc) {
        if (preg_match('/^!(.*)$/', $oc, $match)) {
          $oc = $match[1];
          if (in_array($oc, $classes)) {
            $found = FALSE;
          }
        } else {
          if (!in_array($oc, $classes)) {
            $found = FALSE;
          }
        }
      }

      if ($found) {
        $this->objectDnMapping[$dn] = $type;
        return $this->objectDnMapping[$dn];
      }
    }

    return NULL;
  }

  function getObjectTypeInfos($dn, $attrs)
  {
    $type = $this->getObjectType($dn, $attrs);
    if ($type === NULL) {
      return NULL;
    }
    return $this->objectTypes[$type];
  }

  /*!
   * \brief Icon of the object type
   *
   * \param string $row The row
   *
   * \param string $dn The DN
   */
  function filterObjectType($row, $dn)
  {
    return $this->filterGenericType($dn, $this->objectTypes, $this->getObjectType($dn, $this->entries[$row]));
  }

  /*!
   * \brief Generic method for department and objects once type is known
   */
  protected function filterGenericType($dn, $types, $type)
  {
    $result = "&nbsp;";

    if ($type) {
      $result = '<img class="center" title="'.$dn.'" src="'.htmlentities($types[$type]['image'], ENT_COMPAT, 'UTF-8').'" alt="'.$type.'"/>';
      //~ if (!isset($this->objectTypeCount[$types[$type]['label']])) {
        //~ $this->objectTypeCount[$types[$type]['label']] = 0;
      //~ }
      //~ $this->objectTypeCount[$types[$type]['label']]++;
    }

    return $result;
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
      $result["ROOT"] = "<input class='center' type='image' src='geticon.php?context=actions&amp;icon=go-first&amp;size=16' ".
                       "title='"._("Go to root department")."' name='ROOT' alt='"._("Root")."'>";
    } else {
      $result["ROOT"] = "<img src='geticon.php?context=actions&amp;icon=go-first&amp;size=16&amp;disabled=1' class='center' alt='"._("Root")."'>";
    }

    /* Draw back button */
    if ($enableBack) {
      $result["BACK"] = "<input class='center' type='image' src='geticon.php?context=actions&amp;icon=go-up&amp;size=16' ".
                       "title='"._("Go up one department")."' alt='"._("Up")."' name='BACK'>";
    } else {
      $result["BACK"] = "<img src='geticon.php?context=actions&amp;icon=go-up&amp;size=16&amp;disabled=1' class='center' alt='"._("Up")."'>";
    }

    /* Draw home button */
    if ($enableHome) {
      $result["HOME"] = '<input class="center" type="image" src="geticon.php?context=actions&amp;icon=go-home&amp;size=16"'.
                        ' title="'._("Go to user's department").'" alt="'._('Home').'" name="HOME"/>';
    } else {
      $result["HOME"] = "<img src='geticon.php?context=actions&amp;icon=go-home&amp;size=16&amp;disabled=1' class='center' alt='"._("Home")."'>";
    }

    /* Draw reload button, this button is enabled everytime */
    $result["RELOAD"] = "<input class='center optional' type='image' src='geticon.php?context=actions&amp;icon=view-refresh&amp;size=16' ".
                       "title='"._("Reload list")."' name='REFRESH' alt='"._("Submit")."'>";

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

    // Save position if set
    if (isset($_POST['position_'.$this->pid]) && is_numeric($_POST['position_'.$this->pid])) {
      $this->scrollPosition = $_POST['position_'.$this->pid];
    }

    $result = array('targets' => array(), 'action' => '');

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
   * \brief Check if user have action permission
   *
   * \param string $action
   *
   * \param string $dn The DN
   *
   * \param string $row
   *
   */
  function hasActionPermission(&$action, $dn, $row = NULL)
  {
    global $ui;

    if (isset($action['acl'])) {
      if (isset ($action['aclInfos'])) {
        $aclInfos = $action['aclInfos'];
      } else {
        /* First time we check permission for this action */
        if ($row !== NULL) {
          $otype = $this->getObjectTypeInfos($dn, $this->entries[$row]);
        } else {
          $otype = FALSE;
        }
        $acls = $action['acl'];
        if (!is_array($acls)) {
          $acls = array($acls);
        }
        $aclInfos = array();

        // Every ACL has to pass
        foreach ($acls as $acl) {
          $module   = $this->categories;
          $aclList  = array();

          // Replace %acl if available
          if ($otype) {
            $acl = str_replace('%acl', $otype['category'].'/'.$otype['class'], $acl);
          }

          // Split for category and plugins if needed
          if (preg_match('/^\[([rwcdm]+)\]$/', $acl, $match)) {
            // match for "[rw]" style entries
            $aclList = array($match[1]);
          } elseif (preg_match('/^([a-zA-Z0-9]+\/?[a-zA-Z0-9]+)\[([rwcdm]+)\]$/', $acl, $match)) {
            // match for "user[rw]" style entries
            // match for "user/user[rw]" style entries
            $module   = $match[1];
            $aclList  = array($match[2]);
          } elseif (preg_match('/^([a-zA-Z0-9]+\/[a-zA-Z0-9]+)\[([a-zA-Z0-9]+:[rwcdm]+(,[a-zA-Z0-9]+:[rwcdm]+)*)\]$/', $acl, $match)) {
            // match "user/user[userPassword:rw(,...)*]" style entries
            $module   = $match[1];
            $aclList  = explode(',', $match[2]);
          }

          $modules = $module;
          if (!is_array($modules)) {
            $modules = array($modules);
          }

          $aclInfos[] = array($aclList, $modules);
        }
        $action['aclInfos'] = $aclInfos;
      }

      foreach ($aclInfos as $aclInfo) {
        list ($aclList, $modules) = $aclInfo;
        // Walk thru prepared ACL by using $module
        foreach ($aclList as $sAcl) {
          $checkAcl = '';

          // Category or detailed permission?
          foreach ($modules as $module) {
            if (strpos($module, '/') !== FALSE) {
              if (preg_match('/([a-zA-Z0-9]+):([rwcdm]+)/', $sAcl, $m)) {
                $checkAcl .= $ui->get_permissions($dn, $module, $m[1]);
                $sAcl = $m[2];
              } else {
                $checkAcl .= $ui->get_permissions($dn, $module, '0');
              }
            } else {
              $checkAcl .= $ui->get_category_permissions($dn, $module);
            }
          }

          // Split up remaining part of the acl and check if it we're
          // allowed to do something...
          $parts = str_split($sAcl);
          foreach ($parts as $part) {
            if (strpos($checkAcl, $part) === FALSE) {
              return FALSE;
            }
          }
        }
      }
    }

    return TRUE;
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
    if (isset($this->entriesTypes[$dn])) {
      return $this->entriesTypes[$dn];
    }
    return NULL;
  }
}

?>
