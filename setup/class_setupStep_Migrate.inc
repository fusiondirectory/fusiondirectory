<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007  Fabian Hickert
  Copyright (C) 2011-2013  FusionDirectory

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

/****************
 * FUNCTIONS

Step_Migrate                - Constructor.
update_strings              - Used to update the displayed step informations.
initialize_checks           - Initialize migration steps.
check_ldap_permissions      - Check if the used admin account has full access to the ldap database.
check_gosaAccounts          - Check if there are users without the required objectClasses.
migrate_gosaAccounts        - Migrate selected users to FusionDirectory user accounts.
check_orgUnits   - Check if there are departments, that are not visible for FusionDirectory
migrate_orgUnits - Migrate selected departments
check_adminAccount - Check if there is at least one acl entry available
checkBase                   - Check if there is a root object available

get_user_list               - Get list of available users

create_admin
create_admin_user

execute                     - Generate html output of this plugin
save_object                 - Save posts
array_to_ldif               - Create ldif output of an ldap result array

 ****************/

class CheckFailedException extends Exception
{
  private $error;

  public function __construct($msg, $error)
  {
    parent::__construct($msg);
    $this->error = $error;
  }

  public function getError()
  {
    return $this->error;
  }
}

class StepMigrateDialog extends GenericDialog
{
  protected $post_cancel = 'dialog_cancel';
  protected $post_finish = 'dialog_confirm';

  private $infos;
  private $tplfile;
  private $check;

  public function __construct(&$check, $tpl, $infos)
  {
    $this->attribute  = NULL;
    $this->dialog     = NULL;
    $this->infos      = $infos;
    $this->tplfile    = $tpl;
    $this->check      = $check;
  }

  public function dialog_execute()
  {
    if (
      isset($_POST['dialog_showchanges']) ||
      isset($_POST['dialog_hidechanges']) ||
      isset($_POST['dialog_refresh'])) {
      $this->infos = $this->check->dialog_refresh();
    }
    $smarty = get_smarty();
    $smarty->assign('infos', $this->infos);
    return $smarty->fetch(get_template_path($this->tplfile, TRUE, dirname(__FILE__)));
  }

  function handle_finish ()
  {
    if ($this->check->migrate_confirm()) {
      return FALSE;
    } else {
      return $this->dialog_execute();
    }
  }

  function handle_cancel ()
  {
    return FALSE;
  }
}

class StepMigrateCheck
{
  public $name;
  public $title;
  public $status  = FALSE;
  public $msg     = '';
  public $error   = '';
  public $fnc;
  private $step;

  public function __construct($step, $name, $title)
  {
    $this->name   = $name;
    $this->title  = $title;
    $this->fnc    = 'check_'.$name;
    $this->step   = $step;
  }

  public function run($fnc = NULL)
  {
    if ($fnc === NULL) {
      $fnc          = $this->fnc;
    }
    try {
      $this->msg    = _('Ok');
      $this->error  = $this->step->$fnc($this);
      $this->status = TRUE;
    } catch (CheckFailedException $e) {
      $this->status = FALSE;
      $this->msg    = $e->getMessage();
      $this->error  = $e->getError();
    }
  }

  public function save_object()
  {
    if (isset($_POST[$this->name.'_create'])) {
      $fnc = $this->fnc.'_create';
      $this->step->$fnc($this);
    } elseif (isset($_POST[$this->name.'_migrate'])) {
      $fnc = $this->fnc.'_migrate';
      $this->step->$fnc($this);
    }
  }

  public function submit ($value = NULL, $id = 'migrate')
  {
    if ($value === NULL) {
      $value = _('Migrate');
    }
    return '<input type="submit" name="'.$this->name.'_'.$id.'" value="'.$value.'"/>';
  }

  public function migrate_confirm()
  {
    $fnc = $this->fnc.'_migrate'.'_confirm';
    $res = $this->step->$fnc($this);
    if ($res) {
      $this->run();
    }
    return $res;
  }

  public function dialog_refresh()
  {
    $fnc = $this->fnc.'_migrate'.'_refresh';
    return $this->step->$fnc($this);
  }
}

class Step_Migrate extends setupStep
{
  var $header_image   = "geticon.php?context=applications&icon=utilities-system-monitor&size=48";

  /* Root object classes */
  var $rootOC_details = array();

  /* Entries needing migration */
  var $orgUnits_toMigrate       = array();
  var $gosaAccounts_toMigrate   = array();
  var $outsideUsers_toMigrate   = array();
  var $outsideGroups_toMigrate  = array();

  /* check for multiple use of same uidNumber */
  var $check_uidNumbers = array();

  /* check for multiple use of same gidNumber */
  var $check_gidNumbers = array();

  /* Defaults ACL roles */
  var $defaultRoles;

  static function getAttributesInfo()
  {
    return array(
      'checks' => array(
        'class'     => array('fullwidth'),
        'name'      => _('PHP module and extension checks'),
        'template'  => get_template_path("setup_migrate.tpl", TRUE, dirname(__FILE__)),
        'attrs'     => array(
          new FakeAttribute('checks')
        )
      ),
    );
  }

  function __construct()
  {
    parent::__construct();
    $this->fill_defaultRoles();
  }

  function update_strings()
  {
    $this->s_title      = _("LDAP inspection");
    $this->s_title_long = _("LDAP inspection");
    $this->s_info       = _("Analyze your current LDAP for FusionDirectory compatibility");
  }

