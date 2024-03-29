<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007  Fabian Hickert
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

class setupStepFinish extends setupStep
{
  var $header_image = 'geticon.php?context=devices&icon=server&size=48';

  static function getAttributesInfo (): array
  {
    return [
      'welcome' => [
        'name'      => _('Welcome'),
        'template'  => get_template_path("setup_finish.tpl", TRUE, dirname(__FILE__)),
        'attrs'     => [
        ]
      ]
    ];
  }

  function update_strings ()
  {
    $this->s_short_name   = _('Finish');
    $this->s_title        = _('Finish - write the configuration file');
    $this->s_description  = _('Write configuration file');
  }

  function get_conf_data ()
  {
    $smarty           = get_smarty();
    $cv               = $this->parent->captured_values;
    $cv['debugLevel'] = $this->parent->getDebugLevel();
    $smarty->assign('cv',                       xmlentities($cv));
    $smarty->assign('templateCompileDirectory', SPOOL_DIR);
    return $smarty->fetch(CONFIG_TEMPLATE_DIR.CONFIG_FILE);
  }

  public function readPost ()
  {
    parent::readPost();

    $exists = file_exists(CONFIG_DIR.'/'.CONFIG_FILE);

    /* Redirect to FusionDirectory login, if :
     *   - fusiondirectory.conf exists
     *   - Permisssion are set correctly
     */
    if (isset($_POST['next']) && $exists && !$this->is_world_readable(CONFIG_DIR.'/'.CONFIG_FILE)) {
      session::destroy();
      header('Location: index.php');
      exit();
    }

    /* Download config */
    if (isset($_POST['getconf'])) {
      send_binary_content($this->get_conf_data(), CONFIG_FILE, 'text/plain');
    }
  }

  public function render (): string
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
    $smarty->assign('msg2',     sprintf(_('After downloading and placing the file under %s, please make sure that the user the web server is running with is able to read %s, while other users shouldn\'t.'), CONFIG_DIR, CONFIG_FILE));

    return parent::render();
  }

  /* check if given file is world readable */
  function is_world_readable ($file)
  {
    clearstatcache();
    $p = fileperms($file);
    return (bool) decbin($p & 4);
  }
}
