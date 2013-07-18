<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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
 * \file class_plugin.inc
 * Source code for the class plugin
 */

/*!
 * \brief This is the base class for all plugins.
 *
 * \author  Cajus Pollmeier <pollmeier@gonicus.de>
 * \version 2.00
 * \date    24.07.2003
 *
 * This is the base class for all plugins. It can be used standalone or
 * can be included by the tabs class. All management should be done
 * within this class. Extend your plugins from this class.
 */
class plugin
{
  /*!
   * \brief Reference to parent object
   *
   * This variable is used when the plugin is included in tabs
   * and keeps reference to the tab class. Communication to other
   * tabs is possible by 'name'. So the 'fax' plugin can ask the
   * 'userinfo' plugin for the fax number.
   *
   * \sa tab
   */
  var $parent = NULL;

  /*!
    \brief Configuration container

    Access to global configuration
   */
  var $config = NULL;

  /*!
    \brief Mark plugin as account

    Defines whether this plugin is defined as an account or not.
    This has consequences for the plugin to be saved from tab
    mode. If it is set to 'FALSE' the tab will call the delete
    function, else the save function. Should be set to 'TRUE' if
    the construtor detects a valid LDAP object.

    \sa plugin::plugin()
   */
  var $is_account             = FALSE;
  var $initially_was_account  = FALSE;

  /*!
    \brief Mark plugin as template

    Defines whether we are creating a template or a normal object.
    Has conseqences on the way execute() shows the formular and how
    save() puts the data to LDAP.

    \sa plugin::save() plugin::execute()
   */
  var $is_template    = FALSE;
  var $template_cn    = FALSE;
  var $ignore_account = FALSE;
  var $is_modified    = FALSE;

  /*!
    \brief Represent temporary LDAP data

    This is only used internally.
   */
  var $attrs = array();

  /* Keep set of conflicting plugins */
  var $conflicts = array();

  /*!
    \brief Used standard values

    dn
   */
  var $dn         = "";
  var $acl        = "*none*";
  var $dialog     = FALSE;

  /* attribute list for save action */
  var $attributes       = array();
  var $objectclasses    = array();
  var $is_new           = TRUE;
  var $saved_attributes = array();

  var $acl_base     = "";
  var $acl_category = "";
  var $read_only    = FALSE; // Used when the entry is opened as "readonly" due to locks.

  /* This can be set to render the tabulators in another stylesheet */
  var $pl_notify = FALSE;

  /* Object entry CSN */
  var $entryCSN         = "";
  var $CSN_check_active = FALSE;

  var $selected_edit_values = array();

