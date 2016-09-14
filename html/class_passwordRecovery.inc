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

require_once("../include/php_setup.inc");
require_once("functions.inc");
require_once("variables.inc");

/* base class for passwordRecovery and such classes handling requests on their own */
class standAlonePage {
  var $directory;
  var $activated;

  /* Constructor */
  function __construct()
  {
    global $config, $ssl, $ui;
    /* Destroy old session if exists.
        Else you will get your old session back, if you not logged out correctly. */
    session::destroy();
    session::start();

    /* Reset errors */
    reset_errors();

    $config       = $this->loadConfig();

    /* If SSL is forced, just forward to the SSL enabled site */
    if (($config->get_cfg_value("forcessl") == "TRUE") && ($ssl != '')) {
      header ("Location: $ssl");
      exit;
    }

    $this->setupSmarty();

    $smarty = get_smarty();

    /* Generate server list */
    $servers = array();
    foreach ($config->data['LOCATIONS'] as $key => $ignored) {
      $servers[$key] = $key;
    }

    $smarty->assign("show_directory_chooser", FALSE);

    if (isset($_POST['server'])) {
      $this->directory = validate($_POST['server']);
    } elseif (isset($_GET['directory']) && isset($servers[$_GET['directory']])) {
      $this->directory = validate($_GET['directory']);
    } else {
      $this->directory = $config->data['MAIN']['DEFAULT'];

      if (!isset($servers[$this->directory])) {
        $this->directory = key($servers);
      }

      if (count($servers) > 1) {
        $smarty->assign("show_directory_chooser", TRUE);
        $smarty->assign("server_options", $servers);
        $smarty->assign("server_id", $this->directory);
      }
    }

    /* Set config to selected one */
    $config->set_current($this->directory);
    session::global_set('config', $config);

    $this->activated = $this->readLdapConfig();
    if (!$this->activated) { /* Password recovery has been disabled */
      return;
    }

    initLanguage();

    if (session::global_is_set('plist')) {
      session::global_un_set('plist');
    }
    $ui = new fake_userinfo();
    load_plist();

    $ssl = $this->checkForSSL();
  }

  function loadConfig()
  {
    global $BASE_DIR;

    /* Check if CONFIG_FILE is accessible */
    if (!is_readable(CONFIG_DIR."/".CONFIG_FILE)) {
      msg_dialog::display(_("Fatal error"),
                          sprintf(_("FusionDirectory configuration %s/%s is not readable. Aborted."),
                                  CONFIG_DIR, CONFIG_FILE), FATAL_ERROR_DIALOG);
      exit();
    }

    /* Parse configuration file */
    $config = new config(CONFIG_DIR."/".CONFIG_FILE, $BASE_DIR);
    session::global_set('DEBUGLEVEL', $config->get_cfg_value("debuglevel"));
    @DEBUG(DEBUG_CONFIG, __LINE__, __FUNCTION__, __FILE__, $config->data, "config");
    return $config;
  }

  function setupSmarty()
  {
    global $config;
    $smarty = get_smarty();

    /* Set template compile directory */
    $smarty->compile_dir = $config->get_cfg_value("templateCompileDirectory", SPOOL_DIR);

    /* Check for compile directory */
    if (!(is_dir($smarty->compile_dir) && is_writable($smarty->compile_dir))) {
      msg_dialog::display(_("Configuration error"),
                          sprintf(_("Directory '%s' specified as compile directory is not accessible!"),
                                  $smarty->compile_dir),
                          FATAL_ERROR_DIALOG);
      exit();
    }

    /* Check for old files in compile directory */
    clean_smarty_compile_dir($smarty->compile_dir);

    $smarty->assign('password_img', 'geticon.php?context=status&icon=dialog-password&size=48');
    $smarty->assign('date', gmdate("D, d M Y H:i:s"));
    $smarty->assign('params', "");
    $smarty->assign('message', "");
    $smarty->assign('changed', FALSE);
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
    if ($port != '80') {
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
    $params = "";
    foreach ($keys as $key) {
      $params .= "&amp;$key=".urlencode($this->$key);
    }
    $params = preg_replace('/^&amp;/', '?', $params);
    return $params;
  }

  static function generateRandomHash()
  {
    /* Generate a very long random value */
    $len        = 56;
    $base       = 'ABCDEFGHKLMNOPQRSTWXYZabcdefghjkmnpqrstwxyz123456789';
    $max        = strlen($base) - 1;
    $randomhash = '';
    mt_srand((double) microtime() * 1000000);
    while (strlen($randomhash) < $len + 1) {
      $randomhash .= $base{mt_rand(0, $max)};
    }
    return $randomhash;
  }
}

class passwordRecovery extends standAlonePage {
  var $uid;
  var $message = array();
  var $email_address;

