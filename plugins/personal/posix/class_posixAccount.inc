<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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
  \brief   posixAccount plugin
  \author  Cajus Pollmeier <pollmeier@gonicus.de>
  \version 2.00
  \date    24.07.2003

  This class provides the functionality to read and write all attributes
  relevant for posixAccounts and shadowAccounts from/to the LDAP. It
  does syntax checking and displays the formulars required.
 */

class PosixStatusAttribute extends Attribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "")
  {
    parent::__construct ($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->setInLdap(FALSE);
  }

  function renderFormInput ()
  {
    return $this->getValue();
  }
}

class EpochDaysDateAttribute extends DateAttribute
{
  public static $secondsPerDay = 86400; //60 * 60 * 24

  function __construct ($label, $description, $ldapName, $required, $defaultValue = 'now', $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, '', $defaultValue, $acl);
  }

  protected function ldapToDate($ldapValue)
  {
    $date = DateTime::createFromFormat('U', $ldapValue * self::$secondsPerDay, new DateTimeZone('UTC'));
    if ($date !== FALSE) {
      return $date;
    } else {
      trigger_error('LDAP value for '.$this->getLdapName().' was not in the right date format.');
      return new DateTime($ldapValue, new DateTimeZone('UTC'));
    }
  }

  protected function dateToLdap($dateValue)
  {
    return floor($dateValue->format('U') / self::$secondsPerDay);
  }

  function getEpochDays()
  {
    return $this->dateToLdap($this->getDateValue());
  }
}

