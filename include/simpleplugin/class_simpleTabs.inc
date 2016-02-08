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
 * \file class_simpleTabs.inc
 * Source code for class simpleTabs
 */

/*!
 * \brief This class contains all function to manage tabs classes
 */
class simpleTabs
{
  var $dn;
  var $config;
  var $acl;
  var $is_template;

  public    $objectType   = FALSE;
  protected $specialTabs  = TRUE;

  var $last       = "";
  var $current    = "";
  var $disabled   = "";
  var $by_name    = array();
  var $by_object  = array();
  var $acl_category;

  var $parent = NULL; // A parent object if available, e.g. a management class.

  var $read_only = FALSE; // Used when the entry is opened as "readonly" due to locks.

  var $baseclass = "";

  /*!
   * \brief Tabs classes constructor
   *
   * You should either call __construct($type, $dn, [$copied_object]) or (deprecated) __construct($config, $data, $dn, $category, [$copied_object])
   * */
  function __construct()
  {
    global $config;
    $attrs_object = NULL;
    if (is_string(func_get_arg(0))) {
      $type   = func_get_arg(0);
      $dn     = func_get_arg(1);
      $infos  = objects::infos($type);

      $data           = $config->data['TABS'][$infos['tabGroup']];
      $acl_category   = $infos['aclCategory'];
      if (func_num_args() >= 3) {
        $attrs_object  = func_get_arg(2);
      }
      $this->objectType = $type;
    } else {
      /* Deprecated, used by old management classes */
      //$config_object  = func_get_arg(0); // ignored
      $data           = func_get_arg(1);
      $dn             = func_get_arg(2);
      $acl_category   = func_get_arg(3);
      if (func_num_args() >= 5) {
        $attrs_object  = func_get_arg(4);
      }
    }

    /* Save dn */
    $this->dn     = $dn;
    $this->config = &$config;

    if (!count($data)) {
      $data[] = array("CLASS" => 'plugin',"NAME" => 'Error');
      msg_dialog::display(_("Error"),
        sprintf(_("No plugin definitions found to initialize '%s', please check your configuration file."), get_class($this)),
        ERROR_DIALOG);
    }

    $baseobject         = NULL;
    $this->acl_category = $acl_category;
    foreach ($data as &$tab) {
      if (!plugin_available($tab['CLASS'])) {
        continue;
      }

      $this->by_name[$tab['CLASS']] = $tab['NAME'];

      if ($baseobject === NULL) {
        $baseobject = new $tab['CLASS']($this->config, $this->dn, $attrs_object);
        $this->by_object[$tab['CLASS']] = $baseobject;
        $this->baseclass                = $tab['CLASS'];
      } else {
        $this->by_object[$tab['CLASS']] = new $tab['CLASS']($this->config, $this->dn, $baseobject);
      }

      $this->read_only |= $this->by_object[$tab['CLASS']]->read_only;
      $this->by_object[$tab['CLASS']]->parent = &$this;
      $this->by_object[$tab['CLASS']]->set_acl_category($this->acl_category);
    }
    unset($tab);

    /* Initialize current */
    $this->current = $this->baseclass;

    $infos = $this->objectInfos();
    if ($infos && $infos['mainAttr']) {
      $baseobject = $this->getBaseObject();
      if (($baseobject instanceof simplePlugin) && $baseobject->attributesAccess[$infos['mainAttr']]->getUnique() === FALSE) {
        $baseobject->attributesAccess[$infos['mainAttr']]->setUnique(TRUE);
      }
    }

    if ($this->specialTabs) {
      /* Add references/acls/snapshots */
      $this->addSpecialTabs();
    }
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
        $this->by_object[$name] = $baseobject;
      } else {
        $this->by_object[$name] = new $class($this->config, $this->dn, $baseobject);
      }
      $this->by_object[$name]->parent = &$this;
      $this->by_object[$name]->set_acl_category($this->acl_category);
    }
  }

  function resetCopyInfos()
  {
    $this->dn = 'new';
    foreach ($this->by_object as &$obj) {
      $obj->resetCopyInfos();
    }
    unset($obj);
  }

  function resetBase()
  {
    $baseobject = $this->getBaseObject();
    if (isset($baseobject->base)) {
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $baseobject->base, 'Fixing base');
      if (session::global_is_set('CurrentMainBase')) {
        $baseobject->base = dn2base('cn=dummy,'.session::global_get('CurrentMainBase'));
      } else {
        $baseobject->base = dn2base(get_userinfo()->dn);
      }
      if (!($baseobject instanceOf simplePlugin) && is_object($baseobject->baseSelector)) { // For some plugins not yet migrated to simple plugin.
        $baseobject->baseSelector->setBase($baseobject->base);
      }
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $baseobject->base, 'Fixed base');
    } else {
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, '', 'no base');
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
    /* Look for pressed tab button first */
    foreach ($this->by_object as $class => &$obj) {
      if (isset($_POST[$class]) || (isset($_POST['arg']) && $_POST['arg'] == "$class")) {
        $this->current = $class;
        break;
      }
    }
    unset($obj);

    /* Show object */
    $display = '<div class="tab_content">'."\n";

    $display .= $this->by_object[$this->current]->execute();
    $modal_dialog = $this->by_object[$this->current]->is_modal_dialog();

    /* Build tab line */
    $tabs   = $this->gen_tabs($modal_dialog);

    /* Footer for tabbed dialog */
    $display = $tabs.$display.'</div>';

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

    /* Save last tab */
    if ($this->last != "") {
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->last, "Saving");

      $this->by_object[$this->last]->save_object();
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
      $display .= '<input type="hidden" name="arg" value=""/>';
    }
    $display  .= '<table class="tabs_header"><tbody><tr>';
    $index    = 0;
    $style    = array('tab_left', 'tab_active', 'tab_right');
    foreach ($this->by_name as $class => $name) {

      /* Activate right tabs with style "tab_right" */
      if ($index == 1) {
        $index++;
      } elseif ($class == $this->current) {
        /* Activate current tab with style "tab_active " */
        $index++;
      }

      /* Paint tab */
      $display .= '<td>';

      /* Shorten string if its too long for the tab headers*/
      $title = _($name);
      if (mb_strlen($title, 'UTF-8') > 28) {
        $title = mb_substr($title, 0, 25, 'UTF-8')."...";
      }

      /* nobr causes w3c warnings so we use &nbsp; to keep the tab name in one line */
      $title = str_replace(' ', '&nbsp;', $title);

      $cssClasses = $style[$index];

      /* Take care about notifications */
      $obj = $this->by_object[$class];
      if ($this->by_object[$class]->pl_notify && ($obj->is_account || $obj->ignore_account)) {
        $cssClasses .= ' tab_notify';
      }
      if ($disabled) {
        $cssClasses .= ' tab_disabled';
      }
      if (!$obj->is_account && !$obj->ignore_account) {
        $cssClasses .= ' tab_inactive';
      }


      $display .= '<div class="'.$cssClasses.'">';
      if ($disabled) {
        $display .= '<a>';
      } else {
        $display .= '<a '.
          'id="tab_'.$class.'" '.
          'onclick="return true;" '.
          'href="'."javascript:document.mainform.arg.value='$class';document.mainform.submit();".'">';
      }
      $display .= $title.'</a></div></td>';
    }

    $display .= "<td>\n";
    $display .= '<div class="tab_border">&nbsp;</div></td></tr></tbody></table>';

    return $display;
  }

  /*!
   * \brief Remove object from parent
   */
  function delete()
  {
    /* Check if all plugins will ACK for deletion */
    foreach (array_reverse($this->by_object) as $key => $obj) {
      $reason = $obj->allow_remove();
      if ($reason != "") {
        msg_dialog::display(_("Warning"), sprintf(_("Delete process has been canceled by plugin '%s': %s"), $key, $reason), WARNING_DIALOG);
        return;
      }
    }

    /* Delete for all plugins */
    foreach (array_reverse($this->by_object) as $obj) {
      $obj->remove_from_parent();
    }
  }

  /*!
   * \brief Check
   *
   * \param boolean $ignore_account false
   */
  function check()
  {
    global $config;
    $messages = array();

    if ($this->getBaseObject()->is_template) {
      $ldap = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);
      $filter = '(&(objectClass=fdTemplate)(cn='.ldap_escape_f($this->getBaseObject()->_template_cn).'))';
      $ldap->search($filter, array('dn'));
      while ($attrs = $ldap->fetch()) {
        if ($attrs['dn'] != $this->getBaseObject()->dn) {
          $messages[] = msgPool::duplicated($this->getBaseObject()->attributesAccess['_template_cn']->getLabel(), $attrs['dn']);
        }
      }
      return $messages;
    }

    $current_set = FALSE;

    /* Check all plugins */
    foreach ($this->by_object as $key => &$obj) {
      if (($obj->is_account || $obj->ignore_account) && (!$obj->is_template)) {
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $key, "Checking");

        $msg = $obj->check();

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
    unset($obj);

    return $messages;
  }

  /*
   * \brief Save object in the tab
   *
   * \param boolean $ignore_account false
   */
  function save()
  {
    $baseobject = $this->getBaseObject();
    $new_dn     = $baseobject->compute_dn();
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $new_dn, "Saving");

    /* Move ? */
    if ($this->dn != $new_dn) {
      /* Write entry on new 'dn' */
      if ($this->dn != 'new') {
        if ($baseobject->move($this->dn, $new_dn)) {
          $this->dn = $new_dn;
        } else {
          msg_dialog::display(_('Error'), sprintf(_('Move from "%s" to "%s" failed'), $this->dn, $new_dn), ERROR_DIALOG);
        }
      } else {
        /* use the new one */
        $this->dn = $new_dn;
      }
    }

    /* Save all plugins */
    foreach ($this->by_object as $key => &$obj) {
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $key, "Saving");

      $obj->dn = $this->dn;

      if (!$obj instanceof plugin && !$obj instanceOf management) {
        trigger_error("Something went wrong while saving ".$obj->dn.". Object class '".get_class($obj)."'.");
      } else {
        if ($obj->is_account || $obj->ignore_account) {
          if ($obj->save() == 1) {
            return 1;
          }
        } else {
          $obj->remove_from_parent();
        }
      }
    }
    unset($obj);

    return 0;
  }

  /*!
   * \brief Adapt from template
   *
   * \param string $dn The DN
   *
   * \param array $skip
   */
  function adapt_from_template($attrs, $skip = array())
  {
    foreach ($this->by_object as $key => &$obj) {
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $key, "Adapting");
      $obj->parent = &$this;
      $obj->adapt_from_template($attrs, $skip);
    }
    unset($obj);
  }

  /*!
   * \brief Add special Tabs
   */
  function addSpecialTabs()
  {
    $data       = $this->config->data['TABS']['SPECIALTABS'];
    $baseobject = $this->getBaseObject();
    foreach ($data as &$tab) {
      if (!plugin_available($tab['CLASS'])) {
        continue;
      }

      $this->by_name[$tab['CLASS']] = $tab['NAME'];
      $this->by_object[$tab['CLASS']] = new $tab['CLASS']($this->config, $this->dn, $baseobject);
      $this->by_object[$tab['CLASS']]->parent = &$this;
      $this->by_object[$tab['CLASS']]->set_acl_category($this->acl_category);
    }
    unset($tab);
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
    unset($obj);
  }

  function setTemplateMode($cn)
  {
    $this->getBaseObject()->_template_cn = $cn;

    foreach ($this->by_object as &$obj) {
      $obj->setTemplate(TRUE);
    }
    unset($obj);
  }

  function objectInfos()
  {
    if ($this->objectType === FALSE) {
      return FALSE;
    }
    return objects::infos($this->objectType);
  }
}

class simpleTabs_noSpecial extends simpleTabs
{
  protected $specialTabs = FALSE;
}
?>
