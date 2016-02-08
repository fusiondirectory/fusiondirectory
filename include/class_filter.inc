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
 * \file class_filter.inc
 * Source code for class filter
 */

/*!
 * \brief This class contains all the function needed to manage filter
 */
class filter
{
  var $xmlData;
  var $elements         = array();
  var $elementValues    = array();
  var $alphabetElements = array();
  var $autocompleter    = array();
  var $category         = "";
  var $objectStorage    = array();
  var $base             = "";
  var $scope            = "";
  var $query;
  var $initial          = FALSE;
  var $scopeMode        = "auto";
  var $alphabet         = NULL;
  var $converter        = array();
  var $pid;
  var $headpage;

  /*!
   * \brief Create a filter
   *
   * \param string $filename
   */
  function __construct($filename)
  {
    // Load eventually passed filename
    if (!$this->load($filename)) {
      die("Cannot parse $filename!");
    }

    $this->pid = preg_replace("/[^0-9]/", "", microtime(TRUE));
  }

  /*
   * \brief Load a filter
   *
   * \param string $filename
   */
  function load($filename)
  {
    $contents       = file_get_contents($filename);
    $this->xmlData  = xml::xml2array($contents, 1);

    if (!isset($this->xmlData['filterdef'])) {
      return FALSE;
    }

    $this->xmlData = $this->xmlData["filterdef"];

    // Load filter
    if (isset($this->xmlData['search'])) {
      if (!isset($this->xmlData['search']['query'][0])) {
        $this->xmlData['search']['query'] = array($this->xmlData['search']['query']);
      }

      // Move information
      $entry            = $this->xmlData['search'];
      $this->scopeMode  = $entry['scope'];
      if ($entry['scope'] == "auto") {
        $this->scope = "one";
      } else {
        $this->scope = $entry['scope'];
      }
      $this->query = $entry['query'];
    } else {
      return FALSE;
    }

    // Transfer initial value
    if (isset($this->xmlData['definition']['initial']) && ($this->xmlData['definition']['initial'] == "true")) {
      $this->initial = TRUE;
    }

    // Transfer category
    if (isset($this->xmlData['definition']['category'])) {
      $this->category = $this->xmlData['definition']['category'];
    }

    // Generate formular data
    if (isset($this->xmlData['element'])) {
      if (!isset($this->xmlData['element'][0])) {
        $this->xmlData['element'] = array($this->xmlData['element']);
      }
      foreach ($this->xmlData['element'] as $element) {

        // Ignore elements without type
        if (!isset($element['type']) || !isset($element['tag'])) {
          continue;
        }

        $tag = $element['tag'];

        // Fix arrays
        if (isset($element['value']) && !isset($element['value'][0])) {
          $element['value'] = array($element['value']);
        }

        // Store element for quick access
        $this->elements[$tag] = $element;

        // Preset elementValues with default values if exist
        if (isset($element['default']) && !is_array($element['default'])) {
          $this->elementValues[$tag] = $element['default'];
        } else {
          $this->elementValues[$tag] = "";
        }

        // Does this element react on alphabet links?
        if (isset($element['alphabet']) && $element['alphabet'] == "true") {
          $this->alphabetElements[] = $tag;
        }
      }

      uasort($this->elements, 'strlenSort');
      $this->elements = array_reverse($this->elements);

    }

    return TRUE;
  }

  /*!
   * \brief Get the text in the field
   *
   * \param array $element
   */
  function getTextfield($element)
  {
    $tag  = $element['tag'];
    $size = 30;
    if (isset($element['size'])) {
      $size = $element['size'];
    }
    $maxlength = 30;
    if (isset($element['maxlength'])) {
      $maxlength = $element['maxlength'];
    }
    $result = "<input class='filter_textfield' id='$tag' name='$tag' type='text' size='$size' maxlength='{$maxlength}' value='".$this->elementValues[$tag]."'>";
    if (isset($element['autocomplete'])) {
      $frequency  = "0.5";
      $characters = "1";
      if (isset($element['autocomplete']['frequency'])) {
        $frequency = $element['autocomplete']['frequency'];
      }
      if (isset($element['autocomplete']['characters'])) {
        $characters = $element['autocomplete']['characters'];
      }
      $result .= "<div id='autocomplete$tag' class='autocomplete'></div>".
                "<script type='text/javascript'>".
                "new Ajax.Autocompleter('$tag', 'autocomplete$tag', 'autocomplete.php', { minChars: $characters, frequency: $frequency });".
                "</script>";

       $this->autocompleters[$tag] = $element['autocomplete'];
    }
    return $result;
  }


