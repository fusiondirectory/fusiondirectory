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
class simpleTabs implements FusionDirectoryDialog
{
  var $dn;
  var $acl;
  var $is_template;

  public $objectType      = FALSE;
  protected $specialTabs  = TRUE;
  protected $plNotify     = [];

  var $last       = "";
  var $current    = "";
  var $disabled   = "";
  var $by_name    = [];
  var $by_object  = [];
  var $acl_category;

  /* A parent object if available, e.g. a management class. */
  var $parent = NULL;

  var $baseclass = "";

  public $ignoreAcls = FALSE;

  /*!
   * \brief Tabs classes constructor
   * */
  function __construct (string $type, $dn, $attrs_object = NULL)
  {
    global $config;

    $infos              = objects::infos($type);
    $data               = $config->data['TABS'][$infos['tabGroup']];
    $this->acl_category = $infos['aclCategory'];
    $this->objectType   = $type;
    $this->dn           = $dn;

    if (!count($data)) {
      throw new FusionDirectoryException(
        sprintf(
          _('No plugin definitions found to initialize "%s", please check your configuration file.'),
          get_class($this)
        )
      );
    }

    $baseobject         = NULL;
    foreach ($data as $tab) {
      if (!plugin_available($tab['CLASS'])) {
        continue;
      }
      if (!is_a($tab['CLASS'], 'SimpleTab', TRUE)) {
        throw new FusionDirectoryException('Invalid class '.$tab['CLASS'].' found in '.$type.' tab list');
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
        $baseobject->attributesAccess[$infos['mainAttr']]->setUnique('one');
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
  function re_init ()
  {
    $baseobject = NULL;
    foreach ($this->by_object as $name => $object) {
      $class = get_class($object);
      if (in_array($class, ["reference","acl"])) {
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

  /*!
   * \brief Returns the list of tabs which may appear for a given object type
   */
  public static function getPotentialTabList (string $type, array $infos): array
  {
    global $config;

    return $config->data['TABS'][$infos['tabGroup']];
  }

  /*!
   * \brief Sets the active tabs from this instance to an other one. Used by templates
   */
  function setActiveTabs (&$tabObject)
  {
    foreach ($this->by_object as $class => $plugin) {
      if ($plugin->isActive()) {
        $tabObject->by_object[$class]->is_account = $plugin->is_account;
      }
    }
  }

  function resetCopyInfos ()
  {
    $this->dn = 'new';
    foreach ($this->by_object as &$obj) {
      $obj->resetCopyInfos();
    }
    unset($obj);
  }

  function resetBase ()
  {
    global $ui;
    $baseobject = $this->getBaseObject();
    if (isset($baseobject->base)) {
      logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $baseobject->base, 'Fixing base');
      $baseobject->base = $ui->getCurrentBase();
      logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $baseobject->base, 'Fixed base');
    } else {
      logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, '', 'no base');
    }
  }

  function getBaseObject ()
  {
    return $this->by_object[$this->baseclass];
  }

  /*! \brief Indicates if this tab class is read-only (because of locks) */
  function readOnly ()
  {
    return $this->getBaseObject()->readOnly();
  }

  /*!
   * \brief Save a tabs object
   */
  function save_object ()
  {
    trigger_error('obsolete');
    $this->readPost();
  }

  public function readPostTabChange ()
  {
    /* Look for pressed tab button */
    foreach (array_keys($this->by_object) as $class) {
      if (isset($_POST[$class]) || (isset($_POST['arg']) && ($_POST['arg'] == $class))) {
        $this->current = $class;
        break;
      }
    }
  }

  public function readPost ()
  {
    /* Ensure that the currently selected tab is valid. */
    if (!isset($this->by_name[$this->current])) {
      $this->current = key($this->by_name);
    }

    /* Rotate current to last */
    $this->last = $this->current;

    /* Save last tab */
    if ($this->last != '') {
      logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->last, 'Reading POST');

      $this->by_object[$this->last]->readPost();
    }

    $this->readPostTabChange();
  }

  function execute ()
  {
    trigger_error('obsolete');
    $this->update();
    return $this->render();
  }

  public function update (): bool
  {
    if (($this->last != $this->current) && ($this->last != '')) {
      $this->by_object[$this->last]->update();
    }

    $this->by_object[$this->current]->update();

    return TRUE;
  }

  /*! \brief This function display the plugin and return the html code
   */
  public function render (): string
  {
    /* Compute tab content */
    $tabContent = $this->by_object[$this->current]->render();

    /* Build tab line */
    $display = $this->renderTabList($this->dialogOpened());

    /* Show object */
    $display .= '<div class="tab-content">'."\n";
    $display .= $tabContent;
    $display .= '</div>';

    return $display;
  }

  /*!
   * \brief Load tab list if needed
   */
  public function loadTabs ()
  {
  }

  /*!
   * \brief Generate the tab classes
   *
   * \param boolean $disabled false
   */
  protected function renderTabList (bool $disabled = FALSE): string
  {
    $display = '';
    if (!$disabled) {
      $display .= '<input type="hidden" name="arg" value=""/>';
    }
    $display  .= '<table class="tabs-header"><tbody><tr>';
    $index    = 0;
    $style    = ['tab-left', 'tab-active', 'tab-right'];
    foreach ($this->by_name as $class => $name) {
      $obj = $this->by_object[$class];

      /* Activate right tabs with style "tab-right"
       * Activate current tab with style "tab-active " */
      if (($index == 1) || ($class == $this->current)) {
        $index++;
      }

      /* Paint tab */
      if ($obj->aclHasPermissions() && empty($obj->aclGetPermissions(''))) {
        $display .= '<td class="nonreadable">';
      } else {
        $display .= '<td>';
      }

      /* Shorten string if its too long for the tab headers*/
      $title = _($name);
      if (mb_strlen($title, 'UTF-8') > 28) {
        $title = mb_substr($title, 0, 25, 'UTF-8')."...";
      }

      $cssClasses = $style[$index];

      /* Take care about notifications */
      if ($this->plNotify[$class] && $obj->isActive()) {
        $cssClasses .= ' tab-notify';
      }
      if ($disabled) {
        $cssClasses .= ' tab-disabled';
      }
      if (!$obj->isActive()) {
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
      $display .= htmlescape($title).'</a></div></td>';
    }

    $display .= "<td>\n";
    $display .= '<div class="tab-border">&nbsp;</div></td></tr></tbody></table>';

    return $display;
  }

  /*!
   * \brief Remove object from LDAP
   */
  function delete ()
  {
    if (!$this->getBaseObject()->acl_is_removeable()) {
      $error = new SimplePluginPermissionError($this, msgPool::permDelete($this->getBaseObject()->dn));
      $error->display();
      return FALSE;
    }

    /* Delete all tabs in reverse order */
    foreach (array_reverse($this->by_object) as $obj) {
      $errors = $obj->remove(TRUE);
      if (!empty($errors)) {
        msg_dialog::displayChecks($errors);
        return FALSE;
      }
    }
    return TRUE;
  }

  /*!
   * \brief Check
   */
  public function check (): array
  {
    global $config;
    $messages = [];

    if ($this->getBaseObject()->is_template) {
      $ldap = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);
      $filter = '(&(objectClass=fdTemplate)(cn='.ldap_escape_f($this->getBaseObject()->_template_cn).'))';
      $ldap->search($filter, ['dn']);
      while ($attrs = $ldap->fetch()) {
        if ($attrs['dn'] != $this->getBaseObject()->dn) {
          $messages[] = new SimplePluginCheckError(
            $this->getBaseObject()->attributesAccess['_template_cn'],
            msgPool::duplicated($this->getBaseObject()->attributesAccess['_template_cn']->getLabel(), $attrs['dn'])
          );
        }
      }
      return $messages;
    }

    $current_set = FALSE;

    /* Check all plugins */
    foreach ($this->by_object as $key => $obj) {
      $this->plNotify[$key] = FALSE;
      if ($obj->isActive() && (!$obj->is_template)) {
        logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $key, "Checking");

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
   */
  function save ()
  {
    global $ui;
    $messages = $this->check();
    if (!empty($messages)) {
      return $messages;
    }

    $baseobject = $this->getBaseObject();
    $old_dn     = $this->dn;
    try {
      $new_dn     = $baseobject->compute_dn();
      logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $new_dn, 'Saving');
    } catch (FusionDirectoryException $e) {
      return [
        new SimplePluginError(
          $baseobject,
          htmlescape(sprintf(_('Failed to compute DN for object: %s'), $e->getMessage())),
          0,
          $e
        )
      ];
    }

    $errors   = [];
    $creation = ($this->dn == 'new');

    /* Move ? */
    if ($this->dn != $new_dn) {
      /* Write entry on new 'dn' */
      if ($creation) {
        /* use the new one */
        $this->dn = $new_dn;
      } else {
        if (($error = $baseobject->move($this->dn, $new_dn)) === TRUE) {
          $this->dn = $new_dn;
        } else {
          $errors[] = new SimplePluginError(
            $baseobject,
            htmlescape(sprintf(_('Move from "%s" to "%s" failed: %s'), $this->dn, $new_dn, $error))
          );
          return $errors;
        }
      }
    }

    /* Save all plugins */
    $first = TRUE;
    foreach ($this->by_object as $key => $obj) {
      logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $key, 'Saving');

      $obj->dn = $this->dn;

      if ($obj->isActive()) {
        $result = $obj->save();
      } else {
        $result = $obj->remove(FALSE);
      }
      if (!empty($result)) {
        if ($creation && $first) {
          /* If the save of main tab fails for a creation, cancel the save of other tabs */
          $this->dn = $old_dn;
          $obj->dn  = $this->dn;
          return $result;
        }
        $errors = array_merge($errors, $result);
      }
      if ($first) {
        $first = FALSE;
      }
    }

    if (empty($errors) && isset($ui->dn) && ($this->dn == $ui->dn)) {
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
   * \param array $attrs an LDAP-like values array
   * \param array $skip Attributes to skip
   */
  function adapt_from_template (array $attrs, array $skip = [])
  {
    foreach ($this->by_object as $key => &$obj) {
      logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $key, "Adapting");
      $obj->parent = &$this;
      $obj->adapt_from_template($attrs, $skip);
    }
    unset($obj);
  }

  /*!
   * \brief Add special Tabs
   */
  function addSpecialTabs ()
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
   * \brief Get LDAP base to use for ACL checks
   */
  function getAclBase (): string
  {
    return $this->getBaseObject()->getAclBase(FALSE);
  }

  function setTemplateMode ($cn)
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

  public function setIgnoreAcls ($bool)
  {
    $this->ignoreAcls = $bool;
  }

  public function dialogOpened (): bool
  {
    return $this->by_object[$this->current]->is_modal_dialog();
  }

  function objectInfos ()
  {
    if ($this->objectType === FALSE) {
      return FALSE;
    }
    return objects::infos($this->objectType);
  }

  /* Return tab or service if activated, FALSE otherwise */
  function getTabOrServiceObject ($tab)
  {
    if (isset($this->by_object[$tab]) && $this->by_object[$tab]->isActive()) {
      return $this->by_object[$tab];
    } elseif (is_subclass_of($tab, 'simpleService') && isset($this->by_object['servicesManagement'])) {
      return $this->by_object['servicesManagement']->getServiceObject($tab);
    } else {
      return FALSE;
    }
  }
}

/*!
 * \brief For objects which does not support special tabs such as LDAP and references
 */
class simpleTabs_noSpecial extends simpleTabs
{
  protected $specialTabs = FALSE;
}
