<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2018  FusionDirectory

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

/* base class for passwordRecovery and such classes handling requests on their own */
abstract class standAlonePage
{
  protected $directories;
  protected $directory;
  protected $activated;
  protected $interactive;

  /* Constructor */
  function __construct ($interactive = TRUE)
  {
    global $config, $ssl, $ui;

    $this->interactive = $interactive;

    if ($this->interactive) {
      /* Destroy old session if exists.
          Else you will get your old session back, if you not logged out correctly. */
      session::destroy();
      session::start();

      $config = $this->loadConfig();
      session::set('config', $config);

      /* Generate server list */
      $this->directories = [];
      foreach ($config->data['LOCATIONS'] as $key => $ignored) {
        $this->directories[$key] = $key;
      }

      $ui = new userinfoNoAuth(get_class($this));
      session::set('ui', $ui);
    }

    static::init();
  }

  abstract protected function readLdapConfig (): bool;

  function checkDirectoryChooser ()
  {
    global $config;

    $olddirectory = $this->directory;

    if (isset($_POST['server']) && isset($this->directories[$_POST['server']])) {
      $this->directory = validate($_POST['server']);
    } elseif (isset($_GET['directory']) && isset($this->directories[$_GET['directory']])) {
      $this->directory = validate($_GET['directory']);
    } elseif (empty($this->directory)) {
      $this->directory = $config->data['MAIN']['DEFAULT'];

      if (!isset($this->directories[$this->directory])) {
        $this->directory = key($this->directories);
      }
    }

    if ($this->directory != $olddirectory) {
      /* Set config to selected one */
      $config->set_current($this->directory);

      $this->activated = $this->readLdapConfig();
    }
  }

  function init ()
  {
    global $config, $ssl, $ui;

    if (!$this->interactive) {
      $this->activated = $this->readLdapConfig();
      return;
    }

    static::checkDirectoryChooser();

    reset_errors();

    static::securityHeaders();

    CSRFProtection::check();

    $ui     = session::get('ui');
    $config = session::get('config');

    timezone::setDefaultTimezoneFromConfig();

    Language::init();

    $this->setupSmarty();

    $ssl = $this->checkForSSL();

    /* Prepare plugin list */
    pluglist::load();
  }

  function loadConfig ()
  {
    global $BASE_DIR;

    /* Check if CONFIG_FILE is accessible */
    if (!is_readable(CONFIG_DIR.'/'.CONFIG_FILE)) {
      throw new FatalError(
        htmlescape(sprintf(
          _('FusionDirectory configuration %s/%s is not readable. Aborted.'),
          CONFIG_DIR,
          CONFIG_FILE
        ))
      );
    }

    /* Parse configuration file */
    $config = new config(CONFIG_DIR.'/'.CONFIG_FILE, $BASE_DIR);
    session::set('DEBUGLEVEL', $config->get_cfg_value('debuglevel'));
    logging::debug(DEBUG_CONFIG, __LINE__, __FUNCTION__, __FILE__, $config->data, 'config');
    return $config;
  }

  function setupSmarty ()
  {
    global $config;

    $smarty = get_smarty();

    /* Set template compile directory */
    $smarty->compile_dir = $config->get_cfg_value('templateCompileDirectory', SPOOL_DIR);

    /* Check for compile directory */
    if (!(is_dir($smarty->compile_dir) && is_writable($smarty->compile_dir))) {
      throw new FatalError(
        htmlescape(sprintf(
          _('Directory "%s" specified as compile directory is not accessible!'),
          $smarty->compile_dir
        ))
      );
    }

    /* Check for old files in compile directory */
    clean_smarty_compile_dir($smarty->compile_dir);

    $smarty->assign('date',       gmdate('D, d M Y H:i:s'));
    $smarty->assign('params',     '');
    $smarty->assign('message',    '');
    $smarty->assign('changed',    FALSE);
    $smarty->assign('copynotice', copynotice());

    $lang = session::get('lang');
    $smarty->assign('lang',         preg_replace('/_.*$/', '', $lang));
    $smarty->assign('rtl',          Language::isRTL($lang));
    $smarty->assign('usePrototype', 'FALSE');
    $smarty->assign('CSRFtoken',    CSRFProtection::getToken());

    if (count($this->directories) > 1) {
      $smarty->assign('show_directory_chooser', TRUE);
      $smarty->assign('server_options',         $this->directories);
      $smarty->assign('server_id',              $this->directory);
    } else {
      $smarty->assign('show_directory_chooser', FALSE);
    }
  }

  function assignSmartyVars ()
  {
    global $error_collector, $error_collector_mailto;
    $smarty = get_smarty();

    $smarty->assign('PHPSESSID', session_id());
    if ($error_collector != '') {
      $smarty->assign('php_errors', preg_replace('/%BUGBODY%/', $error_collector_mailto, $error_collector).'</div>');
    } else {
      $smarty->assign('php_errors', '');
    }

    $smarty->assign('msg_dialogs',  msg_dialog::get_dialogs());
  }

  function checkForSSL ()
  {
    global $config;
    $smarty = get_smarty();

    /* Check for SSL connection */
    $ssl = '';
    $smarty->assign('ssl', '');
    if (!URL::sslOn()) {
      $ssl = URL::getSslUrl();

      /* If SSL is forced, just forward to the SSL enabled site */
      if ($config->get_cfg_value('forcessl') == 'TRUE') {
        header("Location: $ssl");
        exit;
      } elseif ($config->get_cfg_value('warnssl') == 'TRUE') {
        /* Display SSL mode warning? */
        $smarty->assign('ssl', sprintf(htmlescape(_('Warning: %sSession is not encrypted!%s')), '<a href="'.$ssl.'">', '</a>'));
      }
    }

    return $ssl;
  }

  function encodeParams ($keys)
  {
    $params = '';
    foreach ($keys as $key) {
      $params .= "&amp;$key=".urlencode($this->$key);
    }
    return preg_replace('/^&amp;/', '?', $params);
  }

  public function isActivated ()
  {
    return $this->activated;
  }

  static function run ()
  {
    session::start();

    $class = get_called_class();
    if (session::is_set('standAlonePage_'.$class)) {
      $page = session::get('standAlonePage_'.$class);
      $page->init();
    } else {
      $page = new $class();
    }
    session::set('standAlonePage_'.$class, $page);

    $page->execute();
  }

  static function securityHeaders ()
  {
    header('X-XSS-Protection: 1; mode=block');
    header('X-Content-Type-Options: nosniff');
    header('X-Frame-Options: deny');
  }

  static function generateRandomHash ()
  {
    /* Generate a very long random value */
    $len        = 56;
    $base       = 'ABCDEFGHKLMNOPQRSTWXYZabcdefghjkmnpqrstwxyz123456789';
    $max        = strlen($base) - 1;
    $randomhash = '';
    while (strlen($randomhash) < $len + 1) {
      $randomhash .= $base[random_int(0, $max)];
    }
    return $randomhash;
  }
}