  var $step = 1;

  /* Some Configuration variable */

  /* Salt needed to mask the uniq id in the ldap */
  var $salt;
  /* Delay allowed for the user to change his password (minutes) */
  var $delay_allowed;

  /* Sender */
  var $from_mail;

  var $mail_body;
  var $mail_subject;

  var $mail2_body;
  var $mail2_subject;

  var $usealternates;

  /* Constructor */
  function __construct()
  {
    parent::__construct();

    if (isset($_GET['email_address']) && $_GET['email_address'] != "") {
      $this->email_address = validate($_GET['email_address']);
    } elseif (isset($_POST['email_address'])) {
      $this->email_address = validate($_POST['email_address']);
    }

    /* Check for selected user... */
    if (isset($_GET['uid']) && $_GET['uid'] != "") {
      $this->uid = validate($_GET['uid']);
    } elseif (isset($_POST['uid'])) {
      $this->uid = validate($_POST['uid']);
    } else {
      $this->uid = '';
    }
  }

  function execute()
  {
    if (!$this->activated) {
      return;
    }

    /* Got a formular answer, validate and try to log in */
    if ($_SERVER["REQUEST_METHOD"] == "POST") {
      if (session::global_is_set('_LAST_PAGE_REQUEST')) {
        session::global_set('_LAST_PAGE_REQUEST', time());
      }

      if (isset($_POST['change'])) {
        $this->step4();
      } elseif (isset($_POST['apply'])) {
        $this->step2();
        if ($this->step == 2) { /* No errors */
          $this->step3();
        }
      }
    } elseif ($_SERVER["REQUEST_METHOD"] == "GET") {
      if (isset($_GET['uniq'])) {
        $this->step4();
      }
    }
  }

  function displayPWchanger()
  {
    global $error_collector, $error_collector_mailto;
    /* Do we need to show error messages? */
    if (count($this->message) != 0) {
      /* Show error message and continue editing */
      msg_dialog::displayChecks($this->message);
    }

    @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->step, "Step");

    $smarty = get_smarty();

    $smarty->assign("PHPSESSID", session_id());
    if (session::is_set('errors')) {
      $smarty->assign("errors", session::get('errors'));
    }
    if ($error_collector != "") {
      $smarty->assign("php_errors", preg_replace("/%BUGBODY%/", $error_collector_mailto, $error_collector)."</div>");
    } else {
      $smarty->assign("php_errors", "");
    }

    $smarty->assign('msg_dialogs',  msg_dialog::get_dialogs());
    $smarty->assign('usePrototype', 'FALSE');
    $smarty->append('js_files',     'include/pwdStrength.js');
    $smarty->append('css_files',    get_template_path('login.css'));
    $lang = session::global_get('lang');
    $smarty->assign('lang', preg_replace('/_.*$/', '', $lang));
    $smarty->assign('rtl', language_is_rtl($lang));
    $smarty->display(get_template_path('headers.tpl'));

    $smarty->assign('version',        FD_VERSION);
    $smarty->assign('step',           $this->step);
    $smarty->assign('delay_allowed',  $this->delay_allowed);
    $smarty->assign('activated',      $this->activated);
    $smarty->assign('email_address',  $this->email_address);
    $smarty->display(get_template_path('recovery.tpl'));
    exit();
  }

