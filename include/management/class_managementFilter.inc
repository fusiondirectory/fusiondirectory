<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2017-2019  FusionDirectory

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

  protected $fixedScope;

  /* The management class */
  public $parent;

  public $pid;

  /*!
   * \brief Create a management filter
   *
   * \param string $parent management instance
   */
  function __construct (management $parent, bool $fixedScope = NULL, array $filterElementDefinitions = [])
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

    $this->filterElements = [];
    foreach ($filterElementDefinitions as $filterElementDefinition) {
      list($class, $args) = $filterElementDefinition;
      $this->filterElements[] = new $class($this, ...$args);
    }

    $this->fixedScope = $fixedScope;
  }

  function setScope (string $scope)
  {
    $this->scope = $scope;
  }

  protected function showScope (): bool
  {
    if ($this->fixedScope !== NULL) {
      return !$this->fixedScope;
    }
    return $this->parent->listing->getBaseMode();
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

      if ($this->showScope()) {
        $this->scope = (isset($_POST['SCOPE']) ? 'sub' : 'one');
      }

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
    $smarty->assign('SHOWSCOPE',  $this->showScope());
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
        $attrsAsked[$infos['mainAttr']] = '*';
      }
      if (!empty($infos['nameAttr']) && !isset($attrsAsked[$infos['nameAttr']])) {
        /* Ask for nameAttr */
        $attrsAsked[$infos['nameAttr']] = '*';
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
          $searchAttributesTmp    = $this->searchAttributes[$type];
          if ($this->showTemplates) {
            $searchAttributesTmp[]  = '_template_cn';
          }
          $elementFilters[] = '(|('.implode('=*'.ldap_escape_f($this->search).'*)(', $searchAttributesTmp).'=*'.ldap_escape_f($this->search).'*))';
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
        try {
          $ldapEntries = objects::ls($type, $attrsAsked, (($this->scope == 'one') ? 'ou=templates,'.$searchBase : $searchBase), $filter, TRUE, $this->scope, TRUE);

          $objectTypeCount['template_'.$type] = count($ldapEntries);
          foreach ($ldapEntries as $dn => $entry) {
            $entries[$dn] = new ListingEntry($this->parent->listing, 'template_'.$type, $dn, $entry, $row++);
          }
        } catch (FusionDirectoryException $e) {
          $warning = new FusionDirectoryWarning(
            nl2br(htmlescape(sprintf(
              _("Could not search for \"%s\" templates:\n%s"),
              $this->types['filter_type_'.$type]['infos']['name'],
              $e->getMessage()
            ))),
            0,
            $e
          );
          $warning->display();
        }
      }

      if (!$this->types['filter_type_'.$type]['show']) {
        continue;
      }

      try {
        $ldapEntries = objects::ls($type, $attrsAsked, $searchBase, $filter, TRUE, $this->scope, FALSE, TRUE);

        $ldapEntries = $this->filterEntries($ldapEntries);
      } catch (FusionDirectoryException $e) {
        $warning = new FusionDirectoryWarning(
          nl2br(htmlescape(sprintf(
            _("Could not search for \"%s\":\n%s"),
            $this->types['filter_type_'.$type]['infos']['name'],
            $e->getMessage()
          ))),
          0,
          $e
        );
        $warning->display();
        $this->types['filter_type_'.$type]['show'] = FALSE;
        continue;
      }

      $objectTypeCount[$type] = count($ldapEntries);
      foreach ($ldapEntries as $dn => $entry) {
        if (isset($entries[$dn])) {
          trigger_error('Same dn '.$dn.' found as several object types');
        }
        $entries[$dn] = new ListingEntry($this->parent->listing, $type, $dn, $entry, $row++);
      }
    }

    return [$entries, $objectTypeCount];
  }

  protected function filterEntries (array $ldapEntries): array
  {
    if (!empty($this->parent->whiteList)) {
      foreach ($ldapEntries as $dn => $entry) {
        if (in_array($dn, $this->parent->whiteList['dn'])) {
          continue;
        }
        foreach ($this->parent->whiteList['branches'] as $branch) {
          if (preg_match('/'.preg_quote($branch, '/').'$/', $dn)) {
            continue 2;
          }
        }
        unset($ldapEntries[$dn]);
      }
    }
    if (isset($this->parent->blackList)) {
      foreach ($this->parent->blackList as $attr_name => $attr_values) {
        foreach ($attr_values as $match) {
          foreach ($ldapEntries as $dn => $entry) {
            if (isset($entry[$attr_name])) {
              $test = $entry[$attr_name];
              if (!is_array($test)) {
                $test = [$test];
              }
              if (in_array($match, $test)) {
                unset($ldapEntries[$dn]);
              }
            }
          }
        }
      }
    }

    return $ldapEntries;
  }
}
