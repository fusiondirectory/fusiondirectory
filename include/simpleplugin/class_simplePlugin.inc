<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2012-2019  FusionDirectory

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
 * \file class_simplePlugin.inc
 * Source code for the class simplePlugin
 */

/*! \brief This class is made for easy plugin creation for editing LDAP attributes
 *
 */
class simplePlugin implements SimpleTab
{
  /*! \brief This attribute store all information about attributes */
  public $attributesInfo;

  /*! \brief This attribute store references toward attributes
   *
   * associative array that stores attributeLdapName => reference on object
   */
  public $attributesAccess = [];

  /*!
    \brief Mark plugin as account

    Defines whether this plugin is defined as an account or not.
    This has consequences for the plugin to be saved from tab
    mode. If it is set to 'FALSE' the tab will call the delete
    function, else the save function. Should be set to 'TRUE' if
    the construtor detects a valid LDAP object.

    \sa simplePlugin::is_this_account()
   */
  public $is_account            = FALSE;
  public $initially_was_account = FALSE;
  protected $ignore_account     = FALSE;

  public $acl_category  = '';

  /*! \brief dn of the opened object */
  public $dn      = '';

  /*! \brief original dn of the opened object */
  public $orig_dn = '';

  /*!
   * \brief Reference to parent object
   *
   * This variable is used when the plugin is included in tabs
   * and keeps reference to the tab class. Communication to other
   * tabs is possible by 'name'. So the 'fax' plugin can ask the
   * 'userinfo' plugin for the fax number.
   *
   * \sa simpleTabs
   */
  public $parent = NULL;

  /*!
    \brief Mark plugin as template

    Defines whether we are editing a template or a normal object.
    Has consequences on the way execute() shows the formular and how
    save() puts the data to LDAP.
   */
  public $is_template    = FALSE;

  /*!
    \brief Represent temporary LDAP data

    This should only be used internally.
   */
  public $attrs = [];

  /*! \brief The objectClasses set by this tab */
  protected $objectclasses = [];

  /*! \brief The state of the attributes when we opened the object */
  protected $saved_attributes = [];

  /*! \brief Do we want a header allowing to able/disable this plugin */
  protected $displayHeader = FALSE;

  /*! \brief Is this plugin the main tab, the one that handle the object itself */
  protected $mainTab = FALSE;

  protected $header = "";

  protected $templatePath;

  protected $dialog = FALSE;

  /*! \brief Are we executed in a edit-mode environment? (this is FALSE if we're called from management, TRUE if we're called from a main.inc)
   */
  protected $needEditMode = FALSE;

  /*! \brief Attributes that needs to be initialized before the others */
  protected $preInitAttributes = [];

  /*! \brief FALSE to disable inheritance. Array like array ('objectClass' => 'attribute') to specify oc of the groups it might be inherited from
   */
  protected $inheritance      = FALSE;
  protected $member_of_group  = FALSE;
  protected $editing_group    = NULL;
  protected $group_attrs      = [];

  /*! \brief Used when the entry is opened as "readonly" due to locks */
  protected $read_only = FALSE;

  /*! \brief Last LDAP error (used by logging calls from post_* methods) */
  protected $ldap_error;

  /*!
   * \brief Object entry CSN
   *
   * If an entry was edited while we have edited the entry too,
   * an error message will be shown.
   * To configure this check correctly read the FAQ.
   */
  protected $entryCSN = '';

  private $hadSubobjects = FALSE;

  /*! \brief constructor
   *
   *  \param string $dn The dn of this instance
   *  \param Object $object An object to copy values from
   *  \param Object $parent A parent instance, usually a simpleTabs instance.
   *  \param boolean $mainTab Whether or not this is the main tab
   *  \param array $attributesInfo An attributesInfo array, if NULL, getAttributesInfo will be used.
   *
   */
  function __construct (string $dn = NULL, $object = NULL, $parent = NULL, bool $mainTab = FALSE, array $attributesInfo = NULL)
  {
    global $config;

    $this->dn       = $dn;
    $this->parent   = $parent;
    $this->mainTab  = $mainTab;

    try {
      $plInfo = pluglist::pluginInfos(get_class($this));
    } catch (UnknownClassException $e) {
      /* May happen in special cases like setup */
      $plInfo = [];
    }

    if (empty($this->objectclasses) && isset($plInfo['plObjectClass'])) {
      $this->objectclasses = $plInfo['plObjectClass'];
    }

    if ($attributesInfo === NULL) {
      $attributesInfo = $this->getAttributesInfo();
    }
    if (!$this->displayHeader) {
      // If we don't display the header to activate/deactive the plugin, that means it's always activated
      $this->ignore_account = TRUE;
    }

    $this->attributesInfo = [];
    foreach ($attributesInfo as $section => $sectionInfo) {
      $attrs = [];
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

    /* Ensure that we've a valid acl_category set */
    if (empty($this->acl_category) && isset($plInfo['plCategory'])) {
      $c = key($plInfo['plCategory']);
      if (is_numeric($c)) {
        $c = $plInfo['plCategory'][0];
      }
      $this->acl_category = $c.'/';
    }

    /* Check if this entry was opened in read only mode */
    if (($this->dn != 'new') &&
        isset($_POST['open_readonly']) &&
        session::is_set('LOCK_CACHE')
      ) {
      $cache = session::get('LOCK_CACHE');
      if (isset($cache['READ_ONLY'][$this->dn])) {
        $this->read_only = TRUE;
      }
    }

    /* Load LDAP data */
    if (($this->dn != 'new' && $this->dn !== NULL) || ($object !== NULL)) {
      /* Load data to 'attrs' */
      if ($object !== NULL) {
        /* From object */
        $this->attrs = $object->attrs;
        if (isset($object->is_template)) {
          $this->setTemplate($object->is_template);
        }
      } else {
        /* From LDAP */
        $ldap = $config->get_ldap_link();
        $ldap->cat($this->dn);
        $this->attrs = $ldap->fetch(TRUE);
        if (empty($this->attrs)) {
          throw new NonExistingLdapNodeException('Could not open dn '.$this->dn);
        }
        if ($this->mainTab) {
          $this->entryCSN = getEntryCSN($this->dn);
          /* Make sure that initially_was_account is TRUE if we loaded an LDAP node,
           *  even if it’s missing an objectClass */
          $this->is_account = TRUE;
        }
      }

      /* Set the template flag according to the existence of objectClass fdTemplate */
      if (isset($this->attrs['objectClass']) && in_array_ics('fdTemplate', $this->attrs['objectClass'])) {
        logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, 'found', 'Template check');
        $this->setTemplate(TRUE);
        $this->templateLoadAttrs($this->attrs);
      }

      /* Is Account? */
      if ($this->is_this_account($this->attrs)) {
        $this->is_account = TRUE;
        logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, get_class($this), 'Tab active');
      }
    }

