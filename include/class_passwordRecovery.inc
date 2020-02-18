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

class passwordRecovery extends standAlonePage
{
  protected $loginAttribute;
  protected $login;
  protected $email_address;

  protected $message;
  protected $step;

  /* Salt needed to mask the uniq id in the ldap */
  protected $salt;

  /* Delay allowed for the user to change his password (minutes) */
  protected $delay_allowed;

  /* Sender */
  protected $from_mail;

  protected $mail_body;
  protected $mail_subject;

  protected $mail2_body;
  protected $mail2_subject;

  protected $usealternates;

  function init ()
  {
    parent::init();

    $this->step     = 1;
    $this->message  = [];

    if (isset($_GET['email_address']) && ($_GET['email_address'] != '')) {
      $this->email_address = validate($_GET['email_address']);
    } elseif (isset($_POST['email_address'])) {
      $this->email_address = validate($_POST['email_address']);
    }

    /* Check for selected user... */
    if (isset($_GET['login']) && $_GET['login'] != '') {
      $this->login = validate($_GET['login']);
    } elseif (isset($_POST['login'])) {
      $this->login = validate($_POST['login']);
    } else {
      $this->login = '';
    }
  }

  function save_object ()
  {
    if (!$this->activated) {
      return;
    }

    /* Got a formular answer, validate and try to log in */
    if ($_SERVER['REQUEST_METHOD'] == 'POST') {
      if (session::is_set('_LAST_PAGE_REQUEST')) {
        session::set('_LAST_PAGE_REQUEST', time());
      }

      if (isset($_POST['change'])) {
        $this->step4();
      } elseif (isset($_POST['apply'])) {
        if ($_POST['email_address'] == '') {
          $this->message[] = msgPool::required(_('Email address'));
          return;
        }
        $this->email_address = $_POST['email_address'];
        $this->step2();
        if ($this->step == 2) { /* No errors */
          $this->step3();
        }
      }
    } elseif ($_SERVER['REQUEST_METHOD'] == 'GET') {
      if (isset($_GET['uniq'])) {
        $this->step4();
      }
    }
  }

  function execute ()
  {
    $this->save_object();

    /* Do we need to show error messages? */
    if (count($this->message) != 0) {
      /* Show error message and continue editing */
      msg_dialog::displayChecks($this->message);
    }

    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->step, "Step");

    $smarty = get_smarty();

    $this->assignSmartyVars();

    $smarty->append('js_files',     'include/pwdStrength.js');
    $smarty->append('css_files',    get_template_path('login.css'));
    $smarty->assign('title',        _('Password recovery'));
    $smarty->display(get_template_path('headers.tpl'));

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
  function readLdapConfig ()
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

