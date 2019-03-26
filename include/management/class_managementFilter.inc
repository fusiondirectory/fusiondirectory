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
 * \brief This class handles the management filter box
 */
class managementFilter
{
  protected $types;

  protected $searchAttributes = [];
  protected $scope = 'one';
  protected $showTemplates = FALSE;

  protected $search = '';

  protected $filterElements = [];

  /* The management class */
  public $parent;

  public $pid;

  /*
   * Each objectType has its filters
   * */

  /*!
   * \brief Create a management filter
   *
   * \param string $parent management instance
   */
  function __construct (management $parent)
  {
    global $config;

    $this->parent = $parent;

    $this->pid = preg_replace('/[^0-9]/', '', microtime(TRUE));

    foreach ($this->parent->objectTypes as $type) {
      $this->types['filter_type_'.$type] = [
        'show'    => TRUE,
        'filters' => [],
        'infos'   => objects::infos($type),
      ];
    }

    $this->filterElements = [
      new TabFilterElement($this),
    ];
  }

  function setScope (string $scope)
  {
    $this->scope = $scope;
  }

  function addElement (FilterElement $element)
  {
    $this->filterElements[] = $element;
  }

  function update (string $base)
  {
    global $ui;

    if (isset($_POST['FILTER_PID']) && ($_POST['FILTER_PID'] == $this->pid)) {
      // Load post values and adapt filter, base and scope accordingly
      foreach ($this->types as $key => &$data) {
        $data['show'] = isset($_POST[$key]);
      }
      unset($data);

      $this->showTemplates = isset($_POST['filter_type_TEMPLATE']);

      $this->scope = (isset($_POST['SCOPE']) ? 'sub' : 'one');

      $this->search = (isset($_POST['SEARCH']) ? $_POST['SEARCH'] : '');

      foreach ($this->filterElements as $element) {
        $element->update();
      }
    }

    foreach ($this->parent->objectTypes as $type) {
      $attrs = objects::getSearchedAttributes($type);

      $this->searchAttributes[$type] = [];
      $this->parent->listing->fillSearchedAttributes($type, $attrs);
      foreach ($attrs as $attr => $acl) {
        $rights = $ui->get_permissions($base, $acl);
        if (strpos($rights, 'r') !== FALSE) {
          $this->searchAttributes[$type][] = $attr;
        }
      }
    }
  }

  /*!
   * \brief Render
   */
  function render (): string
  {
    $smarty = get_smarty();

    $smarty->assign('SCOPE',      $this->scope);
    $smarty->assign('BASEMODE',   $this->parent->listing->getBaseMode());
    $smarty->assign('FILTER_PID', $this->pid);
    $smarty->assign('SEARCH',     $this->search);
    $smarty->assign('TYPES',      $this->types);

    $searchAttrs = [];
    foreach ($this->searchAttributes as $type => $attrs) {
      foreach ($attrs as $attr) {
        $searchAttrs[] = strtolower($type).'/'.$attr;
      }
    }
    $smarty->assign('SEARCHDESC', sprintf(_('Searches in %s'), implode(', ', $searchAttrs)));

    $parentClass = get_class($this->parent);
    if (!$parentClass::$skipTemplates) {
      $smarty->assign('TEMPLATES',  $this->showTemplates);
    }
    $elements = [];
    foreach ($this->filterElements as $element) {
      $elements[] = $element->render();
    }
    $smarty->assign('FILTERS',  $elements);

    // load the file from the theme folder
    $file = get_template_path('management/filter.tpl');

    // Load template
    return $smarty->fetch($file);
  }

  function query (array $attrs, string $base): array
  {
    global $ui;

    $objectTypeCount  = [];
    $entries          = [];
    $row              = 0;
    $ui->getSizeLimitHandler()->setLimitExceeded(FALSE);
    foreach ($this->parent->objectTypes as $type) {
      $infos      = $this->types['filter_type_'.$type]['infos'];
      if ($this->scope == 'one') {
        $searchBase = (empty($infos['ou']) ? $base : $infos['ou'].$base);
      } else {
        $searchBase = $base;
      }

      $attrsAsked = $attrs;
      if (!empty($infos['mainAttr']) && !isset($attrsAsked[$infos['mainAttr']])) {
        /* Ask for mainAttr */
        $attrsAsked[$infos['mainAttr']] = '1';
      }
      if (!empty($infos['nameAttr']) && !isset($attrsAsked[$infos['nameAttr']])) {
        /* Ask for nameAttr */
        $attrsAsked[$infos['nameAttr']] = '1';
      }

      foreach ($attrsAsked as $attr => $data) {
        // TODO: find a better way to avoid Exceptions
        $category = $ui->getAttributeCategory($type, $attr);
        if ($category === FALSE) {
          unset($attrsAsked[$attr]);
        }
      }

      $elementFilters = [];
      if (!empty($this->search)) {
        if (preg_match('/^\(.+\)$/', $this->search)) {
          $elementFilters[] = $this->search;
        } else {
          $elementFilters[] = '(|('.implode('=*'.$this->search.'*)(', $this->searchAttributes[$type]).'=*'.$this->search.'*))';
        }
      }

      $typeElementFilters = $elementFilters;
      foreach ($this->filterElements as $element) {
        $skip = $element->getFilters($type, $typeElementFilters);
        if ($skip === TRUE) {
          continue 2;
        }
      }
      $filter = '';
      if (!empty($typeElementFilters)) {
        $filter = '(&'.implode('', $typeElementFilters).')';
      }

      $parentClass = get_class($this->parent);
      if (!$parentClass::$skipTemplates && $this->showTemplates) {
        $ldapEntries = objects::ls($type, $attrsAsked, (($this->scope == 'one') ? 'ou=templates,'.$searchBase : $searchBase), $filter, TRUE, $this->scope, TRUE);

        $objectTypeCount['template_'.$type] = count($ldapEntries);
        foreach ($ldapEntries as $dn => $entry) {
          $entries[$dn] = new ListingEntry($this->parent->listing, 'template_'.$type, $dn, $entry, $row++);
        }
      }

      if (!$this->types['filter_type_'.$type]['show']) {
        continue;
      }

      $ldapEntries = objects::ls($type, $attrsAsked, $searchBase, $filter, TRUE, $this->scope, FALSE, TRUE);

      $objectTypeCount[$type] = count($ldapEntries);
      foreach ($ldapEntries as $dn => $entry) {
        // TODO: check collisions?
        $entries[$dn] = new ListingEntry($this->parent->listing, $type, $dn, $entry, $row++);
      }
    }

    return [$entries, $objectTypeCount];
  }
}