class posixAccount extends simplePlugin
{
  var $displayHeader = TRUE;
  var $objectclasses = array("posixAccount", "shadowAccount");

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Generic'),
        'icon'  => 'images/rightarrow.png',
        'attrs' => array(
          new PathAttribute(
            _('Home directory'), _('The path to the home directory of this user'),
            'homeDirectory', TRUE
          ),
          new StringAttribute('gecos', 'gecos', 'gecos'),
          new SelectAttribute(
            _('Shell'), _('Which shell should be used when this user log in'),
            'loginShell', TRUE
          ),
          new SelectAttribute(
            _('Primary group'), _('Primary group for this user'),
            'primaryGroup', FALSE
          ),
          new PosixStatusAttribute(
            _('Status'), _('Status of this user unix account'),
            'posixStatus', FALSE
          ),
          new BooleanAttribute(
            _('Force UID/GID'), _('Force UID and GID values for this user'),
            'force_ids', FALSE
          ),
          new IntAttribute(
            _('UID'), _('UID value for this user'),
            'uidNumber', FALSE,
            0, FALSE, ''
          ),
          new IntAttribute(
            _('GID'), _('GID value for this user'),
            'gidNumber', FALSE,
            0, FALSE, ''
          )
        )
      ),
      'groups' => array(
        'name'  => _('Group membership'),
        'icon'  => 'plugins/posix/images/members.png',
        'attrs' => array(
          new GroupsAttribute('', _('Group membership'), 'groupMembership')
        )
      ),
      'account' => array(
        'name'  => _('Account'),
        'icon'  => 'plugins/posix/images/terminal_small.png',
        'attrs' => array(
          new BooleanAttribute(
            _('User must change password on first login'), _('User must change password on first login (leave empty to disable)'),
            'mustchangepassword', FALSE
          ),
          new IntAttribute(
            _('Delay before locking password (days)'), _('The user won\'t be able to change his password after this number of days (leave empty to disable)'),
            'shadowMin', FALSE,
            0, FALSE, ''
          ),
          new IntAttribute(
            _('Delay before forcing password change (days)'), _('The user will be forced to change his password after this number of days (leave empty to disable)'),
            'shadowMax', FALSE,
            0, FALSE, ''
          ),
          new EpochDaysDateAttribute(
            _('Password expiration date'), _('Date after which this user password will expire (leave empty to disable)'),
            'shadowExpire', FALSE,
            ''
          ),
          new IntAttribute(
            _('Delay of inactivity before disabling user (days)'), _('Maximum delay of inactivity after password expiration before the user is disabled (leave empty to disable)'),
            'shadowInactive', FALSE,
            0, FALSE, ''
          ),
          new IntAttribute(
            _('Delay for user warning before password expiry (days)'), _('The user will be warned this number of days before his password expiration (leave empty to disable)'),
            'shadowWarning', FALSE,
            0, FALSE, ''
          ),
          new IntAttribute(
            'No label', 'No description',
            'shadowLastChange', FALSE,
            0, FALSE, ''
          ),
        )
      ),
      'system_trust' => array(
        'name'  => _('System trust'),
        'icon'  => 'images/lists/locked.png',
        'attrs' => array(
          new SelectAttribute(
            _('Trust mode'), _(''),
            'trustMode', FALSE,
            array('', 'fullaccess', 'byhost'),
            '',
            array(_('disabled'), _('full access'), _('allow access to these hosts'))
          ),
          new TrustsAttribute(
            '', _('Only allow this user to connect to this list of hosts'),
            'host', FALSE
          )
        )
      )
    );
  }

  static function plInfo()
  {
    return array(
      "plShortName"     => _("Unix"),
      "plDescription"   => _("Edit users POSIX settings"),
      "plSelfModify"    => TRUE,
      "plDepends"       => array("user"),
      "plPriority"      => 2,
      "plCategory"      => array("users"),
      "plObjectType"    => array("user"),

      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesAccess['gecos']->setVisible(FALSE);

    $this->attributesAccess['trustMode']->setInLdap(FALSE);
    $this->attributesAccess['trustMode']->setManagedAttributes(
      array(
        'multiplevalues' => array('notbyhost' => array('','fullaccess')),
        'erase' => array(
          'notbyhost' => array('host')
        )
      )
    );
    if ((count($this->host) == 1) && ($this->host[0] == '*')) {
      $this->trustMode = 'fullaccess';
    } elseif (count($this->host) > 0) {
      $this->trustMode = 'byhost';
    }


    $this->attributesAccess['uidNumber']->setUnique(TRUE);
    $this->attributesAccess['force_ids']->setInLdap(FALSE);
    $this->attributesAccess['force_ids']->setManagedAttributes(
      array(
        'disable' => array (
          FALSE => array (
            'uidNumber',
            'gidNumber',
          )
        )
      )
    );
    $this->attributesAccess['primaryGroup']->setInLdap(FALSE);

    $this->attributesAccess['mustchangepassword']->setInLdap(FALSE);
    $this->attributesAccess['shadowLastChange']->setVisible(FALSE);

    /* Setting uid to default */
    if (isset($this->attrs['uid'][0])) {
      $this->uid = $this->attrs['uid'][0];
    }

    if ($dn !== NULL) {

      /* Correct is_account. shadowAccount is not required. */
      if (isset($this->attrs['objectClass']) &&
          in_array ('posixAccount', $this->attrs['objectClass'])) {
        $this->is_account = TRUE;
      }

      $this->initially_was_account = $this->is_account;

      // Templates do not have a gidNumber
      if ($this->gidNumber == 2147483647) {
        $this->gidNumber = "";
      }

      /* Fill group */
      $this->primaryGroup = $this->gidNumber;
    }

    /* Generate shell list from SYSTEMS_DIR./shells */
    $loginShellList = array();
    if (file_exists(SYSTEMS_DIR.'/shells')) {
      $shells = file (SYSTEMS_DIR.'/shells');
      foreach ($shells as $line) {
        if (!preg_match ("/^#/", $line)) {
          $loginShellList[] = trim($line);
        }
      }
    } else {
      if ($this->loginShell == "") {
        $loginShellList[] = _("unconfigured");
      }
    }

    /* Insert possibly missing loginShell */
    $loginShell = $this->attributesAccess['loginShell']->getValue();
    if ($loginShell != "" && !in_array($loginShell, $loginShellList)) {
      $loginShellList[]= $loginShell;
    }
    $this->attributesAccess['loginShell']->setChoices($loginShellList);

    $this->ui = get_userinfo();

    $secondaryGroups = array();
    $secondaryGroups[0] = "- "._("automatic")." -";
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=posixGroup)", array("cn", "gidNumber"));
    while ($attrs = $ldap->fetch()) {
      $secondaryGroups[$attrs['gidNumber'][0]] = $attrs['cn'][0];
    }
    asort ($secondaryGroups);
    $this->attributesAccess['primaryGroup']->setChoices(array_keys($secondaryGroups), array_values($secondaryGroups));

    $current = floor(date("U") / EpochDaysDateAttribute::$secondsPerDay);

    $shadowExpire     = $this->attributesAccess['shadowExpire']->getEpochDays();
    $shadowInactive   = $this->attributesAccess['shadowInactive']->getValue();
    $shadowMin        = $this->attributesAccess['shadowMin']->getValue();
    $shadowMax        = $this->attributesAccess['shadowMax']->getValue();
    $shadowLastChange = $this->attributesAccess['shadowLastChange']->getValue();
    if (($current >= $shadowExpire) && ($shadowExpire > 0)) {
      $status = _("expired");
      if ($shadowInactive != "" && ($current - $shadowExpire) < $shadowInactive) {
        $status .= ", "._("grace time active");
      }
    } elseif ($shadowMax != "" && ($shadowLastChange + $shadowMax) <= $current) {
      $status = _("active").", "._("password expired");
    } elseif ($shadowMin != "" && ($shadowLastChange + $shadowMin) <= $current) {
      $status = _("active").", "._("password not changeable");
    } else {
      $status = _("active");
    }
    $this->attributesAccess['posixStatus']->setValue($status);

    /* Groups handling */
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=posixGroup)(memberUid=".$this->uid."))", array("cn", "description"));
    $groupMembership = array();
    while ($attrs = $ldap->fetch()) {
      if (!isset($attrs["description"][0])) {
        $entry = $attrs["cn"][0];
      } else {
        $entry = $attrs["cn"][0]." [".$attrs["description"][0]."]";
      }
      $groupMembership[$ldap->getDN()]= $entry;
    }
    asort($groupMembership);
    reset($groupMembership);
    $this->attributesAccess['groupMembership']->setInLdap(FALSE);
    $this->attributesAccess['groupMembership']->setValue(array_keys($groupMembership));
    $this->attributesAccess['groupMembership']->setDisplayValues(array_values($groupMembership));
    $this->savedGroupMembership = array_keys($groupMembership);
  }

  function resetCopyInfos()
  {
    parent::resetCopyInfos();

    $this->savedGroupMembership = array();

    $this->force_ids = FALSE;
    $this->attributesAccess['uidNumber']->setInitialValue("");
    $this->attributesAccess['gidNumber']->setInitialValue("");
    $this->uidNumber = "";
    $this->gidNumber = "";
  }

  function check()
  {
    $message = parent::check();

    /* Check ID's if they are forced by user */
    if ($this->force_ids) {
      if ($this->uidNumber < $this->config->get_cfg_value("minId")) {
        $message[] = msgPool::toosmall(_("UID"), $this->config->get_cfg_value("minId"));
      }
      if ($this->gidNumber < $this->config->get_cfg_value("minId")) {
        $message[] = msgPool::toosmall(_("GID"), $this->config->get_cfg_value("minId"));
      }
    }

    /* Check shadow settings */
    if ($this->shadowWarning !== "") {
      if ($this->shadowMax === "") {
        $message[] = msgPool::depends("shadowWarning", "shadowMax");
      }
      if ($this->shadowWarning > $this->shadowMax) {
        $message[] = msgPool::toobig("shadowWarning", "shadowMax");
      }
      if (($this->shadowMin !== "") && ($this->shadowWarning < $this->shadowMin)) {
        $message[] = msgPool::toosmall("shadowWarning", "shadowMin");
      }
    }

    if (($this->shadowInactive !== "") && ($this->shadowMax === "")) {
      $message[]= msgPool::depends("shadowInactive", "shadowMax");
    }
    if (($this->shadowMin !== "") && ($this->shadowMax !== "") &&
        ($this->shadowMin > $this->shadowMax)) {
      $message[] = msgPool::toobig("shadowMin", "shadowMax");
    }
  }

  function prepare_save()
  {
    /* Fill gecos */
    if (isset($this->parent) && $this->parent !== NULL) {
      $this->gecos = rewrite($this->parent->getBaseObject()->cn);
      if (!preg_match('/^[a-z0-9 -]+$/i', $this->gecos)) {
        $this->gecos = "";
      }
    }

    if (!$this->force_ids) {
      /* Handle uidNumber.
       * - use existing number if possible
       * - if not, try to create a new uniqe one.
       * */
      if ($this->attributesAccess['uidNumber']->getInitialValue() != "") {
        $this->uidNumber = $this->attributesAccess['uidNumber']->getInitialValue();
      } else {
        /* Calculate new id's. We need to place a lock before calling get_next_id
           to get real unique values.
         */
        $wait = 10;
        while (get_lock("uidnumber") != "") {
          sleep (1);

          /* Oups - timed out */
          if ($wait-- == 0) {
            msg_dialog::display(_("Warning"), _("Timeout while waiting for lock. Ignoring lock!"), WARNING_DIALOG);
            break;
          }
        }
        add_lock ("uidnumber", "gosa");
        $this->uidNumber = get_next_id("uidNumber", $this->dn);
      }
    }

    /* Handle gidNumber
     * - If we do not have a primary group selected (automatic), we will check if there
     *    is already a group  with the same name and use this as primary.
     * - .. if we couldn't find a group with the same name, we will create a new one,
     *    using the users uid as cn and a generated uniqe gidNumber.
     * */
    if ($this->is_template && ($this->primaryGroup == 0)) {
      $this->uidNumber = 2147483647;
    } elseif (($this->primaryGroup == 0) || $this->force_ids) {

      /* Search for existing group */
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);

      /* Are we forced to use a special gidNumber? */
      if($this->force_ids) {
        $ldap->search("(&(objectClass=posixGroup)(gidNumber=".$this->gidNumber."))", array("cn","gidNumber"));
      } else {
        $ldap->search("(&(objectClass=posixGroup)(gidNumber=*)(cn=".$this->uid."))", array("cn","gidNumber"));
      }

      /* No primary group found, create a new one */
      if ($ldap->count() == 0) {
        $groupcn  = $this->uid;
        $pri_attr = $this->config->get_cfg_value("accountPrimaryAttribute");
        $groupdn  = preg_replace ('/^'.preg_quote($pri_attr,'/').'=[^,]+,'.preg_quote(get_people_ou(),'/').'/i',
            'cn='.$groupcn.','.get_groups_ou(), $this->dn);

        /* Request a new and uniqe gidNumber, if required */
        if (!$this->force_ids) {
          $this->gidNumber = get_next_id("gidNumber", $this->dn);
        }

        /* If forced gidNumber could not be found, then check if the given group name already exists
           we do not want to modify the gidNumber of an existing group.
         */
        $cnt = 0;
        while ($ldap->dn_exists($groupdn) && ($cnt < 100)) {
          $cnt++;
          $groupcn = $this->uid."_".$cnt;
          $groupdn = preg_replace ('/^'.preg_quote($pri_attr, '/').'=[^,]+,'.preg_quote(get_people_ou(),'/').'/i',
              'cn='.$groupcn.','.get_groups_ou(), $this->dn);
        }

        /* Create new primary group and enforce the new gidNumber */
        $g = new group($this->config, $groupdn);
        $g->cn = $groupcn;
        $g->force_gid = 1;
        $g->gidNumber = $this->gidNumber;
        $g->description = _("Group of user")." ".$this->givenName;
        $g->save();

        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
          sprintf("Primary group '%s' created, using gidNumber '%s'.", $groupcn,
          $this->gidNumber),"");
      } else {
        $attrs = $ldap->fetch();
        $this->gidNumber = $attrs['gidNumber'][0];
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
          "Found and used: <i>".$attrs['dn']."</i>",
          sprintf("Primary group '%s' exists, gidNumber is '%s'.", $this->uid, $this->gidNumber));
      }
    } else {
      /* Primary group was selected by user */
      $this->gidNumber = $this->primaryGroup;
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
        sprintf("Primary group '%s' for user '%s' manually selected.",
        $this->gidNumber, $this->uid), "");
    }

    if ($this->mustchangepassword) {
      $this->shadowLastChange =
        floor(date("U") / EpochDaysDateAttribute::$secondsPerDay) - $this->shadowMax - 1;
    } elseif ($this->is_account && !$this->initially_was_account) {
      $this->shadowLastChange = floor(date("U") / EpochDaysDateAttribute::$secondsPerDay);
    }

    $this->updateAttributesValues();
    parent::prepare_save();

    if ($this->trustMode == 'fullaccess') {
      $this->attrs['host'] = array('*');
    }

    /* Trust accounts */
    if (($this->trustMode != "") && !in_array('hostObject',$this->attrs['objectClass'])) {
      $this->attrs['objectClass'][] = 'hostObject';
    } elseif (($this->trustMode == "") && (($key = array_search('hostObject',$this->attrs['objectClass'])) !== FALSE)) {
      unset($this->attrs['objectClass'][$key]);
    }
  }

  function save()
  {
    parent::save();
    del_lock("uidnumber");

    /* Take care about groupMembership values: add to groups */
    $groupMembership = $this->attributesInfo['groups']['attrs']['groupMembership']->getValue();
    foreach ($groupMembership as $value) {
      if (!in_array($value, $this->savedGroupMembership)) {
        $g = new grouptabs($this->config, $this->config->data['TABS']['GROUPTABS'], $value, "groups");
        $g->set_acl_base($value);
        $g->by_object['group']->addUser($this->uid);
        $g->save();
      }
    }

    /* Remove groups not listed in groupMembership */
    foreach ($this->savedGroupMembership as $value) {
      if (!in_array($value, $groupMembership)) {
        $g = new grouptabs($this->config, $this->config->data['TABS']['GROUPTABS'], $value, "groups");
        $g->set_acl_base($value);
        $g->by_object['group']->removeUser($this->uid);
        $g->save();
      }
    }
  }

  /* remove object from parent */
  function remove_from_parent()
  {
    /* Cancel if there's nothing to do here */
    if ((!$this->initially_was_account) || (!$this->acl_is_removeable())) {
      return;
    }

    /* Remove and write to LDAP */
    parent::remove_from_parent();

    /* Delete group only if cn is uid and there are no other members inside */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd ($this->config->current['BASE']);
    $ldap->search ("(&(objectClass=posixGroup)(gidNumber=".$this->gidNumber."))", array("cn", "memberUid"));
    if ($ldap->count() != 0) {
      $attrs = $ldap->fetch();
      if ($attrs['cn'][0] == $this->uid && !isset($this->attrs['memberUid'])) {
        $ldap->rmDir($ldap->getDN());
      }
    }
  }

}