    $this->loginAttribute = $config->get_cfg_value('passwordRecoveryLoginAttribute', 'uid');

    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $config->get_cfg_value('passwordRecoveryActivated'), "passwordRecoveryActivated");
    return ($config->get_cfg_value('passwordRecoveryActivated') == "TRUE");
  }

  function storeToken ($temp_password)
  {
    global $config;
    /* Store it in ldap with the salt */
    $salt_temp_password = $this->salt.$temp_password.$this->salt;
    $sha1_temp_password = "{SHA}".base64_encode(pack("H*", sha1($salt_temp_password)));

    $ldap = $config->get_ldap_link();

    // Check if token branch is here
    $token = get_ou('recoveryTokenRDN').get_ou('fusiondirectoryRDN').$config->current['BASE'];
    $ldap->cat($token, ['dn']);
    if (!$ldap->count()) {
      /* It's not, let's create it */
      $ldap->cd($config->current['BASE']);
      $ldap->create_missing_trees($token);
      if (!$ldap->success()) {
        return msgPool::ldaperror($ldap->get_error(),
                                  $token, LDAP_MOD, get_class());
      }
      fusiondirectory_log("Created token branch ".$token);
    }

    $dn = 'ou='.$this->login.','.$token;
    $ldap->cat($dn, ['dn']);
    $add = ($ldap->count() == 0);
    /* We store the token and its validity due date */
    $attrs = [
      'objectClass'   => ['organizationalUnit'],
      'ou'            => $this->login,
      'userPassword'  => $sha1_temp_password,
      'description'   => time() + $this->delay_allowed * 60,
    ];
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

    /* Everything went well */
    return '';
  }

  function checkToken ($token)
  {
    global $config;
    $salt_token = $this->salt.$token.$this->salt;
    $sha1_token = "{SHA}".base64_encode(pack("H*", sha1($salt_token)));

    /* Retrieve hash from the ldap */
    $ldap = $config->get_ldap_link();

    $token  = get_ou('recoveryTokenRDN').get_ou('fusiondirectoryRDN').$config->current['BASE'];
    $dn     = 'ou='.$this->login.','.$token;
    $ldap->cat($dn);
    $attrs = $ldap->fetch();

    $ldap_token         = $attrs['userPassword'][0];
    $last_time_recovery = $attrs['description'][0];

    /* Return TRUE if the token match and is still valid */
    return ($last_time_recovery >= time()) &&
           ($ldap_token == $sha1_token);
  }

  function getUserDn ()
  {
    global $config;
    /* Retrieve dn from the ldap */
    $ldap = $config->get_ldap_link();

    $objectClasses = ['gosaMailAccount'];
    if (class_available('personalInfo') && ($config->get_cfg_value('privateEmailPasswordRecovery', 'FALSE') == 'TRUE')) {
      $objectClasses[] = 'fdPersonalInfo';
    }
    if (class_available('supannAccount') && ($config->get_cfg_value('supannPasswordRecovery', 'TRUE') == 'TRUE')) {
      $objectClasses[] = 'supannPerson';
    }
    $filter = '(&(|(objectClass='.join(')(objectClass=', $objectClasses).'))('.$this->loginAttribute.'='.ldap_escape_f($this->login).'))';
    $ldap->cd($config->current['BASE']);
    $ldap->search($filter, ['dn']);

    if ($ldap->count() < 1) {
      $this->message[] = sprintf(_('Did not find an account with login "%s"'), htmlentities($this->login, ENT_COMPAT, 'UTF-8'));
      return;
    } elseif ($ldap->count() > 1) {
      $this->message[] = sprintf(_('Found multiple accounts with login "%s"'), htmlentities($this->login, ENT_COMPAT, 'UTF-8'));
      return;
    }

    $attrs = $ldap->fetch();

    return $attrs['dn'];
  }

  /* Find the login of for the given email address */
  function step2 ($email = NULL)
  {
    global $config;

    if ($email !== NULL) {
      /* Special case when recovery is called from webservice */
      $this->email_address = $email;
    }

    /* Search login corresponding to the mail */
    $address_escaped = ldap_escape_f($this->email_address);
    if ($this->usealternates) {
      $filter = '(&(objectClass=gosaMailAccount)(|(mail='.$address_escaped.')(gosaMailAlternateAddress='.$address_escaped.')))';
    } else {
      $filter = '(&(objectClass=gosaMailAccount)(mail='.$address_escaped.'))';
    }
    if (class_available('personalInfo') && ($config->get_cfg_value('privateEmailPasswordRecovery', 'FALSE') == 'TRUE')) {
      $filter = '(|'.$filter.'(&(objectClass=fdPersonalInfo)(fdPrivateMail='.$address_escaped.')))';
    }
    if (class_available('supannAccount') && ($config->get_cfg_value('supannPasswordRecovery', 'TRUE') == 'TRUE')) {
      $filter = '(|'.$filter.'(&(objectClass=supannPerson)(supannMailPerso='.$address_escaped.')))';
    }
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search($filter, ['dn', 'userPassword', $this->loginAttribute]);

    /* Only one ldap node should be found */
    if ($ldap->count() < 1) {
      $this->message[] = sprintf(_('There is no account using email "%s"'), htmlentities($this->email_address, ENT_COMPAT, 'UTF-8'));
      return FALSE;
    } elseif ($ldap->count() > 1) {
      $this->message[] = sprintf(_('There are several accounts using email "%s"'), htmlentities($this->email_address, ENT_COMPAT, 'UTF-8'));
      return FALSE;
    }

    $attrs = $ldap->fetch();

    $method = passwordMethod::get_method($attrs['userPassword'][0], $attrs['dn']);
    if (is_object($method) && $method->is_locked($attrs['dn'])) {
      $this->message[] = sprintf(_('The user using email "%s" is locked. Please contact your administrator.'), htmlentities($this->email_address, ENT_COMPAT, 'UTF-8'));
      return FALSE;
    }
    $this->login  = $attrs[$this->loginAttribute][0];
    $this->step   = 2;

    if ($this->interactive) {
      $smarty = get_smarty();

      $smarty->assign('login', $this->login);
      $smarty->assign('email_address', $this->email_address);
      $params = $this->encodeParams(['login', 'directory', 'email_address']);
      $smarty->assign('params', $params);
    }

    return $attrs['dn'];
  }

  function generateAndStoreToken ()
  {
    $activatecode = static::generateRandomHash();

    $error = $this->storeToken($activatecode);

    if (!empty($error)) {
      $this->message[] = $error;
      return FALSE;
    }

    return $activatecode;
  }

  /* generate a token and send it by email */
  function step3 ()
  {
    /* Send a mail, save information in session and create a very random unique id */
    $token = $this->generateAndStoreToken();

    if ($token === FALSE) {
      return;
    }

    $reinit_link = URL::getPageURL();
    $reinit_link .= '?uniq='.urlencode($token);
    $reinit_link .= '&login='.urlencode($this->login);
    $reinit_link .= '&email_address='.urlencode($this->email_address);

    logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $reinit_link, 'Setting link to');

    /* Send the mail */
    $body = sprintf($this->mail_body, $this->login, $reinit_link);

    if (mail_utf8($this->email_address, FALSE, $this->from_mail, $this->mail_subject, $body)) {
      $this->step = 3;
    } else {
      $this->message[] = _('Contact your administrator, there was a problem with mail server');
    }
    $smarty = get_smarty();

    $smarty->assign('login', $this->login);
  }

  /* check if the given token is the good one */
  function step4 ()
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
    $smarty->assign('login', $this->login);
    $params = $this->encodeParams(['login', 'directory', 'email_address', 'uniq']);
    $smarty->assign('params', $params);

    if (isset($_POST['change'])) {
      $this->step5();
    }
  }

  function changeUserPassword ($new_password, $new_password_repeated)
  {
    $dn = $this->getUserDn();
    if (!$dn) {
      return FALSE;
    }

    $userTabs = objects::open($dn, 'user');
    $userTab  = $userTabs->getBaseObject();
    $userTab->userPassword = [
      '',
      $new_password,
      $new_password_repeated,
      $userTab->userPassword,
      $userTab->attributesAccess['userPassword']->isLocked()
    ];

    /* Is there any problem with entered passwords? */
    $userTabs->save_object();
    $errors = $userTabs->save();
    if (!empty($errors)) {
      $this->message = $errors;
      return;
    }

    fusiondirectory_log('User '.$this->login.' password has been changed');

    return TRUE;
  }

  /* change the password and send confirmation email */
  function step5 ()
  {
    $success = $this->changeUserPassword($_POST['new_password'], $_POST['new_password_repeated']);
    if (!$success) {
      return;
    }

    /* Send the mail */
    $body = sprintf($this->mail2_body, $this->login);

    if (mail_utf8($this->email_address, FALSE, $this->from_mail, $this->mail2_subject, $body)) {
      $smarty = get_smarty();
      $this->step = 5;
      $smarty->assign('changed', TRUE);
    } else {
      $this->message[] = _('There was a problem with mail server, confirmation email not sent');
    }
  }

  function getErrorMessages (): array
  {
    return $this->message;
  }

  function setLogin (string $login)
  {
    $this->login = $login;
  }

  function getLogin ()
  {
    return $this->login;
  }
}