    if (is_array($this->inheritance)) {
      /* Check group membership */
      $ldap = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);
      foreach ($this->inheritance as $oc => $at) {
        if ($this->mainTab) {
          $filter = '(&(objectClass='.$oc.')('.$at.'='.ldap_escape_f($this->dn).'))';
        } else {
          $filter = '(&(objectClass='.$oc.')'.static::getLdapFilter().'('.$at.'='.ldap_escape_f($this->dn).'))';
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

    /* Save initial account state */
    $this->initially_was_account = $this->is_account;

    $this->loadAttributes();

    $this->prepareSavedAttributes();

    $this->orig_dn = $dn;

    if ($this->mainTab) {
      $this->is_account = TRUE;
    }

    if (!isset($this->templatePath)) {
      $this->templatePath = get_template_path('simpleplugin.tpl');
    }
  }

  protected function loadAttributes ()
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
          /* skip the preInit ones */
          continue;
        }
        $attr->setParent($this);
        $attr->loadValue($this->attrs);
      }
      unset($attr);
    }
    unset($sectionInfo);
  }

  function is_this_account ($attrs)
  {
    $result = static::isAccount($attrs);
    if ($result === NULL) {
      if (!empty($this->objectclasses)) {
        trigger_error('Deprecated fallback was used for '.get_called_class().'::is_this_account');
      }
      $found = TRUE;
      foreach ($this->objectclasses as $obj) {
        if (preg_match('/^top$/i', $obj)) {
          continue;
        }
        if (!isset($attrs['objectClass']) || !in_array_ics($obj, $attrs['objectClass'])) {
          $found = FALSE;
          break;
        }
      }
      return $found;
    }
    return $result;
  }

  function setTemplate (bool $bool)
  {
    $this->is_template = $bool;
    if ($this->is_template && $this->mainTab) {
      /* Unshift special section for template infos */
      $this->attributesInfo = array_merge(
        [
          '_template' => [
            'class' => ['fullwidth'],
            'name'  => _('Template settings'),
            'attrs' => [
              '_template_cn' => new StringAttribute(
                _('Template name'), _('This is the name of the template'),
                '_template_cn', TRUE,
                '', 'template_cn'
              )
            ]
          ],
          '_template_dummy' => [
            'class' => ['invisible'],
            'name'  => '_template_dummy',
            'attrs' => []
          ]
        ],
        $this->attributesInfo
      );
      $this->attributesAccess['_template_cn'] =& $this->attributesInfo['_template']['attrs']['_template_cn'];
      $this->attributesAccess['_template_cn']->setInLdap(FALSE);
      $this->attributesAccess['_template_cn']->setValue($this->_template_cn);
      $this->attributesAccess['_template_cn']->setParent($this);
      unset($this->_template_cn);
    }
  }

  protected function templateLoadAttrs (array $template_attrs)
  {
    if ($this->mainTab) {
      $this->_template_cn = $template_attrs['cn'][0];
    }
    $this->attrs = templateHandling::fieldsFromLDAP($template_attrs);
  }

  protected function templateSaveAttrs ()
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cat($this->dn);
    $template_attrs = $ldap->fetch(TRUE);
    if (!$template_attrs) {
      if (!$this->mainTab) {
        trigger_error('It seems main tab has not been saved.');
      }
      $template_attrs = [
        'objectClass'     => ['fdTemplate'],
        'fdTemplateField' => []
      ];
    }
    $template_attrs = templateHandling::fieldsToLDAP($template_attrs, $this->attrs);
    if ($this->mainTab) {
      $template_attrs['cn'] = $this->_template_cn;
    }
    return $template_attrs;
  }

  /*! \brief This function returns an LDAP filter for this plugin object classes
   */
  function getObjectClassFilter ()
  {
    trigger_error('Deprecated');
    return static::getLdapFilter();
  }

  /*! \brief This function allows to use the syntax $plugin->attributeName to get attributes values
   *
   * It calls the getValue method on the concerned attribute
   * It also adds the $plugin->attribtues syntax to get attributes list
   */
  public function __get ($name)
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
  public function __set ($name, $value)
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
  public function __isset ($name)
  {
    if ($name == 'attributes') {
      return TRUE;
    }
    return isset($this->attributesAccess[$name]);
  }

  /*! \brief This function returns the dn this object should have
   */
  public function compute_dn (): string
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
      return 'cn='.ldap_escape_dn($this->_template_cn).',ou=templates,'.$ou.$base;
    }
    return $attr.'='.ldap_escape_dn($this->attributesAccess[$attr]->computeLdapValue()).','.$ou.$base;
  }

  protected function addAttribute (string $section, Attribute $attr)
  {
    $name = $attr->getLdapName();
    $this->attributesInfo[$section]['attrs'][$name] = $attr;
    $this->attributesAccess[$name] =& $this->attributesInfo[$section]['attrs'][$name];
    $this->attributesAccess[$name]->setParent($this);
    unset($this->$name);
  }

  protected function removeAttribute (string $section, string $id)
  {
    unset($this->attributesInfo[$section]['attrs'][$id]);
    unset($this->attributesAccess[$id]);
  }

  /*!
   * \brief Returns a list of all available departments for this object.
   *
   * If this object is new, all departments we are allowed to create a new object in are returned.
   * If this is an existing object, return all deps we are allowed to move this object to.
   * Used by BaseSelectorAttribute
   *
   * \return array [dn] => "..name"  // All deps. we are allowed to act on.
  */
  function get_allowed_bases (): array
  {
    global $config;
    $deps = [];

    /* Is this a new object ? Or just an edited existing object */
    $departmentTree = $config->getDepartmentTree();
    foreach ($departmentTree as $dn => $name) {
      if (
          (!$this->initially_was_account && $this->acl_is_createable($dn)) ||
          ($this->initially_was_account && $this->acl_is_moveable($dn))
        ) {
        $deps[$dn] = $name;
      }
    }

    /* Add current base */
    if (isset($this->base) && isset($departmentTree[$this->base])) {
      $deps[$this->base] = $departmentTree[$this->base];
    } elseif (strtolower($this->dn) != strtolower($config->current['BASE'])) {
      trigger_error('Cannot return list of departments, no default base found in class '.get_class($this).'. (base is "'.$this->base.'")');
    }
    return $deps;
  }

  /*!
   * \brief Set acl category
   *
   * \param string $category
   */
  function set_acl_category (string $category)
  {
    $this->acl_category = "$category/";
  }

  /*!
    * \brief Move ldap entries from one place to another
    *
    * \param  string  $src_dn the source DN.
    *
    * \param  string  $dst_dn the destination DN.
    *
    * \return TRUE on success, error string on failure
    */
  function move (string $src_dn, string $dst_dn)
  {
    global $config, $ui;

    /* Do not move if only case has changed */
    if (strtolower($src_dn) == strtolower($dst_dn)) {
      return TRUE;
    }

    /* Try to move with ldap routines */
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $dst_dn));
    if (!$ldap->rename_dn($src_dn, $dst_dn)) {
      logging::log('error', 'ldap', "FROM: $src_dn -- TO: $dst_dn", [], 'Ldap Protocol v3 implementation error, ldap_rename failed: '.$ldap->get_error());
      logging::debug(DEBUG_LDAP, __LINE__, __FUNCTION__, __FILE__, "Rename failed FROM: $src_dn  -- TO:  $dst_dn",
        'Ldap Protocol v3 implementation error. Error:'.$ldap->get_error());
      return $ldap->get_error();
    }

    /* Update userinfo if necessary */
    if (preg_match('/'.preg_quote($src_dn, '/').'$/i', $ui->dn)) {
      $ui->dn = preg_replace('/'.preg_quote($src_dn, '/').'$/i', $dst_dn, $ui->dn);
    }

    /* Check if departments were moved. If so, force the reload of $config departments cache */
    $ldap->cd($dst_dn);
    $ldap->search('(objectClass=gosaDepartment)', ['dn']);
    if ($ldap->count()) {
      $config->resetDepartmentCache();
      $ui->reset_acl_cache();
    }

    $this->handleForeignKeys($src_dn, $dst_dn);
    return TRUE;
  }

  function getRequiredAttributes (): array
  {
    $tmp = [];
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

  /*! \brief Indicates if this object is opened as read-only (because of locks) */
  function readOnly ()
  {
    return $this->read_only;
  }

  /*! \brief This function display the plugin and return the html code
   */
  function execute (): string
  {
    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "execute");

    /* Reset Lock message POST/GET check array, to prevent preg_match errors */
    session::set('LOCK_VARS_TO_USE', []);
    session::set('LOCK_VARS_USED_GET', []);
    session::set('LOCK_VARS_USED_POST', []);
    session::set('LOCK_VARS_USED_REQUEST', []);

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
      if ($this->parent !== NULL) {
        list($disabled, $buttonText, $text) = $this->getDisplayHeaderInfos();
        $this->header = $this->show_header(
          $buttonText,
          $text,
          $this->is_account,
          $disabled,
          get_class($this).'_modify_state'
        );
        if (!$this->is_account) {
          $this->displayPlugin = FALSE;
          return $this->header.$this->inheritanceDisplay();
        }
      } elseif (!$this->is_account) {
        $plInfo = pluglist::pluginInfos(get_class($this));
        $this->header = '<img alt="'.htmlescape(_('Error')).'" src="geticon.php?context=status&amp;icon=dialog-error&amp;size=16" align="middle"/>&nbsp;<b>'.
                        msgPool::noValidExtension($plInfo['plShortName'])."</b>";
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

  public function getDisplayHeaderInfos (): array
  {
    $plInfo   = pluglist::pluginInfos(get_class($this));
    $disabled = $this->acl_skip_write();
    if ($this->is_account) {
      $depends = [];
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
      $buttonText = msgPool::removeFeaturesButton($plInfo['plShortName']);
      $text       = msgPool::featuresEnabled($plInfo['plShortName'], $depends);
    } else {
      $depends    = [];
      $conflicts  = [];
      if (isset($plInfo['plDepends'])) {
        foreach ($plInfo['plDepends'] as $plugin) {
          if (isset($this->parent->by_object[$plugin]) &&
              !$this->parent->by_object[$plugin]->is_account) {
            $disabled   = TRUE;
            $dependPlInfos  = pluglist::pluginInfos($plugin);
            $depends[]      = $dependPlInfos['plShortName'];
          }
        }
      }
      if (isset($plInfo['plConflicts'])) {
        foreach ($plInfo['plConflicts'] as $plugin) {
          if (isset($this->parent->by_object[$plugin]) &&
              $this->parent->by_object[$plugin]->is_account) {
            $disabled   = TRUE;
            $conflictPlInfos  = pluglist::pluginInfos($plugin);
            $conflicts[]      = $conflictPlInfos['plShortName'];
          }
        }
      }
      $buttonText = msgPool::addFeaturesButton($plInfo['plShortName']);
      $text       = msgPool::featuresDisabled($plInfo['plShortName'], $depends, $conflicts);
    }
    return [$disabled,$buttonText,$text];
  }

  /*!
   * \brief Show header message for tab dialogs
   *
   * \param string $button_text The button text
   *
   * \param string $text The text
   *
   * \param boolean $plugin_enabled Is the plugin/tab activated
   *
   * \param boolean $button_disabled Is the button disabled
   *
   * \param string $name The html name of the input, defaults to modify_state
   */
  function show_header (string $button_text, string $text, bool $plugin_enabled, bool $button_disabled = FALSE, string $name = 'modify_state'): string
  {
    if ($button_disabled || ((!$this->acl_is_createable() && !$plugin_enabled) || (!$this->acl_is_removeable() && $plugin_enabled))) {
      $state = 'disabled="disabled"';
    } else {
      $state = '';
    }
    $display = '<div width="100%"><p><b>'.$text.'</b><br/>'."\n";
    $display .= '<input type="submit" formnovalidate="formnovalidate" value="'.$button_text.'" name="'.$name.'" '.$state.'></p></div><hr class="separator"/>';

    return $display;
  }

  /*!
   * \brief Test whether a tab is active
   */
  public function isActive (): bool
  {
    return ($this->is_account || $this->ignore_account);
  }

  /*!
   * \brief Test whether a tab can be deactivated
   */
  public function isActivatable (): bool
  {
    return $this->displayHeader;
  }

  /*! \brief Check if logged in user have enough right to read this attribute value
   *
   * \param mixed $attr Attribute object or name (in this case it will be fetched from attributesAccess)
   */
  function attrIsReadable ($attr): bool
  {
    if (!is_object($attr)) {
      $attr = $this->attributesAccess[$attr];
    }
    if ($attr->getLdapName() == 'base') {
      return TRUE;
    }
    if ($attr->getAcl() == 'noacl') {
      return TRUE;
    }
    return $this->acl_is_readable($attr->getAcl());
  }

  /*! \brief Check if logged in user have enough right to write this attribute value
   *
   * \param mixed $attr Attribute object or name (in this case it will be fetched from attributesAccess)
   */
  function attrIsWriteable ($attr): bool
  {
    if (!is_object($attr)) {
      $attr = $this->attributesAccess[$attr];
    }
    if ($attr->getLdapName() == 'base') {
      return (
        !$this->acl_skip_write() &&
        (!$this->initially_was_account || $this->acl_is_moveable() || $this->acl_is_removeable())
      );
    }
    if ($attr->getAcl() == 'noacl') {
      return FALSE;
    }
    return $this->acl_is_writeable($attr->getAcl(), $this->acl_skip_write());
  }

  /*!
   * \brief Get LDAP base to use for ACL checks
   */
  function getAclBase (bool $callParent = TRUE): string
  {
    global $config;

    if (($this->parent instanceof simpleTabs) && $callParent) {
      return $this->parent->getAclBase();
    }
    if (isset($this->dn) && ($this->dn != 'new')) {
      return $this->dn;
    }
    if (isset($this->base)) {
      return 'new,'.$this->base;
    }

    return $config->current['BASE'];
  }

  function renderAttributes (bool $readOnly = FALSE)
  {
    global $ui;
    $smarty = get_smarty();

    if ($this->is_template) {
      $smarty->assign('template_cnACL', $ui->get_permissions($this->getAclBase(), $this->acl_category.'template', 'template_cn', $this->acl_skip_write()));
    }

    /* Handle rights to modify the base */
    if (isset($this->attributesAccess['base'])) {
      if ($this->attrIsWriteable('base')) {
        $smarty->assign('baseACL', 'rw');
      } else {
        $smarty->assign('baseACL', 'r');
      }
    }

    $sections = [];
    foreach ($this->attributesInfo as $section => $sectionInfo) {
      $smarty->assign('section',      $sectionInfo['name']);
      $smarty->assign('sectionIcon',  ($sectionInfo['icon'] ?? NULL));
      $smarty->assign('sectionId',    $section);
      $sectionClasses = '';
      if (isset($sectionInfo['class'])) {
        $sectionClasses .= ' '.join(' ', $sectionInfo['class']);
      }
      $attributes       = [];
      $readableSection  = FALSE;
      foreach ($sectionInfo['attrs'] as $attr) {
        if ($attr->getAclInfo() !== FALSE) {
          // We assign ACLs so that attributes can use them in their template code
          $smarty->assign($attr->getAcl().'ACL', $this->aclGetPermissions($attr->getAcl(), NULL, $this->acl_skip_write()));
        }
        $readable = $this->attrIsReadable($attr);
        $writable = $this->attrIsWriteable($attr);
        if (!$readableSection && ($readable || $writable)) {
          $readableSection = TRUE;
        }
        $attr->renderAttribute($attributes, $readOnly, $readable, $writable);
      }
      $smarty->assign('attributes', $attributes);
      if (!$readableSection) {
        $sectionClasses .= ' nonreadable';
      }
      $smarty->assign('sectionClasses', $sectionClasses);
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

  function inheritanceDisplay (): string
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

    return "<h1>Inherited information:</h1><div></div>\n".$smarty->fetch($this->templatePath);
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

  public function setNeedEditMode (bool $bool)
  {
    $this->needEditMode = $bool;
  }

  protected function acl_skip_write (): bool
  {
    return ($this->needEditMode && !session::is_set('edit'));
  }

  /*! \brief Can we write the attribute */
  function acl_is_writeable ($attribute, bool $skipWrite = FALSE): bool
  {
    return (strpos($this->aclGetPermissions($attribute, NULL, $skipWrite), 'w') !== FALSE);
  }

  /*!
   * \brief Can we read the acl
   *
   * \param string $attribute
   */
  function acl_is_readable ($attribute): bool
  {
    return (strpos($this->aclGetPermissions($attribute), 'r') !== FALSE);
  }

  /*!
   * \brief Can we create the object
   *
   * \param string $base Empty string
   */
  function acl_is_createable (string $base = NULL): bool
  {
    return (strpos($this->aclGetPermissions('0', $base), 'c') !== FALSE);
  }

  /*!
   * \brief Can we delete the object
   *
   * \param string $base Empty string
   */
  function acl_is_removeable (string $base = NULL): bool
  {
    return (strpos($this->aclGetPermissions('0', $base), 'd') !== FALSE);
  }

  /*!
   * \brief Can we move the object
   *
   * \param string $base Empty string
   */
  function acl_is_moveable (string $base = NULL): bool
  {
    return (strpos($this->aclGetPermissions('0', $base), 'm') !== FALSE);
  }

  /*! \brief Test if there are ACLs for this plugin */
  function aclHasPermissions (): bool
  {
    global $config;

    return in_array(get_class($this), $config->data['CATEGORIES'][rtrim($this->acl_category, '/')]['classes']);
  }

  /*! \brief Get the acl permissions for an attribute or the plugin itself */
  function aclGetPermissions ($attribute = '0', string $base = NULL, bool $skipWrite = FALSE): string
  {
    if (isset($this->parent) && isset($this->parent->ignoreAcls) && $this->parent->ignoreAcls) {
      return 'cdmr'.($skipWrite ? '' : 'w');
    }
    $ui         = get_userinfo();
    $skipWrite  |= $this->readOnly();
    if ($base === NULL) {
      $base = $this->getAclBase();
    }
    return $ui->get_permissions($base, $this->acl_category.get_class($this), $attribute, $skipWrite);
  }

  /*! \brief This function removes the object from LDAP
   */
  function remove (bool $fulldelete = FALSE): array
  {
    if (!$this->initially_was_account) {
      return [];
    }

    if (!$fulldelete && !$this->acl_is_removeable()) {
      trigger_error('remove was called on a tab without enough ACL rights');
      return [];
    }

    $this->prepare_remove();
    if ($this->is_template) {
      $this->attrs = $this->templateSaveAttrs();
      $this->saved_attributes = [];
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
    return [];
  }

  /* Remove FusionDirectory attributes */
  protected function prepare_remove ()
  {
    global $config;
    $this->attrs = [];

    if (!$this->mainTab) {
      /* include global link_info */
      $ldap = $config->get_ldap_link();

      /* Get current objectClasses in order to add the required ones */
      $ldap->cat($this->dn, ['fdTemplateField','objectClass']);
      $tmp  = $ldap->fetch();
      $oc   = [];
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
        $this->attrs["$val"] = [];
      }
    }
  }

  protected function pre_remove ()
  {
    if ($this->initially_was_account) {
      return $this->handle_pre_events('remove', ['modifiedLdapAttrs' => array_keys($this->attrs)]);
    }
  }

  protected function ldap_remove (): array
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
      return [];
    } else {
      return [
        new SimplePluginLdapError(
          $this,
          $this->dn,
          ($this->mainTab ? LDAP_DEL : LDAP_MOD),
          $ldap->get_error(),
          $ldap->get_errno()
        )
      ];
    }
  }

  protected function post_remove ()
  {
    logging::log('remove', 'plugin/'.get_class($this), $this->dn, array_keys($this->attrs), $this->ldap_error);

    /* Optionally execute a command after we're done */
    $errors = $this->handle_post_events('remove', ['modifiedLdapAttrs' => array_keys($this->attrs)]);
    if (!empty($errors)) {
      msg_dialog::displayChecks($errors);
    }
  }

  /*! \brief This function handle $_POST informations
   */
  function save_object ()
  {
    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, 'save_object');
    if ($this->displayHeader && isset($_POST[get_class($this).'_modify_state'])) {
      if ($this->is_account && $this->acl_is_removeable()) {
        $this->is_account = FALSE;
      } elseif (!$this->is_account && $this->acl_is_createable()) {
        $this->is_account = TRUE;
      }
    }
    if (isset($_POST[get_class($this).'_posted'])) {
      // If our form has been posted
      // A first pass that loads the post values
      foreach ($this->attributesInfo as $sectionInfo) {
        foreach ($sectionInfo['attrs'] as $attr) {
          if ($this->attrIsWriteable($attr)) {
            // Each attribute know how to read its value from POST
            $attr->loadPostValue();
          }
        }
      }
      // A second one that applies them. That allow complex stuff such as attribute disabling
      foreach ($this->attributesInfo as $sectionInfo) {
        foreach ($sectionInfo['attrs'] as $attr) {
          if ($this->attrIsWriteable($attr)) {
            // Each attribute know how to read its value from POST
            $attr->applyPostValue();
          }
        }
      }
    }
  }

  protected function prepareSavedAttributes ()
  {
    /* Prepare saved attributes */
    $this->saved_attributes = $this->attrs;
    foreach (array_keys($this->saved_attributes) as $index) {
      if (is_numeric($index)) {
        unset($this->saved_attributes[$index]);
        continue;
      }

      list($attribute, ) = explode(';', $index, 2);
      if (!in_array_ics($index, $this->attributes) && !in_array_ics($attribute, $this->attributes) && strcasecmp('objectClass', $attribute)) {
        unset($this->saved_attributes[$index]);
        continue;
      }

      if (isset($this->saved_attributes[$index][0])) {
        if (!isset($this->saved_attributes[$index]['count'])) {
          $this->saved_attributes[$index]['count'] = count($this->saved_attributes[$index]);
        }
        if ($this->saved_attributes[$index]['count'] == 1) {
          $tmp = $this->saved_attributes[$index][0];
          unset($this->saved_attributes[$index]);
          $this->saved_attributes[$index] = $tmp;
          continue;
        }
      }
      unset($this->saved_attributes[$index]['count']);
    }
  }

  /*!
   * \brief Remove attributes, empty arrays, arrays
   * single attributes that do not differ
   */
  function cleanup ()
  {
    foreach ($this->attrs as $index => $value) {
      /* Convert arrays with one element to non arrays, if the saved
         attributes are no array, too */
      if (is_array($this->attrs[$index]) &&
          (count($this->attrs[$index]) == 1) &&
          isset($this->saved_attributes[$index]) &&
          !is_array($this->saved_attributes[$index])) {
        $this->attrs[$index] = $this->attrs[$index][0];
      }

      /* Remove emtpy arrays if they do not differ */
      if (is_array($this->attrs[$index]) &&
          (count($this->attrs[$index]) == 0) &&
          !isset($this->saved_attributes[$index])) {
        unset($this->attrs[$index]);
        continue;
      }

      /* Remove single attributes that do not differ */
      if (!is_array($this->attrs[$index]) &&
          isset($this->saved_attributes[$index]) &&
          !is_array($this->saved_attributes[$index]) &&
          ($this->attrs[$index] == $this->saved_attributes[$index])) {
        unset($this->attrs[$index]);
        continue;
      }

      /* Remove arrays that do not differ */
      if (is_array($this->attrs[$index]) &&
          isset($this->saved_attributes[$index]) &&
          is_array($this->saved_attributes[$index]) &&
          !array_differs($this->attrs[$index], $this->saved_attributes[$index])) {
        unset($this->attrs[$index]);
        continue;
      }
    }
  }

  function prepareNextCleanup ()
  {
    /* Update saved attributes and ensure that next cleanups will be successful too */
    foreach ($this->attrs as $name => $value) {
      $this->saved_attributes[$name] = $value;
    }
  }

  /*! \brief This function saves the object in the LDAP
   */
  function save (): array
  {
    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "save");
    $errors = $this->prepare_save();
    if (!empty($errors)) {
      return $errors;
    }
    if ($this->is_template) {
      $errors = templateHandling::checkFields($this->attrs);
      if (!empty($errors)) {
        return $errors;
      }
      $this->attrs = $this->templateSaveAttrs();
      $this->saved_attributes = [];
    }
    $this->cleanup();
    if (!$this->shouldSave()) {
      return []; /* Nothing to do here */
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
    return [];
  }

  protected function shouldSave (): bool
  {
    if ($this->mainTab && !$this->initially_was_account) {
      return TRUE;
    }
    return !empty($this->attrs);
  }

  /* Used by prepare_save and template::apply */
  public function mergeObjectClasses (array $oc): array
  {
    return array_merge_unique($oc, $this->objectclasses);
  }

  /* \!brief Prepare $this->attrs */
  protected function prepare_save (): array
  {
    global $config;

    $this->entryCSN = '';

    /* Start with empty array */
    $this->attrs  = [];
    $oc           = [];

    if (!$this->mainTab || $this->initially_was_account) {
      /* Get current objectClasses in order to add the required ones */
      $ldap = $config->get_ldap_link();
      $ldap->cat($this->dn, ['fdTemplateField', 'objectClass']);

      $tmp  = $ldap->fetch();

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
    }

    $this->attrs['objectClass'] = $this->mergeObjectClasses($oc);

    /* Fill attributes LDAP values into the attrs array */
    foreach ($this->attributesInfo as $sectionInfo) {
      foreach ($sectionInfo['attrs'] as $attr) {
        $attr->fillLdapValue($this->attrs);
      }
    }
    /* Some of them have post-filling hook */
    foreach ($this->attributesInfo as $sectionInfo) {
      foreach ($sectionInfo['attrs'] as $attr) {
        $attr->fillLdapValueHook($this->attrs);
      }
    }

    return [];
  }

  protected function pre_save (): array
  {
    if ($this->initially_was_account) {
      return $this->handle_pre_events('modify', ['modifiedLdapAttrs' => array_keys($this->attrs)]);
    } else {
      return $this->handle_pre_events('add', ['modifiedLdapAttrs' => array_keys($this->attrs)]);
    }
  }

  /* Returns an array with the errors or an empty array */
  protected function ldap_save (): array
  {
    global $config;

    /* Check if this is a new entry ... add/modify */
    $ldap = $config->get_ldap_link();
    if ($this->mainTab && !$this->initially_was_account) {
      if ($ldap->dn_exists($this->dn)) {
        return [
          new SimplePluginError(
            $this,
            htmlescape(sprintf(_('There is already an entry with the same dn: %s'), $this->dn))
          )
        ];
      }
      $ldap->cd($config->current['BASE']);
      $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
      $action = 'add';
    } else {
      if (!$ldap->dn_exists($this->dn)) {
        return [
          new SimplePluginError(
            $this,
            htmlescape(sprintf(_('The entry %s is not existing'), $this->dn))
          )
        ];
      }
      $action = 'modify';
    }

    $ldap->cd($this->dn);
    $ldap->$action($this->attrs);
    $this->ldap_error = $ldap->get_error();

    /* Check for errors */
    if (!$ldap->success()) {
      return [
        new SimplePluginLdapError(
          $this,
          $this->dn,
          ($action == 'modify' ? LDAP_MOD : LDAP_ADD),
          $ldap->get_error(),
          $ldap->get_errno()
        )
      ];
    }
    return [];
  }

  /*! \brief This function is called after LDAP save to do some post operations and logging
   *
   * This function calls hooks, update foreign keys and log modification
   */
  protected function post_save ()
  {
    /* Propagate and log the event */
    if ($this->initially_was_account) {
      $errors = $this->handle_post_events('modify', ['modifiedLdapAttrs' => array_keys($this->attrs)]);
      logging::log('modify', 'plugin/'.get_class($this), $this->dn, array_keys($this->attrs), $this->ldap_error);
    } else {
      $errors = $this->handle_post_events('add', ['modifiedLdapAttrs' => array_keys($this->attrs)]);
      logging::log('create', 'plugin/'.get_class($this), $this->dn, array_keys($this->attrs), $this->ldap_error);
    }
    if (!empty($errors)) {
      msg_dialog::displayChecks($errors);
    }
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
  protected function handle_hooks (string $when, string $mode, array $addAttrs = []): array
  {
    switch ($mode) {
      case 'add':
        return $this->callHook($when.'CREATE', $addAttrs);

      case 'modify':
        return $this->callHook($when.'MODIFY', $addAttrs);

      case 'remove':
        return $this->callHook($when.'REMOVE', $addAttrs);

      default:
        trigger_error(sprintf('Invalid %s event type given: "%s"! Valid types are: add, modify, remove.', strtolower($when), $mode));
        return [];
    }
  }

  /*! \brief Forward command execution requests
   *         to the post hook execution method.
   */
  function handle_post_events (string $mode, array $addAttrs = [])
  {
    /* Update foreign keys */
    if ($mode == 'remove') {
      $this->handleForeignKeys($this->dn, NULL);
    } elseif ($mode == 'modify') {
      $this->handleForeignKeys();
    }
    return $this->handle_hooks('POST', $mode, $addAttrs);
  }

  /*!
   *  \brief Forward command execution requests
   *         to the pre hook execution method.
   */
  function handle_pre_events (string $mode, array $addAttrs = []): array
  {
    global $config;

    $this->ldap_error = '';
    if ($this->mainTab && ($mode == 'remove')) {
      /* Store information if there was subobjects before deletion */
      $ldap = $config->get_ldap_link();
      $ldap->cd($this->dn);
      $ldap->search('(objectClass=*)', ['dn'], 'one');
      $this->hadSubobjects = ($ldap->count() > 0);
    }
    return $this->handle_hooks('PRE', $mode, $addAttrs);
  }

  function fillHookAttrs (array &$addAttrs)
  {
    // Walk trough attributes list and add the plugins attributes.
    foreach ($this->attributes as $attr) {
      if (!isset($addAttrs[$attr])) {
        $addAttrs[$attr] = $this->$attr;
      }
    }
  }

  /*!
   * \brief    Calls external hooks which are defined for this plugin (fusiondirectory.conf)
   *           Replaces placeholder by class values of this plugin instance.
   *       Allows to a add special replacements.
   */
  function callHook ($cmd, array $addAttrs = [], &$returnOutput = [], &$returnCode = NULL): array
  {
    if ($this->is_template) {
      return [];
    }
    global $config;

    $commands = $config->searchHooks(get_class($this), $cmd);
    $messages = [];

    foreach ($commands as $command) {
      $this->fillHookAttrs($addAttrs);

      $ui = get_userinfo();

      $addAttrs['callerDN']         = $ui->dn;
      $addAttrs['callerCN']         = $ui->cn;
      $addAttrs['callerUID']        = $ui->uid;
      $addAttrs['callerSN']         = $ui->sn;
      $addAttrs['callerGIVENNAME']  = $ui->givenName;

      $addAttrs['dn']         = $this->dn;
      $addAttrs['location']   = $config->current['NAME'];

      if (isset($this->parent->by_object)) {
        foreach ($this->parent->by_object as $class => $object) {
          if ($class != get_class($this)) {
            $object->fillHookAttrs($addAttrs);
          }
        }
      }

      if (!isset($addAttrs['base']) && isset($this->base)) {
        $addAttrs['base'] = $this->base;
      }

      $command = templateHandling::parseString($command, $addAttrs, 'escapeshellarg');

      logging::debug(DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, 'Execute');
      exec($command, $arr, $returnCode);
      $returnOutput = $arr;

      if ($returnCode != 0) {
        $str = implode("\n", $arr);
        logging::debug(DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, 'Execution failed code: '.$returnCode);
        logging::debug(DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, 'Output: '.$str);
        $messages[] = new SimplePluginHookError(
          $this,
          $cmd,
          $str,
          $returnCode
        );
      } elseif (is_array($arr)) {
        $str = implode("\n", $arr);
        logging::debug(DEBUG_SHELL, __LINE__, __FUNCTION__, __FILE__, $command, 'Output: '.$str);
        if (!empty($str) && $config->get_cfg_value('displayHookOutput', 'FALSE') == 'TRUE') {
          msg_dialog::display('['.get_class($this).' '.strtolower($cmd).'trigger] '.$command, $str, INFO_DIALOG);
        }
      }
    }
    return $messages;
  }

  /*! \brief This function checks the attributes values and yell if something is wrong
   */
  function check (): array
  {
    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, 'check');
    $messages = [];

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

    $error = $this->callHook('CHECK', ['nbCheckErrors' => count($messages)], $returnOutput);
    if (!empty($error)) {
      $messages = array_merge($messages, $error);
    } elseif (!empty($returnOutput)) {
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

  function handleForeignKeys (string $olddn = NULL, string $newdn = NULL, string $mode = 'move')
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

  function browseForeignKeys (string $mode, $param1 = NULL, $param2 = NULL)
  {
    global $config, $plist;

    $subobjects = FALSE;
    if (preg_match('/^handle_/', $mode)) {
      $olddn    = $param1;
      $newdn    = $param2;
      $classes  = [get_class($this)];
      if (($olddn != $newdn) && $this->mainTab) {
        if ($newdn === NULL) {
          $subobjects = $this->hadSubobjects;
        } else {
          $ldap = $config->get_ldap_link();
          $ldap->cd($newdn);
          $ldap->search('(objectClass=*)', ['dn'], 'one');
          $subobjects = ($ldap->count() > 0);
        }
      }
    } elseif ($mode == 'references') {
      $classes = array_keys($this->parent->by_object);
    }
    // We group by objectType concerned
    $foreignRefs = [];
    if ($subobjects) {
      $field = 'dn';
      /* Special treatment for foreign keys on DN when moving an object
       * All references on DN are treated on subobjects */
      foreach ($plist->dnForeignRefs as $ref) {
        $class      = $ref[0];
        $ofield     = $ref[1];
        $filter     = $ref[2];
        $filtersub  = $ref[3];
        if ($filtersub == '*') {
          if ($config->get_cfg_value('wildcardForeignKeys', 'TRUE') == 'TRUE') {
            $filtersub = $ofield.'=*';
          } else {
            continue;
          }
        }
        if ($class == 'aclAssignment') {
          /* Special case: aclAssignment foreignKey is ignored on department types as it’s handled by the aclAssignment objectType */
          $objectTypes = ['ACLASSIGNMENT'];
        } elseif (is_subclass_of($class, 'simpleService')) {
          $objectTypes = ['SERVER'];
        } else {
          $objectTypes = [];
          $cinfos = pluglist::pluginInfos($class);
          foreach ($cinfos['plObjectType'] as $key => $objectType) {
            if (!is_numeric($key)) {
              $objectType = $key;
            }
            if (preg_match('/^ogroup-/i', $objectType)) {
              $objectType = 'OGROUP';
            }
            $objectTypes[] = strtoupper($objectType);
          }
          $objectTypes = array_unique($objectTypes);
        }
        foreach ($objectTypes as $objectType) {
          $oldvalue = $olddn;
          $newvalue = $newdn;

          $foreignRefs[$objectType]['refs'][$class][$ofield][$field] =
            [
              'tab'       => $classes[0],
              'field'     => $field,
              'oldvalue'  => $oldvalue,
              'newvalue'  => $newvalue,
            ];
          $filter = templateHandling::parseString($filtersub, ['oldvalue' => $oldvalue, 'newvalue' => $newvalue], 'ldap_escape_f');
          if (!preg_match('/^\(.*\)$/', $filter)) {
            $filter = '('.$filter.')';
          }
          $foreignRefs[$objectType]['filters'][$filter] = $filter;
        }
      }
    }
    foreach ($classes as $tabclass) {
      try {
        $infos = pluglist::pluginInfos($tabclass);
        foreach ($infos['plForeignRefs'] as $field => $refs) {
          if (preg_match('/^handle_/', $mode)) {
            if (
                (($newdn !== NULL) && ($field != 'dn') && ($mode == 'handle_move')) ||
                (($newdn === NULL) && ($olddn === NULL) && (($field == 'dn') || (!$this->attributeHaveChanged($field))))
              ) {
              // Move action, ignore other fields than dn
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
            if ($class == 'aclAssignment') {
              /* Special case: aclAssignment foreignKey is ignored on department types as it’s handled by the aclAssignment objectType */
              $objectTypes = ['ACLASSIGNMENT'];
            } elseif (is_subclass_of($class, 'simpleService')) {
              $objectTypes = ['SERVER'];
            } else {
              $objectTypes = [];
              foreach ($cinfos['plObjectType'] as $key => $objectType) {
                if (!is_numeric($key)) {
                  $objectType = $key;
                }
                if (preg_match('/^ogroup-/i', $objectType)) {
                  $objectType = 'OGROUP';
                }
                $objectTypes[] = $objectType;
              }
              $objectTypes = array_unique($objectTypes);
            }
            foreach ($objectTypes as $objectType) {
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
                $foreignRefs[$objectType]['refs'][$class][$ofield][$field] =
                  [
                    'tab'       => $tabclass,
                    'field'     => $field,
                    'oldvalue'  => $oldvalue,
                    'newvalue'  => $newvalue,
                  ];
                $filter = templateHandling::parseString($filter, ['oldvalue' => $oldvalue, 'newvalue' => $newvalue], 'ldap_escape_f');
              } elseif ($mode == 'references') {
                $foreignRefs[$objectType]['refs'][$class]['name'] = $cinfos['plShortName'];

                $foreignRefs[$objectType]['refs'][$class]['fields'][$ofield][$field] =
                  [
                    'tab'     => $tabclass,
                    'field'   => $field,
                    'tabname' => $this->parent->by_name[$tabclass],
                    'value'   => $this->parent->by_object[$tabclass]->$field,
                  ];
                $filter = templateHandling::parseString($filter, ['oldvalue' => $this->parent->by_object[$tabclass]->$field], 'ldap_escape_f');
              }
              if (!preg_match('/^\(.*\)$/', $filter)) {
                $filter = '('.$filter.')';
              }
              $foreignRefs[$objectType]['filters'][$filter] = $filter;
            }
          }
        }
      } catch (UnknownClassException $e) {
        /* May happen in special cases like setup */
        continue;
      }
    }

    /* Back up POST content */
    $SAVED_POST = $_POST;
    $refs = [];
    // For each concerned objectType
    foreach ($foreignRefs as $objectType => $tabRefs) {
      // Compute filter
      $filters = array_values($tabRefs['filters']);
      $filter = '(|'.join($filters).')';
      // Search objects
      try {
        $objects = objects::ls($objectType, ['dn' => 'raw'], NULL, $filter);
      } catch (NonExistingObjectTypeException $e) {
        continue;
      } catch (EmptyFilterException $e) {
        continue;
      }
      // For each object of this type
      foreach (array_keys($objects) as $dn) {
        /* Avoid sending POST to opened objects */
        $_POST = [];
        // Build the object
        $tabobject = objects::open($dn, $objectType);
        if (preg_match('/^handle_/', $mode)) {
          // For each tab concerned
          foreach ($tabRefs['refs'] as $tab => $fieldRefs) {
            // If the tab is activated on this object
            $pluginobject = $tabobject->getTabOrServiceObject($tab);
            if ($pluginobject !== FALSE) {
              // For each field
              foreach ($fieldRefs as $ofield => $fields) {
                foreach ($fields as $field) {
                  // call plugin::foreignKeyUpdate(ldapname, oldvalue, newvalue, source) on the object
                  $pluginobject->foreignKeyUpdate(
                    $ofield,
                    $field['oldvalue'],
                    $field['newvalue'],
                    [
                      'CLASS' => $field['tab'],
                      'FIELD' => $field['field'],
                      'MODE'  => preg_replace('/^handle_/', '', $mode),
                      'DN'    => $this->dn,
                    ]
                  );
                }
              }
              $pluginobject->save_object();
            }
          }
          $errors = $tabobject->save();
          msg_dialog::displayChecks($errors);
        } elseif ($mode == 'references') {
          // For each tab concerned
          foreach ($tabRefs['refs'] as $tab => $tab_infos) {
            // If the tab is activated on this object
            $pluginobject = $tabobject->getTabOrServiceObject($tab);
            if ($pluginobject !== FALSE) {
              // For each field
              foreach ($tab_infos['fields'] as $ofield => $fields) {
                foreach ($fields as $field) {
                  if ($pluginobject->foreignKeyCheck(
                        $ofield,
                        $field['value'],
                        [
                          'CLASS' => $field['tab'],
                          'FIELD' => $field['field'],
                          'DN'    => $this->dn,
                        ]
                      )) {
                    if (!isset($refs[$dn])) {
                      $refs[$dn] = [
                        'link'  => '',
                        'tabs'  => [],
                      ];
                      try {
                        $refs[$dn]['link'] = objects::link($dn, $objectType);
                      } catch (FusionDirectoryException $e) {
                        trigger_error("Could not create link to $dn: ".$e->getMessage());
                        $refs[$dn]['link'] = $dn;
                      }
                    }
                    if (!isset($refs[$dn]['tabs'][$tab])) {
                      $refs[$dn]['tabs'][$tab] = [
                        'link'    => '',
                        'fields'  => [],
                      ];
                      try {
                        if (is_subclass_of($tab, 'simpleService')) {
                          $refs[$dn]['tabs'][$tab]['link'] = objects::link($dn, $objectType, "service_$tab", sprintf(_('Service "%s"'), $tab_infos['name']));
                        } else {
                          $refs[$dn]['tabs'][$tab]['link'] = objects::link($dn, $objectType, "tab_$tab", sprintf(_('Tab "%s"'), $tab_infos['name']));
                        }
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
    }
    /* Restore POST */
    $_POST = $SAVED_POST;
    if ($mode == 'references') {
      return $refs;
    }
  }

  /*!
   * \brief Create unique DN
   *
   * \param string $attribute
   *
   * \param string $base
   */
  function create_unique_dn (string $attribute, string $base): string
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $base = preg_replace('/^,*/', '', $base);

    /* Try to use plain entry first */
    $dn = $attribute.'='.ldap_escape_dn($this->$attribute).','.$base;
    if (($dn == $this->orig_dn) || !$ldap->dn_exists($dn)) {
      return $dn;
    }

    /* Build DN with multiple attributes */
    $usableAttributes = [];
    foreach ($this->attributes as $attr) {
      if (($attr != $attribute) && is_scalar($this->$attr) && ($this->$attr != '')) {
        $usableAttributes[] = (string)$attr;
      }
    }
    for ($i = 1; $i < count($usableAttributes); $i++) {
      foreach (new Combinations($usableAttributes, $i) as $attrs) {
        $dn = $attribute.'='.ldap_escape_dn($this->$attribute);
        foreach ($attrs as $attr) {
          $dn .= '+'.$attr.'='.ldap_escape_dn($this->$attr);
        }
        $dn .= ','.$base;
        if (($dn == $this->orig_dn) || !$ldap->dn_exists($dn)) {
          return $dn;
        }
      }
    }

    /* None found */
    throw new FusionDirectoryException(_('Failed to create a unique DN'));
  }

  /*!
   * \brief Adapt from template
   *
   * Adapts fields to the values from a template.
   * Should not empty any fields, only take values for the ones provided by the caller.
   *
   * \param array $attrs LDAP attributes values for template-modified attributes
   * \param array $skip attributes to leave untouched
   */
  function adapt_from_template (array $attrs, array $skip = [])
  {
    $this->attrs = array_merge($this->attrs, $attrs);

    /* Walk through attributes */
    foreach ($this->attributesAccess as $ldapName => &$attr) {
      /* Skip the ones in skip list */
      if (in_array($ldapName, $skip)) {
        continue;
      }
      /* Load values */
      $attr->loadValue($attrs);
    }
    unset($attr);

    /* Is Account? */
    $this->is_account = $this->is_this_account($this->attrs);
  }

  /*!
   * \brief This function is called on the copied object to set its dn to where it will be saved
   */
  function resetCopyInfos ()
  {
    $this->dn       = 'new';
    $this->orig_dn  = $this->dn;

    $this->saved_attributes       = [];
    $this->initially_was_account  = FALSE;
  }

  protected function attributeHaveChanged (string $field): bool
  {
    return $this->attributesAccess[$field]->hasChanged();
  }

  protected function attributeValue (string $field)
  {
    return $this->attributesAccess[$field]->getValue();
  }

  protected function attributeInitialValue (string $field)
  {
    return $this->attributesAccess[$field]->getInitialValue();
  }

  function foreignKeyUpdate (string $field, $oldvalue, $newvalue, array $source)
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
  function foreignKeyCheck (string $field, $value, array $source)
  {
    return $this->attributesAccess[$field]->foreignKeyCheck($value, $source);
  }

  function deserializeValues (array $values, bool $checkAcl = TRUE)
  {
    foreach ($values as $name => $value) {
      if (isset($this->attributesAccess[$name])) {
        if (!$checkAcl || $this->attrIsWriteable($name)) {
          $error = $this->attributesAccess[$name]->deserializeValue($value);
          if (!empty($error)) {
            return $error;
          }
        } else {
          return msgPool::permModify($this->dn, $name);
        }
      } else {
        return sprintf(_('Unknown field "%s"'), $name);
      }
    }
    return TRUE;
  }

  /*! \brief Returns TRUE if this attribute should be asked in the creation by template dialog
   *
   * \return bool whether this attribute should be asked
   */
  function showInTemplate (string $attr, array $templateAttrs): bool
  {
    if (isset($templateAttrs[$attr])) {
      return FALSE;
    }
    return TRUE;
  }

  function is_modal_dialog (): bool
  {
    return (isset($this->dialog) && $this->dialog);
  }

  static function fillAccountAttrsNeeded (&$needed)
  {
    $infos = pluglist::pluginInfos(get_called_class());
    if (isset($infos['plFilterObject'])) {
      $attrs = $infos['plFilterObject']->listUsedAttributes();
      foreach ($attrs as $attr) {
        if (!isset($needed[$attr])) {
          $needed[$attr] = '*';
        }
      }
    }
  }

  static function isAccount ($attrs)
  {
    $infos = pluglist::pluginInfos(get_called_class());
    if (isset($infos['plFilterObject'])) {
      return $infos['plFilterObject']($attrs);
    }
    return NULL;
  }

  static function getLdapFilter ()
  {
    $infos = pluglist::pluginInfos(get_called_class());
    if (isset($infos['plFilter'])) {
      return $infos['plFilter'];
    }
    return NULL;
  }

  static function getLdapFilterObject ()
  {
    $infos = pluglist::pluginInfos(get_called_class());
    if (isset($infos['plFilterObject'])) {
      return $infos['plFilterObject'];
    }
    return NULL;
  }

  /*!
   * \brief Return plugin informations for acl handling
   *
   * \return an array
   */
  static function plInfo (): array
  {
    return [];
  }

  /*! \brief This function generate the needed ACLs for a given attribtues array
   *
   *  \param array $attributesInfo the attribute array
   *
   *  \param bool? $operationalAttributes Whether to add ACLs for operational attributes. Use NULL for autodetection (default)
   */
  static function generatePlProvidedAcls (array $attributesInfo, bool $operationalAttributes = NULL): array
  {
    $plProvidedAcls = [];
    foreach ($attributesInfo as $sectionInfo) {
      foreach ($sectionInfo['attrs'] as $attr) {
        if (($attr->getLdapName() === 'base') && ($operationalAttributes === NULL)) {
          /* If we handle base, we also handle LDAP operational attributes */
          $operationalAttributes = TRUE;
        }
        $aclInfo = $attr->getAclInfo();
        if ($aclInfo !== FALSE) {
          $plProvidedAcls[$aclInfo['name']] = $aclInfo['desc'];
        }
      }
    }
    if ($operationalAttributes) {
      $plProvidedAcls['createTimestamp'] = _('The time the entry was added');
      $plProvidedAcls['modifyTimestamp'] = _('The time the entry was last modified');
    }

    return $plProvidedAcls;
  }

  /*! \brief This function is the needed main.inc for plugins that are not used inside a management class
   *
   *  \param array $classname the class name to read plInfo from. (plIcon, plTitle, plShortname and plObjectType may be used)
   *
   *  \param string $entry_dn the dn of the object to show/edit
   *
   *  \param boolean $tabs TRUE to use tabs, FALSE to show directly the plugin class
   *
   *  \param boolean $edit_mode wether or not this plugin can be edited
   *
   *  \param string $objectType The objectType to use (will be taken in the plInfo if FALSE)
   *
   */
  static function mainInc ($classname = NULL, $entry_dn = NULL, $tabs = FALSE, $edit_mode = TRUE, $objectType = FALSE)
  {
    global $remove_lock, $cleanup, $display, $config, $plug, $ui, $smarty;

    if ($classname === NULL) {
      $classname = get_called_class();
    }

    if ($entry_dn === NULL) {
      $entry_dn = $ui->dn;
    }

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

    $lock_msg = "";
    if ($edit_mode
      && ($remove_lock || (isset($_POST['edit_cancel']) && session::is_set('edit')))
      && session::is_set($classname)) {
      /* Remove locks created by this plugin */
      del_lock($entry_dn);
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
          if ($locks = get_locks($entry_dn)) {
            session::set('LOCK_VARS_TO_USE', ["/^edit$/", "/^plug$/"]);
            $lock_msg = gen_locked_message($locks, $entry_dn);
          } else {
            /* Lock the current entry */
            add_lock($entry_dn, $ui->dn);
            session::set('edit', TRUE);
          }
        }

        /* save changes to LDAP and disable edit mode */
        if (isset($_POST['edit_finish'])) {
          /* Perform checks */
          $errors = $tabObject->save();

          /* No errors, save object */
          if (count($errors) == 0) {
            del_lock($entry_dn);
            session::un_set('edit');

            /* Remove from session */
            session::un_set($classname);
          } else {
            /* Errors found, show errors */
            msg_dialog::displayChecks($errors);
          }
        }
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
        /* Are we in edit mode? */
        if (session::is_set('edit')) {
          $display .= '<p class="plugbottom">'."\n";
          $display .= '<input type="submit" name="edit_finish" style="width:80px" value="'.msgPool::okButton().'"/>'."\n";
          $display .= '&nbsp;';
          $display .= '<input type="submit" formnovalidate="formnovalidate" name="edit_cancel" value="'.msgPool::cancelButton().'"/>'."\n";
          $display .= "</p>\n";
        } elseif (strpos($tabObject->by_object[$tabObject->current]->aclGetPermissions(''), 'w') !== FALSE) {
          /* Only display edit button if there is at least one attribute writable */
          $display  .= '<p class="plugbottom">'."\n";
          $info     .= '<div style="float:left;" class="optional"><img class="center" alt="information" '.
                    'src="geticon.php?context=status&amp;icon=dialog-information&amp;size=16"> '.
                    msgPool::clickEditToChange().'</div>';
          $display  .= '<input type="submit" name="edit" value="'.msgPool::editButton().'"/>'."\n";
          $display  .= "</p>\n";
        }
      }

      /* Page header */
      if (!preg_match('/^geticon/', $plIcon)) {
        $plIcon = get_template_path($plIcon);
      }
      $smarty->assign('headline', $plHeadline);
      $smarty->assign('headline_image', $plIcon);
      $display = '<div class="pluginfo">'.$info."</div>\n".$display;
    }
  }
}
