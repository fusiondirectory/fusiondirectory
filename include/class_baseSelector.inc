<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2020  FusionDirectory

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
 * \file class_baseSelector.inc
 * Source code for Class Base Selector
 */

/*!
 * \brief Class Base Selector
 */
class baseSelector
{
  private $base           = '';
  protected $pid;
  private $action;
  private $height         = 500;
  private $submitButton   = TRUE;
  protected $tree         = NULL;
  protected $pathMapping  = [];
  protected $lastState;

  /*!
   * \brief baseSelector contructor
   *
   * \param Array $bases The Bases
   *
   * \param String $base Empty string
   */
  function __construct (array $bases, string $base = '')
  {
    // Initialize pid
    $this->pid = preg_replace("/[^0-9]/", "", microtime(TRUE));

    // Transfer data
    $this->setBases($bases);
    $this->setBase($base);
  }

  /*!
   * \brief Returns id of the html field
   */
  function getInputHtmlId (): string
  {
    return 'bs_input_'.$this->pid;
  }

  /*!
   * \brief Set a new flag to the submit button
   *
   * \param Boolean $flag Flag for the submit button
   */
  function setSubmitButton ($flag)
  {
    $this->submitButton = $flag;
  }

  /*!
   * \brief Set a new value of the member height
   *
   * \param Integer $value The new value of the height
   */
  function setHeight ($value)
  {
    $this->height = $value;
  }

  /*!
   * \brief Set a new value of the member base
   *
   * \param String $base The new value of the base
   */
  function setBase (string $base): bool
  {
    if (isset($this->pathMapping[$base])) {
      $this->base       = $base;
      $this->lastState  = TRUE;
      return $this->update(TRUE);
    } else {
      $this->lastState = FALSE;
      return FALSE;
    }
  }

  /*!
   * \brief Check the last base value updated
   *
   * \return Boolean the last state of the object
   */
  function checkLastBaseUpdate (): bool
  {
    return $this->lastState;
  }

  /*!
   * \brief Set new bases
   *
   * \param Array $bases The new value of the bases
   */
  function setBases (array $bases)
  {
    global $config;

    $this->pathMapping = [];

    foreach ($bases as $base => $dummy) {
      // Build path style display
      $elements = explode(',', substr($base, 0, strlen($base) - strlen($config->current['BASE'])));
      $elements = array_reverse($elements, TRUE);

      $this->pathMapping[$base] = (($base == $config->current['BASE']) ? '/' : preg_replace('/(^|,)[a-z0-9]+=/i', '/', implode(',', $elements)));
    }

    // Save bases to session for autocompletion
    session::set('pathMapping', $this->pathMapping);
  }

  /*!
   * \brief Update the base
   *
   * \param Boolean $force FALSE
   */
  function update (bool $force = FALSE): bool
  {
    if (!isset($this->base) || ($this->base == '')) {
      $this->lastState = FALSE;
      return FALSE;
    }

    // Analyze for base changes if needed
    $this->action = NULL;
    $last_base    = $this->base;
    if (isset($_REQUEST['BPID']) && $_REQUEST['BPID'] == $this->pid) {
      if (!empty($_POST['bs_rebase_'.$this->pid])) {
        $new_base = base64_decode($_POST['bs_rebase_'.$this->pid]);
        if (isset($this->pathMapping[$new_base])) {
          $this->base   = $new_base;
          $this->action = 'rebase';
        } else {
          $this->lastState = FALSE;
          return FALSE;
        }
      } elseif (isset($_POST[$this->getInputHtmlId()])) {
        // Take over input field base
        if (($this->submitButton && isset($_POST['submit_base_'.$this->pid.'_x'])) || !$this->submitButton) {

          // Check if base is available
          $this->lastState = FALSE;
          foreach ($this->pathMapping as $key => $path) {
            if (mb_strtolower($path) == mb_strtolower($_POST[$this->getInputHtmlId()])) {
              $this->base       = $key;
              $this->lastState  = TRUE;
              break;
            }
          }
        }
      }
    }

    /* Skip if there's no change */
    if ($this->tree && ($this->base == $last_base) && !$force) {
      $this->lastState = TRUE;
      return TRUE;
    }

    /* Force tree render next time render() is called */
    $this->tree = NULL;

    $this->lastState = TRUE;
    return TRUE;
  }

  protected function renderTree ()
  {
    global $config;

    /* Build tree */
    $departmentInfo = $config->getDepartmentInfo();
    $tree           = [];
    foreach (array_keys($this->pathMapping) as $base) {
      if ($base == $config->current['BASE']) {
        /* Skip root */
        continue;
      }

      $elements     = explode(',', substr($base, 0, strlen($base) - strlen($config->current['BASE'])));
      /* Remove last one */
      array_pop($elements);
      /* Remove first one */
      array_shift($elements);

      $array        =& $tree;
      $elementBase  = $config->current['BASE'];
      foreach (array_reverse($elements) as $element) {
        $elementBase = $element.','.$elementBase;
        if (!isset($array[$elementBase])) {
          /* Our parent is missing, add it but without link */
          $array[$elementBase] = [
            'tree'        => [],
            'selected'    => FALSE,
            'link'        => FALSE,
            'img'         => $departmentInfo[$elementBase]['img'],
            'name'        => $departmentInfo[$elementBase]['name'],
            'description' => $departmentInfo[$elementBase]['description'],
          ];
        }
        /* Go down one level */
        $array =& $array[$elementBase]['tree'];
      }

      $array[$base] = [
        'tree'        => [],
        'selected'    => ($this->base == $base),
        'link'        => TRUE,
        'img'         => $departmentInfo[$base]['img'],
        'name'        => $departmentInfo[$base]['name'],
        'description' => $departmentInfo[$base]['description'],
      ];
    }

    $smarty = get_smarty();
    $smarty->assign('htmlid',       $this->getInputHtmlId());
    $smarty->assign('pid',          $this->pid);
    $smarty->assign('currentValue', $this->pathMapping[$this->base]);
    $smarty->assign('submitButton', $this->submitButton);
    $smarty->assign('height',       $this->height);
    $smarty->assign('selected',     ($this->base == $config->current['BASE']));
    $smarty->assign('rootBase',     $config->current['BASE']);
    $smarty->assign('tree',         $tree);

    $this->tree = $smarty->fetch(get_template_path('baseselector.tpl'));
  }

  /*!
   * \brief Accessor of the member tree
   *
   * \return members tree of the ACL class
   */
  function render (): string
  {
    if (!isset($this->tree)) {
      $this->renderTree();
    }

    return $this->tree;
  }

  /*!
   * \brief Accessor of the base
   *
   * \return String containts the base of the object
   */
  function getBase (): string
  {
    return $this->base;
  }

  /*!
   * \brief Accessor of the bases
   *
   * \return Array containts the bases and their display text
   */
  function getBases (): array
  {
    return $this->pathMapping;
  }
}