class old_posixAccount extends plugin
{
  /* Plugin specific values */
  var $homeDirectory      = "";
  var $loginShell         = "/bin/bash";
  var $uidNumber          = "";
  var $gidNumber          = "";
  var $gecos              = "";
  var $shadowMin          = "0";
  var $shadowMax          = "0";
  var $shadowWarning      = "0";
  var $shadowLastChange   = "0";
  var $shadowInactive     = "0";
  var $shadowExpire       = "";
  var $gosaDefaultPrinter = "";
  var $accessTo           = array();
  var $trustModel         = "";
  var $host               = array();

  var $glist                    = array();
  var $status                   = "";
  var $loginShellList           = array();
  var $groupMembership          = array();
  var $savedGroupMembership     = array();
  var $savedUidNumber           = "";
  var $savedGidNumber           = "";
  var $activate_shadowMin       = "0";
  var $activate_shadowMax       = "0";
  var $activate_shadowWarning   = "0";
  var $activate_shadowInactive  = "0";
  var $activate_shadowExpire    = "0";
  var $mustchangepassword       = "0";
  var $force_ids                = 0;
  var $gotoLastSystemLogin      = "";
  var $groupSelect              = FALSE;
  var $trustSelect              = FALSE;
  var $secondaryGroups          = array();
  var $primaryGroup             = 0;
  var $was_trust_account        = FALSE;
  var $memberGroup              = array();
  var $grouplist                = array();
  var $ui                       = array();
  var $ssh                      = null;
  var $sshAcl                   = "";

