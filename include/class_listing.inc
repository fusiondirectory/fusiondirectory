<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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
 * \file class_listing.inc
 * Source code for class listing
 */

/*!
 * \brief This class contains all the function needed to make the list
 * that show the objects inside FusionDirectory
 */
class listing {

  var $xmlData;
  var $entries;
  var $departments            = array();
  var $departmentBrowser      = FALSE;
  var $departmentRootVisible  = FALSE;
  var $multiSelect            = FALSE;
  var $template;
  var $headline;
  var $base;
  var $sortDirection  = NULL;
  var $sortColumn     = NULL;
  var $sortAttribute;
  var $sortType;
  var $numColumns;
  var $baseMode = FALSE;
  var $bases    = array();
  var $header   = array();
  var $colprops = array();
  var $filters  = array();
  var $filter   = NULL;
  var $pid;
  var $objectTypes      = array();
  var $objectTypeCount  = array();
  var $objectDnMapping  = array();
  var $copyPasteHandler = NULL;
  var $snapshotHandler  = NULL;
  var $exporter       = array();
  var $exportColumns  = array();
  var $useSpan        = FALSE;
  var $height         = 0;
  var $scrollPosition = 0;
  var $baseSelector;

  var $typeCache = array();

  /*!
   * \brief Create a listing
   *
   * \param string $data either a filename or an array representation of the XML
   */
  function __construct($data)
  {
    global $config;
    global $class_mapping;

    // Initialize pid
    $this->pid = preg_replace("/[^0-9]/", "", microtime(TRUE));

    if (!$this->load($data)) {
      if (is_array($data)) {
        die("Cannot parse data : ".print_r($data, TRUE));
      } else {
        die("Cannot parse $data!");
      }
    }

    // Set base for filter
    if ($this->baseMode) {
      $this->base = session::global_get("CurrentMainBase");
      if ($this->base == NULL) {
        $this->base = $config->current['BASE'];
      }
      $this->refreshBasesList();
    } else {
      $this->base = $config->current['BASE'];
    }

    // Move footer information
    $this->showFooter = ($config->get_cfg_value("listSummary") == "TRUE");

    // Register build in filters
    $this->registerElementFilter("objectType",      "listing::filterObjectType");
    $this->registerElementFilter("departmentLink",  "listing::filterDepartmentLink");
    $this->registerElementFilter("link",            "listing::filterLink");
    $this->registerElementFilter("nameLink",        "listing::filterNameLink");
    $this->registerElementFilter("actions",         "listing::filterActions");

    // Load exporters
    foreach (array_keys($class_mapping) as $class) {
      if (preg_match('/Exporter$/', $class)) {
        $info = call_user_func(array($class, "getInfo"));
        if ($info != NULL) {
          $this->exporter = array_merge($this->exporter, $info);
        }
      }
    }

    // Instanciate base selector
    $this->baseSelector = new baseSelector($this->bases, $this->base);
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
    $this->snapshotHandler = $handler;
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

  /*!
   * \brief Load a file
   *
   * \param string $data either a filename or an array representation of the XML
   */
  function load($data)
  {
    if (is_array($data)) {
      $this->xmlData = $data;
    } else {
      $contents = file_get_contents($data);
      $this->xmlData = xml::xml2array($contents, 1);
    }

    if (!isset($this->xmlData['list'])) {
      return FALSE;
    }

    $this->xmlData = $this->xmlData["list"];

    // Load some definition values
    foreach (array("departmentBrowser", "departmentRootVisible", "multiSelect", "baseMode") as $token) {
      if (isset($this->xmlData['definition'][$token]) &&
          $this->xmlData['definition'][$token] == "true") {
        $this->$token = TRUE;
      }
    }

    // Fill objectTypes from departments and xml definition
    $types = departmentManagement::getDepartmentTypes();
    foreach ($types as $type) {
      $i = objects::infos($type);
      $this->objectTypes[strtoupper($type)] = array(
        'label'       => $i['name'],
        'image'       => $i['icon'],
        'category'    => $i['aclCategory'],
        'class'       => $i['mainTab'],
        'filter'      => ldapFilter::parse($i['filter']),
        'nameAttr'    => $i['nameAttr'],
      );
    }
    $this->categories = array();
    if (isset($this->xmlData['definition']['objectType'])) {
      if (isset($this->xmlData['definition']['objectType']['label'])) {
        $this->xmlData['definition']['objectType'] = array($this->xmlData['definition']['objectType']);
      }
      foreach ($this->xmlData['definition']['objectType'] as $index => $otype) {
        $this->objectTypes[$otype['objectClass']] = $otype;
        if (isset($otype['category'])) {
          $this->categories[] = $otype['category'];
        }
      }
    }
    $this->categories = array_unique($this->categories);

    // Parse layout per column
    $this->colprops = $this->parseLayout($this->xmlData['table']['layout']);

    // Prepare table headers
    $this->renderHeader();

    // Assign headline/Categories
    $this->headline = _($this->xmlData['definition']['label']);
    if (!is_array($this->categories)) {
      $this->categories = array($this->categories);
    }

    // Evaluate columns to be exported
    if (isset($this->xmlData['table']['column'])) {
      foreach ($this->xmlData['table']['column'] as $index => $cfg) {
        if (isset($cfg['export']) && $cfg['export'] == "true") {
          $this->exportColumns[] = $index;
        }
      }
    }

    if (isset($this->xmlData['actiontriggers']['action']['type'])) {
      $this->xmlData['actiontriggers']['action'] = array($this->xmlData['actiontriggers']['action']);
    }

    return TRUE;
  }


  function renderHeader()
  {
    $this->header = array();
    $this->plainHeader = array();

    // Initialize sort?
    $sortInit = FALSE;
    if (!$this->sortDirection) {
      $this->sortColumn = 0;
      if (isset($this->xmlData['definition']['defaultSortColumn'])) {
        $this->sortColumn = $this->xmlData['definition']['defaultSortColumn'];
      } else {
        $this->sortAttribute = "";
      }
      $this->sortDirection = array();
      $sortInit = TRUE;
    }

    if (isset($this->xmlData['table']['column'])) {
      foreach ($this->xmlData['table']['column'] as $index => $cfg) {
        // Initialize everything to one direction
        if ($sortInit) {
          $this->sortDirection[$index] = FALSE;
        }

        $sorter = "";
        if ($index == $this->sortColumn && isset($cfg['sortAttribute']) &&
            isset($cfg['sortType'])) {
          $this->sortAttribute  = $cfg['sortAttribute'];
          $this->sortType       = $cfg['sortType'];
          $sorter = "&nbsp;<img class='center' title='".($this->sortDirection[$index]?_("Up"):_("Down"))."' src='geticon.php?context=actions&amp;size=16&amp;icon=view-sort-".($this->sortDirection[$index]?"descending":"ascending")."' alt='".($this->sortDirection[$index]?_('Sort up'):_('Sort down'))."'>";
        }
        $sortable = (isset($cfg['sortAttribute']));

        $link = "href='?plug=".$_GET['plug']."&amp;PID=".$this->pid."&amp;act=SORT_$index'";
        if (isset($cfg['label'])) {
          if ($sortable) {
            $this->header[$index] = "<th ".$this->colprops[$index]."><a $link>"._($cfg['label'])."$sorter</a></th>";
          } else {
            $this->header[$index] = "<th ".$this->colprops[$index].">"._($cfg['label'])."</th>";
          }
          $this->plainHeader[] = _($cfg['label']);
        } else {
          if ($sortable) {
            $this->header[$index] = "<th ".$this->colprops[$index]."><a $link>&nbsp;$sorter</a></th>";
          } else {
            $this->header[$index] = "<th ".$this->colprops[$index].">&nbsp;</th>";
          }
          $this->plainHeader[] = "";
        }
      }
    }
  }

  /*!
   * \brief Render
   */
  function render()
  {
    // Check for exeeded sizelimit
    if (($message = check_sizelimit()) != "") {
      return $message;
    }

    // Some browsers don't have the ability do do scrollable table bodies, filter them
    // here.
    $switch = FALSE;
    if (preg_match('/(Opera|Konqueror|Safari)/i', $_SERVER['HTTP_USER_AGENT'])) {
      $switch = TRUE;
    }

    // Initialize list
    $result = "<input type='hidden' value='$this->pid' name='PID'>\n";
    $result .= "<input type='hidden' name='position_".$this->pid."' id='position_".$this->pid."'>\n";
    $height = 450;
    if ($this->height != 0) {
      $result .= "<input type='hidden' value='$this->height' id='d_height'>\n";
      $height = $this->height;
    }

    $result .= "<div id='d_scrollbody' style='width:100%;'>\n";
    $result .= "<table style='width:100%;' id='t_scrolltable' class='listingTable'>\n";
    $this->numColumns = count($this->colprops) + ($this->multiSelect?1:0);

    // Build list header
    $result .= "<thead><tr>\n";
    if ($this->multiSelect) {
      $width = "24px";
      if (preg_match('/Konqueror/i', $_SERVER['HTTP_USER_AGENT'])) {
        $width = "28px";
      }
      $result .= "<th style='text-align:center;padding:0;width:$width;'><input type='checkbox' id='select_all' name='select_all' title='"._("Select all")."' onClick='toggle_all_(\"listing_selected_[0-9]*$\",\"select_all\");' ></th>\n";
    }
    foreach ($this->header as $header) {
      $result .= $header;
    }
    $result .= "</tr></thead>\n";

    // Build list body
    $result .= "<tbody id='t_nscrollbody'>\n";

    // No results? Just take an empty colspanned row
    if (count($this->entries) + count($this->departments) == 0) {
      $result .= "<tr><td class='list1nohighlight' colspan='$this->numColumns' style='height:100%;width:100%;'>&nbsp;</td></tr>";
    }

    // Line color alternation
    $alt  = 0;
    $deps = 0;

    // Draw department browser if configured and we're not in sub mode
    $this->useSpan = FALSE;
    if ($this->departmentBrowser && $this->filter->scope != "sub") {
      // Fill with department browser if configured this way
      $departmentIterator = new departmentSortIterator($this->departments, $this->sortDirection[$this->sortColumn]);
      foreach ($departmentIterator as $row => $entry) {
        $result .= "<tr>";

        // Render multi select if needed
        if ($this->multiSelect) {
          $result .= "<td style='text-align:center;padding:0;'>&nbsp;</td>";
        }

        // Render defined department columns, fill the rest with some stuff
        $rest = $this->numColumns - 1;
        foreach ($this->xmlData['table']['department'] as $index => $cfg) {
          $colspan = 1;
          if (isset($cfg['span'])) {
            $colspan        = $cfg['span'];
            $this->useSpan  = TRUE;
          }
          $result .= "<td colspan='$colspan' ".$this->colprops[$index].">".$this->renderCell($cfg['value'], $entry, $row)."</td>";
          $rest -= $colspan;
        }

        // Fill remaining cols with nothing
        $last = $this->numColumns - $rest;
        for ($i = 0; $i < $rest; $i++) {
          $result .= "<td ".$this->colprops[$last + $i - 1].">&nbsp;</td>";
        }
        $result .= "</tr>";

        $alt++;
      }
      $deps = $alt;
    }

    // Fill with contents, sort as configured
    foreach ($this->entries as $row => $entry) {
      $trow = "";

      // Render multi select if needed
      if ($this->multiSelect) {
        $trow .= "<td style='text-align:center;width:20px;'><input type='checkbox' id='listing_selected_$row' name='listing_selected_$row'></td>\n";
      }

      foreach ($this->xmlData['table']['column'] as $index => $cfg) {
        $renderedCell = $this->renderCell($cfg['value'], $entry, $row);
        $trow .= "<td ".$this->colprops[$index].">".$renderedCell."</td>\n";

        // Save rendered column
        $sort = preg_replace('/.*>([^<]+)<.*$/', '$1', $renderedCell);
        $sort = preg_replace('/&nbsp;/', '', $sort);
        if (preg_match('/</', $sort)) {
          $sort = "";
        }
        $this->entries[$row]["_sort$index"] = $sort;
      }

      // Save rendered entry
      $this->entries[$row]['_rendered'] = $trow;
    }

    // Complete list by sorting entries for _sort$index and appending them to the output
    $entryIterator = new listingSortIterator($this->entries, $this->sortDirection[$this->sortColumn], "_sort".$this->sortColumn, $this->sortType);
    foreach ($entryIterator as $row => $entry) {
      // Apply custom class to row?
      if (preg_match("/<rowClass:([a-z0-9_-]*)\/>/i", $entry['_rendered'], $matches)) {
          $result .= "<tr class='".$matches[1]."'>\n";
          $result .= preg_replace("/<rowClass[^>]+>/", '', $entry['_rendered']);
      } else {
          $result .= "<tr>\n";
          $result .= $entry['_rendered'];
      }

      $result .= "</tr>\n";
      $alt++;
    }

    // Close list body
    $result .= "</tbody></table></div>";

    // Add the footer if requested
    if ($this->showFooter) {
      $result .= "<div class='nlistFooter'><div style='padding:3px'>";

      foreach ($this->objectTypes as $objectType) {
        if (isset($this->objectTypeCount[$objectType['label']])) {
          $label = _($objectType['label']);
          $result .= "<img class='center' src='".htmlentities($objectType['image'], ENT_COMPAT, 'UTF-8')."' title='$label' alt='$label'>&nbsp;".$this->objectTypeCount[$objectType['label']]."&nbsp;&nbsp;&nbsp;&nbsp;";
        }
      }

      $result .= "</div></div>";
    }

    // Close list
    $result .= $switch?"<input type='hidden' id='list_workaround'>":"";

    // Add scroll positioner
    $result .= '<script type="text/javascript">';
    $result .= '$("t_nscrollbody").scrollTop= '.$this->scrollPosition.';';
    $result .= 'var box = $("t_nscrollbody").onscroll= function() {$("position_'.$this->pid.'").value= this.scrollTop;}';
    $result .= '</script>';

    $smarty = get_smarty();
    $smarty->assign("usePrototype", "true");
    $smarty->assign("FILTER", $this->filter->render());
    $smarty->assign("SIZELIMIT", print_sizelimit_warning());
    $smarty->assign("LIST", $result);
    $smarty->assign("MULTISELECT", $this->multiSelect);

    // Assign navigation elements
    $nav = $this->renderNavigation();
    foreach ($nav as $key => $html) {
      $smarty->assign($key, $html);
    }

    // Assign action menu / base
    $smarty->assign("ACTIONS", $this->renderActionMenu());
    $smarty->assign("BASE", $this->renderBase());

    // Assign separator
    $smarty->assign("SEPARATOR", "<img src='images/lists/seperator.png' alt='-' height='16' width='1' class='center'>");

    // Assign summary
    $smarty->assign("HEADLINE", $this->headline);

    // Try to load template from plugin the folder first...
    $file = get_template_path($this->xmlData['definition']['template'], TRUE);

    // ... if this fails, try to load the file from the theme folder.
    if (!file_exists($file)) {
      $file = get_template_path($this->xmlData['definition']['template']);
    }

    return $smarty->fetch($file);
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

    // Reset object counter / DN mapping
    $this->objectTypeCount = array();
    $this->objectDnMapping = array();

    // Do not do anything if this is not our PID
    if ($refresh || !(isset($_REQUEST['PID']) && $_REQUEST['PID'] != $this->pid)) {

      // Save position if set
      if (isset($_POST['position_'.$this->pid]) && is_numeric($_POST['position_'.$this->pid])) {
        $this->scrollPosition = $_POST['position_'.$this->pid];
      }

      // Override the base if we got a message from the browser navigation
      if ($this->departmentBrowser && isset($_GET['act'])) {
        if (preg_match('/^department_([0-9]+)$/', validate($_GET['act']), $match)) {
          if (isset($this->departments[$match[1]])) {
            $this->base = $this->departments[$match[1]]['dn'];
            if ($this->baseMode) {
              $this->baseSelector->setBase($this->base);
            }
            session::global_set("CurrentMainBase", $this->base);
          }
        }
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

      // Override base if we got signals from the navigation elements
      $action = "";
      foreach (array_keys($_POST) as $key) {
        if (preg_match('/^(ROOT|BACK|HOME)_x$/', $key, $match)) {
          $action = $match[1];
          break;
        }
      }

      // Navigation handling
      if ($action == 'ROOT') {
        $deps = $ui->get_module_departments($this->categories);
        $this->setBase($deps[0]);
        session::global_set("CurrentMainBase", $this->base);
      }
      if ($action == 'BACK') {
        $base = preg_replace("/^[^,]+,/", "", $this->base);
        $this->tryAndSetBase($base);
      }
      if ($action == 'HOME') {
        $this->tryAndSetBase(get_base_from_people($ui->dn));
      }
    }

    // Reload departments
    if ($this->departmentBrowser) {
      $this->departments = $this->getDepartments();
    }

    // Update filter and refresh entries
    $this->filter->setBase($this->base);
    $this->entries = $this->filter->query();

    // Fix filter if querie returns NULL
    if ($this->entries == NULL) {
      $this->entries = array();
    }

    $this->dnToRow = array();
    foreach ($this->entries as $row => $entry) {
      $this->dnToRow[$entry['dn']] = $row;
    }

    // Init snapshot list for renderSnapshotActions
    if (isset($this->snapshotHandler) && is_object($this->snapshotHandler) && $this->snapshotHandler->enabled()) {
      $this->snapshotHandler->initSnapshotCache($this->base);
    }
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
    }
  }

  function tryAndSetBase($base)
  {
    $ui   = get_userinfo();
    $deps = $ui->get_module_departments($this->categories);
    if (in_array_ics($base, $deps)) {
      $this->setBase($base);
      session::global_set("CurrentMainBase", $this->base);
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
          if (preg_match("/^r$/", $part)) {
            $res .= "text-align:right;";
            continue;
          }
          if (preg_match("/^l$/", $part)) {
            $res .= "text-align:left;";
            continue;
          }
          if (preg_match("/^c$/", $part)) {
            $res .= "text-align:center;";
            continue;
          }
          if (preg_match("/^o$/", $part)) {
            $classes .= "optional ";
            continue;
          }
          if (preg_match("/^[0-9]+(|px|%)(-d)?$/", $part)) {
            if (!preg_match('/-d$/', $part)) {
              /* d suffix means dynamic, ie no fixed width */
              $res .= "width:$part;";
            } else {
              /* Remove the -d suffix */
              $part = preg_replace('/-d$/', '', $part);
            }
            $res .= "min-width:$part;";
            continue;
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


  function renderCell($data, $cfg, $row)
  {
    // Replace flat attributes in data string
    for ($i = 0; $i < $cfg['count']; $i++) {
      $attr   = $cfg[$i];
      $value  = "";
      if (is_array($cfg[$attr])) {
        $value = $cfg[$attr][0];
      } else {
        $value = $cfg[$attr];
      }
      $data = preg_replace("/%\{$attr\}/", htmlentities($value, ENT_COMPAT, 'UTF-8'), $data);
    }

    // Watch out for filters and prepare to execute them
    $data = $this->processElementFilter($data, $cfg, $row);

    // Replace all non replaced %{...} instances because they
    // are non resolved attributes or filters
    $data = preg_replace('/%{[^}]+}/', '&nbsp;', $data);

    return $data;
  }


  function renderBase()
  {
    if (!$this->baseMode) {
      return;
    }

    return $this->baseSelector->render();
  }


  function processElementFilter($data, $cfg, $row)
  {
    preg_match_all("/%\{filter:([^(]+)\((.*)\)\}/", $data, $matches, PREG_SET_ORDER);

    foreach ($matches as $match) {
      $cl     = "";
      $method = "";
      if (preg_match('/::/', $match[1])) {
        $cl     = preg_replace('/::.*$/', '', $match[1]);
        $method = preg_replace('/^.*::/', '', $match[1]);
      } else {
        if (!isset($this->filters[$match[1]])) {
          continue;
        }
        $cl     = preg_replace('/::.*$/', '', $this->filters[$match[1]]);
        $method = preg_replace('/^.*::/', '', $this->filters[$match[1]]);
      }

      // Prepare params for function call
      $params = array();
      preg_match_all('/"[^"]+"|[^,]+/', $match[2], $parts);
      foreach ($parts[0] as $param) {

        // Row is replaced by the row number
        if ($param == "row") {
          $params[] = $row;
          continue;
        }

        // pid is replaced by the current PID
        if ($param == "pid") {
          $params[] = $this->pid;
          continue;
        }

        // base is replaced by the current base
        if ($param == "base") {
          $params[] = $this->getBase();
          continue;
        }

        // Fixie with "" is passed directly
        if (preg_match('/^".*"$/', $param)) {
          $params[] = preg_replace('/"/', '', $param);
          continue;
        }

        // Move dn if needed
        if ($param == "dn") {
          $params[] = LDAP::fix($cfg["dn"]);
          continue;
        }

        // LDAP variables get replaced by their objects
        for ($i = 0; $i < $cfg['count']; $i++) {
          if ($param == $cfg[$i]) {
            $values = $cfg[$cfg[$i]];
            if (is_array($values)) {
              unset($values['count']);
            }
            $params[] = $values;
            break;
          }
        }
      }

      // Replace information
      if ($cl == "listing") {
        // Non static call - seems to result in errors
        $data = @preg_replace('/'.preg_quote($match[0]).'/', call_user_func_array(array($this, "$method"), $params), $data);
      } else {
        // Static call
        $data = preg_replace('/'.preg_quote($match[0]).'/', call_user_func_array(array($cl, $method), $params), $data);
      }
    }

    return $data;
  }

  /*!
   * \brief Get the object type
   *
   * \param string $classes
   */
  function getObjectType($dn, $attrs)
  {
    if (isset($this->typeCache[$dn])) {
      return $this->typeCache[$dn];
    }
    $classes = $attrs['objectClass'];
    // Walk thru types and see if there's something matching
    foreach ($this->objectTypes as $type => $objectType) {
      if (isset($objectType['filter'])) {
        if ($objectType['filter']($attrs)) {
          $this->typeCache[$dn] = $type;
          return $this->typeCache[$dn];
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
        return $type;
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


  /*
   * \brief If you want to have the summary without displaying informations
   * it returns nothing to do an empty column, but calls filterObjectType to do statistics.
   *
   * \param string $dn The DN
   *
   * \param string $classes
   *
   * \return string an empty string
   */
  function filterObjectTypes($dn, $classes)
  {
    $result = "";
    foreach ($classes as $class) {
        $result .= $this->filterObjectType($dn, array($class))."&nbsp;";
    }
    return "";
  }

  /*!
   * \brief Icon of the object type
   *
   * \param string $dn The DN
   *
   * \param string $classes
   */
  function filterObjectType($dn, $classes = array())
  {
    // Walk thru classes and return on first match
    $result = "&nbsp;";

    if (($attrs = $this->getEntry($dn)) != NULL) {
      $type = $this->getObjectType($dn, $attrs);
    } else {
      $type = $this->getObjectType($dn, array('objectClass' => $classes));
    }
    if ($type) {
      $this->objectDnMapping[$dn] = $type;
      $result = '<img class="center" title="'.LDAP::fix($dn).'" src="'.htmlentities($this->objectTypes[$type]['image'], ENT_COMPAT, 'UTF-8').'" alt="'.$type.'"/>';
      if (!isset($this->objectTypeCount[$this->objectTypes[$type]['label']])) {
        $this->objectTypeCount[$this->objectTypes[$type]['label']] = 0;
      }
      $this->objectTypeCount[$this->objectTypes[$type]['label']]++;
    }

    return $result;
  }

  /*!
   * \brief Filter actions
   *
   * \param string $dn The DN
   *
   * \param string $row
   *
   * \param string $classes
   */
  function filterActions($dn, $row, $classes)
  {
    // Do nothing if there's no menu defined
    if (!isset($this->xmlData['actiontriggers']['action'])) {
      return "&nbsp;";
    }

    // Go thru all actions
    $result   = "";
    $actions  = $this->xmlData['actiontriggers']['action'];
    foreach ($actions as $action) {
      // Skip the entry completely if there's no permission to execute it
      if (!$this->hasActionPermission($action, $dn, $classes)) {
        $result .= "<img src='images/empty.png' alt=' ' class='center optional' style='padding:1px'>";
        continue;
      }

      // Skip entry if the pseudo filter does not fit
      if (isset($action['filter']) && preg_match('/^[a-z0-9_]+!?=[a-z0-9_]+$/i', $action['filter'])) {
        list($fa, $fv) = explode('=', $action['filter']);
        if (preg_match('/^(.*)!$/', $fa, $m)) {
          $fa = $m[1];
          if (isset($this->entries[$row][$fa]) && $this->entries[$row][$fa][0] == $fv) {
            $result .= "<img src='images/empty.png' alt=' ' class='center optional' style='padding:1px'>";
            continue;
          }
        } else {
          if (!isset($this->entries[$row][$fa]) && !$this->entries[$row][$fa][0] == $fv) {
            $result .= "<img src='images/empty.png' alt=' ' class='center optional' style='padding:1px'>";
            continue;
          }
        }
      }

      // If there's an objectclass definition and we don't have it
      // add an empty picture here.
      if (isset($action['objectclass'])) {
        $objectclass = $action['objectclass'];
        if (preg_match('/^!(.*)$/', $objectclass, $m)) {
          $objectclass = $m[1];
          if (in_array($objectclass, $classes)) {
            $result .= "<img src='images/empty.png' alt=' ' class='center optional' style='padding:1px'>";
            continue;
          }
        } elseif (is_string($objectclass)) {
          if (!in_array($objectclass, $classes)) {
            $result .= "<img src='images/empty.png' alt=' ' class='center optional' style='padding:1px'>";
            continue;
          }
        } elseif (is_array($objectclass)) {
          if (count(array_intersect($objectclass, $classes)) != count($objectclass)) {
            $result .= "<img src='images/empty.png' alt=' ' class='center optional' style='padding:1px'>";
            continue;
          }
        }
      }

      // Render normal entries as usual
      if ($action['type'] == "entry") {
        $label = $this->processElementFilter($action['label'], $this->entries[$row], $row);
        $image = $this->processElementFilter($action['image'], $this->entries[$row], $row);
        $result .= "<input class='center' type='image' src='".htmlentities($image, ENT_COMPAT, 'UTF-8')."' title='$label' alt='$label' ".
                 "name='listing_".$action['name']."_$row' style='padding:1px'>";
      }

      // Handle special types
      if ($action['type'] == "copypaste" || $action['type'] == "snapshot") {

        $objectType = $this->getObjectTypeInfos($dn, $this->entries[$row]);
        $category   = $class = NULL;
        if ($objectType) {
          $category = $objectType['category'];
          $class    = $objectType['class'];
        }

        if ($action['type'] == "copypaste") {
          $copy = !isset($action['copy']) || $action['copy'] == "true";
          $cut  = !isset($action['cut']) || $action['cut'] == "true";
          $result .= $this->renderCopyPasteActions($row, $this->entries[$row]['dn'], $category, $class, $copy, $cut);
        } else {
          $result .= $this->renderSnapshotActions($row, $this->entries[$row]['dn'], $category, $class);
        }
      }
    }

    return $result;
  }

  /*!
   * \brief Filter the department link
   *
   * \param string $row
   *
   * \param string $dn The DN
   *
   * \param array $description
   */
  function filterDepartmentLink($row, $dn, $description)
  {
    $attr = $this->departments[$row]['sort-attribute'];
    $name = $this->departments[$row][$attr];
    if (is_array($name)) {
      $name = $name[0];
    }
    $result = htmlentities(sprintf("%s [%s]", $name, $description[0]), ENT_COMPAT, 'UTF-8');
    return "<a href='?plug=".$_GET['plug']."&amp;PID=$this->pid&amp;act=department_$row' title='$dn'>$result</a>";
  }

  /*!
   * \brief Filter link with object name
   */
  function filterNameLink($row, $dn)
  {
    $infos  = $this->getObjectTypeInfos($dn, $this->entries[$row]);
    $attr   = $infos['nameAttr'];
    return $this->filterLink($row, $dn, "%s", $this->entries[$row][$attr]);
  }

  /*!
   * \brief Filter link
   */
  function filterLink()
  {
    $result = "&nbsp;";

    $row = func_get_arg(0);
    $pid = $this->pid;
    $dn = LDAP::fix(func_get_arg(1));
    $params = array(func_get_arg(2));

    // Collect sprintf params
    for ($i = 3;$i < func_num_args();$i++) {
      $val = func_get_arg($i);
      if (is_array($val)) {
        $val = $val[0];
      }
      $params[] = htmlentities($val, ENT_COMPAT, 'UTF-8');
    }

    $result = "&nbsp;";
    $trans  = call_user_func_array("sprintf", $params);
    if ($trans != "") {
      return "<a href='?plug=".$_GET['plug']."&amp;PID=$pid&amp;act=listing_edit_$row' title='$dn'>$trans</a>";
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
    $deps = $ui->get_module_departments($this->categories);

    if (!count($deps) || $deps[0] == $this->filter->base) {
      $enableBack = FALSE;
      $enableRoot = FALSE;
    }

    /* Check if we are in users home  department */
    if (!count($deps) || $this->filter->base == get_base_from_people($ui->dn) || !in_array_ics(get_base_from_people($ui->dn), $deps)) {
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
    // Do not do anything if this is not our PID, or there's even no PID available...
    if (!isset($_REQUEST['dn']) && (!isset($_REQUEST['PID']) || $_REQUEST['PID'] != $this->pid)) {
      return;
    }

    // Save position if set
    if (isset($_POST['position_'.$this->pid]) && is_numeric($_POST['position_'.$this->pid])) {
      $this->scrollPosition = $_POST['position_'.$this->pid];
    }

    $result = array("targets" => array(), "action" => "");

    // Filter GET with "act" attributes
    if (isset($_GET['act'])) {
      $key = validate($_GET['act']);
      if (preg_match('/^listing_([a-zA-Z_]+)_([0-9]+)$/', $key, $m)) {
        $action = $m[1];
        $target = $m[2];
        if (isset($this->entries[$target]['dn'])) {
          $result['action']     = $action;
          $result['targets'][]  = $this->entries[$target]['dn'];
        }
      } elseif (isset($_REQUEST['dn']) && preg_match('/^listing_([a-zA-Z_]+)$/', $key, $m)) {
        $dn                   = urldecode($_REQUEST['dn']);
        $result['action']     = $m[1];
        $result['targets'][]  = $dn;
        // Make sure no other management class intercept the same dn
        unset($_REQUEST['dn']);
      }

      // Drop targets if empty
      if (count($result['targets']) == 0) {
        unset($result['targets']);
      }
      if (preg_match('/^(edit)_([a-zA-Z_]+)$/', $result['action'], $m)) {
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
        if (isset($this->entries[$target]['dn'])) {
          $result['targets'][] = $this->entries[$target]['dn'];
        }
        continue;
      }

      // Capture action with target - this is a one shot
      if (preg_match('/^listing_[a-zA-Z_]+_[0-9]+(|_x)$/', $key)) {
        $target = preg_replace('/^listing_[a-zA-Z_]+_([0-9]+)(|_x)$/', '$1', $key);
        if (isset($this->entries[$target]['dn'])) {
          $result['action']   = preg_replace('/^listing_([a-zA-Z_]+)_[0-9]+(|_x)$/', '$1', $key);
          $result['targets']  = array($this->entries[$target]['dn']);
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
    if (isset($_POST['act']) && $_POST['act'] != '') {
      if (!preg_match('/^export.*$/', $_POST['act'])) {
        $result['action'] = validate($_POST['act']);
      }
    }

    // Drop targets if empty
    if (count($result['targets']) == 0) {
      unset($result['targets']);
    }
    if (preg_match('/^(edit)_([a-zA-Z_]+)/', $result['action'], $m)) {
      $result['action']     = $m[1];
      $result['subaction']  = $m[2];
    }
    return $result;
  }


  function renderActionMenu()
  {
    // Don't send anything if the menu is not defined
    if (!isset($this->xmlData['actionmenu']['action'])) {
      return "";
    }

    // Make sure we got an array of actions
    if (isset($this->xmlData['actionmenu']['action']['type'])) {
      $this->xmlData['actionmenu']['action'] = array($this->xmlData['actionmenu']['action']);
    }

    // Load shortcut
    $result   = '<input type="hidden" name="act" id="actionmenu" value="">'.
                '<div style="display:none"><input type="submit" name="exec_act" id="exec_act" value=""/></div>'.
                '<ul class="level1" id="root"><li><a href="#">'._('Actions').
                '&nbsp;<img class="center optional" src="images/down-arrow.png" alt="down arrow"/></a>';

    // Build ul/li list
    $result .= $this->recurseActions($this->xmlData['actionmenu']['action']);

    return '<div id="pulldown">'.$result.'</li></ul></div>';
  }

  function renderActionMenuActionLink($separator, $action, $name, $icon)
  {
    return '<li'.$separator.' id="actionmenu_'.$action.'">'
                  .'<a href="#" onClick="'
                    ."document.getElementById('actionmenu').value='$action';document.getElementById('exec_act').click();"
                  .'">'
                  .'<img src="'.htmlentities($icon, ENT_COMPAT, 'UTF-8').'" alt="'.$action.'" class="center">&nbsp;'.$name.'</a>'
                  .'</li>';
  }

  function recurseActions($actions)
  {
    global $class_mapping;
    static $level = 2;
    $result       = "<ul class='level$level'>";
    $separator    = "";

    foreach ($actions as $action) {

      // Skip the entry completely if there's no permission to execute it
      if (!$this->hasActionPermission($action, $this->filter->base)) {
        continue;
      }

      // Skip entry if there're missing dependencies
      if (isset($action['depends'])) {
        $deps = is_array($action['depends'])?$action['depends']:array($action['depends']);
        foreach ($deps as $clazz) {
          if (!isset($class_mapping[$clazz])) {
            continue 2;
          }
        }
      }

      if ($action['type'] == "separator") {
        $separator = " style='border-top:1px solid #AAA' ";
        continue;
      }

      // Dive into subs
      if ($action['type'] == "sub" && isset($action['action'])) {
        $level++;
        if (isset($action['label'])) {
          $img = "";
          if (isset($action['image'])) {
            $img = "<img class='center' src='".htmlentities($action['image'], ENT_COMPAT, 'UTF-8')."' alt='".$action['label']."'/>&nbsp;";
          }
          $result .= "<li id='actionmenu_".strtolower($action['label'])."'$separator><a href='#'>$img"._($action['label'])."&nbsp;<img src='images/forward-arrow.png' alt='forward arrow'/></a>";
        }

        // Ensure we've an array of actions, this enables sub menus with only one action.
        if (isset($action['action']['type'])) {
          $action['action'] = array($action['action']);
        }

        $result .= $this->recurseActions($action['action'])."</li>";
        $level--;
        $separator = "";
        continue;
      }

      // Render entry elseways
      if (isset($action['label'])) {
        $result .= $this->renderActionMenuActionLink($separator, $action['name'], _($action['label']), $action['image']);
      }

      // Check for special types
      switch ($action['type']) {
        case 'copypaste':
          $cut    = !isset($action['cut']) || $action['cut'] != "false";
          $copy   = !isset($action['copy']) || $action['copy'] != "false";
          $result .= $this->renderCopyPasteMenu($separator, $copy, $cut);
          break;

        case 'snapshot':
          $result .= $this->renderSnapshotMenu($separator);
          break;

        case 'exporter':
          $result .= $this->renderExporterMenu($separator);
          break;

        case 'daemon':
          $result .= $this->renderDaemonMenu($separator);
          break;
      }

      $separator = "";
    }

    $result .= "</ul>";
    return $result;
  }

  /*!
   * \brief Check if user have action permission
   *
   * \param string $action
   *
   * \param string $dn The DN
   *
   * \param string $classes
   *
   */
  function hasActionPermission($action, $dn, $classes = NULL)
  {
    $ui = get_userinfo();

    if (isset($action['acl'])) {
      if (($attrs = $this->getEntry($dn)) != NULL) {
        $otype = $this->getObjectTypeInfos($dn, $attrs);
      } elseif ($classes) {
        $otype = $this->getObjectTypeInfos($dn, array('objectClass' => $classes));
      } else {
        $otype = FALSE;
      }
      $acls = $action['acl'];
      if (!is_array($acls)) {
        $acls = array($acls);
      }

      // Every ACL has to pass
      foreach ($acls as $acl) {
        $module   = $this->categories;
        $aclList  = array();

        // Replace %acl if available
        if ($otype) {
          $acl = str_replace('%acl', $otype['category']."/".$otype['class'], $acl);
        }

        // Split for category and plugins if needed
        // match for "[rw]" style entries
        if (preg_match('/^\[([rwcdm]+)\]$/', $acl, $match)) {
          $aclList = array($match[1]);
        }

        // match for "user[rw]" style entries
        if (preg_match('/^([a-zA-Z0-9]+)\[([rwcdm]+)\]$/', $acl, $match)) {
          $module   = $match[1];
          $aclList  = array($match[2]);
        }

        // match for "user/user[rw]" style entries
        if (preg_match('/^([a-zA-Z0-9]+\/[a-zA-Z0-9]+)\[([rwcdm]+)\]$/', $acl, $match)) {
          $module   = $match[1];
          $aclList  = array($match[2]);
        }

        // match "user/user[userPassword:rw(,...)*]" style entries
        if (preg_match('/^([a-zA-Z0-9]+\/[a-zA-Z0-9]+)\[([a-zA-Z0-9]+:[rwcdm]+(,[a-zA-Z0-9]+:[rwcdm]+)*)\]$/', $acl, $match)) {
          $module   = $match[1];
          $aclList  = explode(',', $match[2]);
        }

        // Walk thru prepared ACL by using $module
        foreach ($aclList as $sAcl) {
          $checkAcl = "";

          // Category or detailed permission?
          $modules = $module;
          if (!is_array($modules)) {
            $modules = array($modules);
          }
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
    $deps = $ui->get_module_departments($this->categories);
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

  /*!
   * Get the depatments
   */
  function getDepartments()
  {
    $departments = array();
    $ui = get_userinfo();

    // Get list of supported department types
    $types = departmentManagement::getDepartmentTypes();

    // Load departments allowed by ACL
    $validDepartments = $ui->get_module_departments($this->categories);

    // Build filter and look in the LDAP for possible sub departments
    // of current base
    $filter = "(|";
    $attrs  = array("description", "objectClass");
    foreach ($types as $type) {
      $i = objects::infos($type);
      $filter .= $i['filter'];
      $attrs[] = $i['mainAttr']; // Specific key for departement objectTypes
    }
    $filter .= ")";
    $res = get_list($filter, $this->categories, $this->base, $attrs, GL_NONE);

    // Analyze list of departments
    foreach ($res as $department) {
      if (!in_array($department['dn'], $validDepartments)) {
        continue;
      }

      // Add the attribute where we use for sorting
      $oc = NULL;
      foreach ($types as $type) {
        if (objects::isOfType($department, $type)) {
          $oc = $type;
          break;
        }
      }
      $i = objects::infos($oc);
      $department['sort-attribute'] = $i['mainAttr'];

      // Move to the result list
      $departments[] = $department;
    }

    return $departments;
  }


  function renderCopyPasteMenu($separator, $copy = TRUE, $cut = TRUE)
  {
    // We can only provide information if we've got a copypaste handler
    // instance
    if (!(isset($this->copyPasteHandler) && is_object($this->copyPasteHandler))) {
      return "";
    }

    // Presets
    $result = "";
    $read   = FALSE;
    $paste  = FALSE;
    $ui     = get_userinfo();

    // Switch flags to on if there's at least one category which allows read/paste
    foreach ($this->categories as $category) {
      $read   = $read || preg_match('/r/', $ui->get_category_permissions($this->base, $category));
      $paste  = $paste || $ui->is_pasteable($this->base, $category) == 1;
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
        $result .= "<li$separator><a href='#'><img src='geticon.php?context=actions&amp;icon=edit-paste&amp;size=16&amp;disabled=1' alt='paste' class='center'>&nbsp;"._("Paste")."</a></li>";
      }
    }

    return $result;
  }


  function renderCopyPasteActions($row, $dn, $category, $class, $copy = TRUE, $cut = TRUE)
  {
    global $config;
    // We can only provide information if we've got a copypaste handler
    // instance
    if (!(isset($this->copyPasteHandler) && is_object($this->copyPasteHandler))) {
      return "";
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
                    ' style="padding:1px">';
      } else {
        $result .= "<img src='images/empty.png' alt=' ' class='center optional' style='padding:1px'>";
      }
    }

    // Render copy entries
    if ($copy) {
      if ($ui->is_copyable($dn, $category, $class)) {
        $result .= '<input class="center" type="image"'.
                    ' src="geticon.php?context=actions&amp;icon=edit-copy&amp;size=16"'.
                    ' alt="'._('Copy').'" name="listing_copy_'.$row.'" title="'._('Copy this entry').'"'.
                    ' style="padding:1px">';
      } else {
        $result .= "<img src='images/empty.png' alt=' ' class='center optional' style='padding:1px'>";
      }
    }

    return $result;
  }


  function renderSnapshotMenu($separator)
  {
    // We can only provide information if we've got a snapshot handler
    // instance
    if (!(isset($this->snapshotHandler) && is_object($this->snapshotHandler))) {
      return "";
    }

    // Presets
    $result = "";
    $ui     = get_userinfo();

    if ($this->snapshotHandler->enabled() && $ui->allow_snapshot_restore($this->base, $this->categories)) {

      // Check if there is something to restore
      $restore = FALSE;
      foreach ($this->snapshotHandler->getSnapshotBases() as $base) {
        $restore = $restore || count($this->snapshotHandler->getDeletedSnapshots($base)) > 0;
      }

      // Draw icons according to the restore flag
      if ($restore) {
        $result .= $this->renderActionMenuActionLink($separator, 'restore', _('Restore snapshots'), 'geticon.php?context=actions&icon=document-restore&size=16');
      } else {
        $result .= "<li$separator><a href='#'><img src='geticon.php?context=actions&amp;icon=document-restore&amp;size=16&amp;disabled=1' alt='restore' class='center'>&nbsp;"._("Restore snapshots")."</a></li>";
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


  function renderSnapshotActions($row, $dn, $category, $class, $copy = TRUE, $cut = TRUE)
  {
    // We can only provide information if we've got a snapshot handler
    // instance
    if (!(isset($this->snapshotHandler) && is_object($this->snapshotHandler))) {
      return "";
    }

    // Presets
    $result = "";
    $ui = get_userinfo();

    // Only act if enabled here
    if ($this->snapshotHandler->enabled()) {

      // Draw restore button
      if ($ui->allow_snapshot_restore($dn, $category)) {

        // Do we have snapshots for this dn?
        if ($this->snapshotHandler->hasSnapshots($dn)) {
          $result .= "<input class='center' type='image' src='geticon.php?context=actions&amp;icon=document-restore&amp;size=16' ".
                     "alt='"._("Restore snapshot")."' name='listing_restore_$row' title='".
                     _("Restore snapshot")."' style='padding:1px'>";
        } else {
          $result .= "<img src='geticon.php?context=actions&amp;icon=document-restore&amp;size=16&amp;disabled=1' alt='restore' class='center' style='padding:1px'>";
        }
      }

      // Draw snapshot button
      if ($ui->allow_snapshot_create($dn, $category)) {
          $result .= "<input class='center' type='image' src='geticon.php?context=actions&amp;icon=snapshot&amp;size=16' ".
                     "alt='"._("Create snapshot")."' name='listing_snapshot_$row' title='".
                     _("Create a new snapshot from this object")."' style='padding:1px'>";
      } else {
          $result .= "<img src='images/empty.png' alt=' ' class='center optional' style='padding:1px'>";
      }
    }

    return $result;
  }


  function renderDaemonMenu($separator)
  {
    $result = "";

    // If there is a daemon registered, draw the menu entries
    if (class_available("DaemonEvent")) {
      $events = DaemonEvent::get_event_types_by_category($this->categories);
      if (isset($events['BY_CLASS']) && count($events['BY_CLASS'])) {
        foreach ($events['BY_CLASS'] as $name => $event) {
          $result .= "<li$separator><a href='#' onClick='document.getElementById(\"actionmenu\").value=\"$name\";document.getElementById(\"exec_act\").click();'>".$event['MenuImage']."&nbsp;".$event['s_Menu_Name']."</a></li>";
          $separator = "";
        }
      }
    }

    return $result;
  }


  function getEntry($dn)
  {
    if (isset($this->dnToRow[$dn])) {
      return $this->entries[$this->dnToRow[$dn]];
    }
    return NULL;
  }

  /*!
   * \brief Get listing entries
   */
  function getEntries()
  {
    return $this->entries;
  }

  /*!
   * \brief Get type
   *
   * \param string $dn The DN
   */
  function getType($dn)
  {
    if (isset($this->objectDnMapping[$dn])) {
      return $this->objectDnMapping[$dn];
    }
    return NULL;
  }

}

?>