  /*!
   * \brief plugin constructor
   *
   * If 'dn' is set, the node loads the given 'dn' from LDAP
   *
   * \param $config configuration
   *
   * \param $dn Distinguished name to initialize plugin from
   *
   * \param $object NULL
   *
   * \sa plugin()
   */
  function plugin (&$config, $dn = NULL, $object = NULL)
  {
    /* Configuration is fine, allways */
    $this->config = &$config;
    $this->dn     = $dn;

    // Ensure that we've a valid acl_category set.
    if (empty($this->acl_category)) {
      $tmp = pluglist::pluginInfos(get_class($this));
      if (isset($tmp['plCategory'])) {
        $c = key($tmp['plCategory']);
        if (is_numeric($c)) {
          $c = $tmp['plCategory'][0];
        }
        $this->acl_category = $c."/";
      }
    }

    /* Handle new accounts, don't read information from LDAP */
    if ($this->dn != "new") {
      /* Check if this entry was opened in read only mode */
      if (isset($_POST['open_readonly'])) {
        if (session::global_is_set("LOCK_CACHE")) {
          $cache = &session::get("LOCK_CACHE");
          if (isset($cache['READ_ONLY'][$this->dn])) {
            $this->read_only = TRUE;
          }
        }
      }

      /* Save current dn as acl_base */
      $this->acl_base = $this->dn;

      if ($this->dn === NULL) {
        return;
      }
    }

    /* Get LDAP descriptor */
    if (($this->dn != "new") || ($object !== NULL)) {
      /* Load data to 'attrs' and save 'dn' */
      if ($object !== NULL) {
        $this->attrs = $object->attrs;
      } else {
        $ldap = $this->config->get_ldap_link();
        $ldap->cat($this->dn);
        $this->attrs = $ldap->fetch();
      }

      /* Set the template flag according to the existence of objectClass fdTemplate */
      if (isset($this->attrs['objectClass'])) {
        if (in_array_ics ("fdTemplate", $this->attrs['objectClass'])) {
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, "found", "Template check");
          $this->templateLoadAttrs($this->attrs);
        }
      }

      /* Is Account? */
      if ($this->is_this_account($this->attrs)) {
        $this->is_account = TRUE;
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, "found", "Object check");
      }
    }

    $this->loadAttributes();

    $this->prepareSavedAttributes();

    /* Save initial account state */
    $this->initially_was_account = $this->is_account;
  }

  protected function loadAttributes()
  {
    /* Copy needed attributes */
    foreach ($this->attributes as $val) {
      $found = array_key_ics($val, $this->attrs);
      if ($found != "") {
        $this->$val = $found[0];
      }
    }
  }

  function is_this_account($attrs)
  {
    $found = TRUE;
    foreach ($this->objectclasses as $obj) {
      if (preg_match('/top/i', $obj)) {
        continue;
      }
      if (!isset($attrs['objectClass']) || !in_array_ics ($obj, $attrs['objectClass'])) {
        $found = FALSE;
        break;
      }
    }
    return $found;
  }

  function prepareSavedAttributes()
  {
    /* Prepare saved attributes */
    $this->saved_attributes = $this->attrs;
    foreach ($this->saved_attributes as $index => $value) {
      if (is_numeric($index)) {
        unset($this->saved_attributes[$index]);
        continue;
      }

      if (!in_array_ics($index, $this->attributes) && strcasecmp('objectClass', $index)) {
        unset($this->saved_attributes[$index]);
        continue;
      }

      if (isset($this->saved_attributes[$index][0])) {
        if (!isset($this->saved_attributes[$index]["count"])) {
          $this->saved_attributes[$index]["count"] = count($this->saved_attributes[$index]);
        }
        if ($this->saved_attributes[$index]["count"] == 1) {
          $tmp = $this->saved_attributes[$index][0];
          unset($this->saved_attributes[$index]);
          $this->saved_attributes[$index] = $tmp;
          continue;
        }
      }
      unset($this->saved_attributes["$index"]["count"]);
    }
  }

  protected function templateLoadAttrs($template_attrs)
  {
    $this->is_template = TRUE;
    if ($this->mainTab) {
      $this->template_cn = $template_attrs['cn'][0];
    }
    $this->attrs = self::tpl_template_to_attrs($template_attrs);
  }

  protected function templateSaveAttrs()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cat($this->dn);
    $template_attrs = $ldap->fetch();
    if (!$template_attrs) {
      if (!$this->mainTab) {
        trigger_error('It seems main tab has not been saved.');
      }
      $template_attrs = array(
        'objectClass'     => array('fdTemplate'),
        'fdTemplateField' => array()
      );
    } else {
      unset($template_attrs['dn']);
      unset($template_attrs['fdTemplateField']['count']);
      unset($template_attrs['objectClass']['count']);
      for ($i = 0; $i < $template_attrs['count']; ++$i) { // Remove numeric keys
        unset($template_attrs[$i]);
      }
      unset($template_attrs['count']);
    }
    if ($this->mainTab) {
      $template_attrs['cn'] = $this->template_cn;
    }
    /* First remove all concerned values */
    foreach ($template_attrs['fdTemplateField'] as $key => $value) {
      preg_match('/^([^:]+):(.*)$/', $value, $m);
      if (isset($this->attrs[$m[1]])) {
        unset($template_attrs['fdTemplateField'][$key]);
      }
    }
    /* Then insert non-empty values */
    foreach ($this->attrs as $key => $value) {
      if (is_array($value)) {
        foreach ($value as $v) {
          if ($value == "") {
            continue;
          }
          $template_attrs['fdTemplateField'][] = $key.':'.$v;
        }
      } else {
        if ($value == "") {
          continue;
        }
        $template_attrs['fdTemplateField'][] = $key.':'.$value;
      }
    }
    sort($template_attrs['fdTemplateField']);
    return $template_attrs;
  }

  /*!
   * \brief This function is called on the copied object to set its dn to where it will be saved
   */
  function resetCopyInfos()
  {
    $this->dn       = 'new';
    $this->orig_dn  = $this->dn;

    $this->saved_attributes       = array();
    $this->initially_was_account  = FALSE;

    $this->postCopyHook();
  }


  /*!
   * \brief Generates the html output for this node
   */
  function execute()
  {
    /* This one is empty currently. Fabian - please fill in the docu code */
    session::global_set('current_class_for_help', get_class($this));

    /* Reset Lock message POST/GET check array, to prevent perg_match errors*/
    session::set('LOCK_VARS_TO_USE', array());
    session::set('LOCK_VARS_USED_GET', array());
    session::set('LOCK_VARS_USED_POST', array());
    session::set('LOCK_VARS_USED_REQUEST', array());
  }

  /*!
   * \brief Removes object from parent
   */
  function remove_from_parent()
  {
    /* include global link_info */
    $ldap = $this->config->get_ldap_link();

    /* Get current objectClasses in order to add the required ones */
    $ldap->cat($this->dn);
    $tmp  = $ldap->fetch ();
    $oc   = array();
    if (isset($tmp['objectClass'])) {
      $oc = $tmp['objectClass'];
      unset($oc['count']);
    }

    /* Remove objectClasses from entry */
    $ldap->cd($this->dn);
    $this->attrs                = array();
    $this->attrs['objectClass'] = array_remove_entries_ics($this->objectclasses, $oc);

    /* Unset attributes from entry */
    foreach ($this->attributes as $val) {
      $this->attrs["$val"] = array();
    }

    /* Do not write in plugin base class, this must be done by
       children, since there are normally additional attribs,
       lists, etc. */
    if ($this->initially_was_account) {
      $this->handle_pre_events('remove');
    }
  }


  /*!
   * \brief Save HTML posted data to object
   */
  function save_object()
  {
    /* Update entry CSN if it is empty. */
    if (empty($this->entryCSN) && $this->CSN_check_active) {
      $this->entryCSN = getEntryCSN($this->dn);
    }

    /* Save values to object */
    foreach ($this->attributes as $val) {
      if ($this->acl_is_writeable($val) && isset ($_POST["$val"])) {
        /* Check for modifications */
        $data = $_POST["$val"];

        if ($this->$val != $data) {
          $this->is_modified = TRUE;
        }

        $this->$val = $data;

        /* Okay, how can I explain this fix ...
         * In firefox, disabled option fields aren't selectable ... but in IE you can select these fileds.
         * So IE posts these 'unselectable' option, with value = chr(194)
         * chr(194) seems to be the &nbsp; in between the ...option>&nbsp;</option.. because there is no value=".." specified in these option fields
         * This &nbsp; was added for W3c compliance, but now causes these ... ldap errors ...
         * So we set these Fields to ""; a normal empty string, and we can check these values in plugin::check() again ...
         */
        if (isset($data[0]) && ($data[0] == chr(194))) {
          $data = "";
        }
        $this->$val = $data;
      }
    }
  }


  /*!
   * \brief Save data to LDAP, depending on is_account we save or delete
   */
  function save()
  {
    /* include global link_info */
    $ldap = $this->config->get_ldap_link();

    /* Save all plugins */
    $this->entryCSN = "";

    /* Start with empty array */
    $this->attrs = array();

    /* Get current objectClasses in order to add the required ones */
    $ldap->cat($this->dn);

    $tmp = $ldap->fetch ();

    $oc = array();
    if (isset($tmp['objectClass'])) {
      $oc = $tmp["objectClass"];
      unset($oc['count']);
      $this->is_new = FALSE;
    } else {
      $this->is_new = TRUE;
    }

    /* Load (minimum) attributes, add missing ones */
    $this->attrs['objectClass'] = array_merge_unique($oc, $this->objectclasses);

    /* Copy standard attributes */
    foreach ($this->attributes as $val) {
      if ($this->$val != "") {
        $this->attrs["$val"] = $this->$val;
      } elseif (!$this->is_new) {
        $this->attrs["$val"] = array();
      }
    }

    if ($this->is_new) {
      $this->handle_pre_events('add');
    } else {
      $this->handle_pre_events('modify');
    }
  }

  /*!
   * \brief Remove attributes, empty arrays, arrays
   * single attributes that do not differ
   */
  function cleanup()
  {
    foreach ($this->attrs as $index => $value) {

      /* Convert arrays with one element to non arrays, if the saved
         attributes are no array, too */
      if (is_array($this->attrs[$index]) &&
          count ($this->attrs[$index]) == 1 &&
          isset($this->saved_attributes[$index]) &&
          !is_array($this->saved_attributes[$index])) {
        $this->attrs[$index] = $this->attrs[$index][0];
      }

      /* Remove emtpy arrays if they do not differ */
      if (is_array($this->attrs[$index]) &&
          count($this->attrs[$index]) == 0 &&
          !isset($this->saved_attributes[$index])) {
        unset ($this->attrs[$index]);
        continue;
      }

      /* Remove single attributes that do not differ */
      if (!is_array($this->attrs[$index]) &&
          isset($this->saved_attributes[$index]) &&
          !is_array($this->saved_attributes[$index]) &&
          $this->attrs[$index] == $this->saved_attributes[$index]) {
        unset ($this->attrs[$index]);
        continue;
      }

      /* Remove arrays that do not differ */
      if (is_array($this->attrs[$index]) &&
          isset($this->saved_attributes[$index]) &&
          is_array($this->saved_attributes[$index])) {
        if (!array_differs($this->attrs[$index], $this->saved_attributes[$index])) {
          unset ($this->attrs[$index]);
          continue;
        }
      }
    }

    /* Update saved attributes and ensure that next cleanups will be successful too */
    foreach ($this->attrs as $name => $value) {
      $this->saved_attributes[$name] = $value;
    }
  }

  /*!
   * \brief Check formular input
   */
  function check()
  {
    $message = array();

    /* Skip if we've no config object */
    if (!isset($this->config) || !is_object($this->config)) {
      return $message;
    }

    self::callHook($this, 'CHECK', array(), $returnOutput);
    if (!empty($returnOutput)) {
      $message[] = join("\n", $returnOutput);
    }

    /* Check entryCSN */
    if ($this->CSN_check_active) {
      $current_csn = getEntryCSN($this->dn);
      if (($current_csn != $this->entryCSN) && !empty($this->entryCSN) && !empty($current_csn)) {
        $this->entryCSN = $current_csn;
        $message[] = _("The object has changed since opened in FusionDirectory. All changes that may be done by others get lost if you save this entry!");
      }
    }
    return $message;
  }

  /*
   * \brief Adapt from template, using 'dn'
   *
   * \param string $dn The DN
   *
   * \param array $skip A new array
   */
  function adapt_from_template($attrs, $skip = array())
  {
    /* Load requested 'dn' to 'attrs' */
    $this->attrs = $attrs;

    /* Walk through attributes */
    foreach ($this->attributes as $val) {
      /* Skip the ones in skip list */
      if (in_array($val, $skip)) {
        continue;
      }

      if (isset($this->attrs["$val"][0])) {
        $this->$val = $this->attrs["$val"][0];
      }
    }

    /* Is Account? */
    $this->is_account = $this->is_this_account($this->attrs);
  }

  static function tpl_fetch_template($dn)
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cat($dn);
    $attrs    = $ldap->fetch();
    $attrs    = self::tpl_template_to_attrs($attrs);
    $depends  = self::tpl_attrs_depends($attrs);
    $attrs    = self::tpl_sort_attrs($attrs, $depends);
    return array($attrs, $depends);
  }

  static function tpl_template_to_attrs($template_attrs)
  {
    /* Translate template attrs into $attrs as if taken from LDAP */
    unset($template_attrs['fdTemplateField']['count']);
    sort($template_attrs['fdTemplateField']);
    $attrs = array();
    foreach ($template_attrs['fdTemplateField'] as $field) {
      preg_match('/^([^:]+):(.*)$/', $field, $m);
      if (isset($attrs[$m[1]])) {
        $attrs[$m[1]][] = $m[2];
        $attrs[$m[1]]['count']++;
      } else {
        $attrs[$m[1]]           = array($m[2]);
        $attrs[$m[1]]['count']  = 1;
      }
    }
    return $attrs;
  }

  /* Apply a modifier
   * Returns an array of possible values */
  static function tpl_apply_modifier($m, $args, $str)
  {
    if (is_array($str) && (strtolower($m) == $m)) {
      /* $str is an array and $m is lowercase, so it's a string modifier */
      $str = $str[0];
    }
    switch ($m) {
      case 'F': // First
        return array($str[0]);
      case 'L': // Last
        return array(end($str));
      case 'J': // Join
        if (isset($args[0])) {
          return array(join($args[0], $str));
        } else {
          return array(join($str));
        }
      case 'u': // uppercase
        return array(mb_strtoupper($str, 'UTF-8'));
      case 'l': // lowercase
        return array(mb_strtolower($str, 'UTF-8'));
      case 'a': // remove accent
        $str = htmlentities($str, ENT_NOQUOTES, 'UTF-8');

        $str = preg_replace('#&([A-za-z])(?:acute|cedil|circ|grave|orn|ring|slash|th|tilde|uml);#', '\1', $str);
        $str = preg_replace('#&([A-za-z]{2})(?:lig);#', '\1', $str); // handle ligatures
        return array(preg_replace('#&[^;]+;#', '', $str)); // delete unhandled characters
      case 's': // substring
        if (count($args) < 1) {
          trigger_error("Missing 's' substr modifier parameter");
        }
        if (count($args) < 2) {
          array_unshift($args, 0);
        }
        if (preg_match('/^(\d+)-(\d+)$/', $args[1], $m)) {
          $res = array();
          for ($i = $m[1];$i < $m[2]; ++$i) {
            $res[] = substr($str, $args[0], $i);
          }
          return array_unique($res);
        } else {
          return array(substr($str, $args[0], $args[1]));
        }
      default:
        trigger_error("Unkown modifier '$m'");
        return array($str);
    }
  }

  static function tpl_parse_mask($mask, $attrs)
  {
    if ($mask == "|") {
      return array("%");
    }
    $modifiers = '';
    if (preg_match('/^([^|]+)\|/', $mask, $m)) {
      $modifiers = $m[1];
      $mask = substr($mask, strlen($m[0]));
    }
    if (isset($attrs[$mask])) {
      $result = array($attrs[$mask]);
      unset($result[0]['count']);
    } else {
      trigger_error("'$mask' was not found in attributes");
      $result = array('');
    }
    $len    = strlen($modifiers);
    for ($i = 0; $i < $len; ++$i) {
      $args     = array();
      $modifier = $modifiers[$i];
      if (preg_match('/^\[([^\]]+)\].*$/', substr($modifiers, $i + 1), $m)) {
        /* get modifier args */
        $args = explode(',', $m[1]);
        $i += strlen($m[1]) + 2;
      }
      $result_tmp = array();
      foreach ($result as $r) {
        $result_tmp = array_merge($result_tmp, self::tpl_apply_modifier($modifier, $args, $r));
      }
      $result = $result_tmp;
    }
    foreach ($result as &$r) { // Array that were not converted by a modifier into a string are now converted to strings
      if (is_array($r)) {
        $r = $r[0];
      }
    }
    unset($r);
    return $result;
  }

  static function tpl_depends_of (&$cache, $depends, $key, $forbidden = NULL)
  {
    if (isset($cache[$key])) {
      return $cache[$key];
    }
    if ($forbidden === NULL) {
      $forbidden = $key;
    } elseif ($forbidden == $key) {
      die('Error : recursive dependency');
    }
    $array        =
      array_map(
        function ($a) use (&$cache, $depends, $forbidden)
        {
          return plugin::tpl_depends_of ($cache, $depends, $a, $forbidden);
        },
        $depends[$key]
      );
    $array[]      = $depends[$key];
    $cache[$key]  = array_unique(call_user_func_array('array_merge_recursive', $array));
    return $cache[$key];
  }

  static function tpl_attrs_depends($attrs)
  {
    /* Compute dependencies of each attr */
    $depends = array();
    foreach ($attrs as $key => $values) {
      $depends[$key] = array();
      if (!is_array($values))  {
        $values = array($values);
      }
      unset ($values['count']);
      foreach ($values as $value) {
        $offset = 0;
        while (preg_match('/%([^%\|]+\|)?([^%]+)%/', $value, $m, PREG_OFFSET_CAPTURE, $offset)) {
          $offset = $m[0][1] + strlen($m[0][0]);
          $depends[$key][] = $m[2][0];
          if (!isset($attrs[$m[2][0]])) { // Dependency which has no value might be missing
            $attrs[$m[2][0]]    = array();
            $depends[$m[2][0]]  = array();
          }
        }
      }
    }
    /* Flattens dependencies */
    $flatdepends = array();
    foreach ($depends as $key => $value) {
      self::tpl_depends_of($flatdepends, $depends, $key);
    }
    return $flatdepends;
  }

  static function tpl_sort_attrs($attrs, $flatdepends)
  {
    /* Sort attrs depending of dependencies */
    uksort($attrs, function ($k1, $k2) use ($flatdepends) {
      if (in_array($k1, $flatdepends[$k2])) {
        return -1;
      } elseif (in_array($k2, $flatdepends[$k1])) {
        return 1;
      } else { // When no direct dependency, we sort by number of dependencies
        $c1 = count($flatdepends[$k1]);
        $c2 = count($flatdepends[$k2]);
        if ($c1 == $c2) {
            return 0;
        }
        return (($c1 < $c2) ? -1 : 1);
      }
    });
    return $attrs;
  }

  /*! Brief Return attrs needed before applying template
   *
   * return an array of attributes which are needed by the template
   */
  static function tpl_needed_attrs($attrs, $flatdepends)
  {
    $needed = array();
    $dependencies = array_unique(call_user_func_array('array_merge', $flatdepends));
    foreach ($dependencies as $attr) {
      if (empty($flatdepends[$attr])) {
        $needed[] = $attr;
      }
    }
    return $needed;
  }


  /*! Brief Parse attrs template masks
   *
   * return an array with the final values of attributes
   */
  static function tpl_parse_attrs($attrs)
  {
    foreach ($attrs as &$attr) {
      foreach ($attr as $key => &$string) {
        if (!is_numeric($key)) {
          continue;
        }
        $offset = 0;
        while (preg_match('/%([^%]+)%/', $string, $m, PREG_OFFSET_CAPTURE, $offset)) {
          $replace  = self::tpl_parse_mask($m[1][0], $attrs);
          $replace  = $replace[0];
          $string   = substr_replace($string, $replace, $m[0][1], strlen($m[0][0]));
          $offset   = $m[0][1] + strlen($replace);
        }
      }
      unset($string);
    }
    unset($attr);
    return $attrs;
  }

  /*!
   * \brief Show header message for tab dialogs
   *
   * \param string $button_text The button text
   *
   * \param string $text The text
   *
   * \param boolean $disabled FALSE
   */
  function show_enable_header($button_text, $text, $disabled = FALSE, $name = "modify_state")
  {
    return $this->show_header($button_text, $text, FALSE, $disabled, $name);
  }


  /*!
   * \brief Show header message for tab dialogs
   *
   * \param string $button_text The button text
   *
   * \param string $text The text
   *
   * \param boolean $disabled FALSE
   */
  function show_disable_header($button_text, $text, $disabled = FALSE, $name = "modify_state")
  {
    return $this->show_header($button_text, $text, TRUE, $disabled, $name);
  }


  /*!
   * \brief Show header message for tab dialogs
   *
   * \param string $button_text The button text
   *
   * \param string $text The text
   *
   * \param boolean $plugin_enabled
   *
   * \param boolean $button_disabled FALSE
   */
  function show_header($button_text, $text, $plugin_enabled, $button_disabled = FALSE, $name = "modify_state")
  {
    if (($button_disabled) || ((!$this->acl_is_createable() && !$plugin_enabled) || (!$this->acl_is_removeable() && $plugin_enabled))) {
        $state = "disabled=\"disabled\"";
    } else {
        $state = "";
    }
    $display = "<div width=\"100%\"><p><b>$text</b><br/>\n";
    $display .= "<input type=\"submit\" value=\"$button_text\" name=\"$name\" ".$state.
      "></p></div><hr class=\"separator\"/>";

    return $display;
  }

  /*!
   * \brief Executes a command after an object has been copied
   */
  function postCopyHook()
  {
  }

  /*!
   * \brief Create unique DN
   *
   * \param string $attribute
   *
   * \param string $base
   */
  function create_unique_dn($attribute, $base)
  {
    $ldap = $this->config->get_ldap_link();
    $base = preg_replace('/^,*/', '', $base);

    /* Try to use plain entry first */
    $dn = "$attribute=".$this->$attribute.",$base";
    if ($dn == $this->orig_dn) {
      return $dn;
    }
    $ldap->cat($dn, array('dn'));
    if (!$ldap->fetch()) {
      return $dn;
    }

    /* Look for additional attributes */
    foreach ($this->attributes as $attr) {
      if ($attr == $attribute || $this->$attr == "" || is_array($this->$attr)) {
        continue;
      }

      $dn = "$attribute=".$this->$attribute."+$attr=".$this->$attr.",$base";
      if ($dn == $this->orig_dn) {
        return $dn;
      }
      $ldap->cat($dn, array('dn'));
      if (!$ldap->fetch()) {
        return $dn;
      }
    }

    /* None found */
    return 'none';
  }

  /*!
   * \brief ldap rebind
   *
   * \param string $ldap
   *
   * \param string $referral
   */
  function rebind($ldap, $referral)
  {
    $credentials = LDAP::get_credentials($referral, $this->config->current['REFERRAL']);
    if (ldap_bind($ldap, $credentials['ADMIN'], $this->config->get_credentials($credentials['PASSWORD']))) {
      $this->error      = "Success";
      $this->hascon     = TRUE;
      $this->reconnect  = TRUE;
      return 0;
    } else {
      $this->error = "Could not bind to " . $credentials['ADMIN'];
      return NULL;
    }
  }

  /*
   * \brief Recursively copy ldap object
   *
   * \param string $src_dn The DN source
   *
   * \param string $dst_dn The DN destination
   */
  function _copy($src_dn, $dst_dn)
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cat($src_dn);
    $attrs = $ldap->fetch();

    /* Grummble. This really sucks. PHP ldap doesn't support rdn stuff. */
    $ds = ldap_connect($this->config->current['SERVER']);
    ldap_set_option($ds, LDAP_OPT_PROTOCOL_VERSION, 3);
    if (function_exists("ldap_set_rebind_proc") && isset($this->config->current['REFERRAL'])) {
      ldap_set_rebind_proc($ds, array(&$this, "rebind"));
    }

    $pwd  = $this->config->get_credentials($this->config->current['ADMINPASSWORD']);
    $r    = ldap_bind($ds, $this->config->current['ADMINDN'], $pwd);

    /* Fill data from LDAP */
    $new = array();
    if ($sr = ldap_read($ds, LDAP::fix($src_dn), "objectClass=*")) {
      if ($ei = ldap_first_entry($ds, $sr)) {
        foreach ($attrs as $attr => $val) {
          if ($info = @ldap_get_values_len($ds, $ei, $attr)) {
            for ($i = 0; $i < $info['count']; $i++) {
              if ($info['count'] == 1) {
                $new[$attr] = $info[$i];
              } else {
                $new[$attr][] = $info[$i];
              }
            }
          }
        }
      }
    }

    /* close connexion */
    ldap_unbind($ds);

    /* Adapt naming attribute */
    $dst_name       = preg_replace("/^([^=]+)=.*$/", "\\1", $dst_dn);
    $dst_val        = preg_replace("/^[^=]+=([^,+]+).*,.*$/", "\\1", $dst_dn);
    $new[$dst_name] = LDAP::fix($dst_val);

    /* Check if this is a department.
     * If it is a dep. && there is a , override in his ou
     *  change \2C to , again, else this entry can't be saved ...
     */
    if (isset($new['ou']) && preg_match("/\\,/", $new['ou'])) {
      $new['ou'] = str_replace("\\\\,", ",", $new['ou']);
    }

    /* Save copy */
    $ldap->connect();
    $ldap->cd($this->config->current['BASE']);

    $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $dst_dn));

    /* FAIvariable=.../..., cn=..
        could not be saved, because the attribute FAIvariable was different to
        the dn FAIvariable=..., cn=... */

    if (!is_array($new['objectClass'])) {
      $new['objectClass'] = array($new['objectClass']);
    }

    if (in_array_ics("FAIdebconfInfo", $new['objectClass'])) {
      $new['FAIvariable'] = $ldap->fix($new['FAIvariable']);
    }
    $ldap->cd($dst_dn);
    $ldap->add($new);

    if (!$ldap->success()) {
      trigger_error("Trying to save $dst_dn failed.",
          E_USER_WARNING);
      return FALSE;
    }
    return TRUE;
  }

  /*
   * \brief Copy ldap object.
   * This is a workaround function
   *
   * \param string $src_dn The DN source
   *
   * \param string $dst_dn The DN destination
   */
  function copy($src_dn, $dst_dn)
  {
    /* Rename dn in possible object groups */
    $ldap = $this->config->get_ldap_link();
    $ldap->search('(&(objectClass=gosaGroupOfNames)(member='.@LDAP::prepare4filter($src_dn).'))',
        array('cn'));
    while ($attrs = $ldap->fetch()) {
      $og = new ogroup($this->config, $ldap->getDN());
      unset($og->member[$src_dn]);
      $og->member[$dst_dn] = $dst_dn;
      $og->save();
    }

    $ldap->cat($dst_dn);
    $attrs = $ldap->fetch();
    if (count($attrs)) {
      trigger_error("Trying to overwrite ".LDAP::fix($dst_dn).", which already exists.",
          E_USER_WARNING);
      return FALSE;
    }

    $ldap->cat($src_dn);
    $attrs = $ldap->fetch();
    if (!count($attrs)) {
      trigger_error("Trying to move ".LDAP::fix($src_dn).", which does not seem to exist.",
          E_USER_WARNING);
      return FALSE;
    }

    $ldap->cd($src_dn);
    $ldap->search("objectClass=*", array("dn"));
    while ($attrs = $ldap->fetch()) {
      $src = $attrs['dn'];
      $dst = preg_replace("/".preg_quote($src_dn, '/')."$/", $dst_dn, $attrs['dn']);
      $this->_copy($src, $dst);
    }
    return TRUE;
  }


  /*!
   * \brief  Rename/Move a given src_dn to the given dest_dn
   *
   * Move a given ldap object indentified by $src_dn to the
   * given destination $dst_dn
   *
   * - Ensure that all references are updated (ogroups)
   * - Update ACLs
   * - Update accessTo
   *
   * \param  string  $src_dn the source DN.
   *
   * \param  string  $dst_dn the destination DN.
   *
   * \return boolean TRUE on success else FALSE.
   */
  private function rename($src_dn, $dst_dn)
  {
    $start = microtime(1);

    /* Try to move the source entry to the destination position */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->create_missing_trees(preg_replace("/^[^,]+,/", "", $dst_dn));
    if (!$ldap->rename_dn($src_dn, $dst_dn)) {
      new log("debug", "Ldap Protocol v3 implementation error, ldap_rename failed, falling back to manual copy.",
              "FROM: $src_dn  -- TO: $dst_dn", array(), $ldap->get_error());
      @DEBUG(DEBUG_LDAP, __LINE__, __FUNCTION__, __FILE__, "Rename failed FROM: $src_dn  -- TO:  $dst_dn",
          "Ldap Protocol v3 implementation error, falling back to maunal method.");
      return FALSE;
    }

    return TRUE;
  }


   /*!
    * \brief Move ldap entries from one place to another
    *
    * \param  string  $src_dn the source DN.
    *
    * \param  string  $dst_dn the destination DN.
    */
  function move($src_dn, $dst_dn)
  {
    /* Do not move if only upper- lowercase has changed */
    if (strtolower($src_dn) == strtolower($dst_dn)) {
      return TRUE;
    }

    /* Try to move with ldap routines, if this was not successfull
        fall back to the old style copy & remove method
     */
    if (!$this->rename($src_dn, $dst_dn)) {
      /* Copy source to destination */
      if (!$this->copy($src_dn, $dst_dn)) {
        return FALSE;
      }

      /* Delete source */
      $ldap = $this->config->get_ldap_link();
      $ldap->rmdir_recursive($src_dn);
      if (!$ldap->success()) {
        trigger_error("Trying to delete $src_dn failed.", E_USER_WARNING);
        return FALSE;
      }
    }

    /* Get list of users,groups and roles within this tree,
        maybe we have to update ACL references.
     */
    $leaf_objs = get_list("(|(objectClass=posixGroup)(objectClass=gosaAccount)(objectClass=gosaRole))", array("all"), $dst_dn,
                          array("dn","objectClass"), GL_SUBSEARCH | GL_NO_ACL_CHECK);
    foreach ($leaf_objs as $obj) {
      $new_dn = $obj['dn'];
      $old_dn = preg_replace("/".preg_quote(LDAP::convert($dst_dn), '/')."$/i", $src_dn, LDAP::convert($new_dn));
      $this->update_acls($old_dn, $new_dn);
    }

    // Migrate objectgroups if needed
    $ogroups = get_sub_list("(&(objectClass=gosaGroupOfNames)(member=".LDAP::prepare4filter(LDAP::fix($src_dn))."))",
                            "ogroup", array(get_ou("ogroupRDN")), $this->config->current['BASE'],
                            array("dn"), GL_SUBSEARCH | GL_NO_ACL_CHECK);

    // Walk through all objectGroups
    foreach ($ogroups as $ogroup) {
      // Migrate old to new dn
      $o_ogroup = new ogroup($this->config, $ogroup['dn']);
      if (isset($o_ogroup->member[$src_dn])) {
        unset($o_ogroup->member[$src_dn]);
      }
      $o_ogroup->member[$dst_dn] = $dst_dn;

      // Save object group
      $o_ogroup->save();
    }

    // Migrate rfc groups if needed
    $groups = get_sub_list("(&(objectClass=posixGroup)(member=".LDAP::prepare4filter(LDAP::fix($src_dn))."))",
                            "group", array(get_ou("groupRDN")), $this->config->current['BASE'], array("dn"),
                            GL_SUBSEARCH | GL_NO_ACL_CHECK);

    // Walk through all POSIX groups
    foreach ($groups as $group) {
      // Migrate old to new dn
      $o_group = new group($this->config, $group['dn']);
      $o_group->save();
    }

    if (class_available('roleGeneric')) {
      /* Update roles to use the new entry dn */
      $roles = get_sub_list("(&(objectClass=organizationalRole)(roleOccupant=".LDAP::prepare4filter(LDAP::fix($src_dn))."))", "roles", array(get_ou("roleRDN")), $this->config->current['BASE'], array("dn"), GL_SUBSEARCH | GL_NO_ACL_CHECK);

      // Walk through all roles
      foreach ($roles as $role) {
        $role = new roleGeneric($this->config, $role['dn']);
        $key  = array_search($src_dn, $role->roleOccupant);
        if ($key !== FALSE) {
          $role->roleOccupant[$key] = $dst_dn;
          $role->save();
        }
      }
    }

    // Update 'manager' attributes from gosaDepartment and inetOrgPerson
    $filter     = '(&(objectClass=inetOrgPerson)(manager='.LDAP::prepare4filter(LDAP::fix($src_dn)).'))';
    $filter     = '(|'.$filter.'(&(objectClass=gosaDepartment)(manager='.LDAP::prepare4filter(LDAP::fix($src_dn)).')))';
    $leaf_deps  = get_list($filter, array('all'), $this->config->current['BASE'],
                            array('manager','dn','objectClass'), GL_SUBSEARCH | GL_NO_ACL_CHECK);
    foreach ($leaf_deps as $entry) {
      $update = array('manager' => $dst_dn);
      $ldap->cd($entry['dn']);
      $ldap->modify($update);
      if (!$ldap->success()) {
        trigger_error(sprintf("Failed to update manager for '%s', error was '%s'", $entry['dn'], $ldap->get_error()));
      }
    }

    /* Check if there are gosa departments moved.
       If there were deps moved, the force reload of config->deps.
     */
    $leaf_deps = get_list("(objectClass=gosaDepartment)", array("all"), $dst_dn,
                            array("dn","objectClass"), GL_SUBSEARCH | GL_NO_ACL_CHECK);

    if (count($leaf_deps)) {
      $this->config->get_departments();
      $this->config->make_idepartments();
      session::global_set("config", $this->config);
      $ui = get_userinfo();
      $ui->reset_acl_cache();
    }

    return TRUE;
  }


  /* \brief Move/Rename complete trees
   *
   * \param  string  $src_dn the source DN.
   *
   * \param  string  $dst_dn the destination DN.
   */
  function recursive_move($src_dn, $dst_dn)
  {
    /* Check if the destination entry exists */
    $ldap = $this->config->get_ldap_link();

    /* Check if destination exists - abort */
    $ldap->cat($dst_dn, array('dn'));
    if ($ldap->fetch()) {
      trigger_error("recursive_move $dst_dn already exists.",
          E_USER_WARNING);
      return FALSE;
    }

    $this->copy($src_dn, $dst_dn);

    /* Remove src_dn */
    $ldap->cd($src_dn);
    $ldap->recursive_remove($src_dn);
    return TRUE;
  }

  /*! \brief Forward command execution requests
   *         to the pre/post hook execution method.
   *
   * \param  string  $when must be PRE or POST
   *
   * \param  string  $mode add, remove or modify
   *
   * \param  array  $addAttrs
   */
  protected function handle_hooks($when, $mode, $addAttrs = array())
  {
    switch ($mode) {
      case 'add':
        plugin::callHook($this, $when.'CREATE', $addAttrs);
        break;

      case 'modify':
        plugin::callHook($this, $when.'MODIFY', $addAttrs);
        break;

      case 'remove':
        plugin::callHook($this, $when.'REMOVE', $addAttrs);
        break;

      default:
        trigger_error(sprintf('Invalid %s event type given %s! Valid types are [add,modify,remove].', strtolower($when), $mode));
        break;
    }
  }

  /*! \brief Forward command execution requests
   *         to the post hook execution method.
   */
  function handle_post_events($mode, $addAttrs = array())
  {
    return $this->handle_hooks('POST', $mode, $addAttrs);
  }

  /*!
   *  \brief Forward command execution requests
   *         to the pre hook execution method.
   */
  function handle_pre_events($mode, $addAttrs = array())
  {
    return $this->handle_hooks('PRE', $mode, $addAttrs);
  }


  static protected function matchHookVars ($command, $addAttrs)
  {
    // Sort attributes by length, ensures correct replacement
    $tmp = array();
    foreach ($addAttrs as $name => $value) {
      $tmp[$name] = strlen($name);
    }
    arsort($tmp);

    // Now replace the placeholder
    foreach ($tmp as $name => $len) {
      $command  = str_replace("%$name%", escapeshellarg($addAttrs[$name]), $command);
    }
    return $command;
  }

  /*!
   * \brief    Calls external hooks which are defined for this plugin (fusiondirectory.conf)
   *           Replaces placeholder by class values of this plugin instance.
   *       Allows to a add special replacements.
   */
  static function callHook($plugin, $cmd, $addAttrs = array(), &$returnOutput = array(), &$returnCode = NULL)
  {
    global $config;
    $command = $config->search(get_class($plugin), $cmd, array('menu','tabs','hooks'));

    if ($command != "") {
      // Walk trough attributes list and add the plugins attributes.
      foreach ($plugin->attributes as $attr) {
        if (is_array($plugin->$attr)) {
          $addAttrs[$attr] = join('|', $plugin->$attr);
        } else {
          $addAttrs[$attr] = $plugin->$attr;
        }
      }

      $ui = get_userinfo();

      $addAttrs['callerDN'] = $ui->dn;
      $addAttrs['dn']       = $plugin->dn;
      $addAttrs['location'] = $config->current['NAME'];

      $command = self::matchHookVars($command, $addAttrs);

      // If there are still some %.. in our command, try to fill these with some other tab vars
      if (preg_match("/%/", $command)) {
        $addAttrs = array();
        if (isset($plugin->parent->by_object)) {
          foreach ($plugin->parent->by_object as $object) {
            foreach ($object->attributes as $attr) {
              if (!isset($addAttrs[$attr])) {
                if (is_array($object->$attr)) {
                  $addAttrs[$attr] = join('|', $object->$attr);
                } else {
                  $addAttrs[$attr] = $object->$attr;
                }
              }
            }
          }
        }

        $command = self::matchHookVars($command, $addAttrs);
      }

      // If there are still some %.. in our command, try to fill these with some other class vars
      if (preg_match("/%/", $command)) {
        $addAttrs = array();
        $attrs = get_object_vars($plugin);
        foreach ($attrs as $name => $value) {
          if (is_array($value)) {
            $s = "";
            foreach ($value as $val) {
              if (is_string($val) || is_int($val) || is_float($val) || is_bool($val)) {
                $s .= $val.'|';
              }
            }
            $value = trim($s, '|');
          }
          if (!is_string($value) && !is_int($value) && !is_float($value) && !is_bool($value)) {
            continue;
          }
          $addAttrs[$name] = $value;
        }
        $command = self::matchHookVars($command, $addAttrs);
      }

      @DEBUG(DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, "Execute");
      exec($command, $arr, $returnCode);
      $returnOutput = $arr;

      if ($returnCode != 0) {
        $str = implode("\n", $arr);
        @DEBUG(DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, "Execution failed code: ".$returnCode);
        $message = msgPool::cmdexecfailed($cmd, $command, get_class($plugin));
        if (!empty($str)) {
          $message .= "Result: ".$str;
        }
        msg_dialog::display(_("Error"), $message, ERROR_DIALOG);
      } elseif (is_array($arr)) {
        $str = implode("\n", $arr);
        @DEBUG(DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, "Result: ".$str);
      }
    }
  }

  /*! \brief Test for removability of the object
   *
   * Allows testing of conditions for removal of object. If removal should be aborted
   * the function needs to remove an error message.
   */
  function allow_remove()
  {
    $reason = "";
    return $reason;
  }

  /*!
   * \brief Return plugin informations for acl handling
   *
   * \return an array
   */
  static function plInfo()
  {
    return array();
  }

  /*!
   * \brief Set acl base
   *
   * \param string $base
   */
  function set_acl_base($base)
  {
    $this->acl_base = $base;
  }

  /*!
   * \brief Set acl category
   *
   * \param string $category
   */
  function set_acl_category($category)
  {
    $this->acl_category = "$category/";
  }

  /*! \brief Can we write the acl */
  function acl_is_writeable($attribute, $skip_write = FALSE)
  {
    if ($this->read_only) {
      return FALSE;
    }
    $ui = get_userinfo();
    return preg_match('/w/', $ui->get_permissions($this->acl_base, $this->acl_category.get_class($this), $attribute, $skip_write));
  }

  /*!
   * \brief Can we read the acl
   *
   * \param string $attribute
   */
  function acl_is_readable($attribute)
  {
    $ui = get_userinfo();
    return preg_match('/r/', $ui->get_permissions($this->acl_base, $this->acl_category.get_class($this), $attribute));
  }

  /*!
   * \brief Can we create the acl
   *
   * \param string $base Empty string
   */
  function acl_is_createable($base = "")
  {
    if ($this->read_only) {
      return FALSE;
    }
    $ui = get_userinfo();
    if ($base == "") {
      $base = $this->acl_base;
    }
    return preg_match('/c/', $ui->get_permissions($base, $this->acl_category.get_class($this), '0'));
  }

  /*!
   * \brief Can we remove the acl
   *
   * \param string $base Empty string
   */
  function acl_is_removeable($base = "")
  {
    if ($this->read_only) {
      return FALSE;
    }
    $ui = get_userinfo();
    if ($base == "") {
      $base = $this->acl_base;
    }
    return preg_match('/d/', $ui->get_permissions($base, $this->acl_category.get_class($this), '0'));
  }

  /*!
   * \brief Can we move the acl
   *
   * \param string $base Empty string
   */
  function acl_is_moveable($base = "")
  {
    if ($this->read_only) {
      return FALSE;
    }
    $ui = get_userinfo();
    if ($base == "") {
      $base = $this->acl_base;
    }
    return preg_match('/m/', $ui->get_permissions($base, $this->acl_category.get_class($this), '0'));
  }

  /*!
   *  \brief does the acl have permissions
   */
  function acl_have_any_permissions()
  {
  }

  /*! \brief get the acl */
  function getacl($attribute, $skip_write = FALSE)
  {
    $ui         = get_userinfo();
    $skip_write |= $this->read_only;
    return $ui->get_permissions($this->acl_base, $this->acl_category.get_class($this), $attribute, $skip_write);
  }


  /*!
   * \brief Returns a list of all available departments for this object.
   *
   * If this object is new, all departments we are allowed to create a new user in
   * are returned. If this is an existing object, return all deps.
   * We are allowed to move tis object too.
   *
   * \return array [dn] => "..name"  // All deps. we are allowed to act on.
  */
  function get_allowed_bases()
  {
    $ui = get_userinfo();
    $deps = array();

    /* Is this a new object ? Or just an edited existing object */
    if (!$this->initially_was_account && $this->is_account) {
      $new = TRUE;
    } else {
      $new = FALSE;
    }

    foreach ($this->config->idepartments as $dn => $name) {
      if ($new && $this->acl_is_createable($dn)) {
        $deps[$dn] = $name;
      } elseif (!$new && $this->acl_is_moveable($dn)) {
        $deps[$dn] = $name;
      }
    }

    /* Add current base */
    if (isset($this->base) && isset($this->config->idepartments[$this->base])) {
      $deps[$this->base] = $this->config->idepartments[$this->base];
    } elseif (strtolower($this->dn) == strtolower($this->config->current['BASE'])) {

    } else {
      trigger_error("Cannot return list of departments, no default base found in class ".get_class($this).". (base is '".$this->base."')");
    }
    return $deps;
  }


  /*
   * \brief This function updates ACL settings if $old_dn was used.
   *
   * \param string $old_dn specifies the actually used dn
   *
   * \param string $new_dn specifies the destiantion dn
   *
   * \param boolean $output_changes FALSE
   */
  function update_acls($old_dn, $new_dn, $output_changes = FALSE)
  {
    /* Check if old_dn is empty. This should never happen */
    if (empty($old_dn) || empty($new_dn)) {
      trigger_error("Failed to check acl dependencies, wrong dn given.");
      return;
    }

    /* Update userinfo if necessary */
    $ui = session::global_get('ui');
    if ($ui->dn == $old_dn) {
      $ui->dn = $new_dn;
      session::global_set('ui', $ui);
      new log("view", "acl/".get_class($this), $this->dn, array(), "Updated current object dn from '".$old_dn."' to '".$new_dn."'");
    }

    /* Object was moved, ensure that all acls will be moved too */
    if (($new_dn != $old_dn) && ($old_dn != "new")) {

      /* get_ldap configuration */
      $update = array();
      $ldap   = $this->config->get_ldap_link();
      $ldap->cd ($this->config->current['BASE']);
      $ldap->search("(&(objectClass=gosaAcl)(gosaAclEntry=*".base64_encode($old_dn)."*))", array("cn","gosaAclEntry"));
      while ($attrs = $ldap->fetch()) {
        $acls   = array();
        $found  = FALSE;
        for ($i = 0; $i < $attrs['gosaAclEntry']['count']; $i++) {
          $acl_parts = explode(":", $attrs['gosaAclEntry'][$i]);

          /* Roles uses antoher data storage order, members are stored int the third part,
             while the members in direct ACL assignments are stored in the second part.
           */
          $id = ($acl_parts[1] == "role" ? 3 : 2);

          /* Update member entries to use $new_dn instead of old_dn
           */
          $members = explode(",", $acl_parts[$id]);
          foreach ($members as $key => $member) {
            $member = base64_decode($member);
            if ($member == $old_dn) {
              $members[$key]  = base64_encode($new_dn);
              $found          = TRUE;
            }
          }

          /* Check if the selected role has to updated
           */
          if (($acl_parts[1] == "role") && ($acl_parts[2] == base64_encode($old_dn))) {
            $acl_parts[2] = base64_encode($new_dn);
            $found        = TRUE;
          }

          /* Build new acl string */
          $acl_parts[$id] = implode($members, ",");
          $acls[]         = implode($acl_parts, ":");
        }

        /* Acls for this object must be adjusted */
        if ($found) {
          $debug_info = _("Changing ACL dn")."&nbsp;:&nbsp;<br>&nbsp;-"._("from")."&nbsp;<b>&nbsp;".
            $old_dn."</b><br>&nbsp;-"._("to")."&nbsp;<b>".$new_dn."</b><br>";
          @DEBUG (DEBUG_ACL, __LINE__, __FUNCTION__, __FILE__, $debug_info, "ACL");

          $update[$attrs['dn']] = array();
          foreach ($acls as $acl) {
            $update[$attrs['dn']]['gosaAclEntry'][] = $acl;
          }
        }
      }

      /* Write updated acls */
      foreach ($update as $dn => $attrs) {
        $ldap->cd($dn);
        $ldap->modify($attrs);
      }
    }
  }

  /*!
   * \brief Enable the Serial ID check
   *
   * This function enables the entry Serial ID check.  If an entry was edited while
   * we have edited the entry too, an error message will be shown.
   * To configure this check correctly read the FAQ.
   */
  function enable_CSN_check()
  {
    $this->CSN_check_active = TRUE;
    $this->entryCSN = getEntryCSN($this->dn);
  }

  function is_modal_dialog()
  {
    return (isset($this->dialog) && $this->dialog);
  }
}
?>
