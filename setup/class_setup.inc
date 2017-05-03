<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
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

require_once("class_setupStep.inc");

class fake_userinfo extends userinfo
{
  function __construct()
  {
    $this->cn   = 'fake_cn';
    $this->dn   = 'fake_dn';
    $this->uid  = 'fake_uid';
    $this->ip   = $_SERVER['REMOTE_ADDR'];

    /* This fake user have all rights */
    $this->ignoreACL = TRUE;

    /* Initialize ACL_CACHE */
    $this->reset_acl_cache();
  }
}

class setup
{
  /* Number of setup steps */
  var $i_steps;

  /* Current step */
  var $i_current        = 0;

  /* Previous setup step */
  var $i_previous       = 0;

  var $i_config         = 4;
  var $o_steps          = array();
  var $captured_values  = array();

  function __construct()
  {
    $this->o_steps = array(
      new setupStepWelcome($this),
      new setupStepLanguage($this),
      new setupStepChecks($this),
      new setupStepLdap($this),
      new setupStepConfig_before_init($this),
      new setupStepMigrate($this),
      new setupStepFinish($this),
    );

    $this->i_steps = count($this->o_steps);

    /* Ensure that setup is not reachable if fusiondirectory.conf exist (CONFIG_FILE) */
    if (file_exists(CONFIG_DIR."/".CONFIG_FILE)) {
      session::destroy();
      header("Location: index.php");
      exit();
    }
  }

  function execute()
  {
    /* Display phpinfo() dialog when $_GET['info'] is set,
     *  but only do this, if user is allowed to use the setup.
     * If setupsetupStepWelcome is_completed, we are allowed to view those infos-
     */
    if (isset($_GET['info']) && preg_match("/setupStepWelcome/i", get_class($this->o_steps[1])) && $this->o_steps[1]->is_completed()) {
      phpinfo();
      exit();
    }

    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    $this->o_steps[$this->i_previous]->set_active(FALSE);
    $this->o_steps[$this->i_current]->set_active();
    $content = $this->o_steps[$this->i_current]->execute();
    return $content;
  }


  /* Save posted attributes  */
  function save_object()
  {
    /* Call save_object for current setup step */
    $this->o_steps[$this->i_current]->save_object();

    /* Get attributes from setup step */
    $tmp = $this->o_steps[$this->i_current]->get_attributes();
    foreach ($tmp as $name => $value) {
      $this->captured_values[$name] = $value;
    }

    /* Set parent */
    foreach ($this->o_steps as $key => $value) {
      $this->o_steps[$key]->parent = $this;
    }

    /* Check if image button requests next page */
    foreach ($_POST as $name => $value) {
      if (preg_match("/^next_(x|y)/", $name)) {
        $_POST['next'] = TRUE;
      }
      if (preg_match("/^last_(x|y)/", $name)) {
        $_POST['last'] = TRUE;
      }
    }

    /* display step error msgs */
    $msgs = $this->o_steps[$this->i_current]->check();
    foreach ($msgs as $msg) {
      msg_dialog::display(_("Setup error"), $msg, ERROR_DIALOG);
    }

    /* Check if step was selected */
    if (isset($_GET['step']) || isset($_POST['next']) || isset($_POST['last'])) {

      /* check if current setup step is completed now
          and activate the next step if possible */
      for ($i = 0; $i < $this->i_steps; $i++) {
        if ($this->o_steps[$i]->is_completed()) {
          if (isset($this->o_steps[($i + 1)])) {
            $this->o_steps[($i + 1)]->set_enabled();
          }
        } else {
          $this->disable_steps_from($i + 1);
          break;
        }
      }
    }

    /* Disable all following steps, if one step isn't completed right now .*/
    for ($i = 0; $i < $this->i_steps; $i++) {
      if (!$this->o_steps[$i]->is_completed()) {
        $this->disable_steps_from($i + 1);
        break;
      }
    }

    $step = -1;

    if (isset($_POST['setup_goto_step'])) {
      $step = $_POST['setup_goto_step'];
    }

    if (isset($_GET['step'])) {
      $step = $_GET['step'];
    } elseif (isset($_POST['next'])) {
      $step = $this->i_current + 1;
    } elseif (isset($_POST['last'])) {
      $step = $this->i_current - 1;
    }

    foreach ($_POST as $name => $value) {
      if (preg_match("/^step_[0-9]*$/", $name)) {
        $step = preg_replace("/^step_/", "", $name);
        break;
      }
    }

    if ($this->selectable_step($step)) {
      $this->i_previous = $this->i_current;
      $this->i_current  = $step;
    }
  }


  function disable_steps_from($start)
  {
    for ($i = $start; $i < $this->i_steps; $i++) {
      $this->o_steps[$i]->set_enabled(FALSE);
      $this->o_steps[$i]->set_completed(FALSE);
    }
  }