  /*!
   * \brief Get the checkbox
   *
   * \param array $element
   */
  function getCheckbox($element)
  {
    $tag      = $element['tag'];
    $checked  = "";
    if ($this->elementValues[$tag] == "true") {
      $checked = " checked";
    }

    $result = "<input class='filter_checkbox' id='$tag' name='$tag' type='checkbox' onClick='document.mainform.submit();' value='true'$checked>";
    return $result;
  }

  /*!
   * \brief Get a combobox
   *
   * \param array $element
   */
  function getCombobox($element)
  {
    $result = "<select name='".$element['tag']."' size='1' onChange='document.mainform.submit();'>";

    // Fill with presets
    foreach ($element['value'] as $value) {
      $selected = "";
      if ($this->elementValues[$element['tag']] == $value['key']) {
        $selected = " selected";
      }

      // Handle translations
      $result .= "<option value='".$value['key']."'$selected>"._($value['label'])."</option>";
    }

    // Use autocompleter for additional data
    if (isset($element['autocomplete'])) {
      $list = $this->getCompletitionList($element['autocomplete'], $element['tag']);
      foreach ($list as $value) {
        $selected = "";
        if ($this->elementValues[$element['tag']] == $value) {
          $selected = " selected";
        }
        $result .= "<option value='$value'$selected>$value</option>";
      }
    }

    $result .= "</select>";

    return $result;
  }

  /*!
   * \brief Set the combobox options
   *
   * \param string $tag
   *
   * \param array $options
   */
  function setComboBoxOptions($tag, $options)
  {
    if (isset($this->elements[$tag]) && ($this->elements[$tag]['type'] == "combobox")) {
      $this->elements[$tag]['value'] = array();
      foreach ($options as $key => $label) {
        $this->elements[$tag]['value'][] = array('label' => $label, 'key' => $key);
      }
    }
  }

   /*!
   * \brief Set a converter
   *
   * \param string $field
   *
   * \param string $hook
   */
  function setConverter($field, $hook)
  {
    $this->converter[$field] = $hook;
  }

  /*!
   * \brief Set a object storage
   *
   * \param string $storage
   */
  function setObjectStorage($storage)
  {
    $this->objectStorage = $storage;
  }


  /*!
   * \brief Set a base
   *
   * \param string $base
   */
  function setBase($base)
  {
    $this->base = $base;
  }

  /*!
   * \brief Set the current scope
   *
   * \param string $scope
   */
  function setCurrentScope($scope)
  {
    $this->scope = $scope;
  }