  var $GroupRegex               = "*";
  var $GroupUserRegex           = "*";
  var $SubSearch                = false;

  var $view_logged              = false;

  /* attribute list for save action */
  var $CopyPasteVars  =
      array("grouplist","groupMembership","activate_shadowMin",
      "activate_shadowMax","activate_shadowWarning","activate_shadowInactive","activate_shadowExpire",
      "must_change_password","printerList","grouplist","savedGidNumber","savedUidNumber");

  var $attributes     = array("homeDirectory", "loginShell", "uidNumber", "gidNumber", "gecos",
      "shadowMin", "shadowMax", "shadowWarning", "shadowInactive", "shadowLastChange",
      "shadowExpire", "gosaDefaultPrinter", "uid", "host", "gotoLastSystemLogin");

  var $objectclasses  = array("posixAccount", "shadowAccount");

  var $uid                  = "";
  var $multiple_support     = TRUE;
  var $groupMembership_some = array();

  /* constructor, if 'dn' is set, the node loads the given
     'dn' from LDAP */
  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    global $class_mapping;

    /* Configuration is fine, allways */
    $this->config = $config;

    /* Load bases attributes */
    plugin::plugin($config, $dn, $object);

    if ($dn !== NULL) {

      /* Is this account a trustAccount? */
      if ($this->is_account && isset($this->attrs['host'])) {

        if ($this->attrs['host'][0] == "*") {
          $this->trustModel = "fullaccess";
        } else {
          $this->trustModel = "byhost";
          $this->accessTo = array();
            if ($this->is_account && isset($this->attrs['host'])) {
              for ($i = 0; $i < $this->attrs['host']['count']; $i++) {
                $tmp = $this->attrs['host'][$i];
                $this->accessTo[$tmp]= $tmp;
              }
            }
        }
        $this->was_trust_account = TRUE;
      } else {
        $this->was_trust_account = FALSE;
        $this->trustModel= "";
      }

      /* Get group membership */
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(&(objectClass=posixGroup)(memberUid=".$this->uid."))", array("cn", "description"));

      while ($attrs= $ldap->fetch()) {
        if (!isset($attrs["description"][0])) {
          $entry = $attrs["cn"][0];
        } else {
          $entry = $attrs["cn"][0]." [".$attrs["description"][0]."]";
        }
        $this->groupMembership[$ldap->getDN()]= $entry;
      }
      asort($this->groupMembership);
      reset($this->groupMembership);
      $this->savedGroupMembership= $this->groupMembership;

      // Instanciate SSH object if available
      if (isset($class_mapping["sshPublicKey"])){
        if (empty($this->acl_base)){
          $this->acl_base= $config->current['BASE'];
        }

        $this->sshAcl= $this->getacl("sshPublicKey");
        $this->ssh= new sshPublicKey($this->config, $this->dn, $this->sshAcl);
      }
    }

