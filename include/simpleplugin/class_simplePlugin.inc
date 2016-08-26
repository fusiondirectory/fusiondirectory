<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2016  FusionDirectory

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

/*! \brief This class is made for easy plugin creation for editing LDAP attributes
 *
 */
class simplePlugin extends plugin
{
  /*! \brief This attribute store all information about attributes
   *
   */
  var $attributesInfo;
  /*! \brief This attribute store references toward attributes
   *
   * associative array that stores attributeLdapName => reference on object
   */
  var $attributesAccess = array();

  /*! \brief Do we want a header allowing to able/disable this plugin
   *
   */
  protected $displayHeader = FALSE;

  /*! \brief Is this plugin the main tab, the one that handle the object itseld
   *
   */
  protected $mainTab = FALSE;

  protected $header = "";

  public $orig_dn = "";

  protected $templatePath;

  /*! \brief Are we executed in a edit-mode environment? (this is FALSE if we're called from management, TRUE if we're called from a main.inc)
   */
  protected $needEditMode = FALSE;

  /*! \brief ldap error cache
   *
   */
  protected $ldap_error;

  /*! \brief Attributes that needs to be initialized before the others.
   *
   */
  protected $preInitAttributes = array();

  /*! \brief FALSE to disable inheritance. Array like array ('objectClass' => 'attribute') to specify oc of the groups it might be inherited from
   *
   */
  protected $inheritance      = FALSE;
  protected $member_of_group  = FALSE;
  protected $editing_group    = NULL;
  protected $group_attrs      = array();

