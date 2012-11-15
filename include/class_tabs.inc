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
 * \file class_tabs.inc
 * Source code for class tabs
 */

/*!
 * \brief This class contains all function to manage tabs classes
 */
class tabs
{
  var $dn;
  var $config;
  var $acl;
  var $is_template;
  var $is_new = FALSE;

  var $last       = "";
  var $current    = "";
  var $disabled   = "";
  var $by_name    = array();
  var $by_object  = array();
  var $SubDialog  = FALSE;
  var $acl_category;

  var $multiple_support_active = FALSE;

  var $parent = NULL; // A parent object if available, e.g. a management class.

  var $read_only = FALSE; // Used when the entry is opened as "readonly" due to locks.

  var $baseclass = "";

  /*!
   * \brief Tabs classes constructor
   *
   * \param string $config
   *
   * \param string $data
   *
   * \param string $dn The DN
   *
   * \param string $acl_category
   *
   * \param Object $copied_object An object to copy values from
   */
  function tabs(&$config, $data, $dn, $acl_category = "", $copied_object = NULL)
  {
    /* Save dn */
    $this->dn         = $dn;
    $this->config     = &$config;

    if (!count($data)) {
      $data[] = array("CLASS" => 'plugin',"NAME" => 'Error');
      msg_dialog::display(_("Error"),
        sprintf(_("No plugin definitions found to initialize '%s', please check your configuration file."), get_class($this)),
        "ERROR_DIALOG");
    }

    $baseobject         = NULL;
    $this->acl_category = $acl_category;
    foreach ($data as &$tab) {
      if (!plugin_available($tab['CLASS'])) {
        continue;
      }

      $this->by_name[$tab['CLASS']] = $tab['NAME'];

      if ($baseobject === NULL) {
        $baseobject = new $tab['CLASS']($this->config, $this->dn, $copied_object);
        $baseobject->enable_CSN_check();
        $this->by_object[$tab['CLASS']] = $baseobject;
        $this->baseclass                = $tab['CLASS'];
      } else {
        $this->by_object[$tab['CLASS']] = new $tab['CLASS']($this->config, $this->dn, $baseobject);
      }

      $this->read_only |= $this->by_object[$tab['CLASS']]->read_only;
      $this->by_object[$tab['CLASS']]->parent = &$this;
      $this->by_object[$tab['CLASS']]->set_acl_category($this->acl_category);
    }

    /* Initialize current */
    $this->current = $this->baseclass;
  }


  /*!
   * \brief Reinitializes the tab classes with fresh ldap values.
   *
   * This maybe usefull if for example the apply button was pressed.
   */
  function re_init()
  {
    $baseobject = NULL;
    foreach ($this->by_object as $name => $object) {
      $class = get_class($object);
      if (in_array($class, array("reference","acl"))) {
        continue;
      }
      if ($baseobject === NULL) {
        $baseobject = new $class($this->config, $this->dn);
        $baseobject->enable_CSN_check();
        $this->by_object[$name] = $baseobject;
      } else {
        $this->by_object[$name] = new $class($this->config, $this->dn, $baseobject);
      }
      $this->by_object[$name]->parent = &$this;
      $this->by_object[$name]->set_acl_category($this->acl_category);
    }
  }

  function getBaseObject()
  {
    return $this->by_object[$this->baseclass];
  }

  /*!
   * \brief Save the tab(s) contents
   */
  function execute()
  {
    /* Show object */
    $display = "<table summary=\"\" cellpadding=4 cellspacing=0 border=0 style=\"width:100%; background-color:#F8F8F8; border-style:solid; border-color:#AAA; border-top-width:0px; border-bottom-width:1px; border-left-width:1px; border-right-width:1px;\">\n";
    $display .= "<tr><td>\n";

    /* If multiple edit is enabled for this tab,
       we have to display different templates */
    if (!$this->multiple_support_active) {
      $display .= $this->by_object[$this->current]->execute();
    } else {
      $display .= $this->by_object[$this->current]->multiple_execute();
    }
    $modal_dialog = $this->by_object[$this->current]->is_modal_dialog();

    /* Build tab line */
    $tabs   = $this->gen_tabs($modal_dialog);

    /* Footer for tabbed dialog */
    $display = $tabs.$display."</td></tr></table>";

    return $display;
  }

