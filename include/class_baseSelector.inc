<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2017  FusionDirectory

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
class baseSelector {

  private $base;
  protected $pid;
  private $action;
  private $height       = 500;
  private $submitButton = TRUE;
  protected $tree;
  protected $pathMapping;
  protected $lastState;

  /*!
   * \brief baseSelector contructor
   *
   * \param Array $bases The Bases
   *
   * \param String $base Empty string
   */
  function __construct($bases, $base = "")
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
  function getInputHtmlId()
  {
    return 'bs_input_'.$this->pid;
  }

  /*!
   * \brief Set a new flag to the submit button
   *
   * \param Boolean $flag Flag for the submit button
   */
  function setSubmitButton($flag)
  {
    $this->submitButton = $flag;
  }

  /*!
   * \brief Set a new value of the member height
   *
   * \param Integer $value The new value of the height
   */
  function setHeight($value)
  {
    $this->height = $value;
  }

  /*!
   * \brief Set a new value of the member base
   *
   * \param String $base The new value of the base
   */
  function setBase($base)
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
  function checkLastBaseUpdate()
  {
    return $this->lastState;
  }

  /*!
   * \brief Set new bases
   *
   * \param Array $bases The new value of the bases
   */
  function setBases($bases)
  {
    global $config;

    $this->pathMapping = array();

    foreach ($bases as $base => $dummy) {
      // Build path style display
      $elements = explode(',', substr($base, 0, strlen($base) - strlen($config->current['BASE'])));
      $elements = array_reverse($elements, TRUE);

      $this->pathMapping[$base] = (($base == $config->current['BASE']) ? '/' : preg_replace('/(^|,)[a-z0-9]+=/i', '/', implode(',', $elements)));
    }

    // Save bases to session for autocompletion
    session::global_set('pathMapping', $this->pathMapping);
  }

  /*!
   * \brief Update the base
   *
   * \param Boolean $force FALSE
   */
  function update($force = FALSE)
  {
    global $config;

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

    $link = "onclick=\"\$('bs_rebase_".$this->pid."').value='".base64_encode($config->current['BASE'])."';  $('submit_tree_base_".$this->pid."').click();\"";
    $this->tree = '<input class="base-selector" type="text" name="'.$this->getInputHtmlId().'"'.
                  ' id="'.$this->getInputHtmlId().'"'.
                  ' onkeydown="$(\'bs_'.$this->pid.'\').hide()"'.
                  ' onfocus="$(\'bs_'.$this->pid.'\').hide()"'.
                  ' onmouseover="Element.clonePosition($(\'bs_'.$this->pid.'\'), \''.$this->getInputHtmlId().'\', {setHeight: false, setWidth: false, offsetTop:(Element.getHeight(\''.$this->getInputHtmlId().'\'))});$(\'bs_'.$this->pid.'\').show();"'.
                  ' onmouseout="rtimer= Element.hide.delay(0.25, \'bs_'.$this->pid.'\')"'.
                  ' value="'.htmlentities($this->pathMapping[$this->base], ENT_COMPAT, 'UTF-8').'"/>';

    // Autocompleter
    $this->tree .= "<div id='autocomplete_".$this->pid."' class='autocomplete'></div>".
                  "<script type='text/javascript'>".
                  "new Ajax.Autocompleter('".$this->getInputHtmlId()."', 'autocomplete_".$this->pid."', 'autocomplete.php?type=base', { minChars: 3, frequency: 0.5 });";
    if ($this->submitButton) {
      $this->tree .= "$('".$this->getInputHtmlId()."').observe('keypress', function(event) { if(event.keyCode == Event.KEY_RETURN) { $('submit_base_".$this->pid."').click(); } });";
    }
    $this->tree .= "</script>";

    $selected     = ($this->base == $config->current['BASE'] ? 'Selected' : '');
    $this->tree   .= "<div class='treeList' style='display:none;max-height:".$this->height."px' id='bs_".$this->pid."' onmouseover=\"window.clearTimeout(rtimer);\" onmouseout=\"rtimer= Element.hide.delay(0.25, 'bs_".$this->pid."')\"><a class='treeList$selected' $link>/&nbsp;["._("Root")."]</a><ul class='treeList'>\n";
    $first        = TRUE;
    $last_indent  = 2;

    foreach ($this->pathMapping as $base => $dummy) {
      // Skip root for tree
      if ($base == $config->current['BASE']) {
        continue;
      }

      // Build path style display
      $elements = explode(',', substr($base, 0, strlen($base) - strlen($config->current['BASE'])));

      $indent = count($elements);
      if (!$first && ($indent == $last_indent)) {
        $this->tree .= "</li>\n";
      } elseif ($indent > $last_indent) {
        $this->tree .= "<ul>\n";
      } elseif ($indent < $last_indent) {
        for ($i = 0; $i < ($last_indent - $indent); $i++) {
          $this->tree .= "</li></ul>\n";
        }
        $this->tree .= "</li>\n";
      }
      $selected   = ($this->base == $base ? ' class="treeListSelected"' : '');
      $link       = "onclick=\"\$('bs_rebase_".$this->pid."').value='".base64_encode($base)."';$('submit_tree_base_".$this->pid."').click();\"";
      $this->tree .= "<li><a$selected $link>".
                    '<img class="center" '.
                    'src="'.htmlentities($config->department_info[$base]['img'], ENT_COMPAT, 'UTF-8').'" '.
                    'alt="'.$config->department_info[$base]['name'].'"/>&nbsp;'.
                    $this->gennonbreaks($config->department_info[$base]['name']).
                    ($config->department_info[$base]['description'] == '' ? '' : '&nbsp;<span class="informal">['.$this->gennonbreaks($config->department_info[$base]['description']).']</span>').
                    '</a>';

      $last_indent  = $indent;
      $first        = FALSE;
    }

    // Close tree
    for ($i = 1; $i < $last_indent; $i++) {
      if (($i > 1) || (!$first)) {
        $this->tree .= "</li>";
      }
      $this->tree .= "</ul>\n";
    }
    $this->tree .= "</div>\n";

    // Draw submitter if required
    if ($this->submitButton) {
      $this->tree .= "&nbsp;<input class='center' type='image' src='geticon.php?context=actions&amp;icon=submit&amp;size=16' title='"._("Submit")."' name='submit_base_".$this->pid."' id='submit_base_".$this->pid."' alt='"._("Submit")."'>";
    }
    $this->tree .= '<input type="submit" style="display:none" name="submit_tree_base_'.$this->pid.'" id="submit_tree_base_'.$this->pid.'"/>';
    $this->tree .= '<input type="hidden" name="bs_rebase_'.$this->pid.'" id="bs_rebase_'.$this->pid.'"/>';
    $this->tree .= '<input type="hidden" name="BPID" id="BPID" value="'.$this->pid.'"/>';

    $this->lastState = TRUE;
    return TRUE;
  }


  /*!
   * \brief Replace all space of the string by non-breaking space
   *
   * \param String $string The string which his space will be replaced
   */
  function gennonbreaks($string)
  {
    return str_replace('-', '&#8209;', str_replace(' ', '&nbsp;', $string));
  }

  /*!
   * \brief Accessor of the member tree
   *
   * \return members tree of the ACL class
   */
  function render()
  {
    return $this->tree;
  }

  /*!
   * \brief Accessor of the base
   *
   * \return String containts the base of the object
   */
  function getBase()
  {
    return $this->base;
  }

  /*!
   * \brief Accessor of the bases
   *
   * \return Array containts the bases and their display text
   */
  function getBases()
  {
    return $this->pathMapping;
  }
}
?>
