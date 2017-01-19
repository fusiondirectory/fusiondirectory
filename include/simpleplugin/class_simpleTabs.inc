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
  var $acl;
  var $is_template;

  public    $objectType   = FALSE;
  protected $specialTabs  = TRUE;
  protected $plNotify     = array();

  var $last       = "";
  var $current    = "";
  var $disabled   = "";
  var $by_name    = array();
  var $by_object  = array();
  var $acl_category;

  /* A parent object if available, e.g. a management class. */
  var $parent = NULL;

  var $baseclass = "";

  /*!
   * \brief Tabs classes constructor
   * */
  function __construct($type, $dn, $attrs_object = NULL)
  {
    global $config;

    if (!is_string($type)) {
      die('deprecated call to old tabclass constructor');
    }

    $infos              = objects::infos($type);
    $data               = $config->data['TABS'][$infos['tabGroup']];
    $this->acl_category = $infos['aclCategory'];
    $this->objectType   = $type;
    $this->dn           = $dn;

    if (!count($data)) {
      $data[] = array('CLASS' => 'plugin','NAME' => 'Error');
      msg_dialog::display(_('Error'),
        sprintf(_('No plugin definitions found to initialize "%s", please check your configuration file.'), get_class($this)),
        ERROR_DIALOG);
    }

    $baseobject         = NULL;
    foreach ($data as $tab) {
      if (!plugin_available($tab['CLASS'])) {
        continue;
      }

      $this->by_name[$tab['CLASS']]   = $tab['NAME'];
      $this->plNotify[$tab['CLASS']]  = FALSE;

      if ($baseobject === NULL) {
        $baseobject = new $tab['CLASS']($this->dn, $attrs_object, $this, TRUE);
        $this->by_object[$tab['CLASS']] = $baseobject;
        $this->baseclass                = $tab['CLASS'];
      } else {
        $this->by_object[$tab['CLASS']] = new $tab['CLASS']($this->dn, $baseobject, $this, FALSE);
      }

      $this->by_object[$tab['CLASS']]->set_acl_category($this->acl_category);
    }

    /* Initialize current */
    $this->current = $this->baseclass;

    if ($infos['mainAttr']) {
      $baseobject = $this->getBaseObject();
      if (
        ($baseobject instanceof simplePlugin) &&
        ($baseobject->attributesAccess[$infos['mainAttr']]->getUnique() === FALSE)
      ) {
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
        $baseobject = new $class($this->dn, NULL, $this, TRUE);
        $this->by_object[$name] = $baseobject;
      } else {
        $this->by_object[$name] = new $class($this->dn, $baseobject, $this, FALSE);
      }
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
      if (!($baseobject instanceOf simplePlugin) && is_object($baseobject->baseSelector)) {
        /* For some plugins not yet migrated to simple plugin. */
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

  /*! \brief Indicates if this tab class is read-only (because of locks) */
  function readOnly()
  {
    return $this->getBaseObject()->readOnly();
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
    $display = '<div class="tab-content">'."\n";

    $display .= $this->by_object[$this->current]->execute();

    /* Build tab line */
    $tabs   = $this->gen_tabs($this->dialogOpened());

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
    $display  .= '<table class="tabs-header"><tbody><tr>';
    $index    = 0;
    $style    = array('tab-left', 'tab-active', 'tab-right');
    foreach ($this->by_name as $class => $name) {

      /* Activate right tabs with style "tab-right" */
      if ($index == 1) {
        $index++;
      } elseif ($class == $this->current) {
        /* Activate current tab with style "tab-active " */
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
      if ($this->plNotify[$class] && ($obj->is_account || $obj->ignore_account)) {
        $cssClasses .= ' tab-notify';
      }
      if ($disabled) {
        $cssClasses .= ' tab-disabled';
      }
      if (!$obj->is_account && !$obj->ignore_account) {
        $cssClasses .= ' tab-inactive';
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
    $display .= '<div class="tab-border">&nbsp;</div></td></tr></tbody></table>';

    return $display;
  }

  /*!
   * \brief Remove object from parent
   */
  function delete()
  {
    /* Delete for all plugins */
    foreach (array_reverse($this->by_object) as $obj) {
      $errors = $obj->remove_from_parent();
      if (!empty($errors)) {
        msg_dialog::displayChecks($errors);
      }
    }
  }

  /*!
   * \brief Check
   *
   * \param boolean $ignore_account false
   */
  protected function check()
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
    foreach ($this->by_object as $key => $obj) {
      $this->plNotify[$key] = FALSE;
      if (($obj->is_account || $obj->ignore_account) && (!$obj->is_template)) {
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $key, "Checking");

        $msg = $obj->check();

        if (count($msg)) {
          $this->plNotify[$key] = TRUE;
          if (!$current_set) {
            $current_set    = TRUE;
            $this->current  = $key;
            $messages       = $msg;
          }
        }
      }
    }

    return $messages;
  }

  /*
   * \brief Save object in the tab
   *
   * \param boolean $ignore_account false
   */
  function save()
  {
    global $ui;
    $messages = $this->check();
    if (!empty($messages)) {
      return $messages;
    }

    $baseobject = $this->getBaseObject();
    $old_dn     = $this->dn;
    $new_dn     = $baseobject->compute_dn();
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $new_dn, "Saving");

    $errors   = array();
    $creation = ($this->dn == 'new');

    /* Move ? */
    if ($this->dn != $new_dn) {
      /* Write entry on new 'dn' */
      if ($creation) {
        /* use the new one */
        $this->dn = $new_dn;
      } else {
        if ($baseobject->move($this->dn, $new_dn)) {
          $this->dn = $new_dn;
        } else {
          $errors[] = sprintf(_('Move from "%s" to "%s" failed'), $this->dn, $new_dn);
        }
      }
    }

    /* Save all plugins */
    $first = TRUE;
    foreach ($this->by_object as $key => $obj) {
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $key, 'Saving');

      $obj->dn = $this->dn;

      if (!($obj instanceof simplePlugin) && !($obj instanceOf simpleManagement)) {
        trigger_error('Something went wrong while saving '.$obj->dn.'. Class "'.get_class($obj).'".');
      } else {
        if ($obj->is_account || $obj->ignore_account) {
          $result = $obj->save();
        } else {
          $result = $obj->remove_from_parent();
        }
        if (!empty($result)) {
          if ($creation && $first) {
            /* If the fail of main tab fails for a creation, cancel the save of other tabs */
            $this->dn = $old_dn;
            $obj->dn  = $this->dn;
            return $result;
          }
          $errors = array_merge($errors, $result);
        }
      }
      if ($first) {
        $first = FALSE;
      }
    }

    if (empty($errors) && ($this->dn == $ui->dn)) {
      /* If the logged in user was edited, update his information */
      $ui->loadLDAPInfo();
    }

    if (!empty($errors)) {
      $this->dn = $old_dn;
      foreach ($this->by_object as $obj) {
        $obj->dn = $this->dn;
      }
    }

    return $errors;
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
    global $config;
    $baseobject = $this->getBaseObject();
    foreach ($config->data['TABS']['SPECIALTABS'] as $tab) {
      if (!plugin_available($tab['CLASS'])) {
        continue;
      }

      $this->by_name[$tab['CLASS']]   = $tab['NAME'];
      $this->plNotify[$tab['CLASS']]  = FALSE;
      $this->by_object[$tab['CLASS']] = new $tab['CLASS']($this->dn, $baseobject, $this, FALSE);
      $this->by_object[$tab['CLASS']]->set_acl_category($this->acl_category);
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

  public function setNeedEditMode ($bool)
  {
    foreach ($this->by_object as &$obj) {
      $obj->setNeedEditMode($bool);
    }
    unset($obj);
  }

  public function dialogOpened ()
  {
    return $this->by_object[$this->current]->is_modal_dialog();
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