  /*!
   * \brief Render alphabet
   *
   * \param integer $columns
   */
  function renderAlphabet($columns = 10)
  {
    // Return pre-rendered alphabet if available
    if ($this->alphabet) {
      return $this->alphabet;
    }

    $characters = _("*ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    $alphabet   = "";
    $c          = 0;

    /* Fill cells with charaters */
    for ($i = 0, $l = mb_strlen($characters, 'UTF8'); $i < $l; $i++) {
      if ($c == 0) {
        $alphabet .= "<tr>";
      }

      $ch       = mb_substr($characters, $i, 1, "UTF8");
      $alphabet .= "<td><a class=\"alphaselect\" href=\"main.php?plug=".
        validate($_GET['plug'])."&amp;filter=".$ch."\">&nbsp;".$ch."&nbsp;</a></td>";

      if ($c++ == $columns) {
        $alphabet .= "</tr>";
        $c        = 0;
      }
    }

    /* Fill remaining cells */
    while ($c++ <= $columns) {
      $alphabet .= "<td>&nbsp;</td>";
    }

    /* Save alphabet */
    $this->alphabet = "<table width='100%'>$alphabet</table>";

    return $this->alphabet;
  }

  /*!
   * \brief Render apply filter
   *
   * \return string
   */
  function renderApply()
  {
    return "<input type='submit' name='apply' value='"._("Apply filter")."'>";
  }

  /*!
   * \brief Render scope
   *
   * \return string
   */
  function renderScope()
  {
    $checked = ($this->scope == "sub"?" checked":"");
    return "<input type='checkbox' id='SCOPE' name='SCOPE' value='1' onClick='document.mainform.submit();'$checked>&nbsp;<LABEL for='SCOPE'>"._("Search in subtrees")."</LABEL>";
  }

  /*!
   * \brief Render
   */
  function render()
  {
    /* If template is not filled, we display nothing */
    if (!isset ($this->xmlData['definition']['template'])) {
      return "";
    }

    $smarty = get_smarty();
    $smarty->assign("ALPHABET", $this->renderAlphabet());
    $smarty->assign("APPLY", $this->renderApply());
    $smarty->assign("SCOPE", $this->renderScope());

    // Load template and replace elementsHtml[]
    foreach ($this->elements as $tag => $element) {
      $htmlCode = "";
      switch ($element['type']) {
        case "textfield":
          $htmlCode = $this->getTextfield($element);
          break;

        case "checkbox":
          $htmlCode = $this->getCheckbox($element);
          break;

        case "combobox":
          $htmlCode = $this->getCombobox($element);
          break;

        default:
          die ("Unknown element type specified!");
      }
      $smarty->assign("$tag", $htmlCode);
    }

    // Try to load template from plugin the folder first...
    $file = get_template_path($this->xmlData['definition']['template'], TRUE);

    // ... if this fails, try to load the file from the theme folder.
    if (!file_exists($file)) {
      $file = get_template_path($this->xmlData['definition']['template']);
    }

    // Load template
    return "<input type='hidden' name='FILTER_PID' value='".$this->pid."'/>".$smarty->fetch($file);
  }

  /*!
   * \brief Query
   */
  function query()
  {
    global $class_mapping;
    $result = array();

    // Return empty list if initial is not set
    if (!$this->initial) {
      $this->initial = TRUE;
      return $result;
    }

    // Go thru all queries and merge results
    foreach ($this->query as $query) {
      if (!isset($query['backend']) || !isset($query['filter']) || !isset($query['attribute'])) {
        die("No backend specified in search config.");
      }

      // Is backend available?
      $backend = "filter".$query['backend'];
      if (!isset($class_mapping["$backend"])) {
        die("Invalid backend specified in search config.");
      }

      // Load filter and attributes
      $filter     = $query['filter'];
      $attributes = $query['attribute'];

      // ObjectClass is required to check permissions later.
      if (!in_array('objectClass', $attributes)) {
        $attributes[] = 'objectClass';
      }

      // Generate final filter
      foreach ($this->elements as $tag => $element) {
        if (!isset($element['set']) || !isset($element['unset'])) {
          continue;
        }

        // Handle converters if present
        if (isset($this->converter[$tag])) {
          preg_match('/([^:]+)::(.*)$/', $this->converter[$tag], $m);
          $e_set    = call_user_func(array($m[1], $m[2]), preg_replace('/\$/', $this->elementValues[$tag], is_array($element['set'])?"":$element['set']));
          $e_unset  = call_user_func(array($m[1], $m[2]), preg_replace('/\$/', $this->elementValues[$tag], is_array($element['unset'])?"":$element['unset']));
        } else {
          $e_set    = is_array($element['set'])?"":$element['set'];
          $e_unset  = is_array($element['unset'])?"":$element['unset'];
        }

        // Do not replace escaped \$ - This is required to be able to search for e.g. windows machines.
        if ($this->elementValues[$tag] == "") {
          $e_unset  = preg_replace('/([^\\\\])\$/', '${1}'.ldap_escape_f($this->elementValues[$tag]), $e_unset);
          $e_unset  = preg_replace('/\\\\\$/', '$', $e_unset);
          $filter   = preg_replace("/\\$$tag/", $e_unset, $filter);
        } else {
          $e_set  = preg_replace('/([^\\\\])\$/', '${1}'.ldap_escape_f($this->elementValues[$tag]), $e_set);
          $e_set  = preg_replace('/\\\\\$/', '$', $e_set);
          $filter = preg_replace("/\\$$tag/", $e_set, $filter);
        }
      }

      // Now call filter method and merge resulting entries.
      $result = array_merge($result, call_user_func(array($backend, 'query'),
            $this, $this->base, $this->scope, $filter, $attributes, $this->category, $this->objectStorage));
    }

    return $result;
  }

  /*!
   * \brief Check if a filter is valid
   *
   * \return TRUE if is valid, FALSE otherwise
   */
  function isValid()
  {
    foreach ($this->elements as $tag => $element) {
      if (isset($element->regex)) {
        if (!preg_match('/'.(string)$element->regex.'/', $this->elementValues[$tag])) {
          return FALSE;
        }
      }
    }
    return TRUE;
  }

  /*!
   * \brief Update
   */
  function update()
  {
    /* React on alphabet links if needed */
    if (isset($_GET['filter'])) {
      $s = mb_substr(validate($_GET['filter']), 0, 1, "UTF8");
      foreach ($this->alphabetElements as $tag) {
        $this->elementValues[$tag] = $s;
      }
    }

    if (isset($_POST['FILTER_PID']) && $_POST['FILTER_PID'] == $this->pid) {
      // Load post values and adapt filter, base and scope accordingly - but
      // only if we didn't get a _GET
      foreach ($this->elements as $tag => $element) {
        if (isset($_POST[$tag])) {
          $this->elementValues[$tag] = validate($_POST[$tag]);
        } else {
          $this->elementValues[$tag] = "";
        }
      }

      // Save scope if needed
      if ($this->scopeMode == "auto") {
        $this->scope = (isset($_POST['SCOPE'])?"sub":"one");
      }
    }

  }

  /*!
   * \brief Get competition list
   *
   * \param string $config
   *
   * \param string $tag
   *
   * \param string $value
   */
  function getCompletitionList($config, $tag, $value = "*")
  {
    global $class_mapping;
    $res = array();

    // Is backend available?
    $backend = "filter".$config['backend'];
    if (!isset($class_mapping["$backend"])) {
      die("Invalid backend specified in search config.");
    }

    // Load filter and attributes
    $filter     = $config['filter'];
    $attributes = $config['attribute'];
    if (!is_array($attributes)) {
      $attributes = array($attributes);
    }

    // ObjectClass is required to check permissions later.
    if (!in_array('objectClass', $attributes)) {
      $attributes[] = 'objectClass';
    }

    // Make filter
    $filter = preg_replace("/\\$$tag/", ldap_escape_f($value), $filter);
    if (isset($config['base']) && isset($config['scope']) && isset($config['category'])) {
      $result = call_user_func(array($backend, 'query'), $this, $config['base'], $config['scope'], $filter, $attributes,
                           $config["category"], $config["objectStorage"]);
    } else {
      $result = call_user_func(array($backend, 'query'), $this, $this->base, $this->scope, $filter, $attributes,
                           $this->category, $this->objectStorage);
    }

    foreach ($result as $entry) {
      foreach ($attributes as $attribute) {
        if (is_array($entry[$attribute])) {
          for ($i = 0; $i < $entry[$attribute]['count']; $i++) {
            if (mb_stristr($entry[$attribute][$i], $value)) {
              $res[] = $entry[$attribute][$i];
            }
          }
        } else {
          $res[] = $entry[$attribute];
        }
      }
    }

    return $res;
  }

  /*!
   * \brief Auto complete
   */
  function processAutocomplete()
  {
    $result = array();

    // Introduce maximum number of entries
    $max = 25;

    foreach ($this->autocompleters as $tag => $config) {
      if (isset($_POST[$tag])) {
        $result = $this->getCompletitionList($config, $tag, $_POST[$tag]);
        $result = array_unique($result);
        asort($result);

        echo '<ul>';
        foreach ($result as $entry) {
          echo '<li>'.mark($_POST[$tag], $entry).'</li>';
          if ($max-- == 0) {
            break;
          }
        }

        echo '</ul>';
      }
    }
  }
}

/*!
 * \brief Sort elements for element length to allow proper replacing later on
 *
 * \param array $a
 *
 * \param array $b
 */
function strlenSort($a, $b)
{
  if (strlen($a['tag']) == strlen($b['tag'])) {
    return 0;
  }
  return (strlen($a['tag']) < strlen($b['tag']) ? -1 : 1);
}

?>
