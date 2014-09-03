<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007  Fabian Hickert
  Copyright (C) 2011-2014  FusionDirectory

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

class Step_Finish extends setupStep
{
  var $header_image = 'geticon.php?context=devices&icon=server&size=48';

  static function getAttributesInfo()
  {
    return array(
      'welcome' => array(
        'name'      => _('Welcome'),
        'template'  => get_template_path("setup_finish.tpl", TRUE, dirname(__FILE__)),
        'attrs'     => array(
        )
      )
    );
  }

  function update_strings()
  {
    $this->s_title      = _('Finish');
    $this->s_info       = _('Write configuration file');
    $this->s_title_long = _('Finish - write the configuration file');
  }

  function get_conf_data()
  {
    $smarty = get_smarty();
    $smarty->assign('cv',                       xmlentities($this->parent->captured_values));
    $smarty->assign('config_checksum',          md5(file_get_contents(CONFIG_TEMPLATE_DIR.CONFIG_FILE)));
    $smarty->assign('templateCompileDirectory', SPOOL_DIR);
    return $smarty->fetch(CONFIG_TEMPLATE_DIR.CONFIG_FILE);
  }

  function insertConfigDefaults()
  {
    /* Insert default config values, even for installed plugin */
    global $config, $ui, $plist, $BASE_DIR;
    $cv = $this->parent->captured_values;

    /* Create config object */

    $config = new config(CONFIG_DIR."/".CONFIG_FILE, $BASE_DIR);
    $config->set_current($config->data['MAIN']['DEFAULT']);

    /* Create ui object */

    /* got user dn, fill acl's */
    $ui           = new userinfo($config, $cv['valid_admin']);
    $ui->username = 'fd-admin';
    /* Username is set, load subtreeACL's now */
    $ui->loadACL();

    /* We need the pluglist object */
    load_all_classes();
    $plist = new pluglist($config, $ui);

    /* Now we can save LDAP config */
    $config->loadPlist($plist);
    $config->checkLdapConfig();
  }

  function execute()
  {
    /* Check if there is currently an active fusiondirectory.conf */
    $exists   = file_exists(CONFIG_DIR."/".CONFIG_FILE);
    $err_msg  = '';

    if ($exists && $this->is_world_readable(CONFIG_DIR.'/'.CONFIG_FILE)) {
      $err_msg = _('Your configuration file is currently world readable. Please update the file permissions!');
    } elseif (!$exists) {
      $err_msg = _('The configuration is currently not readable or it does not exists.');
    }

    $smarty = get_smarty();
    $smarty->assign('err_msg',  $err_msg);
    $smarty->assign('msg2',     sprintf(_('After downloading and placing the file under %s, please make sure that the user the webserver is running with is able to read %s, while other users shouldn\'t.'), CONFIG_DIR, CONFIG_FILE));

    return parent::execute();
  }

  function save_object()
  {
    parent::save_object();
    $exists = file_exists(CONFIG_DIR.'/'.CONFIG_FILE);

    /* Redirect to FusionDirectory login, if :
     *   - fusiondirectory.conf exists
     *   - Permisssion are set correctly
     */
    if (isset($_POST['next']) && $exists && !$this->is_world_readable(CONFIG_DIR.'/'.CONFIG_FILE)) {
      $this->insertConfigDefaults();
      session::destroy();
      header('Location: index.php');
      exit();
    }

    /* Download config */
    if (isset($_POST['getconf'])) {
      send_binary_content($this->get_conf_data(), CONFIG_FILE, 'text/plain');
    }
  }

  /* check if given file is world readable */
  function is_world_readable($file)
  {
    clearstatcache();
    $p = fileperms($file);
    return (decbin($p & 4) == TRUE);
  }
}
?>
