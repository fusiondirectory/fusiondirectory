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
class standAlonePage {
  protected $directories;
  protected $directory;
  protected $activated;
  protected $interactive;

  /* Constructor */
  function __construct($interactive = TRUE)
  {
    global $config, $ssl, $ui;

    $this->interactive = $interactive;

    /* Destroy old session if exists.
        Else you will get your old session back, if you not logged out correctly. */
    session::destroy();
    session::start();

    $config = $this->loadConfig();
    session::global_set('config', $config);

    /* Generate server list */
    $this->directories = array();
    foreach ($config->data['LOCATIONS'] as $key => $ignored) {
      $this->directories[$key] = $key;
    }

    $ui = new userinfoNoAuth(get_class($this));
    session::global_set('ui', $ui);

    static::init();
  }

  function checkDirectoryChooser()
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

  function init()
  {
    global $config, $ssl, $ui;

    static::checkDirectoryChooser();

    reset_errors();

    static::securityHeaders();

    CSRFProtection::check();

    $ui     = session::global_get('ui');
    $config = session::global_get('config');

    if ($this->interactive) {
      timezone::setDefaultTimezoneFromConfig();

      Language::init();

      $this->setupSmarty();
    }

    $ssl = $this->checkForSSL();

    /* Prepare plugin list */
    pluglist::load();
  }

  function loadConfig()
  {
    global $BASE_DIR;

    /* Check if CONFIG_FILE is accessible */
    if (!is_readable(CONFIG_DIR.'/'.CONFIG_FILE)) {
      msg_dialog::display(_('Fatal error'),
                          sprintf(_('FusionDirectory configuration %s/%s is not readable. Aborted.'),
                                  CONFIG_DIR, CONFIG_FILE), FATAL_ERROR_DIALOG);
      exit();
    }

    /* Parse configuration file */
    $config = new config(CONFIG_DIR.'/'.CONFIG_FILE, $BASE_DIR);
    session::global_set('DEBUGLEVEL', $config->get_cfg_value('debuglevel'));
    @DEBUG(DEBUG_CONFIG, __LINE__, __FUNCTION__, __FILE__, $config->data, 'config');
    return $config;
  }

  function setupSmarty()
  {
    global $config;
    $smarty = get_smarty();

    /* Set template compile directory */
    $smarty->compile_dir = $config->get_cfg_value('templateCompileDirectory', SPOOL_DIR);

    /* Check for compile directory */
    if (!(is_dir($smarty->compile_dir) && is_writable($smarty->compile_dir))) {
      msg_dialog::display(_('Configuration error'),
                          sprintf(_("Directory '%s' specified as compile directory is not accessible!"),
                                  $smarty->compile_dir),
                          FATAL_ERROR_DIALOG);
      exit();
    }

    /* Check for old files in compile directory */
    clean_smarty_compile_dir($smarty->compile_dir);

    $smarty->assign('date',     gmdate('D, d M Y H:i:s'));
    $smarty->assign('params',   '');
    $smarty->assign('message',  '');
    $smarty->assign('changed',  FALSE);
    $smarty->assign('revision', FD_VERSION);
    $smarty->assign('year',     date('Y'));

    $lang = session::global_get('lang');
    $smarty->assign('lang',         preg_replace('/_.*$/', '', $lang));
    $smarty->assign('rtl',          Language::isRTL($lang));
    $smarty->assign('must',         '<span class="must">*</span>');
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

  function assignSmartyVars()
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

  function checkForSSL()
  {
    global $config;
    $smarty = get_smarty();

    /* Check for SSL connection */
    $ssl = '';
    $smarty->assign('ssl', '');
    if (!sslOn()) {
      $ssl = sslUrl();

      /* If SSL is forced, just forward to the SSL enabled site */
      if ($config->get_cfg_value('forcessl') == 'TRUE') {
        header("Location: $ssl");
        exit;
      } elseif ($config->get_cfg_value('warnssl') == 'TRUE') {
        /* Display SSL mode warning? */
        $smarty->assign ('ssl', sprintf(_('Warning: <a href="%s">Session is not encrypted!</a>'), $ssl));
      }
    }

    return $ssl;
  }

  function getPageURL()
  {
    $protocol = 'http';
    if (isset($_SERVER['HTTPS']) && ($_SERVER['HTTPS'] == 'on')) {
      $protocol .= 's';
    }
    $port = '80';
    if (!empty($_SERVER['HTTP_X_FORWARDED_HOST'])) {
      $host = $_SERVER['HTTP_X_FORWARDED_HOST'];
      if (isset($_SERVER['HTTP_X_FORWARDED_PORT'])) {
        $port = $_SERVER['HTTP_X_FORWARDED_PORT'];
      }
      if (isset($_SERVER['HTTP_X_FORWARDED_PROTO'])) {
        $protocol = $_SERVER['HTTP_X_FORWARDED_PROTO'];
      }
    } else {
      $host = $_SERVER['SERVER_NAME'];
      $port = $_SERVER['SERVER_PORT'];
    }

    $pageURL = $protocol.'://';
    $pageURL .= $host;
    if ((($protocol == 'http') && ($port != '80')) || (($protocol == 'https') && ($port != '443'))) {
      $pageURL .= ':'.$port;
    }
    if (empty($_SERVER['PATH_INFO'])) {
      $pageURL .= $_SERVER['PHP_SELF'];
    } else {
      $pageURL .= $_SERVER['PATH_INFO'];
    }

    return $pageURL;
  }

  function encodeParams($keys)
  {
    $params = '';
    foreach ($keys as $key) {
      $params .= "&amp;$key=".urlencode($this->$key);
    }
    return preg_replace('/^&amp;/', '?', $params);
  }

  static function run()
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

  static function securityHeaders()
  {
    header('X-XSS-Protection: 1; mode=block');
    header('X-Content-Type-Options: nosniff');
    header('X-Frame-Options: deny');
  }

  static function generateRandomHash()
  {
    /* Generate a very long random value */
    $len        = 56;
    $base       = 'ABCDEFGHKLMNOPQRSTWXYZabcdefghjkmnpqrstwxyz123456789';
    $max        = strlen($base) - 1;
    $randomhash = '';
    while (strlen($randomhash) < $len + 1) {
      $randomhash .= $base{random_int(0, $max)};
    }
    return $randomhash;
  }
}
