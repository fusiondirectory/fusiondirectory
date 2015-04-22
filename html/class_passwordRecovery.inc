<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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

class passwordRecovery {
  var $config;
  var $uid;
  var $message = array();
  var $email_address;
  var $method;
  var $directory;

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

  var $activated;
  var $usealternates;

  /* Constructor */
  function __construct()
  {
    global $config, $ssl;
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

    $this->config = $config;

    $this->setupSmarty();

    $smarty = get_smarty();

    /* Generate server list */
    $servers = array();
    foreach ($this->config->data['LOCATIONS'] as $key => $ignored) {
      $servers[$key] = $key;
    }

    $smarty->assign("show_directory_chooser", FALSE);

    if (isset($_POST['server'])) {
      $this->directory = validate($_POST['server']);
    } elseif (isset($_GET['directory']) && isset($servers[$_GET['directory']])) {
      $this->directory = validate($_GET['directory']);
    } else {
      $this->directory = $this->config->data['MAIN']['DEFAULT'];

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
    $this->config->set_current($this->directory);
    session::global_set('config', $this->config);
    $config = $this->config;

    $this->activated = $this->readLdapConfig();
    if (!$this->activated) { /* Password recovery has been disabled */
      return;
    }

    $this->setupLanguage();

    $ssl = $this->checkForSSL();

    /* Check for selected password method */
    $this->method = $this->config->get_cfg_value("passwordDefaultHash", "ssha");
    if (isset($_GET['method'])) {
      $this->method = validate($_GET['method']);
      $tmp = new passwordMethod($this->config);
      $available = $tmp->get_available_methods();
      if (!isset($available[$this->method])) {
        msg_dialog::display(_("Password method"),
                            _("Error: Password method not available!"),
                            FATAL_ERROR_DIALOG);
        exit();
      }
    }

    if (isset($_GET['email_address']) && $_GET['email_address'] != "") {
      $this->email_address = validate($_GET['email_address']);
      $smarty->assign('email_address', $this->email_address);
    } elseif (isset($_POST['email_address'])) {
      $this->email_address = validate($_POST['email_address']);
      $smarty->assign('email_address', $this->email_address);
    }

    /* Check for selected user... */
    if (isset($_GET['uid']) && $_GET['uid'] != "") {
      $this->uid = validate($_GET['uid']);
    } elseif (isset($_POST['uid'])) {
      $this->uid = validate($_POST['uid']);
    } else {
      $this->uid = "";
    }
  }

  function execute()
  {
    if (!$this->activated) {
      return;
    }

    /* Got a formular answer, validate and try to log in */
    if ($_SERVER["REQUEST_METHOD"] == "POST") {
      /* Destroy old sessions, they cause a successfull login to relog again ... */
      if (session::global_is_set('_LAST_PAGE_REQUEST')) {
        session::global_set('_LAST_PAGE_REQUEST', time());
      }

      if (isset($_POST['send'])) {
        /* should not happen */
        $this->step3();
      } elseif (isset($_POST['change'])) {
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

    $smarty->assign("msg_dialogs", msg_dialog::get_dialogs());
    $smarty->assign("usePrototype", "FALSE");
    $smarty->display (get_template_path('headers.tpl'));

    $smarty->assign("version", FD_VERSION);
    $smarty->assign("step", $this->step);
    $smarty->assign("delay_allowed", $this->delay_allowed);

    $smarty->assign("activated", $this->activated);
    $smarty->append ('css_files',  get_template_path('login.css'));

    $smarty->display(get_template_path('recovery.tpl'));
    exit();
  }

  function loadConfig()
  {
    global $_SERVER, $BASE_DIR;

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

  /* Check that password recovery is activated, read config in ldap
   * Returns a boolean saying if password recovery is activated
   */
  function readLdapConfig()
  {
    $this->salt           = $this->config->get_cfg_value('passwordRecoverySalt');
    $this->delay_allowed  = $this->config->get_cfg_value('passwordRecoveryValidity');

    $this->mail_subject   = $this->config->get_cfg_value('passwordRecoveryMailSubject');
    $this->mail_body      = $this->config->get_cfg_value('passwordRecoveryMailBody');
    $this->mail2_subject  = $this->config->get_cfg_value('passwordRecoveryMail2Subject');
    $this->mail2_body     = $this->config->get_cfg_value('passwordRecoveryMail2Body');

    $this->from_mail      = $this->config->get_cfg_value('passwordRecoveryEmail');

    $this->usealternates  = $this->config->get_cfg_value('passwordRecoveryUseAlternate');

    @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->config->get_cfg_value('passwordRecoveryActivated'), "passwordRecoveryActivated");
    return ($this->config->get_cfg_value('passwordRecoveryActivated') == "TRUE");
  }

  function setupLanguage()
  {
    global $GLOBALS,$BASE_DIR;

    /* Language setup */
    $lang = get_browser_language();

    putenv("LANGUAGE=");
    putenv("LANG=$lang");
    setlocale(LC_ALL, $lang);
    $GLOBALS['t_language']            = $lang;
    $GLOBALS['t_gettext_message_dir'] = $BASE_DIR.'/locale/';

    @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $lang, "Setting language to");

    /* Set the text domain as 'messages' */
    $domain = 'messages';
    bindtextdomain($domain, LOCALE_DIR);
    textdomain($domain);
  }

  function setupSmarty()
  {
    $smarty = get_smarty();

    /* Set template compile directory */
    $smarty->compile_dir = $this->config->get_cfg_value("templateCompileDirectory", SPOOL_DIR);

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
    $smarty->assign('other_method', FALSE);
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

  function storeToken($temp_password)
  {
    /* Store it in ldap with the salt */
    $salt_temp_password = $this->salt.$temp_password.$this->salt;
    $sha1_temp_password = "{SHA}".base64_encode(pack("H*", sha1($salt_temp_password)));

    $ldap = $this->config->get_ldap_link();

    // Check if token branch is here
    $token = get_ou('tokenRDN').get_ou('fusiondirectoryRDN').$this->config->current['BASE'];
    $ldap->cat($token, array('dn'));
    if (!$ldap->count()) {
      /* It's not, let's create it */
      $ldap->cd ($this->config->current['BASE']);
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
    $salt_token = $this->salt.$token.$this->salt;
    $sha1_token = "{SHA}".base64_encode(pack("H*", sha1($salt_token)));

    /* Retrieve hash from the ldap */
    $ldap = $this->config->get_ldap_link();

    $token  = get_ou('tokenRDN').get_ou('fusiondirectoryRDN').$this->config->current['BASE'];
    $dn     = "ou=".$this->uid.",$token";
    $ldap->cat($dn);
    $attrs = $ldap->fetch();

    $ldap_token         = $attrs['userPassword'][0];
    $last_time_recovery = $attrs['description'][0];

    /* Return TRUE if the token match and is still valid */
    return ($last_time_recovery >= time()) &&
           ($ldap_token == $sha1_token);
  }

  function isValidPassword($new_password, $repeated_password)
  {
    $MinLength = $this->config->get_cfg_value("passwordMinLength", 0);

    if ($new_password != $repeated_password) {
      return _("The passwords you've entered as 'New password' and 'Repeated new password' do not match.");
    } elseif ($new_password == "") {
      return msgPool::required(_("New password"));
    } elseif (strlen($new_password) < $MinLength) {
      return _("The password used as new is too short.");
    } elseif (!passwordMethod::is_harmless($new_password)) {
      return _("The password contains possibly problematic Unicode characters!");
    }
  }

  function checkForSSL()
  {
    $smarty = get_smarty();

    /* Check for SSL connection */
    $ssl = "";
    $smarty->assign("ssl", "");
    if (!isset($_SERVER['HTTPS']) || !stristr($_SERVER['HTTPS'], "on")) {
      if (empty($_SERVER['REQUEST_URI'])) {
        $ssl = "https://".$_SERVER['HTTP_HOST'].$_SERVER['PATH_INFO'];
      } else {
        $ssl = "https://".$_SERVER['HTTP_HOST'].$_SERVER['REQUEST_URI'];
      }

      /* If SSL is forced, just forward to the SSL enabled site */
      if ($this->config->get_cfg_value("forcessl") == 'TRUE') {
        header("Location: $ssl");
        exit;
      } elseif ($this->config->get_cfg_value("warnssl") == 'TRUE') {
        /* Display SSL mode warning? */
        $smarty->assign("ssl", "<b>"._("Warning").":</b> ".
                        _("Session will not be encrypted.").
                        " <a style=\"color:red;\" href=\"".htmlentities($ssl).
                        "\"><b>"._("Enter SSL session")."</b></a>!");
      }
    }

    return $ssl;
  }

  function getPageURL()
  {
    $pageURL = "http";
    if (isset($_SERVER['HTTPS']) && ($_SERVER["HTTPS"] == "on")) {
      $pageURL .= "s";
    }
    $pageURL .= "://".$_SERVER["SERVER_NAME"];
    if ($_SERVER["SERVER_PORT"] != "80") {
      $pageURL .= ":".$_SERVER["SERVER_PORT"];
    }
    $pageURL .= $_SERVER["PHP_SELF"];

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

  function getUserDn()
  {
    /* Retrieve dn from the ldap */
    $ldap = $this->config->get_ldap_link();

    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=gosaMailAccount)(uid=".$this->uid."))", array("dn"));

    if ($ldap->count() < 1) {
      $this->message[] = sprintf(_('Did not found an account with login "%s"'), $this->uid);
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
    /* Ask for the method */
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
    if (class_available('personalInfo') && ($this->config->get_cfg_value('privateEmailPasswordRecovery', 'FALSE') == 'TRUE')) {
      $filter = '(|'.$filter.'(&(objectClass=fdPersonalInfo)(fdPrivateMail='.$this->email_address.')))';
    }
    $uids = get_list( $filter, "",
                      $this->config->current['BASE'], array("uid"),
                      GL_SUBSEARCH | GL_NO_ACL_CHECK);

    /* Only one uid should be found */
    if (count($uids) < 1) {
      $this->message[] = sprintf(_("There is no account using email %s"), $this->email_address);
      return;
    } elseif (count($uids) > 1) {
      $this->message[] = sprintf(_("There are several accounts using email %s"), $this->email_address);
      return;
    }

    $smarty = get_smarty();

    $this->uid = $uids[0]['uid'][0];
    $smarty->assign('uid', $this->uid);
    $smarty->assign('email_address', $this->email_address);
    $this->step = 2;
    $params = $this->encodeParams(array('uid', 'method', 'directory', 'email_address'));
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

    /* From */
    $headers = "From: ".$this->from_mail."\r\n";
    $headers .= "Reply-To: ".$this->from_mail."\r\n";

    $additional_parameters = "-f".$this->from_mail;

    if (mail($this->email_address, $this->mail_subject, $mail_body, $headers, $additional_parameters)) {
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
    $params = $this->encodeParams(array('uid', 'method', 'directory', 'email_address', 'uniq'));
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
    /* Do new and repeated password fields match? */
    $error = $this->isValidPassword( $_POST['new_password'],
                                     $_POST['new_password_repeated']);
    if (!empty($error)) {
      $this->message[] = $error;
      return;
    }

    change_password($dn, $_POST['new_password'], $this->method);
    fusiondirectory_log("User ".$this->uid." password has been changed");
    /* Send the mail */
    $mail_body = sprintf($this->mail2_body, $this->uid);

    /* From */
    $headers = "From: ".$this->from_mail."\r\n";
    $headers .= "Reply-To: ".$this->from_mail."\r\n";

    $additional_parameters = "-f".$this->from_mail;

    if (mail($this->email_address, $this->mail2_subject, $mail_body, $headers, $additional_parameters)) {
      $smarty = get_smarty();
      $this->step = 5;
      $smarty->assign('changed', TRUE);
    } else {
      $this->message[] = msgPool::invalid(_("There was a problem with mail server, confirmation email not sent"));
    }
  }
}

?>