  /* Check that password recovery is activated, read config in ldap
   * Returns a boolean saying if password recovery is activated
   */
  function readLdapConfig()
  {
    global $config;
    $this->salt           = $config->get_cfg_value('passwordRecoverySalt');
    $this->delay_allowed  = $config->get_cfg_value('passwordRecoveryValidity');

    $this->mail_subject   = $config->get_cfg_value('passwordRecoveryMailSubject');
    $this->mail_body      = $config->get_cfg_value('passwordRecoveryMailBody');
    $this->mail2_subject  = $config->get_cfg_value('passwordRecoveryMail2Subject');
    $this->mail2_body     = $config->get_cfg_value('passwordRecoveryMail2Body');

    $this->from_mail      = $config->get_cfg_value('passwordRecoveryEmail');

    $this->usealternates  = $config->get_cfg_value('passwordRecoveryUseAlternate');

    @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $config->get_cfg_value('passwordRecoveryActivated'), "passwordRecoveryActivated");
    return ($config->get_cfg_value('passwordRecoveryActivated') == "TRUE");
  }

  function storeToken($temp_password)
  {
    global $config;
    /* Store it in ldap with the salt */
    $salt_temp_password = $this->salt.$temp_password.$this->salt;
    $sha1_temp_password = "{SHA}".base64_encode(pack("H*", sha1($salt_temp_password)));

    $ldap = $config->get_ldap_link();

    // Check if token branch is here
    $token = get_ou('tokenRDN').get_ou('fusiondirectoryRDN').$config->current['BASE'];
    $ldap->cat($token, array('dn'));
    if (!$ldap->count()) {
      /* It's not, let's create it */
      $ldap->cd ($config->current['BASE']);
      $ldap->create_missing_trees($token);
      if (!$ldap->success()) {
        return msgPool::ldaperror($ldap->get_error(),
                                  $token, LDAP_MOD, get_class());
      }
      fusiondirectory_log("Created token branch ".$token);
    }

    $dn = "ou=".$this->uid.",$token";
    $ldap->cat($dn, array('dn'));
    $add = ($ldap->count() == 0);
    /* We store the token and its validity due date */
    $attrs = array(
                    'objectClass' => array('organizationalUnit'),
                    'ou' => $this->uid,
                    'userPassword' => $sha1_temp_password,
                    'description' => time() + $this->delay_allowed * 60,
                  );
    $ldap->cd($dn);
    if ($add) {
      $ldap->add($attrs);
    } else {
      $ldap->modify($attrs);
    }

    if (!$ldap->success()) {
      return msgPool::ldaperror($ldap->get_error(),
                                $dn, LDAP_ADD, get_class());
    }

    return ""; /* Everything went well */
  }

  function checkToken($token)
  {
    global $config;
    $salt_token = $this->salt.$token.$this->salt;
    $sha1_token = "{SHA}".base64_encode(pack("H*", sha1($salt_token)));

    /* Retrieve hash from the ldap */
    $ldap = $config->get_ldap_link();

    $token  = get_ou('tokenRDN').get_ou('fusiondirectoryRDN').$config->current['BASE'];
    $dn     = "ou=".$this->uid.",$token";
    $ldap->cat($dn);
    $attrs = $ldap->fetch();

    $ldap_token         = $attrs['userPassword'][0];
    $last_time_recovery = $attrs['description'][0];

    /* Return TRUE if the token match and is still valid */
    return ($last_time_recovery >= time()) &&
           ($ldap_token == $sha1_token);
  }

  function getUserDn()
  {
    global $config;
    /* Retrieve dn from the ldap */
    $ldap = $config->get_ldap_link();

    $objectClasses = array('gosaMailAccount');
    if (class_available('personalInfo') && ($config->get_cfg_value('privateEmailPasswordRecovery', 'FALSE') == 'TRUE')) {
      $objectClasses[] = 'fdPersonalInfo';
    }
    if (class_available('supannAccount') && ($config->get_cfg_value('supannPasswordRecovery', 'TRUE') == 'TRUE')) {
      $objectClasses[] = 'supannPerson';
    }
    $filter = '(&(|(objectClass='.join(')(objectClass=', $objectClasses).'))(uid='.$this->uid.'))';
    $ldap->cd($config->current['BASE']);
    $ldap->search($filter, array('dn'));

    if ($ldap->count() < 1) {
      $this->message[] = sprintf(_('Did not find an account with login "%s"'), $this->uid);
      return;
    } elseif ($ldap->count() > 1) {
      $this->message[] = sprintf(_('Found multiple accounts with login "%s"'), $this->uid);
      return;
    }

    $attrs = $ldap->fetch();

    return $attrs['dn'];
  }

  /* find the uid of for the given email address */
  function step2()
  {
    global $config;
    if ($_POST['email_address'] == "") {
      $this->message[] = msgPool::required(_("Email address"));
      return;
    }
    $this->email_address = $_POST['email_address'];

    /* Search uid corresponding to the mail */
    if ($this->usealternates) {
      $filter = "(&(objectClass=gosaMailAccount)(|(mail=".$this->email_address.")(gosaMailAlternateAddress=".$this->email_address.")))";
    } else {
      $filter = "(&(objectClass=gosaMailAccount)(mail=".$this->email_address."))";
    }
    if (class_available('personalInfo') && ($config->get_cfg_value('privateEmailPasswordRecovery', 'FALSE') == 'TRUE')) {
      $filter = '(|'.$filter.'(&(objectClass=fdPersonalInfo)(fdPrivateMail='.$this->email_address.')))';
    }
    if (class_available('supannAccount') && ($config->get_cfg_value('supannPasswordRecovery', 'TRUE') == 'TRUE')) {
      $filter = '(|'.$filter.'(&(objectClass=supannPerson)(supannMailPerso='.$this->email_address.')))';
    }
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search($filter, array('dn', 'userPassword', 'uid'));

    /* Only one uid should be found */
    if ($ldap->count() < 1) {
      $this->message[] = sprintf(_('There is no account using email "%s"'), $this->email_address);
      return;
    } elseif ($ldap->count() > 1) {
      $this->message[] = sprintf(_('There are several accounts using email "%s"'), $this->email_address);
      return;
    }

    $attrs = $ldap->fetch();

    $method = passwordMethod::get_method($attrs['userPassword'][0], $attrs['dn']);
    if (is_object($method) && $method->is_locked($attrs['dn'])) {
      $this->message[] = sprintf(_('The user using email "%s" is locked. Please contact your administrator.'), $this->email_address);
      return;
    }

    $smarty = get_smarty();

    $this->uid = $attrs['uid'][0];
    $smarty->assign('uid', $this->uid);
    $smarty->assign('email_address', $this->email_address);
    $this->step = 2;
    $params = $this->encodeParams(array('uid', 'directory', 'email_address'));
    $smarty->assign('params', $params);
  }

  /* generate a token and send it by email */
  function step3()
  {
    $smarty = get_smarty();
    /* Send a mail, save information in session and create a very random unique id */

    $activatecode = $this->generateRandomHash();

    $error = $this->storeToken($activatecode);

    if (!empty($error)) {
      msg_dialog::display(_("LDAP error"), $error, LDAP_ERROR);
      return;
    }

    $reinit_link = $this->getPageURL();
    $reinit_link .= "?uniq=".urlencode($activatecode);
    $reinit_link .= "&uid=".urlencode($this->uid);
    $reinit_link .= "&email_address=".urlencode($this->email_address);

    @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $reinit_link, "Setting link to");

    /* Send the mail */
    $mail_body = sprintf($this->mail_body, $this->uid, $reinit_link);

    if (mail_utf8($this->email_address, FALSE, $this->from_mail, $this->mail_subject, $mail_body)) {
      $this->step = 3;
    } else {
      $this->message[] = msgPool::invalid(_("Contact your administrator, there was a problem with mail server"));
    }
    $smarty->assign('uid', $this->uid);
  }

  /* check if the given token is the good one */
  function step4()
  {
    $uniq_id_from_mail = validate($_GET['uniq']);

    if (!$this->checkToken($uniq_id_from_mail)) {
      $this->message[] = _("This token is invalid");
      return;
    }

    $smarty = get_smarty();

    $smarty->assign('uniq', $uniq_id_from_mail);
    $this->uniq = $uniq_id_from_mail;
    $this->step = 4;
    $smarty->assign('uid', $this->uid);
    $params = $this->encodeParams(array('uid', 'directory', 'email_address', 'uniq'));
    $smarty->assign('params', $params);

    if (isset($_POST['change'])) {
      $this->step5();
    }
  }

  /* change the password and send confirmation email */
  function step5()
  {
    $dn = $this->getUserDn();
    if (!$dn) {
      return;
    }

    $userTabs = objects::open($dn, 'user');
    $userTab  = $userTabs->getBaseObject();
    $userTab->userPassword = array(
      '',
      $_POST['new_password'],
      $_POST['new_password_repeated'],
      $userTab->userPassword,
      $userTab->attributesAccess['userPassword']->isLocked()
    );

    /* Is there any problem with entered passwords? */
    $userTabs->save_object();
    $errors = $userTabs->save();
    if (!empty($errors)) {
      $this->message = $errors;
      return;
    }

    fusiondirectory_log("User ".$this->uid." password has been changed");
    /* Send the mail */
    $mail_body = sprintf($this->mail2_body, $this->uid);

    if (mail_utf8($this->email_address, FALSE, $this->from_mail, $this->mail2_subject, $mail_body)) {
      $smarty = get_smarty();
      $this->step = 5;
      $smarty->assign('changed', TRUE);
    } else {
      $this->message[] = msgPool::invalid(_("There was a problem with mail server, confirmation email not sent"));
    }
  }
}

?>
