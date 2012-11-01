<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012  FusionDirectory

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

  /*! \brief ldap error cache
   *
   */
  protected $ldap_error;

  /*! \brief Attributes that needs to be initialized before the others.
   *
   */
  protected $preInitAttributes = array();

  /*! \brief constructor
   *
   *  \param mixed $config The config object
   *  \param string $dn The dn of this instance
   *  \param Object $object An object to copy values from
   *  \param array $attributesInfo An attributesInfo array, if NULL, getAttributesInfo will be used.
   *
   */
  function __construct (&$config, $dn = NULL, $object = NULL, $attributesInfo = NULL)
  {
    if ($attributesInfo === NULL) {
      $attributesInfo = $this->getAttributesInfo();
    }
    if (!$this->displayHeader) {
      // If we don't display the header to activate/deactive the plugin, that means it's always activated
      $this->ignore_account = TRUE;
    }

    $this->attributes     = array();
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
        if ($attr->isInLdap()) {
          $this->attributes[] = $name;
        }
      }
      $sectionInfo['attrs']           = $attrs;
      $this->attributesInfo[$section] = $sectionInfo;
    }
    // We call plugin constructor
    parent::__construct($config, $dn, $object);

    $this->orig_dn = $dn;

    if ($this->mainTab) {
      $this->is_account = TRUE;
    }

    // We load attributes values
    // First the one flagged as preInit
    foreach ($this->preInitAttributes as $section => $attrs) {
      foreach ($attrs as $attr) {
        $this->attributesInfo[$section]['attrs'][$attr]->setParent($this);
        $this->attributesInfo[$section]['attrs'][$attr]->loadValue($this->attrs);
      }
    }
    // Then the others
    foreach ($this->attributesInfo as $section => &$sectionInfo) {
      foreach ($sectionInfo['attrs'] as $name => &$attr) {
        if (isset($this->preInitAttributes[$section]) && in_array($name,$this->preInitAttributes[$section])) {
          continue; // skip the preInit ones
        }
        $attr->setParent($this);
        $attr->loadValue($this->attrs);
      }
    }

    $this->updateAttributesValues();
  }

  /*! \brief This function update the values of LDAP attributes as class attribute,
    so that plugin code works correctly

    This should be removed once plugin class is not used anymore
   */
  function updateAttributesValues ()
  {
    $this->attributes = array();
    foreach ($this->attributesInfo as $section => $sectionInfo) {
      foreach ($sectionInfo['attrs'] as $attr) {
        if ($attr->isInLdap()) {
          $name = $attr->getLdapName();
          if ($name == "") {
            die ('Fatal error : attribute "'.$attr->getLabel().'" have an empty ldap name.');
          }
          $this->$name = $attr->getValue();
          $this->attributes[] = $name;
        }
      }
    }
  }

  /*! \brief This function display the plugin and return the html code
   */
  function execute ()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "execute");
    parent::execute();

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
        $plInfos = $this->plInfo();
        if ($this->is_account) {
          $this->header = $this->show_disable_header(
            msgPool::removeFeaturesButton($plInfos['plShortName']),
            msgPool::featuresEnabled($plInfos['plShortName']),
            FALSE,
            get_class($this).'_modify_state'
          );
        } else {
          $this->header = $this->show_enable_header(
            msgPool::addFeaturesButton($plInfos['plShortName']),
            msgPool::featuresDisabled($plInfos['plShortName']),
            FALSE,
            get_class($this).'_modify_state'
          );
          $this->displayPlugin = FALSE;
          return $this->header;
        }
      } elseif (!$this->is_account) {
        $plInfos = $this->plInfo();
        $this->header = "<img alt=\"\" src=\"images/small-error.png\" align=\"middle\"/>&nbsp;<b>".
                        msgPool::noValidExtension($plInfos['plShortName'])."</b>";
        $this->header .= back_to_main();
        $this->displayPlugin = FALSE;
        return $this->header;
      }
    }

    $smarty = get_smarty();

    $sections = array();
    foreach ($this->attributesInfo as $section => $sectionInfo) {
      $legend = $sectionInfo['name'];
      if (isset($sectionInfo['icon'])) {
        $legend = '<img src="'.$sectionInfo['icon'].'" alt="icon" title="section '.$sectionInfo['name'].' icon"/>'.$legend;
      }
      $smarty->assign("section", $legend);
      $smarty->assign("sectionId", $section);
      $attributes = array();
      foreach ($sectionInfo['attrs'] as $attr) {
        if ($attr->getAclInfo() !== FALSE) {
          // We assign ACLs so that attributes can use them in their template code
          $smarty->assign($attr->getAcl()."ACL", $this->getacl($attr->getAcl(), $this->acl_skip_write()));
        }
        $attr->renderAttribute($attributes);
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
    $smarty->assign("hiddenPostedInput", get_class($this)."_posted");

    return $this->header.$smarty->fetch(get_template_path('simpleplugin.tpl'));
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

  protected function acl_skip_write ()
  {
    return (!is_object($this->parent) && !session::is_set('edit'));
  }

  /*! \brief This function returns an LDAP filter for this plugin object classes
   */
  function getObjectClassFilter ()
  {
    if (!empty($this->objectclasses)) {
      return "(&(objectClass=".implode(")(objectClass=", $this->objectclasses)."))";
    } else {
      return "";
    }
  }

  /*! \brief This function removes the object from LDAP
   */
  function remove_from_parent()
  {
    if (!$this->initially_was_account) {
      return;
    }

    /* Remove FusionDirectory attributes */
    plugin::remove_from_parent();

    $ldap = $this->config->get_ldap_link();
    if ($this->mainTab) {
      $ldap->rmdir_recursive($this->dn);
    } else {
      $this->cleanup();
      $ldap->cd($this->dn);
      $ldap->modify($this->attrs);
    }
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));
    }

    new log("remove", "plugin/".get_class($this), $this->dn, array_keys($this->attrs), $ldap->get_error());

    /* Optionally execute a command after we're done */
    $this->handle_post_events("remove");
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
          if ($this->acl_is_writeable($attr->getAcl())) {
            // Each attribute know how to read its value from POST
            $attr->loadPostValue();
          }
        }
      }
      // A second one that applies them. That allow complex stuff such as attribute disabling
      foreach ($this->attributesInfo as $section => &$sectionInfo) {
        foreach ($sectionInfo['attrs'] as &$attr) {
          if ($this->acl_is_writeable($attr->getAcl())) {
            // Each attribute know how to read its value from POST
            $attr->applyPostValue();
          }
        }
      }
      $this->updateAttributesValues();
    }
  }

  /*! \brief This function saves the object in the LDAP
   */
  function save ()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "save");
    $this->prepare_save();
    $this->ldap_save();
    $this->log_save();
  }

  protected function prepare_save ()
  {
    /* Call parents save to prepare $this->attrs */
    parent::save();

    /* Adapt some values to LDAP syntax (mostly booleans) */
    foreach ($this->attributesInfo as $section => $sectionInfo) {
      foreach ($sectionInfo['attrs'] as $attr) {
        $attr->fillLdapValue($this->attrs);
      }
    }
    /* Adapt some values to LDAP syntax (mostly booleans) */
    foreach ($this->attributesInfo as $section => $sectionInfo) {
      foreach ($sectionInfo['attrs'] as $attr) {
        $attr->fillLdapValueHook($this->attrs);
      }
    }
  }

  protected function ldap_save ($cleanup = TRUE)
  {
    /* Check if this is a new entry ... add/modify */
    $ldap = $this->config->get_ldap_link();
    $ldap->cat($this->dn, array("objectClass"));
    if ($ldap->count()) {
      $ldap->cd($this->dn);
      if ($cleanup) {
        $this->cleanup();
      }
      $ldap->modify($this->attrs);
    } else {
      if ($this->mainTab) {
        $ldap->cd($this->config->current['BASE']);
        $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
      }
      if ($cleanup) {
        $this->cleanup();
      }
      $ldap->cd($this->dn);
      $ldap->add($this->attrs);
    }

    $this->ldap_error = $ldap->get_error();

    /* Check for errors */
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }
  }

  protected function log_save()
  {
    /* Propagate and log the event */
    if ($this->initially_was_account) {
      $this->handle_post_events("modify");
      new log("modify", "plugin/".get_class($this), $this->dn, array_keys($this->attrs), $this->ldap_error);
    } else {
      $this->handle_post_events("add");
      new log("create", "plugin/".get_class($this), $this->dn, array_keys($this->attrs), $this->ldap_error);
    }
  }

  /*! \brief This function checks the attributes values and yell if something is wrong
   */
  function check ()
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, "check");
    $message = parent::check();

    foreach ($this->attributesInfo as $section => $sectionInfo) {
      foreach ($sectionInfo['attrs'] as $attr) {
        $error = $attr->check();
        if (!empty($error)) {
          $message[] = $error;
        }
      }
    }

    return $message;
  }

  /*! \brief This function generate the needed ACLs for a given attribtues array
   *
   *  \param array $attributesInfo the attribute array
   */
  static function generatePlProvidedAcls ($attributesInfo)
  {
    $plProvidedAcls = array();
    foreach ($attributesInfo as $section => $sectionInfo) {
      foreach ($sectionInfo['attrs'] as $attr) {
        $aclInfo = $attr->getAclInfo();
        if ($aclInfo !== FALSE) {
          $plProvidedAcls[$aclInfo['name']] = $aclInfo['desc'];
        }
      }
    }

    return $plProvidedAcls;
  }

  /*! \brief This function is the needed main.inc for users tab
   *
   *  \param array $classname the plugin class name
   */
  static function userMainInc ($classname, $entry_dn = FALSE)
  {
    global $remove_lock, $cleanup, $ui, $display, $config;
    if ($entry_dn === FALSE) {
      $entry_dn = $ui->dn;
    }
    $plInfo     = call_user_func(array($classname, 'plInfo'));
    $plIcon     = (isset($plInfo['plIcon'])?$plInfo['plIcon']:"plugin.png");
    $plHeadline = $plInfo['plShortName'];
    $plCategory = (isset($plInfo['plCategory'])?$plInfo['plCategory']:array("users"));
    $key = key($plCategory);
    if (is_numeric($key)) {
      $plCategory = $plCategory[$key];
    } else {
      $plCategory = $key;
    }
    /* Remove locks created by this plugin */
    $lock_msg = "";
    if ($remove_lock || isset($_POST['edit_cancel'])) {
      if (session::is_set($classname) && session::is_set('edit')) {
        del_lock($entry_dn);
      }
    }

    /* Remove this plugin from session */
    if ($cleanup) {
      session::un_set($classname);
      session::un_set('edit');
    } else {
      /* Reset requested? */
      if (isset($_POST['edit_cancel'])) {
        session::un_set('edit');
        session::un_set($classname);
      }

      /* Create account object on demand */
      if (!session::is_set($classname) || (isset($_GET['reset']) && $_GET['reset'] == 1)) {
        $account = new $classname($config, $entry_dn);
        $account->enable_CSN_check();
        $account->set_acl_base($entry_dn);
        $account->set_acl_category($plCategory);
        session::set($classname, $account);
      }
      $account = session::get($classname);

      /* save changes back to object */
      if (session::is_set('edit')) {
        $account->save_object();
      }

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
        $message = $account->check();

        /* No errors, save object */
        if (count($message) == 0) {
          $account->save();
          del_lock($entry_dn);
          session::un_set('edit');

          /* Remove from session */
          session::un_set($classname);
        } else {
          /* Errors found, show message */
          msg_dialog::displayChecks($message);
        }
      }

      /* Execute formular */
      if ($lock_msg) {
        $display = $lock_msg;
      } else {
        $display .= $account->execute();
      }

      /* Store changes  in session */
      if (session::is_set('edit')) {
        session::set($classname, $account);
      }

      /* Show page footer depending on the mode */
      if (!$account->dialog && $account->is_account && empty($lock_msg)) {
        $display .= "<p class=\"plugbottom\">";

        /* Are we in edit mode? */
        if (session::is_set('edit')) {
          $display .= "<input type=submit name=\"edit_finish\" style=\"width:80px\" value=\"".msgPool::okButton()."\">\n";
          $display .= "&nbsp;";
          $display .= "<input type=submit name=\"edit_cancel\" value=\"".msgPool::cancelButton()."\">\n";
          $info = "<img alt=\"\" align=\"middle\" src=\"".
                  get_template_path('images/lists/locked.png')."\"> ".
                  $entry_dn."&nbsp;";
        } else {
          $info = "<img alt=\"\" align=\"middle\" src=\"".
                  get_template_path('images/lists/unlocked.png')."\"> ".
                  $entry_dn."&nbsp;";

          /* Only display edit button if there is at least one attribute editable */
          if (preg_match("/r/", $ui->get_permissions($entry_dn, "$plCategory/$classname"))) {
            $info .= "<img alt=\"\" align=\"middle\" src=\"".
                      get_template_path('images/lists/on.png')."\"> ".
                      msgPool::clickEditToChange();

            $display .= "<input type=submit name=\"edit\" value=\"".msgPool::editButton()."\">\n";
          }
          $display .= "<input type=\"hidden\" name=\"ignore\">\n";
        }
        $display .= "</p>\n";
      }

      /* Page header */
      $display = print_header(get_template_path($plIcon), $plHeadline, $info).$display;
    }
  }
}

?>