  /*!
   * \brief Save a tabs object
   *
   * \param boolean $save_current false
   */
  function save_object()
  {
    /* Ensure that the currently selected tab is valid. */
    if (!isset($this->by_name[$this->current])) {
      $this->current = key($this->by_name);
    }

    /* Rotate current to last */
    $this->last = $this->current;

    /* Look for pressed tab button */
    foreach ($this->by_object as $class => &$obj) {
      if (isset($_POST[$class]) || (isset($_POST['arg']) && $_POST['arg'] == "$class")) {
        $this->current = $class;
        break;
      }
    }

    /* Save last tab */
    if ($this->last != "") {
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->last, "Saving");

      if (!$this->multiple_support_active) {
        $this->by_object[$this->last]->save_object();
      } else {
        $this->by_object[$this->last]->multiple_save_object();
      }
    }
  }

  /*!
   * \brief Generate the tab classes
   *
   * \param boolean $disabled false
   */
  function gen_tabs($disabled = FALSE)
  {
    $display = "";
    if (!$disabled) {
      $display .= "<input type=\"hidden\" name=\"arg\" value=\"\">";
    }
    $display .= "<table summary=\"\" cellpadding=0 cellspacing=0 border=0 style=\"width:100%;\"><tr>";
    $index = 0;
    $style = array("tab_left", "tab_active", "tab_near_active", "tab_right");
    foreach ($this->by_name as $class => $name) {

      /* Activate right tabs with style "tab_right"
         Activate near current with style "tab_near_active" */
      if ($index == 2 || $index == 1) {
        $index++;
      }

      /* Activate current tab with style "tab_active " */
      if ($class == $this->current) {
        $index++;
      }

      /* Paint tab */
      $display .= "<td style=\"vertical-align:bottom;width:1px;white-space:nowrap;\">";

      /* Shorten string if its too long for the tab headers*/
      $title = _($name);
      if (mb_strlen($title, 'UTF-8') > 28) {
        $title = mb_substr($title, 0, 25, 'UTF-8')."...";
      }

      /* nobr causes w3c warnings so we use &nbsp; to keep the tab name in one line */
      $title = str_replace(" ", "&nbsp;", $title);

      /* Take care about notifications */
      $obj = $this->by_object[$class];
      if ($this->by_object[$class]->pl_notify && ($obj->is_account || $obj->ignore_account)) {
        $notify = "id=\"notify\"";
      } else {
        $notify = "";
      }

      if ($disabled) {
        $display.= "<div ".$notify." class=\"$style[$index]\"
          style=' font-family:arial,helvetica,sans-serif;
                  font-weight:bold;
                  font-size:13px;
                  color: gray;'
          title=\"$title\">".$title."</div>";
      }elseif (session::global_get('js')==FALSE){
        $display.= "<div ".$notify." class=\"$style[$index]\"><input type=\"submit\" name=\"$class\"".
          " class=\"$style[$index]\" value=\"$title\"></div></td>";
      } else {
        $display.= "<div ".$notify." class=\"$style[$index]\"><a class=\"$style[$index]\" onclick=\"return true;\" href=\"javascript:document.mainform.arg.value='$class';document.mainform.submit();\">$title</a></div></td>";
      }
    }
    $display.= "<td style=\"vertical-align:bottom;\">\n";
    $display.= "<div class=\"tab_border\">&nbsp;</div></td></tr></table>";

    return($display);
  }

  /*!
   * \brief Remove object from parent
   */
  function delete()
  {
    /* Check if all plugins will ACK for deletion */
    foreach (array_reverse($this->by_object) as $key => $obj){
      $reason= $obj->allow_remove();
      if ($reason != ""){
        msg_dialog::display(_("Warning"), sprintf(_("Delete process has been canceled by plugin '%s': %s"), $key, $reason), WARNING_DIALOG);
        return;
      }
    }

    /* Delete for all plugins */
    foreach (array_reverse($this->by_object) as $obj){
      $obj->remove_from_parent();
    }
  }

  /*
   * \brief Ask all plugins for needed password changes
   */
  function password_change_needed()
  {
    /* Ask all plugins for needed password changes */
    foreach ($this->by_object as &$obj){
      if ($obj->password_change_needed()){
        return TRUE;
      }
    }

    return FALSE;
  }

  /*!
   * \brief Check
   *
   * \param boolean $ignore_account false
   */
  function check($ignore_account = FALSE)
  {
    $messages = array();

    $current_set = FALSE;

    /* Check all plugins */
    foreach ($this->by_object as $key => &$obj) {
      if ($obj->is_account || $ignore_account || $obj->ignore_account) {
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$key, "Checking");

        if (!$this->multiple_support_active) {
          $msg = $obj->check();
        } else {
          $msg = $obj->multiple_check();
        }

        if (count($msg)) {
          $obj->pl_notify = TRUE;
          if (!$current_set) {
            $current_set    = TRUE;
            $this->current  = $key;
            $messages       = $msg;
          }
        } else {
          $obj->pl_notify = FALSE;
        }
      } else {
        $obj->pl_notify = FALSE;
      }
    }
    return $messages;
  }

  /*
   * \brief Save object in the tab
   *
   * \param boolean $ignore_account false
   */
  function save($ignore_account = FALSE)
  {
    /* Save all plugins */
    foreach ($this->by_object as $key => &$obj){
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
          $key, "Saving");

      $obj->dn = $this->dn;

      if (!$obj instanceof plugin && !$obj instanceOf management) {
        trigger_error("Something went wrong while saving ".$obj->dn.". Object class '".get_class($obj)."'.");
      } else {
        if ($obj->is_account || $ignore_account || $obj->ignore_account) {
          if ($obj->save() == 1) {
            return 1;
          }
        } else {
          $obj->remove_from_parent();
        }
      }
    }
    return 0;
  }

  /*!
   * \brief Adapt from template
   *
   * \param string $dn The DN
   *
   * \param array $skip
   */
  function adapt_from_template($dn, $skip= array())
  {
    foreach ($this->by_object as $key => &$obj){
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
          $key, "Adapting");
      $obj->parent= &$this;
      $obj->adapt_from_template($dn, $skip);
    }
  }

  /*!
   * \brief Save attributes posted by copy & paste dialog
   */
  function postCopyHook()
  {
    foreach ($this->by_object as &$obj) {
      if ($obj->is_account || $obj->ignore_account) {
        $obj->postCopyHook();
      }
    }
  }

  /*!
   * \brief Add special Tabs
   */
  function addSpecialTabs($hide_refs = FALSE, $hide_acls = FALSE)
  {
    if (!$hide_acls) {
      $this->by_name['acl']           = _("ACL");
      $this->by_object['acl']         = new acl($this->config, $this->dn, $this->getBaseObject());
      $this->by_object['acl']->parent = &$this;
    }
    if (!$hide_refs) {
      $this->by_name['reference']           = _("References");
      $this->by_object['reference']         = new reference($this->config, $this->dn, $this->getBaseObject());
      $this->by_object['reference']->parent = &$this;
    }
  }

  /*!
   * \brief Set acl base
   *
   * \param string $base The new acl base
   */
  function set_acl_base($base = "")
  {
    /* Update reference, transfer variables */
    $first = ($base == "");
    foreach ($this->by_object as &$obj) {
      if ($first) {
        $first  = FALSE;
        $base   = $obj->acl_base;
      } else {
        $obj->set_acl_base($base);
      }
    }
  }


  /*!
   * \brief Checks if one of the used tab plugins supports multiple edit.
   *
   * \return boolean TRUE if at least one plugins supports multiple edit.
   */
  function multiple_support_available()
  {
    foreach($this->by_object as $name => $obj){
      if($obj->multiple_support){
        return(TRUE);
      }
    }
    return(FALSE);
  }


  /*!
   * \brief Enables multiple edit support for the given tab.
   *
   * All unsupported plugins will be disabled.
   *
   * \return boolean TRUE if at least one plugin supports multiple edit
   */
  function enable_multiple_support()
  {
    if(!$this->multiple_support_available()){
      return(FALSE);
    }else{
      $this->multiple_support_active = TRUE;
      foreach($this->by_object as $name => $obj){
        if($obj->multiple_support){
          $this->by_object[$name]->enable_multiple_support();
        }else{
          unset($this->by_object[$name]);
          unset($this->by_name[$name]);
        }
      }
    }
    return(TRUE);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
