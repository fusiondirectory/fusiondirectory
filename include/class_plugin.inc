<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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
  var $parent= NULL;

  /*!
    \brief Configuration container

    Access to global configuration
   */
  var $config= NULL;

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
  var $ignore_account = FALSE;
  var $is_modified    = FALSE;

  /*!
    \brief Represent temporary LDAP data

    This is only used internally.
   */
  var $attrs = array();

  /* Keep set of conflicting plugins */
  var $conflicts = array();

  /* Save unit tags */
  var $gosaUnitTag  = "";
  var $skipTagging  = FALSE;

  /*!
    \brief Used standard values

    dn
   */
  var $dn         = "";
  var $uid        = "";
  var $sn         = "";
  var $givenName  = "";
  var $acl        = "*none*";
  var $dialog     = FALSE;
  var $snapDialog = NULL;

  /* attribute list for save action */
  var $attributes       = array();
  var $objectclasses    = array();
  var $is_new           = TRUE;
  var $saved_attributes = array();

  var $acl_base= "";
  var $acl_category= "";
  var $read_only = FALSE; // Used when the entry is opened as "readonly" due to locks.

  /* This can be set to render the tabulators in another stylesheet */
  var $pl_notify= FALSE;

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
      $tmp = $this->plInfo();
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

      /* Copy needed attributes */
      foreach ($this->attributes as $val) {
        $found = array_key_ics($val, $this->attrs);
        if ($found != "") {
          $this->$val = $found[0];
        }
      }

      /* gosaUnitTag loading... */
      if (isset($this->attrs['gosaUnitTag'][0])) {
        $this->gosaUnitTag = $this->attrs['gosaUnitTag'][0];
      }

      /* Set the template flag according to the existence of objectClass
         gosaUserTemplate */
      if (isset($this->attrs['objectClass'])) {
        if (in_array_ics ("gosaUserTemplate", $this->attrs['objectClass'])) {
          $this->is_template = TRUE;
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
              "found", "Template check");
        }
      }

      /* Is Account? */
      if ($this->is_this_account($this->attrs)) {
        $this->is_account = TRUE;
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, "found", "Object check");
      }

      $this->prepareSavedAttributes();
    }

    /* Save initial account state */
    $this->initially_was_account = $this->is_account;
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

    if (isset($this->attrs['gosaUnitTag'])) {
      $this->saved_attributes['gosaUnitTag'] = $this->attrs['gosaUnitTag'][0];
    }
  }

  /*!
   * \brief This function is called on the copied object to set its dn to where it will be saved
   */
  function resetCopyInfos()
  {
    $this->dn = "new";
    $this->saved_attributes = array();

    $this->orig_dn = $this->dn;
    $this->initially_was_account = FALSE;
  }


  /*!
   * \brief Generates the html output for this node
   */
  function execute()
  {
    /* This one is empty currently. Fabian - please fill in the docu code */
    session::global_set('current_class_for_help',get_class($this));

    /* Reset Lock message POST/GET check array, to prevent perg_match errors*/
    session::set('LOCK_VARS_TO_USE',array());
    session::set('LOCK_VARS_USED_GET',array());
    session::set('LOCK_VARS_USED_POST',array());
    session::set('LOCK_VARS_USED_REQUEST',array());
  }

  /*!
   * \brief Removes object from parent
   */
  function remove_from_parent()
  {
    /* include global link_info */
    $ldap= $this->config->get_ldap_link();

    /* Get current objectClasses in order to add the required ones */
    $ldap->cat($this->dn);
    $tmp= $ldap->fetch ();
    $oc= array();
    if (isset($tmp['objectClass'])){
      $oc= $tmp['objectClass'];
      unset($oc['count']);
    }

    /* Remove objectClasses from entry */
    $ldap->cd($this->dn);
    $this->attrs= array();
    $this->attrs['objectClass']= array_remove_entries_ics($this->objectclasses,$oc);

    /* Unset attributes from entry */
    foreach ($this->attributes as $val){
      $this->attrs["$val"]= array();
    }

    /* Do not write in plugin base class, this must be done by
       children, since there are normally additional attribs,
       lists, etc. */
    /*
       $ldap->modify($this->attrs);
     */
    if($this->initially_was_account){
        $this->handle_pre_events('remove');
    }
  }


  /*!
   * \brief Save HTML posted data to object
   */
  function save_object()
  {
    /* Update entry CSN if it is empty. */
    if(empty($this->entryCSN) && $this->CSN_check_active){
      $this->entryCSN = getEntryCSN($this->dn);
    }

    /* Save values to object */
    foreach ($this->attributes as $val){
      if ($this->acl_is_writeable($val) && isset ($_POST["$val"])){
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
        if (isset($data[0]) && $data[0] == chr(194)) {
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
    $ldap= $this->config->get_ldap_link();

    /* Save all plugins */
    $this->entryCSN = "";

    /* Start with empty array */
    $this->attrs= array();

    /* Get current objectClasses in order to add the required ones */
    $ldap->cat($this->dn);

    $tmp= $ldap->fetch ();

    $oc= array();
    if (isset($tmp['objectClass'])){
      $oc= $tmp["objectClass"];
      $this->is_new= FALSE;
      unset($oc['count']);
    } else {
      $this->is_new= TRUE;
    }

    /* Load (minimum) attributes, add missing ones */
    $this->attrs['objectClass'] = array_merge_unique($oc,$this->objectclasses);

    /* Copy standard attributes */
    foreach ($this->attributes as $val){
      if ($this->$val != ""){
        $this->attrs["$val"]= $this->$val;
      } elseif (!$this->is_new) {
        $this->attrs["$val"]= array();
      }
    }

    /* Handle tagging */
    $this->tag_attrs($this->attrs);

    if($this->is_new){
        $this->handle_pre_events('add');
    }else{
        $this->handle_pre_events('modify');
    }
  }

  /*!
   * \brief Remove attributes, empty arrays, arrays
   * single attributes that do not differ
   */
  function cleanup()
  {
    foreach ($this->attrs as $index => $value){

      /* Convert arrays with one element to non arrays, if the saved
         attributes are no array, too */
      if (is_array($this->attrs[$index]) &&
          count ($this->attrs[$index]) == 1 &&
          isset($this->saved_attributes[$index]) &&
          !is_array($this->saved_attributes[$index])){

        $tmp= $this->attrs[$index][0];
        $this->attrs[$index]= $tmp;
      }

      /* Remove emtpy arrays if they do not differ */
      if (is_array($this->attrs[$index]) &&
          count($this->attrs[$index]) == 0 &&
          !isset($this->saved_attributes[$index])){

        unset ($this->attrs[$index]);
        continue;
      }

      /* Remove single attributes that do not differ */
      if (!is_array($this->attrs[$index]) &&
          isset($this->saved_attributes[$index]) &&
          !is_array($this->saved_attributes[$index]) &&
          $this->attrs[$index] == $this->saved_attributes[$index]){

        unset ($this->attrs[$index]);
        continue;
      }

      /* Remove arrays that do not differ */
      if (is_array($this->attrs[$index]) &&
          isset($this->saved_attributes[$index]) &&
          is_array($this->saved_attributes[$index])){

        if (!array_differs($this->attrs[$index],$this->saved_attributes[$index])){
          unset ($this->attrs[$index]);
          continue;
        }
      }
    }

    /* Update saved attributes and ensure that next cleanups will be successful too */
    foreach($this->attrs as $name => $value){
      $this->saved_attributes[$name] = $value;
    }
  }

  /*!
   * \brief Check formular input
   */
  function check()
  {
    $message= array();

    /* Skip if we've no config object */
    if (!isset($this->config) || !is_object($this->config)){
      return $message;
    }

    /* Find hooks entries for this class */
    $command= $this->config->search(get_class($this), "CHECK", array('menu', 'tabs'));

    if ($command != ""){

      if (!check_command($command)){
        $message[]= msgPool::cmdnotfound("CHECK", get_class($this));
      } else {

        /* Generate "ldif" for check hook */
        $ldif= "dn: $this->dn\n";

        /* ... objectClasses */
        foreach ($this->objectclasses as $oc){
          $ldif.= "objectClass: $oc\n";
        }

        /* ... attributes */
        foreach ($this->attributes as $attr){
          if ($this->$attr == ""){
            continue;
          }
          if (is_array($this->$attr)){
            foreach ($this->$attr as $val){
              $ldif.= "$attr: $val\n";
            }
          } else {
              $ldif.= "$attr: ".$this->$attr."\n";
          }
        }

        /* Append empty line */
        $ldif.= "\n";

        /* Feed "ldif" into hook and retrieve result*/
        $descriptorspec = array( 0 => array("pipe", "r"), 1 => array("pipe", "w"), 2 => array("pipe", "w"));
        $fh= proc_open($command, $descriptorspec, $pipes);
        if (is_resource($fh)) {
          fwrite ($pipes[0], $ldif);
          fclose($pipes[0]);

          $result= stream_get_contents($pipes[1]);
          if ($result != ""){
            $message[]= $result;
          }

          fclose($pipes[1]);
          fclose($pipes[2]);
          proc_close($fh);
        }
      }

    }

    /* Check entryCSN */
    if($this->CSN_check_active){
      $current_csn = getEntryCSN($this->dn);
      if($current_csn != $this->entryCSN && !empty($this->entryCSN) && !empty($current_csn)){
        $this->entryCSN = $current_csn;
        $message[] = _("The object has changed since opened in FusionDirectory. All changes that may be done by others get lost if you save this entry!");
      }
    }
    return ($message);
  }

  /*
   * \brief Adapt from template, using 'dn'
   *
   * \param string $dn The DN
   *
   * \param array $skip A new array
   */
  function adapt_from_template($dn, $skip = array())
  {
    /* Include global link_info */
    $ldap = $this->config->get_ldap_link();

    /* Load requested 'dn' to 'attrs' */
    $ldap->cat($dn);
    $this->attrs = $ldap->fetch();

    /* Walk through attributes */
    foreach ($this->attributes as $val) {

      /* Skip the ones in skip list */
      if (in_array($val, $skip)) {
        continue;
      }

      if (isset($this->attrs["$val"][0])) {

        /* If attribute is set, replace dynamic parts:
           %sn, %givenName and %uid. Fill these in our local variables. */
        $value = $this->tpl_parse($this->attrs["$val"][0]);

        $this->$val = $value;
      }
    }

    /* Is Account? */
    $this->is_account = $this->is_this_account($this->attrs);
  }

  /* Apply a modifier
   * Returns an array of possible values */
  static function tpl_apply_modifier($m, $args, $str)
  {
    switch($m) {
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

  static function tpl_parse_mask($mask, $objects)
  {
    if ($mask == "|") {
      return array("%");
    }
    $modifiers = '';
    if (preg_match('/^([^|]+)\|/', $mask, $m)) {
      $modifiers = $m[1];
      $mask = substr($mask, strlen($m[0]));
    }
    $result = NULL;
    foreach ($objects as &$object) {
      if (isset($object->$mask)) {
        $result = array($object->$mask);
        break;
      }
    }
    unset($object);
    if ($result === NULL) {
      trigger_error("'$mask' was not found in objects");
      $result = array('');
    }
    $len    = strlen($modifiers);
    for ($i = 0; $i<$len; ++$i) {
      $args     = array();
      $modifier = $modifiers[$i];
      if (preg_match('/^\[([^\]]+)\].*$/', substr($modifiers, $i+1), $m)) {
        /* get modifier args */
        $args = explode(',', $m[1]);
        $i += strlen($m[1])+2;
      }
      $result_tmp = array();
      foreach ($result as $r) {
        $result_tmp = array_merge($result_tmp, self::tpl_apply_modifier($modifier, $args, $r));
      }
      $result = $result_tmp;
    }
    return $result;
  }

  function tpl_parse($string)
  {
    $offset = 0;
    while (preg_match('/%([^%]+)%/', $string, $m, PREG_OFFSET_CAPTURE, $offset)) {
      $replace = self::tpl_parse_mask($m[1][0], array($this->parent, $this));
      $replace = $replace[0];
      $string = substr_replace($string, $replace, $m[0][1], strlen($m[0][0]));
      $offset = $m[0][1] + strlen($replace);
    }
    return $string;
  }

  /*
   * \brief Indicate whether a password change is needed or not
   */
  function password_change_needed()
  {
    return FALSE;
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
    if (($button_disabled) || ((!$this->acl_is_createable() && !$plugin_enabled) || (!$this->acl_is_removeable() && $plugin_enabled))){
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
   * \brief Executes commands after an object has been created
   *
   * \param array $add_attrs
   */
  function postcreate($add_attrs = array())
  {
    return self::callHook ($this, "POSTCREATE", $add_attrs);
  }

  /*!
   * \brief Execute commands after an object has been modified
   *
   * \param array $add_attrs
   */
  function postmodify($add_attrs = array())
  {
    return self::callHook ($this, "POSTMODIFY", $add_attrs);
  }

  /*!
   * \brief Executes a command after an object has been removed
   *
   * \param array $add_attrs
   */
  function postremove($add_attrs = array())
  {
    return self::callHook ($this, "POSTREMOVE", $add_attrs);
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
   * \param string $data
   *
   * \param string $base
   */
  function create_unique_dn2($data, $base)
  {
    $ldap= $this->config->get_ldap_link();
    $base= preg_replace("/^,*/", "", $base);

    /* Try to use plain entry first */
    $dn= "$data,$base";
    $attribute= preg_replace('/=.*$/', '', $data);
    $ldap->cat ($dn, array('dn'));
    if (!$ldap->fetch()){
      return ($dn);
    }

    /* Look for additional attributes */
    foreach ($this->attributes as $attr){
      if ($attr == $attribute || $this->$attr == ""){
        continue;
      }

      $dn= "$data+$attr=".$this->$attr.",$base";
      $ldap->cat ($dn, array('dn'));
      if (!$ldap->fetch()){
        return ($dn);
      }
    }

    /* None found */
    return ("none");
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
    $ldap= $this->config->get_ldap_link();
    $base= preg_replace("/^,*/", "", $base);

    /* Try to use plain entry first */
    $dn= "$attribute=".$this->$attribute.",$base";
    $ldap->cat ($dn, array('dn'));
    if (!$ldap->fetch()){
      return ($dn);
    }

    /* Look for additional attributes */
    foreach ($this->attributes as $attr){
      if ($attr == $attribute || $this->$attr == ""){
        continue;
      }

      $dn= "$attribute=".$this->$attribute."+$attr=".$this->$attr.",$base";
      $ldap->cat ($dn, array('dn'));
      if (!$ldap->fetch()){
        return ($dn);
      }
    }

    /* None found */
    return ("none");
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
    $credentials= LDAP::get_credentials($referral, $this->config->current['REFERRAL']);
    if (ldap_bind($ldap, $credentials['ADMIN'], $this->config->get_credentials($credentials['PASSWORD']))) {
      $this->error = "Success";
      $this->hascon=true;
      $this->reconnect= true;
      return (0);
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
  function _copy($src_dn,$dst_dn)
  {
    $ldap=$this->config->get_ldap_link();
    $ldap->cat($src_dn);
    $attrs= $ldap->fetch();

    /* Grummble. This really sucks. PHP ldap doesn't support rdn stuff. */
    $ds= ldap_connect($this->config->current['SERVER']);
    ldap_set_option($ds, LDAP_OPT_PROTOCOL_VERSION, 3);
    if (function_exists("ldap_set_rebind_proc") && isset($this->config->current['REFERRAL'])) {
      ldap_set_rebind_proc($ds, array(&$this, "rebind"));
    }

    $pwd = $this->config->get_credentials($this->config->current['ADMINPASSWORD']);
    $r=ldap_bind($ds,$this->config->current['ADMINDN'], $pwd);
    $sr=ldap_read($ds, LDAP::fix($src_dn), "objectClass=*");

    /* Fill data from LDAP */
    $new= array();
    if ($sr) {
      $ei=ldap_first_entry($ds, $sr);
      if ($ei) {
        foreach($attrs as $attr => $val){
          if ($info = @ldap_get_values_len($ds, $ei, $attr)){
            for ($i= 0; $i<$info['count']; $i++){
              if ($info['count'] == 1){
                $new[$attr]= $info[$i];
              } else {
                $new[$attr][]= $info[$i];
              }
            }
          }
        }
      }
    }

    /* close connexion */
    ldap_unbind($ds);

    /* Adapt naming attribute */
    $dst_name= preg_replace("/^([^=]+)=.*$/", "\\1", $dst_dn);
    $dst_val = preg_replace("/^[^=]+=([^,+]+).*,.*$/", "\\1", $dst_dn);
    $new[$dst_name]= LDAP::fix($dst_val);

    /* Check if this is a department.
     * If it is a dep. && there is a , override in his ou
     *  change \2C to , again, else this entry can't be saved ...
     */
    if((isset($new['ou'])) &&( preg_match("/\\,/",$new['ou']))){
      $new['ou'] = str_replace("\\\\,",",",$new['ou']);
    }

    /* Save copy */
    $ldap->connect();
    $ldap->cd($this->config->current['BASE']);

    $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $dst_dn));

    /* FAIvariable=.../..., cn=..
        could not be saved, because the attribute FAIvariable was different to
        the dn FAIvariable=..., cn=... */

    if(!is_array($new['objectClass'])) $new['objectClass'] = array($new['objectClass']);

    if(in_array_ics("FAIdebconfInfo",$new['objectClass'])){
      $new['FAIvariable'] = $ldap->fix($new['FAIvariable']);
    }
    $ldap->cd($dst_dn);
    $ldap->add($new);

    if (!$ldap->success()){
      trigger_error("Trying to save $dst_dn failed.",
          E_USER_WARNING);
      return(FALSE);
    }
    return(TRUE);
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
    $ldap= $this->config->get_ldap_link();
    $ldap->search('(&(objectClass=gosaGroupOfNames)(member='.@LDAP::prepare4filter($src_dn).'))',
        array('cn'));
    while ($attrs= $ldap->fetch()){
      $og= new ogroup($this->config, $ldap->getDN());
      unset($og->member[$src_dn]);
      $og->member[$dst_dn]= $dst_dn;
      $og->save ();
    }

    $ldap->cat($dst_dn);
    $attrs= $ldap->fetch();
    if (count($attrs)){
      trigger_error("Trying to overwrite ".LDAP::fix($dst_dn).", which already exists.",
          E_USER_WARNING);
      return (FALSE);
    }

    $ldap->cat($src_dn);
    $attrs= $ldap->fetch();
    if (!count($attrs)){
      trigger_error("Trying to move ".LDAP::fix($src_dn).", which does not seem to exist.",
          E_USER_WARNING);
      return (FALSE);
    }

    $ldap->cd($src_dn);
    $ldap->search("objectClass=*",array("dn"));
    while($attrs = $ldap->fetch()){
      $src = $attrs['dn'];
      $dst = preg_replace("/".preg_quote($src_dn, '/')."$/",$dst_dn,$attrs['dn']);
      $this->_copy($src,$dst);
    }
    return (TRUE);
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
  function rename($src_dn, $dst_dn)
  {
    $start = microtime(1);

    /* Try to move the source entry to the destination position */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->create_missing_trees(preg_replace("/^[^,]+,/","",$dst_dn));
    if (!$ldap->rename_dn($src_dn,$dst_dn)){
#      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $src_dn, "", get_class()));
      new log("debug","Ldap Protocol v3 implementation error, ldap_rename failed, falling back to manual copy.","FROM: $src_dn  -- TO: $dst_dn",array(),$ldap->get_error());
      @DEBUG(DEBUG_LDAP,__LINE__,__FUNCTION__,__FILE__,"Rename failed FROM: $src_dn  -- TO:  $dst_dn",
          "Ldap Protocol v3 implementation error, falling back to maunal method.");
      return(FALSE);
    }

    /* Get list of users,groups and roles within this tree,
        maybe we have to update ACL references.
     */
    $leaf_objs = get_list("(|(objectClass=posixGroup)(objectClass=gosaAccount)(objectClass=gosaRole))",array("all"),$dst_dn,
          array("dn","objectClass"),GL_SUBSEARCH | GL_NO_ACL_CHECK);
    foreach($leaf_objs as $obj){
      $new_dn = $obj['dn'];
      $old_dn = preg_replace("/".preg_quote(LDAP::convert($dst_dn), '/')."$/i",$src_dn,LDAP::convert($new_dn));
      $this->update_acls($old_dn,$new_dn);
    }

    // Migrate objectgroups if needed
    $ogroups = get_sub_list("(&(objectClass=gosaGroupOfNames)(member=".LDAP::prepare4filter(LDAP::fix($src_dn))."))",
      "ogroups", array(get_ou("ogroupRDN")),$this->config->current['BASE'],array("dn"), GL_SUBSEARCH | GL_NO_ACL_CHECK);

    // Walk through all objectGroups
    foreach($ogroups as $ogroup){
      // Migrate old to new dn
      $o_ogroup= new ogroup($this->config,$ogroup['dn']);
      if (isset($o_ogroup->member[$src_dn])) {
        unset($o_ogroup->member[$src_dn]);
      }
      $o_ogroup->member[$dst_dn]= $dst_dn;

      // Save object group
      $o_ogroup->save();
    }

    // Migrate rfc groups if needed
    $groups = get_sub_list("(&(objectClass=posixGroup)(member=".LDAP::prepare4filter(LDAP::fix($src_dn))."))","groups", array(get_ou("groupRDN")),$this->config->current['BASE'],array("dn"), GL_SUBSEARCH | GL_NO_ACL_CHECK);

    // Walk through all POSIX groups
    foreach($groups as $group){

      // Migrate old to new dn
      $o_group= new group($this->config,$group['dn']);
      $o_group->save();
    }

    /* Update roles to use the new entry dn */
    $roles = get_sub_list("(&(objectClass=organizationalRole)(roleOccupant=".LDAP::prepare4filter(LDAP::fix($src_dn))."))","roles", array(get_ou("roleRDN")),$this->config->current['BASE'],array("dn"), GL_SUBSEARCH | GL_NO_ACL_CHECK);

    // Walk through all roles
    foreach($roles as $role){
      $role = new roleGeneric($this->config,$role['dn']);
      $key= array_search($src_dn, $role->roleOccupant);
      if($key !== FALSE){
        $role->roleOccupant[$key] = $dst_dn;
        $role->save();
      }
    }

    // Update 'manager' attributes from gosaDepartment and inetOrgPerson
    $filter = "(&(objectClass=inetOrgPerson)(manager=".LDAP::prepare4filter(LDAP::fix($src_dn))."))";
    $ocs = $ldap->get_objectclasses();
    if(isset($ocs['gosaDepartment']['MAY']) && in_array('manager', $ocs['gosaDepartment']['MAY'])){
      $filter = "(|".$filter."(&(objectClass=gosaDepartment)(manager=".LDAP::prepare4filter(LDAP::fix($src_dn)).")))";
    }
    $leaf_deps=  get_list($filter,array("all"),$this->config->current['BASE'],
        array("manager","dn","objectClass"),GL_SUBSEARCH | GL_NO_ACL_CHECK);
    foreach($leaf_deps as $entry){
      $update = array('manager' => $dst_dn);
      $ldap->cd($entry['dn']);
      $ldap->modify($update);
      if(!$ldap->success()){
        trigger_error(sprintf("Failed to update manager for '%s', error was '%s'", $entry['dn'], $ldap->get_error()));
      }
    }

    /* Check if there are gosa departments moved.
       If there were deps moved, the force reload of config->deps.
     */
    $leaf_deps=  get_list("(objectClass=gosaDepartment)",array("all"),$dst_dn,
          array("dn","objectClass"),GL_SUBSEARCH | GL_NO_ACL_CHECK);

    if(count($leaf_deps)){
      $this->config->get_departments();
      $this->config->make_idepartments();
      session::global_set("config",$this->config);
      $ui =get_userinfo();
      $ui->reset_acl_cache();
    }

    return(TRUE);
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
    /* Do not copy if only upper- lowercase has changed */
    if(strtolower($src_dn) == strtolower($dst_dn)){
      return(TRUE);
    }


    /* Try to move the entry instead of copy & delete
     */
    if(TRUE){

      /* Try to move with ldap routines, if this was not successfull
          fall back to the old style copy & remove method
       */
      if($this->rename($src_dn, $dst_dn)){
        return(TRUE);
      }else{
        // See code below.
      }
    }

    /* Copy source to destination */
    if (!$this->copy($src_dn, $dst_dn)){
      return (FALSE);
    }

    /* Delete source */
    $ldap= $this->config->get_ldap_link();
    $ldap->rmdir_recursive($src_dn);
    if (!$ldap->success()){
      trigger_error("Trying to delete $src_dn failed.",
          E_USER_WARNING);
      return (FALSE);
    }

    return (TRUE);
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
    $ldap= $this->config->get_ldap_link();

    /* Check if destination exists - abort */
    $ldap->cat($dst_dn, array('dn'));
    if ($ldap->fetch()){
      trigger_error("recursive_move $dst_dn already exists.",
          E_USER_WARNING);
      return (FALSE);
    }

    $this->copy($src_dn, $dst_dn);

    /* Remove src_dn */
    $ldap->cd($src_dn);
    $ldap->recursive_remove($src_dn);
    return (TRUE);
  }


  /*! \brief Forward command execution requests
   *         to the post hook execution method.
   */
  function handle_post_events($mode, $addAttrs = array())
  {
    if (!in_array($mode, array('add','remove','modify'))) {
      trigger_error(sprintf("Invalid post event type given %s! Valid types are [add,modify,remove].", $mode));
      return;
    }
    switch ($mode) {
      case "add":
        plugin::callHook($this, "POSTCREATE", $addAttrs);
      break;

      case "modify":
        plugin::callHook($this, "POSTMODIFY", $addAttrs);
      break;

      case "remove":
        plugin::callHook($this, "POSTREMOVE", $addAttrs);
      break;
    }
  }


  /*!
   *  \brief Forward command execution requests
   *         to the pre hook execution method.
   */
  function handle_pre_events($mode, $addAttrs = array())
  {
    if (!in_array($mode, array('add','remove','modify'))) {
      trigger_error(sprintf("Invalid pre event type given %s! Valid types are [add,modify,remove].", $mode));
      return;
    }
    switch ($mode) {
      case "add":
        plugin::callHook($this, "PRECREATE", $addAttrs);
      break;

      case "modify":
        plugin::callHook($this, "PREMODIFY", $addAttrs);
      break;

      case "remove":
        plugin::callHook($this, "PREREMOVE", $addAttrs);
      break;
    }
  }


  static protected function matchHookVars ($command, $addAttrs) {
    // Sort attributes by length, ensures correct replacement
    $tmp = array();
    foreach ($addAttrs as $name => $value) {
      $tmp[$name] =  strlen($name);
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
   *		   Allows to a add special replacements.
   */
  static function callHook($plugin, $cmd, $addAttrs = array(), &$returnOutput = array(), &$returnCode = NULL)
  {
    global $config;
    $command = $config->search(get_class($plugin), $cmd, array('menu','tabs','hooks'));

    if ($command != "") {
      // Walk trough attributes list and add the plugins attributes.
      foreach ($plugin->attributes as $attr) {
        if (is_array($plugin->$attr)) {
          $addAttrs[$attr] = join('|',$plugin->$attr);
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
      if (preg_match("/%/",$command)) {
        $addAttrs = array();
        if (isset($plugin->parent->by_object)) {
          foreach ($plugin->parent->by_object as $object) {
            foreach ($object->attributes as $attr) {
              if (!isset($addAttrs[$attr])) {
                if (is_array($object->$attr)) {
                  $addAttrs[$attr] = join('|',$object->$attr);
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
      if (preg_match("/%/",$command)) {
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
            $value = trim($s,'|');
          }
          if (!is_string($value) && !is_int($value) && !is_float($value) && !is_bool($value)) {
            continue;
          }
          $addAttrs[$name] = $value;
        }
        $command = self::matchHookVars($command, $addAttrs);
      }

      @DEBUG (DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, "Execute");
      exec($command, $arr, $returnCode);
      $returnOutput = $arr;

      if ($returnCode != 0) {
        $str = implode("\n", $arr);
        @DEBUG (DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, "Execution failed code: ".$returnCode);
        $message = msgPool::cmdexecfailed($cmd, $command, get_class($plugin));
        if (!empty($str)) {
          $message .= "Result: ".$str;
        }
        msg_dialog::display(_("Error"), $message, ERROR_DIALOG);
      } elseif(is_array($arr)) {
        $str = implode("\n", $arr);
        @DEBUG (DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, "Result: ".$str);
      }
    }
  }

  /*! \brief Get gosaUnitTag for the given DN
   *
   *  If this is called from departmentGeneric, we have to skip this
   *  tagging procedure.
  */
  function tag_attrs(&$at, $dn= "", $tag= "", $show= false)
  {
    /* Skip tagging? */
    if($this->skipTagging){
      return;
    }

    /* No dn? Self-operation... */
    if ($dn == ""){
      $dn= $this->dn;

      /* No tag? Find it yourself... */
      if ($tag == ""){
        $len= strlen($dn);

        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, "No tag for $dn - looking for one...", "Tagging");
        $relevant= array();
        foreach ($this->config->adepartments as $key => $ntag){

          /* This one is bigger than our dn, its not relevant... */
          if ($len < strlen($key)){
            continue;
          }

          /* This one matches with the latter part. Break and don't fix this entry */
          if (preg_match('/(^|,)'.preg_quote($key, '/').'$/', $dn)){
            @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, "DEBUG: Possibly relevant: $key", "Tagging");
            $relevant[strlen($key)]= $ntag;
            continue;
          }

        }

        /* If we've some relevant tags to set, just get the longest one */
        if (count($relevant)){
          ksort($relevant);
          $tmp= array_keys($relevant);
          $idx= end($tmp);
          $tag= $relevant[$idx];
          $this->gosaUnitTag= $tag;
        }
      }
    }

  /*! \brief Add unit tag */
    /* Remove tags that may already be here... */
    remove_objectClass("gosaAdministrativeUnitTag", $at);
    if (isset($at['gosaUnitTag'])){
        unset($at['gosaUnitTag']);
    }

    /* Set tag? */
    if ($tag != ""){
      add_objectClass("gosaAdministrativeUnitTag", $at);
      $at['gosaUnitTag']= $tag;
    }

    /* Initially this object was tagged.
       - But now, it is no longer inside a tagged department.
       So force the remove of the tag.
       (objectClass was already removed obove)
     */
    if($tag == "" && $this->gosaUnitTag){
      $at['gosaUnitTag'] = array();
    }
  }


  /*! \brief Test for removability of the object
   *
   * Allows testing of conditions for removal of object. If removal should be aborted
   * the function needs to remove an error message.
   */
  function allow_remove()
  {
    $reason= "";
    return $reason;
  }


  /*!
   * \brief Create a snapshot of the current object
   *
   * \param string $type Initialized at "snapshot"
   *
   * \param array $description
   */
  function create_snapshot($type= "snapshot", $description= array())
  {

    /* Check if snapshot functionality is enabled */
    if(!$this->snapshotEnabled()){
      return;
    }

    /* Get configuration from fusiondirectory.conf */
    $config = $this->config;

    /* Create lokal ldap connection */
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);

    /* check if there are special server configurations for snapshots */
    if($config->get_cfg_value("snapshotURI") == ""){

      /* Source and destination server are both the same, just copy source to dest obj */
      $ldap_to      = $ldap;
      $snapldapbase = $this->config->current['BASE'];

    }else{
      $server         = $config->get_cfg_value("snapshotURI");
      $user           = $config->get_cfg_value("snapshotAdminDn");
      $password       = $this->config->get_credentials($config->get_cfg_value("snapshotAdminPassword"));
      $snapldapbase   = $config->get_cfg_value("snapshotBase");

      $ldap_to        = new ldapMultiplexer(new LDAP($user,$password, $server));
      $ldap_to -> cd($snapldapbase);

      if (!$ldap_to->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap_to->get_error(), $snapldapbase, "", get_class()));
      }

    }

    /* check if the dn exists */
    if ($ldap->dn_exists($this->dn)){

      /* Extract seconds & mysecs, they are used as entry index */
      list($usec, $sec)= explode(" ", microtime());

      /* Collect some infos */
      $base           = $this->config->current['BASE'];
      $snap_base      = $config->get_cfg_value("snapshotBase");
      $base_of_object = preg_replace ('/^[^,]+,/i', '', $this->dn);
      $new_base       = preg_replace("/".preg_quote($base, '/')."$/","",$base_of_object).$snap_base;

      /* Create object */
      $data             = $ldap->generateLdif(LDAP::fix($dn),
              "(&(!(objectClass=gosaDepartment))(!(objectClass=FAIclass)))",array(),'base');

      $newName          = str_replace(".", "", $sec."-".$usec);
      $target= array();
      $target['objectClass']            = array("top", "gosaSnapshotObject");
      $target['gosaSnapshotData']       = gzcompress($data, 6);
      $target['gosaSnapshotType']       = $type;
      $target['gosaSnapshotDN']         = $this->dn;
      $target['description']            = $description;
      $target['gosaSnapshotTimestamp']  = $newName;

      /* Insert the new snapshot
         But we have to check first, if the given gosaSnapshotTimestamp
         is already used, in this case we should increment this value till there is
         an unused value. */
      $new_dn                           = "gosaSnapshotTimestamp=".$newName.",".$new_base;
      $ldap_to->cat($new_dn);
      while($ldap_to->count()){
        $ldap_to->cat($new_dn);
        $newName = str_replace(".", "", $sec."-".($usec++));
        $new_dn                           = "gosaSnapshotTimestamp=".$newName.",".$new_base;
        $target['gosaSnapshotTimestamp']  = $newName;
      }

      /* Inset this new snapshot */
      $ldap_to->cd($snapldapbase);
      $ldap_to->create_missing_trees($snapldapbase);
      $ldap_to->create_missing_trees($new_base);
      $ldap_to->cd($new_dn);
      $ldap_to->add($target);
      if (!$ldap_to->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap_to->get_error(), $new_dn, LDAP_ADD, get_class()));
      }

      if (!$ldap->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $new_base, "", get_class()));
      }

    }
  }

  /*!
   * \brief Remove a snapshot
   *
   * \param string $dn The DN
   */
  function remove_snapshot($dn)
  {
    $ui       = get_userinfo();
    $old_dn   = $this->dn;
    $this->dn = $dn;
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->rmdir_recursive($this->dn);
    if(!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn));
    }
    $this->dn = $old_dn;
  }


  /*! \brief Test if snapshoting is enabled
   *
   * Test weither snapshotting is enabled or not. There will also be some errors posted,
   * if the configuration failed
   *
   * \return TRUE if snapshots are enabled, and FALSE if it is disabled
   */
  function snapshotEnabled()
  {
    return $this->config->snapshotEnabled();
  }


  /*
   * \brief Return available snapshots for the given base
   *
   * \param string $dn The DN
   *
   * \param string boolean $raw FALSE
   */
  function Available_SnapsShots($dn,$raw = false)
  {
    if(!$this->snapshotEnabled()) return(array());

    /* Create an additional ldap object which
       points to our ldap snapshot server */
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $cfg= &$this->config->current;

    /* check if there are special server configurations for snapshots */
    if($this->config->get_cfg_value("snapshotURI") == ""){
      $ldap_to      = $ldap;
    }else{
      $server         = $this->config->get_cfg_value("snapshotURI");
      $user           = $this->config->get_cfg_value("snapshotAdminDn");
      $password       = $this->config->get_credentials($this->config->get_cfg_value("snapshotAdminPassword"));
      $snapldapbase   = $this->config->get_cfg_value("snapshotBase");
      $ldap_to        = new ldapMultiplexer(new LDAP($user,$password, $server));
      $ldap_to -> cd($snapldapbase);
      if (!$ldap_to->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap_to->get_error(), $snapldapbase, "", get_class()));
      }
    }

    /* Prepare bases and some other infos */
    $base           = $this->config->current['BASE'];
    $snap_base      = $this->config->get_cfg_value("snapshotBase");
    $base_of_object = preg_replace ('/^[^,]+,/i', '', $dn);
    $new_base       = preg_replace("/".preg_quote($base, '/')."$/","",$base_of_object).$snap_base;
    $tmp            = array();

    /* Fetch all objects with  gosaSnapshotDN=$dn */
    $ldap_to->cd($new_base);
    $ldap_to->ls("(&(objectClass=gosaSnapshotObject)(gosaSnapshotDN=".$dn."))",$new_base,
        array("gosaSnapshotType","gosaSnapshotTimestamp","gosaSnapshotDN","description"));

    /* Put results into a list and add description if missing */
    while($entry = $ldap_to->fetch()){
      if(!isset($entry['description'][0])){
        $entry['description'][0]  = "";
      }
      $tmp[] = $entry;
    }

    /* Return the raw array, or format the result */
    if($raw){
      return($tmp);
    }else{
      $tmp2 = array();
      foreach($tmp as $entry){
        $tmp2[base64_encode($entry['dn'])] = $entry['description'][0];
      }
    }
    return($tmp2);
  }

  /*!
   * \brief Get all deleted snapshots
   */
  function getAllDeletedSnapshots($base_of_object,$raw = false)
  {
    if(!$this->snapshotEnabled()) return(array());

    /* Create an additional ldap object which
       points to our ldap snapshot server */
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $cfg= &$this->config->current;

    /* check if there are special server configurations for snapshots */
    if($this->config->get_cfg_value("snapshotURI") == ""){
      $ldap_to      = $ldap;
    }else{
      $server         = $this->config->get_cfg_value("snapshotURI");
      $user           = $this->config->get_cfg_value("snapshotAdminDn");
      $password       = $this->config->get_credentials($this->config->get_cfg_value("snapshotAdminPassword"));
      $snapldapbase   = $this->config->get_cfg_value("snapshotBase");
      $ldap_to        = new ldapMultiplexer(new LDAP($user,$password, $server));
      $ldap_to -> cd($snapldapbase);
      if (!$ldap_to->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap_to->get_error(), $snapldapbase, "", get_class()));
      }
    }

    /* Prepare bases */
    $base           = $this->config->current['BASE'];
    $snap_base      = $this->config->get_cfg_value("snapshotBase");
    $new_base       = preg_replace("/".preg_quote($base, '/')."$/","",$base_of_object).$snap_base;

    /* Fetch all objects and check if they do not exist anymore */
    $ui = get_userinfo();
    $tmp = array();
    $ldap_to->cd($new_base);
    $ldap_to->ls("(objectClass=gosaSnapshotObject)",$new_base,array("gosaSnapshotType","gosaSnapshotTimestamp","gosaSnapshotDN","description"));
    while($entry = $ldap_to->fetch()){

      $chk =  str_replace($new_base,"",$entry['dn']);
      if(preg_match("/,ou=/",$chk)) continue;

      if(!isset($entry['description'][0])){
        $entry['description'][0]  = "";
      }
      $tmp[] = $entry;
    }

    /* Check if entry still exists */
    foreach($tmp as $key => $entry){
      $ldap->cat($entry['gosaSnapshotDN'][0]);
      if($ldap->count()){
        unset($tmp[$key]);
      }
    }

    /* Format result as requested */
    if($raw) {
      return($tmp);
    }else{
      $tmp2 = array();
      foreach($tmp as $key => $entry){
        $tmp2[base64_encode($entry['dn'])] = $entry['description'][0];
      }
    }
    return($tmp2);
  }


  /*
   * \brief Restore selected snapshot
   *
   * \param strind $dn The DN
   */
  function restore_snapshot($dn)
  {
    if(!$this->snapshotEnabled()) return(array());

    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $cfg= &$this->config->current;

    /* check if there are special server configurations for snapshots */
    if($this->config->get_cfg_value("snapshotURI") == ""){
      $ldap_to      = $ldap;
    }else{
      $server         = $this->config->get_cfg_value("snapshotURI");
      $user           = $this->config->get_cfg_value("snapshotAdminDn");
      $password       = $this->config->get_credentials($this->config->get_cfg_value("snapshotAdminPassword"));
      $snapldapbase   = $this->config->get_cfg_value("snapshotBase");
      $ldap_to        = new ldapMultiplexer(new LDAP($user,$password, $server));
      $ldap_to -> cd($snapldapbase);
      if (!$ldap_to->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap_to->get_error(), $snapldapbase, "", get_class()));
      }
    }

    /* Get the snapshot */
    $ldap_to->cat($dn);
    $restoreObject = $ldap_to->fetch();

    /* Prepare import string */
    $data  = gzuncompress($ldap_to->get_attribute($dn,'gosaSnapshotData'));

    /* Import the given data */
    $err = "";
    $ldap->import_complete_ldif($data,$err,false,false);
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $dn, "", get_class()));
    }
  }

  /*!
   * \brief Show snapshot dialog
   */
  function showSnapshotDialog($base,$baseSuffixe,&$parent)
  {
    $once = true;
    $ui = get_userinfo();
    $this->parent = $parent;

    foreach($_POST as $name => $value){

      /* Create a new snapshot, display a dialog */
      if(preg_match("/^CreateSnapShotDialog_[^_]*_[xy]$/",$name) && $once){

        $entry = base64_decode(preg_replace("/^CreateSnapShotDialog_([^_]*)_[xy]$/","\\1",$name));
        $once = false;
        $entry = preg_replace("/^CreateSnapShotDialog_/","",$entry);

        if(!empty($entry) && $ui->allow_snapshot_create($entry,$this->parent->acl_module)){
          $this->snapDialog = new SnapShotDialog($this->config,$entry,$this);
        }else{
          msg_dialog::display(_("Permission"),sprintf(_("You are not allowed to create a snapshot for %s."),$entry),ERROR_DIALOG);
        }
      }

      /* Restore a snapshot, display a dialog with all snapshots of the current object */
      if(preg_match("/^RestoreSnapShotDialog_/",$name) && $once){
        $once = false;
        $entry = base64_decode(preg_replace("/^RestoreSnapShotDialog_([^_]*)_[xy]$/i","\\1",$name));
        if(!empty($entry) && $ui->allow_snapshot_restore($this->dn,$this->parent->acl_module)){
          $this->snapDialog = new SnapShotDialog($this->config,$entry,$this);
          $this->snapDialog->display_restore_dialog = true;
        }else{
          msg_dialog::display(_("Permission"),sprintf(_("You are not allowed to restore a snapshot for %s."),$entry),ERROR_DIALOG);
        }
      }

      /* Restore one of the already deleted objects */
      if(((isset($_POST['menu_action']) && $_POST['menu_action'] == "RestoreDeletedSnapShot")
          || preg_match("/^RestoreDeletedSnapShot_/",$name)) && $once){
        $once = false;

        if($ui->allow_snapshot_restore($this->dn,$this->parent->acl_module)){
          $this->snapDialog = new SnapShotDialog($this->config,"",$this);
          $this->snapDialog->set_snapshot_bases($baseSuffixe);
          $this->snapDialog->display_restore_dialog      = true;
          $this->snapDialog->display_all_removed_objects  = true;
        }else{
          msg_dialog::display(_("Permission"),sprintf(_("You are not allowed to restore a snapshot for %s."),$base),ERROR_DIALOG);
        }
      }

      /* Restore selected snapshot */
      if(preg_match("/^RestoreSnapShot_/",$name) && $once){
        $once = false;
        $entry = base64_decode(preg_replace("/^RestoreSnapShot_([^_]*)_[xy]$/i","\\1",$name));

        if(!empty($entry) && $ui->allow_snapshot_restore($this->dn,$this->parent->acl_module)){
          $this->restore_snapshot($entry);
          $this->snapDialog = NULL;
        }else{
          msg_dialog::display(_("Permission"),sprintf(_("You are not allowed to restore a snapshot for %s."),$entry),ERROR_DIALOG);
        }
      }
    }

    /* Create a new snapshot requested, check
       the given attributes and create the snapshot*/
    if(isset($_POST['CreateSnapshot']) && is_object($this->snapDialog)){
      $this->snapDialog->save_object();
      $msgs = $this->snapDialog->check();
      if(count($msgs)){
        foreach($msgs as $msg){
          msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
        }
      }else{
        $this->dn =  $this->snapDialog->dn;
        $this->create_snapshot("snapshot",$this->snapDialog->CurrentDescription);
        $this->snapDialog = NULL;
      }
    }

    /* Restore is requested, restore the object with the posted dn .*/
    if((isset($_POST['RestoreSnapshot'])) && (isset($_POST['SnapShot']))){
    }

    if(isset($_POST['CancelSnapshot'])){
      $this->snapDialog = NULL;
    }

    if(is_object($this->snapDialog )){
      $this->snapDialog->save_object();
      return($this->snapDialog->execute());
    }
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
    $this->acl_base= $base;
  }

  /*!
   * \brief Set acl category
   *
   * \param string $category
   */
  function set_acl_category($category)
  {
    $this->acl_category= "$category/";
  }

  /*! \brief Can we write the acl */
  function acl_is_writeable($attribute,$skip_write = FALSE)
  {
    if($this->read_only) return(FALSE);
    $ui= get_userinfo();
    return preg_match('/w/', $ui->get_permissions($this->acl_base, $this->acl_category.get_class($this), $attribute, $skip_write));
  }

  /*!
   * \brief Can we read the acl
   *
   * \param string $attribute
   */
  function acl_is_readable($attribute)
  {
    $ui= get_userinfo();
    return preg_match('/r/', $ui->get_permissions($this->acl_base, $this->acl_category.get_class($this), $attribute));
  }

  /*!
   * \brief Can we create the acl
   *
   * \param string $base Empty string
   */
  function acl_is_createable($base ="")
  {
    if($this->read_only) return(FALSE);
    $ui= get_userinfo();
    if($base == "") $base = $this->acl_base;
    return preg_match('/c/', $ui->get_permissions($base, $this->acl_category.get_class($this), '0'));
  }

  /*!
   * \brief Can we remove the acl
   *
   * \param string $base Empty string
   */
  function acl_is_removeable($base ="")
  {
    if($this->read_only) return(FALSE);
    $ui= get_userinfo();
    if($base == "") $base = $this->acl_base;
    return preg_match('/d/', $ui->get_permissions($base, $this->acl_category.get_class($this), '0'));
  }

  /*!
   * \brief Can we move the acl
   *
   * \param string $base Empty string
   */
  function acl_is_moveable($base = "")
  {
    if($this->read_only) return(FALSE);
    $ui= get_userinfo();
    if($base == "") $base = $this->acl_base;
    return preg_match('/m/', $ui->get_permissions($base, $this->acl_category.get_class($this), '0'));
  }

  /*!
   *  \brief does the acl have permissions
   */
  function acl_have_any_permissions()
  {
  }

  /*! \brief get the acl */
  function getacl($attribute,$skip_write= FALSE)
  {
    $ui= get_userinfo();
    $skip_write |= $this->read_only;
    return  $ui->get_permissions($this->acl_base, $this->acl_category.get_class($this), $attribute,$skip_write);
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
    if(!$this->initially_was_account && $this->is_account){
      $new = true;
    }else{
      $new = false;
    }

    foreach($this->config->idepartments as $dn => $name){
      if($new && $this->acl_is_createable($dn)){
        $deps[$dn] = $name;
      }elseif(!$new && $this->acl_is_moveable($dn)){
        $deps[$dn] = $name;
      }
    }

    /* Add current base */
    if(isset($this->base) && isset($this->config->idepartments[$this->base])){
      $deps[$this->base] = $this->config->idepartments[$this->base];
    }elseif(strtolower($this->dn) == strtolower($this->config->current['BASE'])){

    }else{
      trigger_error("Cannot return list of departments, no default base found in class ".get_class($this).". ".$this->base);
    }
    return($deps);
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
  function update_acls($old_dn,$new_dn,$output_changes = FALSE)
  {
    /* Check if old_dn is empty. This should never happen */
    if(empty($old_dn) || empty($new_dn)){
      trigger_error("Failed to check acl dependencies, wrong dn given.");
      return;
    }

    /* Update userinfo if necessary */
    $ui = session::global_get('ui');
    if($ui->dn == $old_dn){
      $ui->dn = $new_dn;
      session::global_set('ui',$ui);
      new log("view","acl/".get_class($this),$this->dn,array(),"Updated current object dn from '".$old_dn."' to '".$new_dn."'");
    }

    /* Object was moved, ensure that all acls will be moved too */
    if($new_dn != $old_dn && $old_dn != "new"){

      /* get_ldap configuration */
      $update = array();
      $ldap = $this->config->get_ldap_link();
      $ldap->cd ($this->config->current['BASE']);
      $ldap->search("(&(objectClass=gosaAcl)(gosaAclEntry=*".base64_encode($old_dn)."*))",array("cn","gosaAclEntry"));
      while($attrs = $ldap->fetch()){
        $acls = array();
        $found = false;
        for($i = 0 ; $i <  $attrs['gosaAclEntry']['count'] ; $i ++ ){
          $acl_parts = explode(":",$attrs['gosaAclEntry'][$i]);

          /* Roles uses antoher data storage order, members are stored int the third part,
             while the members in direct ACL assignments are stored in the second part.
           */
          $id = ($acl_parts[1] == "role") ? 3 : 2;

          /* Update member entries to use $new_dn instead of old_dn
           */
          $members = explode(",",$acl_parts[$id]);
          foreach($members as $key => $member){
            $member = base64_decode($member);
            if($member == $old_dn){
              $members[$key] = base64_encode($new_dn);
              $found = TRUE;
            }
          }

          /* Check if the selected role has to updated
           */
          if($acl_parts[1] == "role" && $acl_parts[2] == base64_encode($old_dn)){
            $acl_parts[2] = base64_encode($new_dn);
            $found = TRUE;
          }

          /* Build new acl string */
          $acl_parts[$id] = implode($members,",");
          $acls[] = implode($acl_parts,":");
        }

        /* Acls for this object must be adjusted */
        if($found){

          $debug_info=  _("Changing ACL dn")."&nbsp;:&nbsp;<br>&nbsp;-"._("from")."&nbsp;<b>&nbsp;".
            $old_dn."</b><br>&nbsp;-"._("to")."&nbsp;<b>".$new_dn."</b><br>";
          @DEBUG (DEBUG_ACL, __LINE__, __FUNCTION__, __FILE__,$debug_info,"ACL");

          $update[$attrs['dn']] =array();
          foreach($acls as $acl){
            $update[$attrs['dn']]['gosaAclEntry'][] = $acl;
          }
        }
      }

      /* Write updated acls */
      foreach($update as $dn => $attrs){
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
    $this->CSN_check_active =TRUE;
    $this->entryCSN = getEntryCSN($this->dn);
  }

  /*!
   * \brief  Returns the snapshot header part for "Actions" menu in management dialogs
   *
   * \param string $base
   *
   * \param string $category
   */
  function get_snapshot_header($base,$category)
  {
    $str = "";
    $ui = get_userinfo();
    if($this->snapshotEnabled() && $ui->allow_snapshot_restore($base,$category)){

      $ok = false;
      foreach($this->get_used_snapshot_bases() as $base){
        $ok |= count($this->getAllDeletedSnapshots($base)) >= 1 ;
      }

      if($ok){
        $str = "..|<img class='center' src='images/lists/restore.png' ".
          "alt='"._("Restore")."'>&nbsp;"._("Restore").                       "|RestoreDeletedSnapShot|\n";
      }else{
        $str = "..|<img class='center' src='images/lists/restore_grey.png' alt=''>&nbsp;"._("Restore")."||\n";
      }
    }
    return($str);
  }

  /*!
   * \brief Get the snapshot action
   *
   * \param string $base
   *
   * \param string $category
   */
  function get_snapshot_action($base,$category)
  {
    $str= "";
    $ui = get_userinfo();
    if($this->snapshotEnabled()){
      if ($ui->allow_snapshot_restore($base,$category)){

        if(count($this->Available_SnapsShots($base))){
          $str.= "<input class='center' type='image' src='images/lists/restore.png'
            alt='"._("Restore snapshot")."' name='RestoreSnapShotDialog_".base64_encode($base)."' title='"._("Restore snapshot")."'>&nbsp;";
        } else {
          $str = "<img class='center' src='images/lists/restore_grey.png' alt=''>&nbsp;";
        }
      }
      if($ui->allow_snapshot_create($base,$category)){
        $str.= "<input class='center' type='image' src='images/snapshot.png'
          alt='"._("Create snapshot")."' name='CreateSnapShotDialog_".base64_encode($base)."'
          title='"._("Create a new snapshot from this object")."'>&nbsp;";
      }else{
        $str = "<img class='center' src='images/empty.png' alt=' '>&nbsp;";
      }
    }

    return($str);
  }


  function get_copypaste_action($base,$category,$class,$copy = TRUE, $cut = TRUE)
  {
    $ui = get_userinfo();
    $action = "";
    if($this->CopyPasteHandler){
      if($cut){
        if($ui->is_cutable($base,$category,$class)){
          $action .= "<input class='center' type='image'
            src='images/lists/cut.png' alt='"._("cut")."' name='cut_%KEY%' title='"._("Cut this entry")."'>&nbsp;";
        }else{
          $action.="<img src='images/empty.png' alt=' ' class='center'>&nbsp;";
        }
      }
      if($copy){
        if($ui->is_copyable($base,$category,$class)){
          $action.= "<input class='center' type='image'
            src='images/lists/copy.png' alt='"._("copy")."' name='copy_%KEY%' title='"._("Copy this entry")."'>&nbsp;";
        }else{
          $action.="<img src='images/empty.png' alt=' ' class='center'>&nbsp;";
        }
      }
    }

    return($action);
  }


  function get_copypaste_header($base,$category,$copy = TRUE, $cut = TRUE)
  {
    $s = "";
    $ui =get_userinfo();

    if(!is_array($category)){
      $category = array($category);
    }

    /* Check permissions for each category, if there is at least one category which
        support read or paste permissions for the given base, then display the specific actions.
     */
    $readable = $pasteable = false;
    foreach($category as $cat){
      $readable= $readable || preg_match('/r/', $ui->get_category_permissions($base, $cat));
      $pasteable= $pasteable || $ui->is_pasteable($base, $cat) == 1;
    }

    if(($cut || $copy) && isset($this->CopyPasteHandler) && is_object($this->CopyPasteHandler)){
      if($readable){
        $s.= "..|---|\n";
        if($copy){
          $s.= "..|<img src='images/lists/copy.png' alt='' border='0' class='center'>".
            "&nbsp;"._("Copy")."|"."multiple_copy_systems|\n";
        }
        if($cut){
          $s.= "..|<img src='images/lists/cut.png' alt='' border='0' class='center'>".
            "&nbsp;"._("Cut")."|"."multiple_cut_systems|\n";
        }
      }

      if ($pasteable) {
        if ($this->CopyPasteHandler->entries_queued()) {
          $img  = "<img border='0' class='center' src='images/lists/paste.png' alt=''>";
          $s    .= "..|".$img."&nbsp;"._("Paste")."|editPaste|\n";
        } else {
          $img  = "<img border='0' class='center' src='images/lists/paste-grey.png' alt=''>";
          $s    .= "..|".$img."&nbsp;"._("Paste")."\n";
        }
      }
    }
    return $s;
  }


  function get_used_snapshot_bases()
  {
     return array();
  }

  function is_modal_dialog()
  {
    return (isset($this->dialog) && $this->dialog);
  }
}
?>