  function fill_defaultRoles()
  {
    $this->defaultRoles = array(
      array(
        'cn'              => 'manager',
        'description'     => _('Give all rights on users in the given branch'),
        'objectclass'     => array('top', 'gosaRole'),
        'gosaAclTemplate' => '0:user/password;cmdrw,user/user;cmdrw,user/posixAccount;cmdrw'
      ),
      array(
        'cn'              => 'editowninfos',
        'description'     => _('Allow users to edit their own information (main tab and posix use only on base)'),
        'objectclass'     => array('top', 'gosaRole'),
        'gosaAclTemplate' => '0:user/posixAccount;srw,user/user;srw'
      ),
      array(
        'cn'              => 'editowninfos',
        'description'     => _('Allow users to edit their own password (use only on base)'),
        'objectclass'     => array('top', 'gosaRole'),
        'gosaAclTemplate' => '0:user/password;srw'
      ),
    );
  }

  function initialize_checks()
  {
    global $config;
    $config->get_departments();

    $checks = array(
      'base'          => new StepMigrateCheck($this, 'base',          _('Checking for root object')),
      'baseOC'        => new StepMigrateCheck($this, 'baseOC',        _('Inspecting object classes in root object')),
      'permissions'   => new StepMigrateCheck($this, 'permissions',   _('Checking permission for LDAP database')),
      'gosaAccounts'  => new StepMigrateCheck($this, 'gosaAccounts',  _('Checking for invisible users')),
      'adminAccount'  => new StepMigrateCheck($this, 'adminAccount',  _('Checking for super administrator')),
      'defaultACLs'   => new StepMigrateCheck($this, 'defaultACLs',   _('Checking for default ACL roles and groups')),
      'outsideUsers'  => new StepMigrateCheck($this, 'outsideUsers',  _('Checking for users outside the people tree')),
      'outsideGroups' => new StepMigrateCheck($this, 'outsideGroups', _('Checking for groups outside the groups tree')),
      'orgUnits'      => new StepMigrateCheck($this, 'orgUnits',      _('Checking for invisible departments')),
      'uidNumber'     => new StepMigrateCheck($this, 'uidNumber',     _('Checking for duplicated UID numbers')),
      'gidNumber'     => new StepMigrateCheck($this, 'gidNumber',     _('Checking for duplicate GID numbers')),
    );

    $this->checks = $checks;
  }

  /* Return ldif information for a given attribute array */
  function array_to_ldif($attrs)
  {
    $ret = '';
    unset($attrs['count']);
    unset($attrs['dn']);
    foreach ($attrs as $name => $value) {
      if (is_numeric($name)) {
        continue;
      }
      if (is_array($value)) {
        unset($value['count']);
        foreach ($value as $a_val) {
          $ret .= $name.': '. $a_val."\n";
        }
      } else {
        $ret .= $name.': '. $value."\n";
      }
    }
    return preg_replace("/\n$/", '', $ret);
  }

  function execute()
  {
    if (empty($this->checks) || isset($_POST['reload'])) {
      $this->initialize_checks();
      foreach ($this->checks as &$check) {
        $check->run();
      }
      unset($check);
    }
    return parent::execute();
  }

  function save_object()
  {
    $this->is_completed = TRUE;
    parent::save_object();
    foreach ($this->checks as &$check) {
      $check->save_object();
    }
    unset($check);
  }

  /* Check if the root object exists.
   * If the parameter just_check is TRUE, then just check if the
   *  root object is missing and update the info messages.
   * If the Parameter is FALSE, try to create a new root object.
   */
  function check_base(&$checkobj)
  {
    global $config;

    $ldap = $config->get_ldap_link();

    /* Check if root object exists */
    $ldap->cd($config->current['BASE']);
    $ldap->set_size_limit(1);
    $res = $ldap->search("(objectClass=*)");
    $ldap->set_size_limit(0);
    $err = ldap_errno($ldap->cid);

    if ( !$res ||
        $err == 0x20 || // LDAP_NO_SUCH_OBJECT
        $err == 0x40) { // LDAP_NAMING_VIOLATION

      /* Root object doesn't exists */
      throw new CheckFailedException(
        _('Failed'),
        _('The LDAP root object is missing. It is required to use your LDAP service.').'&nbsp;'.
        $checkobj->submit(_('Try to create root object'), 'create')
      );
    }

    /* Root object exists */
    return '';
  }

  function check_base_create (&$checkobj)
  {
    global $config;

    $ldap = $config->get_ldap_link();

    /* Add root object */
    $ldap->cd($config->current['BASE']);
    $ldap->create_missing_trees($config->current['BASE']);
    /* Re-run test */
    $checkobj->run();
  }