    /* Convert shadowExpire for usage */
    if ($this->shadowExpire == 0) {
      $this->shadowExpire = "";
    } else {
      $this->shadowExpire = date('d.m.Y', $this->shadowExpire * 60 * 60 * 24);
    }

    /* Get global filter config */
    if (!session::is_set("sysfilter")){
      $ui= get_userinfo();
      $base= get_base_from_people($ui->dn);
      $sysfilter= array( "depselect"       => $base,
          "regex"           => "*");
      session::set("sysfilter", $sysfilter);
    }
    $this->ui = get_userinfo();
  }

  /* execute generates the html output for this node */
  function execute($isCopyPaste = false)
  {
    /* Call parent execute */
    plugin::execute();
    $display= "";

    /* Department has changed? */
    if(isset($_POST['depselect'])){
      session::set('CurrentMainBase',validate($_POST['depselect']));
    }

    if (!$isCopyPaste) {

      /* Show tab dialog headers */
      if ($this->parent !== NULL){
        if ($this->is_account){
          if (isset($this->parent->by_object['sambaAccount'])){
            $obj= $this->parent->by_object['sambaAccount'];
          }
          if (isset($obj) && $obj->is_account == TRUE &&
              ((isset($this->parent->by_object['sambaAccount']))&&($this->parent->by_object['sambaAccount']->is_account))
              ||(isset($this->parent->by_object['environment'] ))&&($this->parent->by_object['environment'] ->is_account)){

            /* Samba3 dependency on posix accounts are enabled
               in the moment, because I need to rely on unique
               uidNumbers. There'll be a better solution later
               on. */
            $display= $this->show_disable_header(msgPool::removeFeaturesButton(_("POSIX")), msgPool::featuresEnabled(_("POSIX"), array(_("Samba"), _("Environment"))), TRUE);
          } else {
            $display= $this->show_disable_header(msgPool::removeFeaturesButton(_("POSIX")), msgPool::featuresEnabled(_("POSIX")));
          }
        } else {
          $display= $this->show_enable_header(msgPool::addFeaturesButton(_("POSIX")), msgPool::featuresDisabled(_("POSIX")));
          return($display);
        }
      }
    }

    /* Templates now! */
    $smarty= get_smarty();
    $smarty->assign("usePrototype", "true");

    // Handle ssh dialog?
    if ($this->ssh instanceOf sshPublicKey && preg_match('/[rw]/', $this->getacl("sshPublicKey"))) {
        $smarty->assign("usePrototype", "false");
       if ($result= $this->ssh->execute()) {
         $this->dialog= true;
         return $result;
       }
       $this->dialog= false;
    }

    /* Show main page */
    $smarty= get_smarty();
    $smarty->assign("usePrototype", "true");

    /* In 'MyAccount' mode, we must remove write acls if we are not in editing mode. */
    $SkipWrite = (!isset($this->parent) || !$this->parent) && !session::is_set('edit');

    $smarty->assign("sshPublicKeyACL", $this->getacl("sshPublicKey", $SkipWrite));

    if (count($this->groupMembership) > 16){
      $smarty->assign("groups", "too_many_for_nfs");
    } else {
      $smarty->assign("groups", "");
    }

    /* Avoid "Undefined index: forceMode" */
    $smarty->assign("forceMode", "");

    // Add SSH button if available
    $smarty->assign("sshPublicKey", $this->ssh?1:0);

    $smarty->assign("apply", apply_filter());
    $display .= $smarty->fetch (get_template_path('generic.tpl', TRUE, dirname(__FILE__)));
    return $display;
  }

  function save_object()
  {
    /* Get regex from alphabet */
    if(isset($_GET['search'])){
      $this->GroupRegex = $_GET['search']."*";
    }

    /* Check checkboxes and regexes */
    if(isset($_POST["PosixGroupDialogPosted"])){

      if(isset($_POST['SubSearch']) && ($_POST['SubSearch'])){
        $this->SubSearch = true;
      }else{
        $this->SubSearch = false;
      }
      if(isset($_POST['guser'])){
        $this->GroupUserRegex = $_POST['guser'];
      }
      if(isset($_POST['regex'])){
        $this->GroupRegex = $_POST['regex'];
      }
    }
    $this->GroupRegex = preg_replace("/\*\**/","*",$this->GroupRegex);
    $this->GroupUserRegex = preg_replace("/\*\**/","*",$this->GroupUserRegex);
  }


  /* Save data to LDAP, depending on is_account we save or delete */
  function save()
  {
    /* Call parents save to prepare $this->attrs */
    plugin::save();

    if (empty($this->attrs['gosaDefaultPrinter'])) {
      $thid->attrs['gosaDefaultPrinter']=array();
    }

    /* include global link_info */
    $this->cleanup();

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->dn);
    unset($this->attrs['uid']);
    $ldap->modify ($this->attrs);

    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));
    }

    // Save ssh stuff if needed
    if ($this->ssh) {
      $this->ssh->setDN($this->dn);
      $this->ssh->save();
    }
  }

  /* Adapt from template, using 'dn' */
  function adapt_from_template($dn, $skip= array())
  {
    /* Include global link_info */
    $ldap= $this->config->get_ldap_link();

    plugin::adapt_from_template($dn, $skip);
    $template= $this->attrs['uid'][0];

    /* Adapt group membership */
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=posixGroup)(memberUid=".$this->attrs["uid"][0]."))", array("description", "cn"));

    while ($this->attrs= $ldap->fetch()){
      if (!isset($this->attrs["description"][0])){
        $entry= $this->attrs["cn"][0];
      } else {
        $entry= $this->attrs["cn"][0]." [".$this->attrs["description"][0]."]";
      }
      $this->groupMembership[$ldap->getDN()]= $entry;
    }

    /* Fix primary group settings */
    if($this->gidNumber == 2147483647){
      $this->gidNumber = "";
    }

    if($this->gidNumber){
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(&(objectClass=posixGroup)(cn=$template)(gidNumber=".$this->gidNumber."))", array("cn"));
      if ($ldap->count() != 1){
        $this->primaryGroup= $this->gidNumber;
      }
    }

    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=gosaUserTemplate)(uid=".$template.")(accessTo=*))", array("cn","accessTo"));
    while($attr = $ldap->fetch()){
      $tmp = $attr['accessTo'];
      unset ($tmp['count']);
      $this->accessTo = $tmp;
    }

    /* Adjust shadow checkboxes */
    foreach (array("shadowMin", "shadowMax", "shadowWarning", "shadowInactive", "shadowExpire") as $val){
      if ($this->$val != 0){
        $oval= "activate_".$val;
        $this->$oval= "1";
      }
    }

    /* Only enable checkbox, if shadowExpire is in the future */
    if($this->shadowExpire > time()) {
        $this->activate_shadowExpire= "1";
    }

    /* Convert shadowExpire for usage */
    if ($this->shadowExpire == 0){
        $this->shadowExpire= "";
    } else {
        $this->shadowExpire= date('d.m.Y', $this->shadowExpire * 60 * 60 * 24);
    }

  }
}

?>