  /* Create navigation menu */
  function get_navigation_html()
  {
    $str = '<ul class="menu"><li><a>FusionDirectory Setup</a><ul>';
    foreach ($this->o_steps as $key => $step) {

      $step->update_strings();

      $s_short_name   = $step->get_short_name();
      $s_description  = $step->get_description();
      $b_active       = $step->is_active();
      $b_enabled      = $step->is_enabled();
      $b_completed    = $step->is_completed();

      if ($b_completed) {
        $s = '<img src="geticon.php?context=status&amp;icon=task-complete&amp;size=16" alt="'._('Completed').'" class="center optional"/>&nbsp;';
      } else {
        $s = '<img src="images/empty.png" alt=" " class="center optional"/>&nbsp;';
      }

      if ($b_enabled) {
        if ($b_active) {
          $str .= '<li class="menuitem menucurrent" title="'.$s_description.'">';
          $str .= '<a class="navigation-title">'.$s.$s_short_name.'</a>';
          $str .= '<a class="navigation-info">'.$s_description.'</a>';
          $str .= '</li>';
        } else {
          $str .= '<li class="menuitem" title="'.$s_description.'">';
          $str .= '<a onClick="document.mainform.setup_goto_step.value=\''.$key.'\';document.mainform.submit();"
            class="navigation-title">'.$s.$s_short_name.'</a>';
          $str .= '</li>';
        }
      } else {
        $str .= '<li class="menuitem disabled" title="'.$s_description.'">';
        $str .= '<a class="navigation-title">'.$s.$s_short_name.'</a>';
        $str .= '</li>';
      }
    }
    $str .= '</li></ul>';
    return $str;
  }


  function get_bottom_html()
  {
    /* Skip adding forward/backward button,
     *  if the currently opened step is a sub dialog
     */
    if ($this->o_steps[$this->i_current]->is_modal_dialog()) {
      $str = '';
    } else {
      $str = '   <p class="plugbottom">';
      if (isset($this->o_steps[$this->i_current - 1]) && $this->o_steps[$this->i_current - 1]->is_enabled()) {
        $str .= '<input type="submit" name="last" value="'.msgPool::backButton().'"/>';
      } else {
        $str .= '<input type="button" name="last" value="'.msgPool::backButton().'" disabled="disabled"/>';
      }
      $str .= '&nbsp;';
      $str .= '<input type="submit" name="next" value="'._('Next').'"/>';
      $str .= '</p>';
    }
    return $str;
  }

  /* Create header entry */
  function get_header_text()
  {
    return $this->o_steps[$this->i_current]->get_title();
  }

  /* Create header entry */
  function get_header_image()
  {
    return $this->o_steps[$this->i_current]->header_image;
  }


  /* Check if the given step id is valid and selectable */
  function selectable_step($id)
  {
    if (isset($this->o_steps[$id]) && $this->o_steps[$id]->is_enabled()) {
      return TRUE;
    }
    return FALSE;
  }

  function step_name_to_id($name)
  {
    foreach ($this->o_steps as $id => $class) {
      if (get_class($class) == $name) {
        return $id;
      }
    }
    return 0;
  }

  /* Called when LDAP is configured */
  function read_ldap_config()
  {
    global $config;
    /* Get attributes from current ldap step */
    $tmp = $this->o_steps[$this->i_current]->get_attributes();
    foreach ($tmp as $name => $value) {
      $this->captured_values[$name] = $value;
    }
    $smarty = get_smarty();
    $cv = $this->captured_values;
    /* Fill missing values needed by config file template (config step not done yet) */
    $cv['fdLogging']        = FALSE;
    $cv['fdDisplayErrors']  = FALSE;
    $cv['fdForceSSL']       = TRUE;
    $cv['debugLevel']       = 0;
    $smarty->assign('cv',                       xmlentities($cv));
    $smarty->assign('templateCompileDirectory', SPOOL_DIR);
    $xml = $smarty->fetch(CONFIG_TEMPLATE_DIR.CONFIG_FILE);
    $config->parse_data($xml);
    $config->set_current($config->data['MAIN']['DEFAULT']);
    session::global_un_set('plist');
    load_plist();
    $this->reBuildConfigStep();
  }

  function getDebugLevel ()
  {
    return $this->o_steps[$this->i_config]->attributesAccess['fdDebugLevel']->computeLdapValue();
  }

  function reBuildConfigStep ($completed = NULL)
  {
    $this->o_steps[$this->i_config] = new setupStepConfig($this, $this->captured_values);
    if ($completed !== NULL) {
      $this->o_steps[$this->i_config]->is_completed = $completed;
    }
  }
}
?>