  /* Check if the root object includes the required object classes, e.g. gosaDepartment is required for ACLs.
   * If the parameter just_check is TRUE, then just check for the OCs.
   * If the Parameter is FALSE, try to add the required object classes.
   */
  function check_baseOC(&$checkobj)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    /* Check if root object exists */
    $ldap->cd($config->current['BASE']);
    $ldap->cat($config->current['BASE']);
    if (!$ldap->count()) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    }

    $attrs = $ldap->fetch();

    /* Root object doesn't exists */
    if (!in_array("gosaDepartment", $attrs['objectClass'])) {
      $this->rootOC_details = array();
      $mods = array();

      /* Get list of possible container objects, to be able to detect naming
       *  attributes and missing attribute types.
       */
      if (!class_available("departmentManagement")) {
        throw new CheckFailedException(
          _("Failed"),
          sprintf(_("Missing FusionDirectory object class '%s'!"), "departmentManagement").
          "&nbsp;"._("Please check your installation.")
        );
      }

      /* Try to detect base class type, e.g. is it a dcObject */
      $dep_types  = departmentManagement::getDepartmentTypes();
      $dep_type   = "";
      $attrs['objectClass'][] = 'gosaDepartment'; // This allow us to filter it as if it was already migrated
      foreach ($dep_types as $type) {
        if (objects::isOfType($attrs, $type)) {
          $dep_type = $type;
          break;
        }
      }
      $key = array_search('gosaDepartment', $attrs['objectClass']);
      unset($attrs['objectClass'][$key]);

      /* If no known base class was detect, abort with message */
      if (empty($dep_type)) {
        throw new CheckFailedException(
          _("Failed"),
          sprintf(_("Cannot handle the structural object type of your root object. Please try to add the object class '%s' manually."), "gosaDepartment")
        );
      }
      $dep_infos = objects::infos($dep_type);

      /* Create 'current' and 'target' object properties, to be able to display
       *  a set of modifications required to create a valid FusionDirectory department.
       */
      $str = "dn: ".$config->current['BASE']."\n";
      for ($i = 0; $i < $attrs['objectClass']['count']; $i++) {
        $str .= "objectClass: ".$attrs['objectClass'][$i]."\n";
      }
      $this->rootOC_details['current'] = $str;

      /* Create target infos */
      $str = "dn: ".$config->current['BASE']."\n";
      for ($i = 0; $i < $attrs['objectClass']['count']; $i++) {
        $str .= "objectClass: ".$attrs['objectClass'][$i]."\n";
        $mods['objectClass'][] = $attrs['objectClass'][$i];
      }
      $mods['objectClass'][] = "gosaDepartment";

      $str .= "<b>objectClass: gosaDepartment</b>\n";

      /* Append attribute 'ou', it is required by gosaDepartment */
      if (!isset($attrs['ou'])) {
        $val = "GOsa";
        if (isset($attrs[$dep_infos['mainAttr']][0])) {
          $val = $attrs[$dep_infos['mainAttr']][0];
        }
        $str .= "<b>ou: ".$val."</b>\n";

        $mods['ou'] = $val;
      }

      /*Append description, it is required by gosaDepartment too */
      if (!isset($attrs['description'])) {
        $val = "GOsa";
        if (isset($attrs[$dep_infos['mainAttr']][0])) {
          $val = $attrs[$dep_infos['mainAttr']][0];
        }
        $str .= "<b>description: ".$val."</b>\n";

        $mods['description'] = $val;
      }
      $this->rootOC_details['target'] = $str;
      $this->rootOC_details['mods']   = $mods;

      /*  Add button that allows to open the migration details */
      throw new CheckFailedException(
        _('Failed'),
        '&nbsp;'.$checkobj->submit()
      );
    }

    /* Create & remove of dummy object was successful */
    return '';
  }

  function check_baseOC_migrate (&$checkobj)
  {
    $this->openDialog(new StepMigrateDialog($checkobj, 'setup_migrate_baseOC.tpl', $this->rootOC_details));
  }

  function check_baseOC_migrate_confirm ()
  {
    global $config;
    $ldap = $config->get_ldap_link();

    /* Check if root object exists */
    $ldap->cd($config->current['BASE']);
    $ldap->cat($config->current['BASE']);

    $attrs = $ldap->fetch();

    /* Root object doesn't exists */
    if (!in_array("gosaDepartment", $attrs['objectClass'])) {
      /* Add root object */
      $ldap->cd($config->current['BASE']);
      if (isset($this->rootOC_details['mods'])) {
        $res = $ldap->modify($this->rootOC_details['mods']);
        if (!$res) {
          msg_dialog::display(_('LDAP error'), msgPool::ldaperror($ldap->get_error(), $config->current['BASE'], LDAP_MOD, get_class()), LDAP_ERROR);
        }
        $this->checks['adminAccount']->run();
        return $res;
      } else {
        trigger_error('No modifications to make... ');
      }
      return TRUE;
    }
    return TRUE;
  }

  /* Check ldap accessibility
   * Create and remove a dummy object,
   *  to ensure that we have the necessary permissions
   */
  function check_permissions(&$checkobj)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    /* Create dummy entry */
    $name       = 'GOsa_setup_text_entry_'.session_id().rand(0, 999999);
    $dn         = 'ou='.$name.','.$config->current['BASE'];
    $testEntry  = array();

    $testEntry['objectClass'][] = 'top';
    $testEntry['objectClass'][] = 'organizationalUnit';
    $testEntry['objectClass'][] = 'gosaDepartment';
    $testEntry['description']   = 'Created by FusionDirectory setup, this object can be removed.';
    $testEntry['ou']            = $name;

    /* check if simple ldap cat will be successful */
    $res = $ldap->cat($config->current['BASE']);
    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    }

    /* Try to create dummy object */
    $ldap->cd ($dn);
    $res = $ldap->add($testEntry);
    $ldap->cat($dn);
    if (!$ldap->count()) {
      new log("view", "setup/".get_class($this), $dn, array(), $ldap->get_error());
      throw new CheckFailedException(
        _('Failed'),
        sprintf(_('The specified user "%s" does not have full access to your LDAP database.'), $config->current['ADMINDN'])
      );
    }

    /* Try to remove created entry */
    $res = $ldap->rmDir($dn);
    $ldap->cat($dn);
    if ($ldap->count()) {
      new log("view", "setup/".get_class($this), $dn, array(), $ldap->get_error());
      throw new CheckFailedException(
        _('Failed'),
        sprintf(_('The specified user "%s" does not have full access to your ldap database.'), $config->current['ADMINDN'])
      );
    }

    /* Create & remove of dummy object was successful */
    return '';
  }

  /* Check if there are users which will
   *  be invisible for FusionDirectory
   */
  function check_gosaAccounts(&$checkobj)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    /* Remember old list of invisible users, to be able to set
     *  the 'html checked' status for the checkboxes again
     */
    $old    = $this->gosaAccounts_toMigrate;
    $this->gosaAccounts_toMigrate = array();

    /* Get all invisible users */
    $ldap->cd($config->current['BASE']);
    $res = $ldap->search("(&(|(objectClass=posixAccount)(&(objectClass=inetOrgPerson)(objectClass=organizationalPerson))(objectClass=OpenLDAPperson))(!(objectClass=gosaAccount))(!(&(objectClass=Account)(objectClass=sambaSamAccount)))(uid=*))", array("sn","givenName","cn","uid"));

    while ($attrs = $ldap->fetch()) {
      if (!preg_match("/,dc=addressbook,/", $attrs['dn'])) {
        $attrs['checked'] = FALSE;
        $attrs['before']  = "";
        $attrs['after']   = "";

        /* Set objects to selected, that were selected before reload */
        if (isset($old[base64_encode($attrs['dn'])])) {
          $attrs['checked'] = $old[base64_encode($attrs['dn'])]['checked'];
        }
        $this->gosaAccounts_toMigrate[base64_encode($attrs['dn'])] = $attrs;
      }
    }

    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    } elseif (count($this->gosaAccounts_toMigrate) == 0) {
      /* No invisible */
      return '';
    } else {
      throw new CheckFailedException(
        "<div style='color:#F0A500'>"._("Warning")."</div>",
        sprintf(
          _('Found %s user(s) that will not be visible in FusionDirectory or which are incomplete.'),
          count($this->gosaAccounts_toMigrate)
        ).$checkobj->submit()
      );
    }
  }

  function check_gosaAccounts_migrate (&$checkobj)
  {
    $this->check_multipleGeneric_migrate($checkobj, array('title' => _('User migration')));
  }

  function check_gosaAccounts_migrate_refresh (&$checkobj)
  {
    return $this->check_multipleGeneric_migrate_refresh($checkobj, array('title' => _('User migration')));
  }

  function check_gosaAccounts_migrate_confirm(&$checkobj, $only_ldif = FALSE)
  {
    return $this->check_multipleGeneric_migrate_confirm(
      $checkobj,
      array('gosaAccount','inetOrgPerson','organizationalPerson','person'),
      array(),
      $only_ldif
    );
  }

  function check_multipleGeneric_migrate (&$checkobj, $infos)
  {
    $var = $checkobj->name.'_toMigrate';
    /* Fix displayed dn syntax */
    $infos['entries'] = $this->$var;
    foreach ($infos['entries'] as $key => $data) {
      $infos['entries'][$key]['dn'] = LDAP::fix($data['dn']);
    }
    $this->openDialog(new StepMigrateDialog($checkobj, 'setup_migrate_gosaAccounts.tpl', $infos));
  }

  function check_multipleGeneric_migrate_refresh (&$checkobj, $infos)
  {
    if (isset($_POST['dialog_showchanges'])) {
      /* Show changes */
      $fnc = 'check_'.$checkobj->name.'_migrate_confirm';
      $this->$fnc($checkobj, TRUE);
    } else {
      /* Hide changes */
      $checkobj->run();
    }
    /* Fix displayed dn syntax */
    $var = $checkobj->name.'_toMigrate';
    $infos['entries'] = $this->$var;
    foreach ($infos['entries'] as $key => $data) {
      $infos['entries'][$key]['dn'] = LDAP::fix($data['dn']);
    }
    return $infos;
  }

  function check_multipleGeneric_migrate_confirm(&$checkobj, $oc, $mandatory, $only_ldif)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    /* Add objectClasses to the selected entries */
    $var = $checkobj->name.'_toMigrate';
    foreach ($this->$var as $key => &$entry) {
      $entry['checked'] = isset($_POST['migrate_'.$key]);
      if ($entry['checked']) {
        /* Get old objectClasses */
        $ldap->cat($entry['dn'], array_merge(array('objectClass'), array_keys($mandatory)));
        $attrs = $ldap->fetch();

        /* Create new objectClass array */
        $new_attrs  = array();
        $new_attrs['objectClass'] = $oc;
        for ($i = 0; $i < $attrs['objectClass']['count']; $i++) {
          if (!in_array_ics($attrs['objectClass'][$i], $new_attrs['objectClass'])) {
            $new_attrs['objectClass'][] = $attrs['objectClass'][$i];
          }
        }

        /* Append mandatories if missing */
        foreach ($mandatory as $name => $value) {
          if (!isset($attrs[$name])) {
            $new_attrs[$name] = $value;
          }
        }

        /* Set info attributes for current object,
         *  or write changes to the ldap database
         */
        if ($only_ldif) {
          $entry['before'] = $this->array_to_ldif($attrs);
          $entry['after']  = $this->array_to_ldif($new_attrs);
        } else {
          $ldap->cd($attrs['dn']);
          if (!$ldap->modify($new_attrs)) {
            msg_dialog::display(
              _('Migration error'),
              sprintf(
                _('Cannot migrate entry "%s":').'<br/><br/><i>%s</i>',
                LDAP::fix($attrs['dn']), $ldap->get_error()
              ),
              ERROR_DIALOG
            );
            return FALSE;
          }
        }
      }
    }
    unset($entry);
    return TRUE;
  }

  /* Check Acls if there is at least one object with acls defined */
  function check_adminAccount(&$checkobj)
  {
    global $config;

    /* Reset settings */
    $FD_1_0_8_found = FALSE;

    /* Establish ldap connection */
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $res = $ldap->cat($config->current['BASE']);

    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    } else {
      $FD_1_0_8_found = FALSE;
      $FD_1_0_7_found = FALSE;

      $attrs = $ldap->fetch();

      /* Collect a list of available FusionDirectory users and groups */
      $users = array();
      $ldap->search("(&(objectClass=gosaAccount)(objectClass=person)".
        "(objectClass=inetOrgPerson)(objectClass=organizationalPerson))", array("uid","dn"));
      while ($user_attrs = $ldap->fetch()) {
        $users[$user_attrs['dn']] = $user_attrs['uid'][0];
        $rusers[$user_attrs['uid'][0]] = $user_attrs['dn'];
      }
      $groups = array();
      $ldap->search("objectClass=posixGroup", array("cn","dn"));
      while ($group_attrs = $ldap->fetch()) {
        $groups[$group_attrs['dn']] = $group_attrs['cn'][0];
      }

      /* Check if a valid FusionDirectory 1.0.8 admin exists
          -> gosaAclEntry for an existing and accessible user.
       */
      $valid_users  = "";
      $valid_groups = "";
      if (isset($attrs['gosaAclEntry'])) {
        $acls = $attrs['gosaAclEntry'];
        for ($i = 0; $i < $acls['count']; $i++) {
          $acl = $acls[$i];
          $tmp = explode(":", $acl);

          if ($tmp[1] == "subtree") {
            /* Check if acl owner is a valid FusionDirectory user account */
            $ldap->cat(base64_decode($tmp[2]), array("gosaAclTemplate"), '(gosaAclTemplate=*:all;cmdrw)');
            if ($ldap->count()) {
              $members = explode(",", $tmp[3]);
              foreach ($members as $member) {
                $member = base64_decode($member);

                if (isset($users[$member])) {
                  $valid_users    .= $users[$member].", ";
                  $FD_1_0_8_found = TRUE;
                }
                if (isset($groups[$member])) {
                  $ldap->cat($member);
                  $group_attrs = $ldap->fetch();
                  $val_users = "";
                  if (isset($group_attrs['memberUid'])) {
                    for ($e = 0; $e < $group_attrs['memberUid']['count']; $e ++) {
                      if (isset($rusers[$group_attrs['memberUid'][$e]])) {
                        $val_users .= $group_attrs['memberUid'][$e].", ";
                      }
                    }
                  }
                  if (!empty($val_users)) {
                    $valid_groups .= $groups[$member]."(<i>".trim($val_users, ", ")."</i>), ";
                    $FD_1_0_8_found  = TRUE;
                  }
                }
              }
            }
          }
        }
      }

      /* Try to find an old FD 1.0.7 administrative account that may be migrated */
      if (!$FD_1_0_8_found) {
        $valid_users  = "";
        $valid_groups = "";
        if (isset($attrs['gosaAclEntry'])) {
          $acls = $attrs['gosaAclEntry'];
          for ($i = 0; $i < $acls['count']; $i++) {
            $acl = $acls[$i];
            $tmp = explode(":", $acl);

            if ($tmp[1] == "psub") {
              $members = explode(",", $tmp[2]);
              foreach ($members as $member) {
                $member = base64_decode($member);
                if (isset($users[$member])) {
                  if (preg_match("/all;cmdrw/i", $tmp[3])) {
                    $valid_users    .= $users[$member].", ";
                    $FD_1_0_7_found = TRUE;
                  }
                }
                if (isset($groups[$member])) {
                  if (preg_match("/all;cmdrw/i", $tmp[3])) {
                    $ldap->cat($member);
                    $group_attrs = $ldap->fetch();
                    $val_users = "";
                    if (isset($group_attrs['memberUid'])) {
                      for ($e = 0; $e < $group_attrs['memberUid']['count']; $e++) {
                        if (isset($rusers[$group_attrs['memberUid'][$e]])) {
                          $val_users .= $group_attrs['memberUid'][$e].", ";
                        }
                      }
                    }
                    if (!empty($val_users)) {
                      $valid_groups .= $groups[$member]."(<i>".trim($val_users, ", ")."</i>), ";
                      $FD_1_0_7_found  = TRUE;
                    }
                  }
                }
              }
            } elseif ($tmp[1] == "role") {
              /* Check if acl owner is a valid FusionDirectory user account */
              $ldap->cat(base64_decode($tmp[2]), array("gosaAclTemplate"));
              $ret = $ldap->fetch();

              if (isset($ret['gosaAclTemplate'])) {
                $cnt = $ret['gosaAclTemplate']['count'];
                for ($e = 0; $e < $cnt; $e++) {

                  $a_str = $ret['gosaAclTemplate'][$e];
                  if (preg_match("/^[0-9]*:psub:/", $a_str) && preg_match("/:all;cmdrw$/", $a_str)) {

                    $members = explode(",", $tmp[3]);
                    foreach ($members as $member) {
                      $member = base64_decode($member);

                      if (isset($users[$member])) {
                        $valid_users    .= $users[$member].", ";
                        $FD_1_0_7_found = TRUE;
                      }
                      if (isset($groups[$member])) {
                        $ldap->cat($member);
                        $group_attrs = $ldap->fetch();
                        $val_users = "";
                        if (isset($group_attrs['memberUid'])) {
                          for ($e = 0; $e < $group_attrs['memberUid']['count']; $e ++) {
                            if (isset($rusers[$group_attrs['memberUid'][$e]])) {
                              $val_users .= $group_attrs['memberUid'][$e].", ";
                            }
                          }
                        }
                        if (!empty($val_users)) {
                          $valid_groups .= $groups[$member]."(<i>".trim($val_users, ", ")."</i>), ";
                          $FD_1_0_7_found  = TRUE;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }

      /* Print out results */
      if ($FD_1_0_7_found) {
        $str = "";
        if (!empty($valid_users)) {
          $str .= '<i>'.sprintf(_('FD 1.0.7 administrative accounts found: %s'), trim($valid_users, ', ')).'</i><br/>';
        }
        if (!empty($valid_groups)) {
          $str .= '<i>'.sprintf(_('FD 1.0.7 administrative groups found: %s'), trim($valid_groups, ', ')).'</i><br/>';
        }
        $str .= _('You may run <i>fusiondirectory-setup --migrate-acls</i> after saving config file at the end of the setup to migrate it.<br/>');
        throw new CheckFailedException(
          _('Failed'),
          $str._('There is no valid FusionDirectory 1.0.8 administrator account inside your LDAP.').'&nbsp;'.
          $checkobj->submit(_('Create'), 'create')
        );
      } elseif ($FD_1_0_8_found) {
        $str = "";
        if (!empty($valid_users)) {
          $str .= "<b>"._("Users")."</b>:&nbsp;".trim($valid_users, ", ")."<br>";
        }
        if (!empty($valid_groups)) {
          $str .= "<b>"._("Groups")."</b>:&nbsp;".trim($valid_groups, ", ")."<br>";
        }
        return $str;
      } else {
        throw new CheckFailedException(
          _('Failed'),
          _('There is no FusionDirectory administrator account inside your LDAP.').'&nbsp;'.
          $checkobj->submit(_('Create'), 'create')
        );
      }
    }

    // Reload base OC
    $this->checks['baseOC']->run();
    return '';
  }

  function check_adminAccount_create(&$checkobj)
  {
    $infos = array(
      'uid'       => 'fd-admin',
      'password'  => '',
      'password2' => '',
    );
    $this->openDialog(new StepMigrateDialog($checkobj, 'setup_migrate_adminAccount.tpl', $infos));
  }

  function check_adminAccount_migrate_confirm(&$checkobj)
  {
    global $config;
    session::global_set('CurrentMainBase', $config->current['BASE']);

    /* Creating role */
    $ldap = $config->get_ldap_link();

    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=gosaRole)(gosaAclTemplate=*:all;cmdrw))', array('dn'));
    if ($attrs = $ldap->fetch()) {
      $roledn = $attrs['dn'];
    } else {
      $tabObject  = objects::create('aclRole');
      $baseObject = $tabObject->getBaseObject();

      $baseObject->cn               = 'admin';
      $baseObject->description      = _('Gives all rights on all objects');
      $baseObject->gosaAclTemplate  = array(array('all' => array('0' => 'cmdrw')));

      $tabObject->save();
      $roledn = $tabObject->dn;
    }

    /* Creating user */
    $tabObject = objects::create('user');
    $_POST['givenName']                   = 'System';
    $_POST['sn']                          = 'Administrator';
    $_POST[$tabObject->current.'_posted'] = TRUE;
    $_POST['dialog_refresh']              = TRUE;
    $tabObject->save_object();
    $errors = $tabObject->check();
    if (!empty($errors)) {
      foreach ($errors as $error) {
        msg_dialog::display(_('Error'), $error, ERROR_DIALOG);
      }
      return FALSE;
    }
    $tabObject->save();
    $admindn = $tabObject->dn;

    /* Assigning role */
    $tabObject  = objects::open($config->current['BASE'], 'aclAssignment');
    $baseObject = $tabObject->getBaseObject();

    $assignments = $baseObject->gosaAclEntry;
    array_unshift(
      $assignments,
      array(
        'scope'   => 'subtree',
        'role'    => $roledn,
        'members' => array($admindn),
      )
    );
    $baseObject->gosaAclEntry = $assignments;
    $tabObject->save();

    return TRUE;
  }

  function check_adminAccount_migrate_refresh(&$checkobj)
  {
    return array(
      'uid'       => $_POST['uid'],
      'password'  => $_POST['userPassword_password'],
      'password2' => $_POST['userPassword_password2'],
    );
  }

  /* Check if default roles and groupes have been inserted */
  function check_defaultACLs(&$checkobj)
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $res = $ldap->cat($config->current['BASE']);

    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    }

    $existings = 0;
    foreach ($this->defaultRoles as $role) {
      $dn = 'cn='.$role['cn'].','.get_ou('aclRoleRDN').$config->current['BASE'];
      $ldap->cat($dn, array('dn'));
      if ($ldap->count() > 0) {
        $existings++;
      }
    }
    $status = ($existings == count($this->defaultRoles));
    if ($existings == 0) {
      $checkobj->msg = _('Default ACL roles have not been inserted');
    } elseif ($existings < count($this->defaultRoles)) {
      $checkobj->msg = _('Some default ACL roles are missing');
    } else {
      $checkobj->msg = _('Default ACL roles have been inserted');
    }
    if ($status === FALSE) {
      throw new CheckFailedException(
        $checkobj->msg,
        '&nbsp;'.$checkobj->submit()
      );
    } else {
      return '';
    }
  }

  function check_defaultACLs_migrate(&$checkobj)
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);

    foreach ($this->defaultRoles as $role) {
      $dn = 'cn='.$role['cn'].','.get_ou('aclRoleRDN').$config->current['BASE'];
      $ldap->cat($dn);
      if ($ldap->count() == 0) {
        $ldap->cd($config->current['BASE']);
        $ldap->create_missing_trees(get_ou('aclRoleRDN').$config->current['BASE']);
        $ldap->cd($dn);
        $ldap->add($role);
        if (!$ldap->success()) {
          msg_dialog::display(
            _('Migration error'),
            sprintf(
              _('Cannot add ACL role "%s":').'<br/><br/><i>%s</i>',
              LDAP::fix($dn), $ldap->get_error()
            ),
            ERROR_DIALOG
          );
          return FALSE;
        }
      }
    }
    $checkobj->run();
    return TRUE;
  }

  /* Search for users outside the people ou */
  function check_outsideUsers(&$checkobj)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $ldap->cd($config->current['BASE']);

    /***********
     * Search for all users
     ***********/
    $res = $ldap->search('(&(objectClass=gosaAccount)(!(uid=*$)))', array('dn'));
    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    }

    /***********
     * Check if returned users are within a valid department. (peopleou,gosaDepartment,base)
     ***********/
    $this->outsideUsers_toMigrate = array();
    $people_ou = trim(get_ou('userRDN'));

    while ($attrs = $ldap->fetch()) {
      $people_db_base = preg_replace('/^[^,]+,'.preg_quote($people_ou, '/').'/i', '', $attrs['dn']);

      /* Check if entry is not an addressbook only user
       *  and verify that he is in a valid department
       */
      if (!preg_match('/dc=addressbook,/', $people_db_base) &&
          !in_array($people_db_base, $config->departments)) {
        $attrs['checked'] = FALSE;
        $attrs['ldif']    = '';
        $this->outsideUsers_toMigrate[base64_encode($attrs['dn'])] = $attrs;
      }
    }

    if (count($this->outsideUsers_toMigrate)) {
      throw new CheckFailedException(
        "<div style='color:#F0A500'>"._("Warning")."</div>",
        sprintf(_('Found %s user(s) outside the configured tree "%s".'), count($this->outsideUsers_toMigrate), $people_ou).
        $checkobj->submit()
      );
    } else {
      return '';
    }
  }

  function check_outsideUsers_migrate(&$checkobj)
  {
    global $config;
    $this->check_multipleGeneric_migrate(
      $checkobj,
      array(
        'title'       => _('Move users into configured user tree'),
        'outside'     => TRUE,
        'ous'         => $config->departments,
        'destination' => $_POST['destination'],
      )
    );
  }

  function check_outsideUsers_migrate_refresh(&$checkobj)
  {
    global $config;
    return $this->check_multipleGeneric_migrate_refresh(
      $checkobj,
      array(
        'title'       => _('Move users into configured user tree'),
        'outside'     => TRUE,
        'ous'         => $config->departments,
        'destination' => $_POST['destination'],
      )
    );
  }

  function check_outsideUsers_migrate_confirm(&$checkobj, $only_ldif = FALSE, $ou = 'userRDN')
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);

    /* Check if there was a destination department posted */
    if (isset($_POST['destination'])) {
      $destination_dep = get_ou($ou).$_POST['destination'];
    } else {
      msg_dialog::display(_('LDAP error'), _('Cannot move entries to the requested department!'), ERROR_DIALOG);
      return FALSE;
    }

    $var = $checkobj->name.'_toMigrate';
    foreach ($this->$var as $b_dn => &$entry) {
      $entry['checked'] = isset($_POST['migrate_'.$b_dn]);
      $entry['ldif']    = '';
      if ($entry['checked']) {
        $dn = base64_decode($b_dn);
        $d_dn = preg_replace('/,.*$/', ','.$destination_dep, $dn);
        if ($only_ldif) {
          $entry['ldif'] = _('Entry will be moved from').":<br/>\t".($ldap->fix($dn)).'<br/>'._('to').":<br/>\t".($ldap->fix($d_dn));

          /* Check if there are references to this object */
          $ldap->search('(&(member='.LDAP::prepare4filter($dn).')(|(objectClass=gosaGroupOfNames)(objectClass=groupOfNames)))', array('dn'));
          $refs = '';
          while ($attrs = $ldap->fetch()) {
            $ref_dn = $attrs['dn'];
            $refs .= "<br/>\t".$ref_dn;
          }
          if (!empty($refs)) {
            $entry['ldif'] .= '<br/><br/><i>'._('The following references will be updated').':</i>'.$refs;
          }
        } else {
          $this->move($dn, $d_dn);
        }
      }
    }
    unset($entry);

    return TRUE;
  }

  /* Search for groups outside the group ou */
  function check_outsideGroups(&$checkobj)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $group_ou = get_ou('groupRDN');
    $ldap->cd($config->current['BASE']);

    /***********
     * Get all groups
     ***********/
    $res = $ldap->search('(objectClass=posixGroup)', array('dn'));
    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    }

    $this->outsideGroups_toMigrate = array();
    while ($attrs = $ldap->fetch()) {
      $group_db_base = preg_replace('/^[^,]+,'.preg_quote($group_ou, '/').'/i', '', $attrs['dn']);

      /* Check if entry is not an addressbook only user
       *  and verify that he is in a valid department
       */
      if ( !preg_match('/'.preg_quote('dc=addressbook,', '/').'/', $group_db_base) &&
          !in_array($group_db_base, $config->departments)
        ) {
        $attrs['checked'] = FALSE;
        $attrs['ldif']    = '';
        $this->outsideGroups_toMigrate[base64_encode($attrs['dn'])] = $attrs;
      }
    }

    if (count($this->outsideGroups_toMigrate)) {
      throw new CheckFailedException(
        "<div style='color:#F0A500'>"._("Warning")."</div>",
        sprintf(_("Found %s groups outside the configured tree '%s'."), count($this->outsideGroups_toMigrate), $group_ou).
        '&nbsp;'.$checkobj->submit()
      );
    } else {
      return '';
    }
  }

  function check_outsideGroups_migrate(&$checkobj)
  {
    global $config;
    $this->check_multipleGeneric_migrate(
      $checkobj,
      array(
        'title'       => _('Move groups into configured groups tree'),
        'outside'     => TRUE,
        'ous'         => $config->departments,
        'destination' => $_POST['destination'],
      )
    );
  }

  function check_outsideGroups_migrate_refresh(&$checkobj)
  {
    global $config;
    return $this->check_multipleGeneric_migrate_refresh(
      $checkobj,
      array(
        'title'       => _('Move groups into configured groups tree'),
        'outside'     => TRUE,
        'ous'         => $config->departments,
        'destination' => $_POST['destination'],
      )
    );
  }

  function check_outsideGroups_migrate_confirm(&$checkobj, $only_ldif = FALSE)
  {
    return $this->check_outsideUsers_migrate_confirm($checkobj, $only_ldif, 'groupRDN');
  }

  /* Check if there are invisible organizational Units */
  function check_orgUnits(&$checkobj)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $old                      = $this->orgUnits_toMigrate;
    $this->orgUnits_toMigrate = array();

    /* Skip FusionDirectory internal departments */
    $skip_dns = array(
      '/dc=addressbook,/',
      '/ou=systems,'.preg_quote($config->current['BASE']).'$/',
      '/ou=snapshots,/'
    );
    foreach (objects::types() as $type) {
      $infos = objects::infos($type);
      if (isset($infos['ou']) && ($infos['ou'] != '')) {
        $skip_dns[] = '/^'.preg_quote($infos['ou'], '/').'/';
      }
    }

    /* Get all invisible departments */
    $ldap->cd($config->current['BASE']);
    $res = $ldap->search('(&(objectClass=organizationalUnit)(!(objectClass=gosaDepartment)))', array('ou','description','dn'));
    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    }
    while ($attrs = $ldap->fetch()) {
      $attrs['checked'] = FALSE;
      $attrs['before']  = '';
      $attrs['after']   = '';

      /* Set objects to selected, that were selected before reload */
      if (isset($old[base64_encode($attrs['dn'])])) {
        $attrs['checked'] = $old[base64_encode($attrs['dn'])]['checked'];
      }
      $this->orgUnits_toMigrate[base64_encode($attrs['dn'])] = $attrs;
    }

    /* Filter returned list of departments and ensure that
     *  FusionDirectory internal departments will not be listed
     */
    foreach ($this->orgUnits_toMigrate as $key => $attrs) {
      $dn   = $attrs['dn'];
      $skip = FALSE;

      foreach ($skip_dns as $skip_dn) {
        if (preg_match($skip_dn, $dn)) {
          $skip = TRUE;
          break;
        }
      }
      if ($skip) {
        unset($this->orgUnits_toMigrate[$key]);
      }
    }

    /* If we have no invisible departments found
     *  tell the user that everything is ok
     */
    if (count($this->orgUnits_toMigrate) == 0) {
      return '';
    } else {
      throw new CheckFailedException(
        '<font style="color:#FFA500">'._("Warning").'</font>',
        sprintf(_('Found %s department(s) that will not be visible in FusionDirectory.'), count($this->orgUnits_toMigrate)).
        '&nbsp;'.$checkobj->submit()
      );
      /* TODO: maybe warnings should be an other kind of exception? */
    }
  }

  function check_orgUnits_migrate(&$checkobj)
  {
    $this->check_multipleGeneric_migrate($checkobj, array('title' => _('Department migration')));
  }

  function check_orgUnits_migrate_refresh(&$checkobj)
  {
    return $this->check_multipleGeneric_migrate_refresh($checkobj, array('title' => _('Department migration')));
  }

  function check_orgUnits_migrate_confirm(&$checkobj, $only_ldif)
  {
    return $this->check_multipleGeneric_migrate_confirm(
      $checkobj,
      array('gosaDepartment'),
      array('description' => 'FusionDirectory department'),
      $only_ldif
    );
  }

  /* Check if there are uidNumbers which are used more than once */
  function check_uidNumber(&$checkobj)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $ldap->cd($config->current['BASE']);
    $res = $ldap->search("(&(objectClass=posixAccount)(uidNumber=*))", array("dn","uidNumber"));
    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    }

    $this->check_uidNumbers = array();
    $tmp = array();
    while ($attrs = $ldap->fetch()) {
      $tmp[$attrs['uidNumber'][0]][] = $attrs;
    }

    foreach ($tmp as $entries) {
      if (count($entries) > 1) {
        foreach ($entries as $entry) {
          $this->check_uidNumbers[base64_encode($entry['dn'])] = $entry;
        }
      }
    }

    if ($this->check_uidNumbers) {
      throw new CheckFailedException(
        "<div style='color:#F0A500'>"._("Warning")."</div>",
        sprintf(_('Found %s duplicate values for attribute "uidNumber".'), count($this->check_uidNumbers))
      );
    } else {
      return '';
    }
  }

  /* Check if there are duplicated gidNumbers present in ldap */
  function check_gidNumber(&$checkobj)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $ldap->cd($config->current['BASE']);
    $res = $ldap->search("(&(objectClass=posixGroup)(gidNumber=*))", array("dn","gidNumber"));
    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    }

    $this->check_gidNumbers = array();
    $tmp = array();
    while ($attrs = $ldap->fetch()) {
      $tmp[$attrs['gidNumber'][0]][] = $attrs;
    }

    foreach ($tmp as $entries) {
      if (count($entries) > 1) {
        foreach ($entries as $entry) {
          $this->check_gidNumbers[base64_encode($entry['dn'])] = $entry;
        }
      }
    }

    if ($this->check_gidNumbers) {
      throw new CheckFailedException(
        "<div style='color:#F0A500'>"._("Warning")."</div>",
        sprintf(_('Found %s duplicate values for attribute "gidNumber".'), count($this->check_gidNumbers))
      );
    } else {
      return '';
    }
  }
}
?>