  /*! \brief constructor
   *
   *  \param string $dn The dn of this instance
   *  \param Object $object An object to copy values from
   *  \param array $attributesInfo An attributesInfo array, if NULL, getAttributesInfo will be used.
   *
   */
  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE, $attributesInfo = NULL)
  {
    global $config;

    $this->parent   = $parent;
    $this->mainTab  = $mainTab;

    if ($attributesInfo === NULL) {
      $attributesInfo = $this->getAttributesInfo();
    }
    if (!$this->displayHeader) {
      // If we don't display the header to activate/deactive the plugin, that means it's always activated
      $this->ignore_account = TRUE;
    }

    $this->attributesInfo = array();
    foreach ($attributesInfo as $section => $sectionInfo) {
      $attrs = array();
      foreach ($sectionInfo['attrs'] as $attr) {
        $name = $attr->getLdapName();
        if (isset($attrs[$name])) {
          // We check that there is no duplicated attribute name
          trigger_error("Duplicated attribute LDAP name '$name' in a simplePlugin subclass");
        }
        // We make so that attribute have their LDAP name as key
        // That allow the plugin to use $this->attributesInfo[$sectionName]['attrs'][$myLdapName] to retreive the attribute info.
        $attrs[$name] = $attr;
      }
      $sectionInfo['attrs']           = $attrs;
      $this->attributesInfo[$section] = $sectionInfo;
      foreach ($this->attributesInfo[$section]['attrs'] as $name => $attr) {
        if (isset($this->attributesAccess[$name])) {
          // We check that there is no duplicated attribute name
          trigger_error("Duplicated attribute LDAP name '$name' in a simplePlugin subclass");
        }
        $this->attributesAccess[$name] =& $this->attributesInfo[$section]['attrs'][$name];
        unset($this->$name);
      }
    }

    // We call plugin constructor
    parent::__construct($dn, $object);

    $this->setTemplate($this->is_template);

    $this->orig_dn = $dn;

    if ($this->mainTab) {
      $this->is_account = TRUE;
      /* TODO: When merging plugin and simplePlugin we should find a way to avoid doing a separate LDAP fetch for this */
      $this->entryCSN = getEntryCSN($this->dn);
    }

    if (!isset($this->templatePath)) {
      $this->templatePath = get_template_path('simpleplugin.tpl');
    }

    if (is_array($this->inheritance)) {
      /* Check group membership */
      $ldap = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);
      foreach ($this->inheritance as $oc => $at) {
        if ($this->mainTab) {
          $filter = '(&(objectClass='.$oc.')('.$at.'='.ldap_escape_f($this->dn).'))';
        } else {
          $filter = '(&(objectClass='.$oc.')'.$this->getObjectClassFilter().'('.$at.'='.ldap_escape_f($this->dn).'))';
        }
        $ldap->search($filter, $this->attributes);
        if ($ldap->count() == 1) {
          $this->member_of_group = TRUE;
          $attrs = $ldap->fetch();
          $this->group_attrs = $attrs;
          break;
        }
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

  function setTemplate ($bool)
  {
    $this->is_template = $bool;
    if ($this->is_template && $this->mainTab) {
      /* Unshift special section for template infos */
      $this->attributesInfo = array_merge(
        array(
          '_template' => array(
            'class' => array('fullwidth'),
            'name'  => _('Template settings'),
            'attrs' => array(
              '_template_cn' => new StringAttribute(
                _('Template name'), _('This is the name of the template'),
                '_template_cn', TRUE,
                '', 'template_cn'
              )
            )
          ),
          '_template_dummy' => array(
            'class' => array('invisible'),
            'name'  => '_template_dummy',
            'attrs' => array()
          )
        ),
        $this->attributesInfo
      );
      $this->attributesAccess['_template_cn'] =& $this->attributesInfo['_template']['attrs']['_template_cn'];
      $this->attributesAccess['_template_cn']->setInLdap(FALSE);
      $this->attributesAccess['_template_cn']->setValue($this->_template_cn);
      $this->attributesAccess['_template_cn']->setParent($this);
      unset($this->_template_cn);
    }
  }

  /*! \brief This function returns an LDAP filter for this plugin object classes
   */
  function getObjectClassFilter ()
  {
    if (!empty($this->objectclasses)) {
      return '(&(objectClass='.implode(')(objectClass=', $this->objectclasses).'))';
    } else {
      return '';
    }
  }

  protected function loadAttributes()
  {
    // We load attributes values
    // First the one flagged as preInit
    foreach ($this->preInitAttributes as $attr) {
      $this->attributesAccess[$attr]->setParent($this);
      $this->attributesAccess[$attr]->loadValue($this->attrs);
    }
    // Then the others
    foreach ($this->attributesInfo as &$sectionInfo) {
      foreach ($sectionInfo['attrs'] as $name => &$attr) {
        if (in_array($name, $this->preInitAttributes)) {
          continue; // skip the preInit ones
        }
        $attr->setParent($this);
        $attr->loadValue($this->attrs);
      }
      unset($attr);
    }
    unset($sectionInfo);
  }

  /*! \brief This function allows to use the syntax $plugin->attributeName to get attributes values
   * It calls the getValue method on the concerned attribute
   * It also adds the $plugin->attribtues syntax to get attributes list
   */
  public function __get($name)
  {
    if ($name == 'attributes') {
      $plugin = $this;
      return array_filter(array_keys($this->attributesAccess),
        function ($a) use ($plugin)
        {
          return $plugin->attributesAccess[$a]->isInLdap();
        }
      );
    } elseif (isset($this->attributesAccess[$name])) {
      return $this->attributesAccess[$name]->getValue();
    } else {
      /* Calling default behaviour */
      return $this->$name;
    }
  }

  /*! \brief This function allows to use the syntax $plugin->attributeName to set attributes values

    It calls the setValue method on the concerned attribute
   */
  public function __set($name, $value)
  {
    if ($name == 'attributes') {
      trigger_error('Tried to set obsolete attribute "attributes" (it is now dynamic)');
    } elseif (isset($this->attributesAccess[$name])) {
      $this->attributesAccess[$name]->setValue($value);
    } else {
      /* Calling default behaviour */
      $this->$name = $value;
    }
  }

  /*! \brief This function allows to use the syntax isset($plugin->attributeName)

    It returns FALSE if the attribute has an empty value.
   */
  public function __isset($name)
  {
    if ($name == 'attributes') {
      return TRUE;
    }
    return isset($this->attributesAccess[$name]);
  }

  /*! \brief This function returns the dn this object should have
   */
  public function compute_dn()
  {
    global $config;
    if (!$this->mainTab) {
      msg_dialog::display(_('Fatal error'), _('Only main tab can compute dn'), FATAL_ERROR_DIALOG);
      exit;
    }
    if (!isset($this->parent) || !($this->parent instanceof simpleTabs)) {
      msg_dialog::display(
        _('Fatal error'),
        sprintf(
          _('Could not compute dn: no parent tab class for "%s"'),
          get_class($this)
        ),
        FATAL_ERROR_DIALOG
      );
      exit;
    }
    $infos = $this->parent->objectInfos();
    if ($infos === FALSE) {
      msg_dialog::display(
        _('Fatal error'),
        sprintf(
          _('Could not compute dn: could not find objectType infos from tab class "%s"'),
          get_class($this->parent)
        ),
        FATAL_ERROR_DIALOG
      );
      exit;
    }
    $attr = $infos['mainAttr'];
    $ou   = $infos['ou'];
    if (isset($this->base)) {
      $base = $this->base;
    } else {
      $base = $config->current['BASE'];
    }
    if ($this->is_template) {
      $dn = 'cn='.ldap_escape_dn($this->_template_cn).',ou=templates,'.$ou.$base;
      return $dn;
    }
    return $attr.'='.ldap_escape_dn($this->attributesAccess[$attr]->computeLdapValue()).','.$ou.$base;
  }

  function getRequiredAttributes()
  {
    $tmp = array();
    foreach ($this->attributesAccess as $attr) {
      if ($attr->isRequired()) {
        $tmp[] = $attr->getLdapName();
      }
    }
    return $tmp;
  }

  function editing_group ()
  {
    if ($this->editing_group == NULL) {
      if (isset($this->parent)) {
        $this->editing_group = (get_class($this->parent->getBaseObject()) == 'ogroup');
      } else {
        return NULL;
      }
    }
    return $this->editing_group;
  }

  /*! \brief This function display the plugin and return the html code
   */
  function execute ()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "execute");

    /* Reset Lock message POST/GET check array, to prevent preg_match errors */
    session::set('LOCK_VARS_TO_USE', array());
    session::set('LOCK_VARS_USED_GET', array());
    session::set('LOCK_VARS_USED_POST', array());
    session::set('LOCK_VARS_USED_REQUEST', array());

    $this->displayPlugin  = TRUE;
    $this->header         = "";

    if (is_object($this->dialog)) {
      $dialogResult = $this->dialog->execute();
      if ($dialogResult === FALSE) {
        $this->closeDialog();
      } else {
        $this->header         = $dialogResult;
        $this->displayPlugin  = FALSE;
        return $this->header;
      }
    }

    if ($this->displayHeader) {
      /* Show tab dialog headers */
      $plInfo = pluglist::pluginInfos(get_class($this));
      if ($this->parent !== NULL) {
        $disabled = $this->acl_skip_write();
        $depends = array();
        if ($this->is_account) {
          if (isset($plInfo['plDepending'])) {
            foreach ($plInfo['plDepending'] as $plugin) {
              if (isset($this->parent->by_object[$plugin]) &&
                  $this->parent->by_object[$plugin]->is_account) {
                $disabled       = TRUE;
                $dependPlInfos  = pluglist::pluginInfos($plugin);
                $depends[]      = $dependPlInfos['plShortName'];
              }
            }
          }
          $this->header = $this->show_header(
            msgPool::removeFeaturesButton($plInfo['plShortName']),
            msgPool::featuresEnabled($plInfo['plShortName'], $depends),
            $this->is_account,
            $disabled,
            get_class($this).'_modify_state'
          );
        } else {
          if (isset($plInfo['plDepends'])) {
            foreach ($plInfo['plDepends'] as $plugin) {
              if (!isset($this->parent->by_object[$plugin]) ||
                  !$this->parent->by_object[$plugin]->is_account) {
                $disabled   = TRUE;
                $dependPlInfos  = pluglist::pluginInfos($plugin);
                $depends[]      = $dependPlInfos['plShortName'];
              }
            }
          }
          $this->header = $this->show_header(
            msgPool::addFeaturesButton($plInfo['plShortName']),
            msgPool::featuresDisabled($plInfo['plShortName'], $depends),
            $this->is_account,
            $disabled,
            get_class($this).'_modify_state'
          );
          $this->displayPlugin = FALSE;
          return $this->header.$this->inheritanceDisplay();
        }
      } elseif (!$this->is_account) {
        $this->header = '<img alt="" src="geticon.php?context=status&amp;icon=dialog-error&amp;size=16" align="middle"/>&nbsp;<b>'.
                        msgPool::noValidExtension($plInfo['plShortName'])."</b>";
        $this->header .= back_to_main();
        $this->displayPlugin = FALSE;
        return $this->header.$this->inheritanceDisplay();
      }
    }

    $smarty = get_smarty();

    $this->renderAttributes(FALSE);
    $smarty->assign("hiddenPostedInput", get_class($this)."_posted");
    if (isset($this->focusedField)) {
      $smarty->assign("focusedField", $this->focusedField);
      unset($this->focusedField);
    } else {
      $smarty->assign("focusedField", key($this->attributesAccess));
    }

    return $this->header.$smarty->fetch($this->templatePath);
  }

  function attr_is_writeable($attr)
  {
    if ($attr->getLdapName() == 'base') {
      if (!$this->acl_skip_write() && (!$this->initially_was_account || $this->acl_is_moveable() || $this->acl_is_removeable())) {
        return TRUE;
      } else {
        return FALSE;
      }
    }
    return $this->acl_is_writeable($attr->getAcl(), $this->acl_skip_write());
  }

  function renderAttributes($readOnly = FALSE)
  {
    global $ui;
    $smarty = get_smarty();

    if ($this->is_template) {
      $smarty->assign('template_cnACL', $ui->get_permissions($this->acl_base, $this->acl_category.'template', 'template_cn', $this->acl_skip_write()));
    }

    /* Handle rights to modify the base */
    if (isset($this->attributesAccess['base'])) {
      if ($this->attr_is_writeable($this->attributesAccess['base'])) {
        $smarty->assign('baseACL', 'rw');
      } else {
        $smarty->assign('baseACL', 'r');
      }
    }

    $sections = array();
    foreach ($this->attributesInfo as $section => $sectionInfo) {
      $legend = $sectionInfo['name'];
      if (isset($sectionInfo['icon'])) {
        $legend = '<img '.
                  'src="'.htmlentities($sectionInfo['icon'], ENT_COMPAT, 'UTF-8').'" '.
                  'alt="section '.$sectionInfo['name'].' icon" '.
                  'title="section '.$sectionInfo['name'].' icon" '.
                  '/>'.$legend;
      }
      $smarty->assign("section", $legend);
      $smarty->assign("sectionId", $section);
      if (isset($sectionInfo['class'])) {
        $smarty->assign("sectionClasses", ' '.join(' ', $sectionInfo['class']));
      } else {
        $smarty->assign("sectionClasses", '');
      }
      $attributes = array();
      foreach ($sectionInfo['attrs'] as $attr) {
        if ($attr->getAclInfo() !== FALSE) {
          // We assign ACLs so that attributes can use them in their template code
          $smarty->assign($attr->getAcl()."ACL", $this->getacl($attr->getAcl(), $this->acl_skip_write()));
        }
        $attr->renderAttribute($attributes, $readOnly);
      }
      $smarty->assign("attributes", $attributes);
      // We fetch each section with the section template
      if (isset($sectionInfo['template'])) {
        $displaySection = $smarty->fetch($sectionInfo['template']);
      } else {
        $displaySection = $smarty->fetch(get_template_path('simpleplugin_section.tpl'));
      }
      $sections[$section] = $displaySection;
    }
    $smarty->assign("sections", $sections);
  }

  function inheritanceDisplay()
  {
    if (!$this->member_of_group) {
      return "";
    }
    $class = get_class($this);
    $attrsWrapper = new stdClass();
    $attrsWrapper->attrs = $this->group_attrs;
    $group = new $class($this->group_attrs['dn'], $attrsWrapper, $this->parent, $this->mainTab);
    $smarty = get_smarty();

    $group->renderAttributes(TRUE);
    $smarty->assign("hiddenPostedInput", get_class($this)."_posted");

    return "<h1>Inherited informations:</h1><div></div>\n".$smarty->fetch($this->templatePath);
  }

  /*! \brief This function allows you to open a dialog
   *
   *  \param mixed $dialog The dialog object
   */
  function openDialog ($dialog)
  {
    $this->dialog = $dialog;
  }

  /*! \brief This function closes the dialog
   */
  function closeDialog ()
  {
    $this->dialog = NULL;
  }

  public function setNeedEditMode ($bool)
  {
    $this->needEditMode = $bool;
  }

  protected function acl_skip_write ()
  {
    return ($this->needEditMode && !session::is_set('edit'));
  }

  /*! \brief This function removes the object from LDAP
   */
  function remove_from_parent()
  {
    if (!$this->initially_was_account || !$this->acl_is_removeable()) {
      return;
    }
    $this->prepare_remove();
    if ($this->is_template && (!defined('_OLD_TEMPLATES_') || !_OLD_TEMPLATES_)) {
      $this->attrs = $this->templateSaveAttrs();
      $this->saved_attributes = array();
    }
    /* Pre hooks */
    $errors = $this->pre_remove();
    if (!empty($errors)) {
      return $errors;
    }
    $errors = $this->ldap_remove();
    if (!empty($errors)) {
      return $errors;
    }
    $this->post_remove();
    return;
  }

  /* Remove FusionDirectory attributes */
  protected function prepare_remove ()
  {
    global $config;
    $this->attrs = array();

    if (!$this->mainTab) {
      /* include global link_info */
      $ldap = $config->get_ldap_link();

      /* Get current objectClasses in order to add the required ones */
      $ldap->cat($this->dn);
      $tmp  = $ldap->fetch ();
      $oc   = array();
      if ($this->is_template) {
        if (isset($tmp['fdTemplateField'])) {
          foreach ($tmp['fdTemplateField'] as $tpl_field) {
            if (preg_match('/^objectClass:(.+)$/', $tpl_field, $m)) {
              $oc[] = $m[1];
            }
          }
        }
      } else {
        if (isset($tmp['objectClass'])) {
          $oc = $tmp['objectClass'];
          unset($oc['count']);
        }
      }

      /* Remove objectClasses from entry */
      $this->attrs['objectClass'] = array_remove_entries_ics($this->objectclasses, $oc);

      /* Unset attributes from entry */
      foreach ($this->attributes as $val) {
        $this->attrs["$val"] = array();
      }
    }
  }

  protected function pre_remove ()
  {
    if ($this->initially_was_account) {
      return $this->handle_pre_events('remove');
    }
  }

  protected function ldap_remove ()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    if ($this->mainTab) {
      $ldap->rmdir_recursive($this->dn);
    } else {
      $this->cleanup();
      $ldap->cd($this->dn);
      $ldap->modify($this->attrs);
    }

    $this->ldap_error = $ldap->get_error();

    if ($ldap->success()) {
      return array();
    } else {
      return msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class());
    }
  }

  protected function post_remove ()
  {
    logging::log('remove', 'plugin/'.get_class($this), $this->dn, array_keys($this->attrs), $this->ldap_error);

    /* Optionally execute a command after we're done */
    $errors = $this->handle_post_events('remove');
    if (!empty($errors)) {
      msg_dialog::displayChecks($errors);
    }
  }

  /*! \brief This function handle $_POST informations
   */
  function save_object ()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "save_object");
    if ($this->displayHeader && isset($_POST[get_class($this).'_modify_state'])) {
      if ($this->is_account && $this->acl_is_removeable()) {
        $this->is_account = FALSE;
      } elseif (!$this->is_account && $this->acl_is_createable()) {
        $this->is_account = TRUE;
      }
    }
    if (isset($_POST[get_class($this)."_posted"])) {
      // If our form has been posted
      // A first pass that loads the post values
      foreach ($this->attributesInfo as $section => &$sectionInfo) {
        foreach ($sectionInfo['attrs'] as &$attr) {
          if ($this->attr_is_writeable($attr)) {
            // Each attribute know how to read its value from POST
            $attr->loadPostValue();
          }
        }
        unset ($attrs);
      }
      unset($sectionInfo);
      // A second one that applies them. That allow complex stuff such as attribute disabling
      foreach ($this->attributesInfo as $section => &$sectionInfo) {
        foreach ($sectionInfo['attrs'] as &$attr) {
          if ($this->attr_is_writeable($attr)) {
            // Each attribute know how to read its value from POST
            $attr->applyPostValue();
          }
        }
        unset ($attrs);
      }
      unset($sectionInfo);
    }
  }

  /*! \brief This function saves the object in the LDAP
   */
  function save ()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "save");
    $this->prepare_save();
    if ($this->is_template && (!defined('_OLD_TEMPLATES_') || !_OLD_TEMPLATES_)) {
      $this->attrs = $this->templateSaveAttrs();
      $this->saved_attributes = array();
    }
    $this->cleanup();
    if (!$this->shouldSave()) {
      return; /* Nothing to do here */
    }
    /* Pre hooks */
    $errors = $this->pre_save();
    if (!empty($errors)) {
      return $errors;
    }
    /* LDAP save itself */
    $errors = $this->ldap_save();
    if (!empty($errors)) {
      return $errors;
    }
    $this->prepareNextCleanup();
    /* Post hooks and logging */
    $this->post_save();
    return;
  }

  protected function shouldSave()
  {
    if ($this->mainTab && !$this->initially_was_account) {
      return TRUE;
    }
    return !empty($this->attrs);
  }

  protected function prepare_save ()
  {
    global $config;
    /* prepare $this->attrs */
    $ldap = $config->get_ldap_link();

    $this->entryCSN = '';

    /* Start with empty array */
    $this->attrs = array();

    /* Get current objectClasses in order to add the required ones */
    $ldap->cat($this->dn, array('fdTemplateField', 'objectClass'));

    $tmp  = $ldap->fetch();
    $oc   = array();

    if ($this->is_template) {
      if (isset($tmp['fdTemplateField'])) {
        foreach ($tmp['fdTemplateField'] as $tpl_field) {
          if (preg_match('/^objectClass:(.+)$/', $tpl_field, $m)) {
            $oc[] = $m[1];
          }
        }
      }
    } else {
      if (isset($tmp['objectClass'])) {
        $oc = $tmp['objectClass'];
        unset($oc['count']);
      }
    }

    $this->attrs['objectClass'] = array_merge_unique($oc, $this->objectclasses);

    /* Fill attributes LDAP values into the attrs array */
    foreach ($this->attributesInfo as $section => $sectionInfo) {
      foreach ($sectionInfo['attrs'] as $attr) {
        $attr->fillLdapValue($this->attrs);
      }
    }
    /* Some of them have post-filling hook */
    foreach ($this->attributesInfo as $section => $sectionInfo) {
      foreach ($sectionInfo['attrs'] as $attr) {
        $attr->fillLdapValueHook($this->attrs);
      }
    }
  }

  protected function pre_save ()
  {
    if ($this->initially_was_account) {
      return $this->handle_pre_events('modify');
    } else {
      return $this->handle_pre_events('add');
    }
  }

  /* Returns an array with the errors or an empty array */
  protected function ldap_save ()
  {
    global $config;

    /* Check if this is a new entry ... add/modify */
    $ldap = $config->get_ldap_link();
    $ldap->cat($this->dn, array("objectClass"));
    if ($this->mainTab && !$this->initially_was_account) {
      if ($ldap->count()) {
        return array(sprintf(_('There is already an entry with the same dn : %s'), $this->dn));
      }
      $ldap->cd($config->current['BASE']);
      $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
      $action = "add";
    } else {
      if (!$ldap->count()) {
        return array(sprintf(_('The entry %s is not existing'), $this->dn));
      }
      $action = "modify";
    }

    $ldap->cd($this->dn);
    $ldap->$action($this->attrs);
    $this->ldap_error = $ldap->get_error();

    /* Check for errors */
    if (!$ldap->success()) {
      return array(msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }
    return array();
  }

  /*! \brief This function is called after LDAP save to do some post operations and logging
   *
   * This function calls hooks, update foreign keys and log modification
   */
  protected function post_save()
  {
    /* Propagate and log the event */
    if ($this->initially_was_account) {
      $errors = $this->handle_post_events('modify');
      logging::log('modify', 'plugin/'.get_class($this), $this->dn, array_keys($this->attrs), $this->ldap_error);
    } else {
      $errors = $this->handle_post_events('add');
      logging::log('create', 'plugin/'.get_class($this), $this->dn, array_keys($this->attrs), $this->ldap_error);
    }
    if (!empty($errors)) {
      msg_dialog::displayChecks($errors);
    }
  }

  /*! \brief This function checks the attributes values and yell if something is wrong
   */
  function check ()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, 'check');
    $messages = array();

    foreach ($this->attributesInfo as $sectionInfo) {
      foreach ($sectionInfo['attrs'] as $attr) {
        $error = $attr->check();
        if (!empty($error)) {
          if (is_array($error)) {
            $messages = array_merge($messages, $error);
          } else {
            $messages[] = $error;
          }
        }
      }
    }

    $error = $this->callHook('CHECK', array('nbCheckErrors' => count($messages)), $returnOutput);
    if (!empty($error)) {
      $messages = array_merge($messages, $error);
    }
    if (!empty($returnOutput)) {
      $messages[] = join("\n", $returnOutput);
    }

    /* Check entryCSN */
    if (!empty($this->entryCSN)) {
      $current_csn = getEntryCSN($this->dn);
      if (($current_csn != $this->entryCSN) && !empty($current_csn)) {
        $this->entryCSN = $current_csn;
        $messages[] = _('The object has changed since opened in FusionDirectory. All changes that may be done by others will get lost if you save this entry!');
      }
    }

    return $messages;
  }

  function handleForeignKeys ($olddn = NULL, $newdn = NULL, $mode = 'move')
  {
    if (($olddn !== NULL) && ($olddn == $newdn)) {
      return;
    }
    if ($this->is_template) {
      return;
    }
    $this->browseForeignKeys(
      'handle_'.$mode,
      $olddn,
      $newdn
    );
  }

  function browseForeignKeys($mode, $param1 = NULL, $param2 = NULL)
  {
    if (preg_match('/^handle_/', $mode)) {
      $olddn    = $param1;
      $newdn    = $param2;
      $classes  = array(get_class($this));
    } elseif ($mode == 'references') {
      $classes = array_keys($this->parent->by_object);
    }
    // We group by objetType concerned
    $foreignRefs = array();
    foreach ($classes as $tabclass) {
      $infos = pluglist::pluginInfos($tabclass);
      foreach ($infos['plForeignRefs'] as $field => $refs) {
        if (preg_match('/^handle_/', $mode)) {
          if (($newdn !== NULL) && ($field != 'dn') && ($mode == 'handle_move')) {
            // Move action, ignore other fields than dn
            continue;
          } elseif (($newdn === NULL) && ($olddn === NULL) && (($field == 'dn') || (!$this->attributeHaveChanged($field)))) {
            // Edit action, ignore dn changes or attributes which did not change
            continue;
          }
          // else = delete action, all fields are concerned, nothing to do here
        }
        foreach ($refs as $ref) {
          $class  = $ref[0];
          $ofield = $ref[1];
          $filter = $ref[2];
          $cinfos = pluglist::pluginInfos($class);
          foreach ($cinfos['plObjectType'] as $key => $objectType) {
            if (!is_numeric($key)) {
              $objectType = $key;
            }
            if (preg_match('/^handle_/', $mode)) {
              if ($field == 'dn') {
                $oldvalue = $olddn;
                $newvalue = $newdn;
              } elseif (($olddn !== NULL) && ($newdn === NULL)) {
                $oldvalue = $this->attributeInitialValue($field);
                $newvalue = NULL;
              } else {
                $oldvalue = $this->attributeInitialValue($field);
                $newvalue = $this->attributeValue($field);
              }
              $foreignRefs[$objectType]['refs'][$class][$ofield] =
                array(
                  'field'     => $field,
                  'oldvalue'  => $oldvalue,
                  'newvalue'  => $newvalue,
                  'tab'       => $tabclass,
                );
              $filter = plugin::tpl_parse_string($filter, array('oldvalue' => $oldvalue, 'newvalue' => $newvalue), 'ldap_escape_f');
            } elseif ($mode == 'references') {
              $foreignRefs[$objectType]['refs'][$class]['name'] = $cinfos['plShortName'];
              $foreignRefs[$objectType]['refs'][$class]['fields'][$ofield] =
                array(
                  'tab'     => $tabclass,
                  'tabname' => $this->parent->by_name[$tabclass],
                  'field'   => $field,
                  'value'   => $this->parent->by_object[$tabclass]->$field,
                );
              $filter = plugin::tpl_parse_string($filter, array('oldvalue' => $this->parent->by_object[$tabclass]->$field), 'ldap_escape_f');
            }
            if (!preg_match('/^\(.*\)$/', $filter)) {
              $filter = '('.$filter.')';
            }
            $foreignRefs[$objectType]['filters'][$filter] = $filter;
          }
        }
      }
    }

    /* Back up POST content */
    $SAVED_POST = $_POST;
    $refs = array();
    // For each concerned objectType
    foreach ($foreignRefs as $objectType => $tabRefs) {
      // Compute filter
      $filters = array_values($tabRefs['filters']);
      $filter = '(|'.join($filters).')';
      // Search objects
      try {
        $objects = objects::ls($objectType, array('dn' => 'raw'), NULL, $filter);
      } catch (NonExistingObjectTypeException $e) {
        continue;
      } catch (EmptyFilterException $e) {
        continue;
      }
      // For each object of this type
      foreach (array_keys($objects) as $dn) {
        /* Avoid sending POST to opened objects */
        $_POST = array();
        // Build the object
        $tabobject = objects::open($dn, $objectType);
        if (preg_match('/^handle_/', $mode)) {
          // For each tab concerned
          foreach ($tabRefs['refs'] as $tab => $fieldRefs) {
            // If the tab is activated on this object
            if (isset($tabobject->by_object[$tab])) {
              // For each field
              foreach ($fieldRefs as $ofield => $field) {
                // call plugin::foreignKeyUpdate(ldapname, oldvalue, newvalue, source) on the object
                $tabobject->by_object[$tab]->foreignKeyUpdate(
                  $ofield,
                  $field['oldvalue'],
                  $field['newvalue'],
                  array(
                    'CLASS' => $field['tab'],
                    'FIELD' => $field['field'],
                    'MODE'  => preg_replace('/^handle_/', '', $mode),
                    'DN'    => $this->dn,
                  )
                );
              }
              $tabobject->by_object[$tab]->save_object();
              $tabobject->by_object[$tab]->save();
            }
          }
        } elseif ($mode == 'references') {
          // For each tab concerned
          foreach ($tabRefs['refs'] as $tab => $tab_infos) {
            // If the tab is activated on this object
            if (isset($tabobject->by_object[$tab])) {
              // For each field
              foreach ($tab_infos['fields'] as $ofield => $field) {
                if ($tabobject->by_object[$tab]->foreignKeyCheck(
                      $ofield,
                      $field['value'],
                      array(
                        'CLASS' => $field['tab'],
                        'FIELD' => $field['field'],
                        'DN'    => $this->dn,
                      )
                    )) {
                  if (!isset($refs[$dn])) {
                    $refs[$dn] = array(
                      'link'  => '',
                      'tabs'  => array(),
                    );
                    try {
                      $refs[$dn]['link'] = objects::link($dn, $objectType);
                    } catch (FusionDirectoryException $e) {
                      trigger_error("Could not create link to $dn: ".$e->getMessage());
                      $refs[$dn]['link'] = $dn;
                    }
                  }
                  if (!isset($refs[$dn]['tabs'][$tab])) {
                    $refs[$dn]['tabs'][$tab] = array(
                      'link'    => '',
                      'fields'  => array(),
                    );
                    try {
                      $refs[$dn]['tabs'][$tab]['link'] = objects::link($dn, $objectType, "tab_$tab", sprintf(_('Tab "%s"'), $tab_infos['name']));
                    } catch (FusionDirectoryException $e) {
                      trigger_error("Could not create link to $dn $tab: ".$e->getMessage());
                      $refs[$dn]['tabs'][$tab]['link'] = $tab;
                    }
                  }
                  $refs[$dn]['tabs'][$tab]['fields'][$ofield] = $field;
                }
              }
            }
          }
        }
      }
    }
    /* Restore POST */
    $_POST = $SAVED_POST;
    if ($mode == 'references') {
      return $refs;
    }
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
    $this->attrs = $attrs;

    /* Walk through attributes */
    foreach ($this->attributesAccess as $ldapName => &$attr) {
      /* Skip the ones in skip list */
      if (in_array($ldapName, $skip)) {
        continue;
      }
      /* Load values */
      $attr->loadValue($this->attrs);
    }
    unset($attr);

    /* Is Account? */
    $this->is_account = $this->is_this_account($this->attrs);
  }

  protected function attributeHaveChanged($field)
  {
    return $this->attributesAccess[$field]->hasChanged();
  }

  protected function attributeValue($field)
  {
    return $this->attributesAccess[$field]->getValue();
  }

  protected function attributeInitialValue($field)
  {
    return $this->attributesAccess[$field]->getInitialValue();
  }

  function foreignKeyUpdate ($field, $oldvalue, $newvalue, $source)
  {
    if (!isset($source['MODE'])) {
      $source['MODE'] = 'move';
    }
    $this->attributesAccess[$field]->foreignKeyUpdate($oldvalue, $newvalue, $source);
  }

  /*
   * Source is an array like this:
   * array(
   *  'CLASS' => class,
   *  'FIELD' => field,
   *  'DN'    => dn,
   *  'MODE'  => mode
   * )
   * mode being either 'copy' or 'move', defaults to 'move'
   */
  function foreignKeyCheck ($field, $value, $source)
  {
    return $this->attributesAccess[$field]->foreignKeyCheck($value, $source);
  }

  function deserializeValues($values, $checkAcl = TRUE)
  {
    foreach ($values as $name => $value) {
      if (isset($this->attributesAccess[$name])) {
        if (!$checkAcl || $this->acl_is_writeable($this->attributesAccess[$name]->getAcl())) {
          $this->attributesAccess[$name]->setValue($value);
        } else {
          return sprintf(_('You don\'t have sufficient rights to edit field "%s"'), $name);
        }
      } else {
        return sprintf(_('Unknown field "%s"'), $name);
      }
    }
    return TRUE;
  }

  /* Returns TRUE if this attribute should be asked in the creation by template dialog */
  function showInTemplate($attr, $templateAttrs)
  {
    if (isset($templateAttrs[$attr])) {
      return FALSE;
    }
    return TRUE;
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

  /*! \brief This function generate the needed ACLs for a given attribtues array
   *
   *  \param array $attributesInfo the attribute array
   */
  static function generatePlProvidedAcls ($attributesInfo)
  {
    $plProvidedAcls = array();
    foreach ($attributesInfo as $sectionInfo) {
      foreach ($sectionInfo['attrs'] as $attr) {
        $aclInfo = $attr->getAclInfo();
        if ($aclInfo !== FALSE) {
          $plProvidedAcls[$aclInfo['name']] = $aclInfo['desc'];
        }
      }
    }

    return $plProvidedAcls;
  }

  /*! \brief This function is the needed main.inc for plugins that are not used inside a management class
   *
   *  \param array $classname the class name to read plInfo from. (plIcon, plShortname and plCategory are gonna be used)
   *
   *  \param string $entry_dn the dn of the object to show/edit
   *
   *  \param string $tabs TRUE to use tabs, FALSE to show directly the plugin class
   *
   *  \param boolean $edit_mode wether or not this plugin can be edited
   *
   *  \param string $objectType The objectType to use (will be taken in the plInfo if FALSE)
   *
   */
  static function mainInc ($classname, $entry_dn, $tabs = FALSE, $edit_mode = TRUE, $objectType = FALSE)
  {
    global $remove_lock, $cleanup, $display, $config, $plug, $ui;

    $plInfo     = pluglist::pluginInfos($classname);
    $plIcon     = (isset($plInfo['plIcon']) ? $plInfo['plIcon'] : 'plugin.png');
    $plHeadline = $plInfo['plTitle'];
    if ($objectType === FALSE) {
      $key = key($plInfo['plObjectType']);
      if (is_numeric($key)) {
        $key = $plInfo['plObjectType'][$key];
      }
      $objectType = $key;
    }
    $plCategory = (isset($plInfo['plCategory']) ? $plInfo['plCategory'] : array('user'));
    $key = key($plCategory);
    if (is_numeric($key)) {
      $plCategory = $plCategory[$key];
    } else {
      $plCategory = $key;
    }

    $lock_msg = "";
    if ($edit_mode) {
      /* Remove locks created by this plugin */
      if ($remove_lock || (isset($_POST['edit_cancel']) && session::is_set('edit'))) {
        if (session::is_set($classname)) {
          del_lock($entry_dn);
        }
      }
    }

    /* Remove this plugin from session */
    if ($cleanup) {
      session::un_set($classname);
      session::un_set('edit');
    } else {
      /* Reset requested? */
      if ($edit_mode && isset($_POST['edit_cancel'])) {
        session::un_set($classname);
        session::un_set('edit');
      }

      /* Create tab object on demand */
      if (!session::is_set($classname) || (isset($_GET['reset']) && $_GET['reset'] == 1)) {
        try {
          $tabObject = objects::open($entry_dn, $objectType);
        } catch (NonExistingLdapNodeException $e) {
          $tabObject = objects::open('new', $objectType);
        }
        if ($edit_mode) {
          $tabObject->setNeedEditMode(TRUE);
        }
        if (!$tabs) {
          $tabObject->current = $classname;
        }
        if (($entry_dn != '') && ($entry_dn != 'new')) {
          $tabObject->set_acl_base($entry_dn);
        } else {
          $tabObject->set_acl_base($config->current['BASE']);
        }
        session::set($classname, $tabObject);
      }
      $tabObject = session::get($classname);

      /* save changes back to object */
      if (!$edit_mode || session::is_set('edit')) {
        $tabObject->save_object();
      }

      if ($edit_mode) {
        /* Enter edit mode? */
        if ((isset($_POST['edit'])) && (!session::is_set('edit'))) {
          /* Check locking */
          if (($username = get_lock($entry_dn)) != "") {
            session::set('back_plugin', $plug);
            session::set('LOCK_VARS_TO_USE', array("/^edit$/", "/^plug$/"));
            $lock_msg = gen_locked_message($username, $entry_dn);
          } else {
            /* Lock the current entry */
            add_lock($entry_dn, $ui->dn);
            session::set('edit', TRUE);
          }
        }

        /* save changes to LDAP and disable edit mode */
        $info = "";
        if (isset($_POST['edit_finish'])) {
          /* Perform checks */
          $message = $tabObject->save();

          /* No errors, save object */
          if (count($message) == 0) {
            del_lock($entry_dn);
            session::un_set('edit');

            /* Remove from session */
            session::un_set($classname);
          } else {
            /* Errors found, show message */
            msg_dialog::displayChecks($message);
          }
        }
      } else {
        $info = "";
      }

      /* Execute formular */
      if ($edit_mode && $lock_msg) {
        $display = $lock_msg;
      } else {
        if ($tabs) {
          $display .= $tabObject->execute();
        } else {
          $display .= $tabObject->by_object[$classname]->execute();
        }
      }

      /* Store changes  in session */
      if (!$edit_mode || session::is_set('edit')) {
        session::set($classname, $tabObject);
      }

      /* Show page footer depending on the mode */
      $info = $entry_dn.'&nbsp;';
      if ($edit_mode && (!$tabObject->dialogOpened()) && empty($lock_msg)) {
        $display .= '<p class="plugbottom">';

        /* Are we in edit mode? */
        if (session::is_set('edit')) {
          $display .= '<input type="submit" name="edit_finish" style="width:80px" value="'.msgPool::okButton().'"/>'."\n";
          $display .= '&nbsp;';
          $display .= '<input type="submit" name="edit_cancel" value="'.msgPool::cancelButton().'"/>'."\n";
        } else {
          /* Only display edit button if there is at least one attribute editable */
          if (preg_match('/r/', $ui->get_permissions($entry_dn, $plCategory.'/'.$tabObject->current))) {
            $info .= '<div style="display:inline-block" class="optional"><img class="center" alt="information" '.
                      'src="geticon.php?context=status&amp;icon=dialog-information&amp;size=16"> '.
                      msgPool::clickEditToChange().'</div>';

            $display .= '<input type="submit" name="edit" value="'.msgPool::editButton().'"/>'."\n";
          }
          $display .= '<input type="hidden" name="ignore"/>'."\n";
        }
        $display .= "</p>\n";
      }

      /* Page header */
      if (!preg_match('/^geticon/', $plIcon)) {
        $plIcon = get_template_path($plIcon);
      }
      $display = print_header($plIcon, $plHeadline, $info).$display;
    }
  }
}
?>
