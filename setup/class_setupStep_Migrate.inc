<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007  Fabian Hickert
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

/****************
 * FUNCTIONS

Step_Migrate                - Constructor.
update_strings              - Used to update the displayed step informations.
initialize_checks           - Initialize migration steps.
check_ldap_permissions      - Check if the used admin account has full access to the ldap database.
check_gosaAccounts          - Check if there are users without the required objectClasses.
migrate_gosaAccounts        - Migrate selected users to FusionDirectory user accounts.
check_organizationalUnits   - Check if there are departments, that are not visible for FusionDirectory
migrate_organizationalUnits - Migrate selected departments
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
    /* TODO rerun depending tests? (done by hand for now) */
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

  /* Invisible users */
  var $users_to_migrate = array();

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
    $checks = array(
      'base'                  => new StepMigrateCheck($this, 'base',                  _('Checking for root object')),
      'baseOC'                => new StepMigrateCheck($this, 'baseOC',                _('Inspecting object classes in root object')),
      'permissions'           => new StepMigrateCheck($this, 'permissions',           _('Checking permission for LDAP database')),
      'gosaAccounts'          => new StepMigrateCheck($this, 'gosaAccounts',          _('Checking for invisible users')),
      'adminAccount'          => new StepMigrateCheck($this, 'adminAccount',          _('Checking for super administrator')),
      'defaultACLs'           => new StepMigrateCheck($this, 'defaultACLs',           _('Checking for default ACL roles and groups')),
      'outsideUsers'          => new StepMigrateCheck($this, 'outsideUsers',          _('Checking for users outside the people tree')),
      'outsideGroups'         => new StepMigrateCheck($this, 'outsideGroups',         _('Checking for groups outside the groups tree')),
      'organizationalUnits'   => new StepMigrateCheck($this, 'organizationalUnits',   _('Checking for invisible departments')),
      'uidNumber'             => new StepMigrateCheck($this, 'uidNumber',             _('Checking for duplicated UID numbers')),
      'gidNumber'             => new StepMigrateCheck($this, 'gidNumber',             _('Checking for duplicate GID numbers')),
    );

    $this->checks = $checks;
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

  function check_base_create (&$check)
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

  function check_baseOC_migrate (&$check)
  {
    $this->openDialog(new StepMigrateDialog($check, 'setup_migrate_baseOC.tpl', $this->rootOC_details));
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
    $old    = $this->users_to_migrate;
    $this->users_to_migrate = array();

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
        $this->users_to_migrate[base64_encode($attrs['dn'])] = $attrs;
      }
    }

    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    } elseif (count($this->users_to_migrate) == 0) {
      /* No invisible */
      return '';
    } else {
      throw new CheckFailedException(
        "<div style='color:#F0A500'>"._("Warning")."</div>",
        sprintf(
          _('Found %s user(s) that will not be visible in FusionDirectory or which are incomplete.'),
          count($this->users_to_migrate)
        ).$checkobj->submit()
      );
    }
  }

  function check_gosaAccounts_migrate (&$check)
  {
    /* Fix displayed dn syntax */
    $tmp = $this->users_to_migrate;
    foreach ($tmp as $key => $data) {
      $tmp[$key]['dn'] = LDAP::fix($data['dn']);
    }
    $this->openDialog(new StepMigrateDialog($check, 'setup_migrate_gosaAccounts.tpl', $tmp));
  }

  function check_gosaAccounts_migrate_refresh (&$check)
  {
    if (isset($_POST['dialog_showchanges'])) {
      /* Show changes */
      $this->check_gosaAccounts_migrate_confirm($check, TRUE);
    } else {
      /* Hide changes */
      $check->run();
    }
    /* Fix displayed dn syntax */
    $tmp = $this->users_to_migrate;
    foreach ($tmp as $key => $data) {
      $tmp[$key]['dn'] = LDAP::fix($data['dn']);
    }
    return $tmp;
  }

  /* Start user account migration */
  function check_gosaAccounts_migrate_confirm(&$check, $only_ldif = FALSE)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    /* Add gosaAccount objectClass to the selected users */
    foreach ($this->users_to_migrate as $key => $dep) {
      if ($dep['checked']) {

        /* Get old objectClasses */
        $ldap->cat($dep['dn'], array("objectClass"));
        $attrs      = $ldap->fetch();

        /* Create new objectClass array */
        $new_attrs  = array();
        $new_attrs['objectClass'] = array("gosaAccount","inetOrgPerson","organizationalPerson","person");
        for ($i = 0; $i < $attrs['objectClass']['count']; $i++) {
          if (!in_array_ics($attrs['objectClass'][$i], $new_attrs['objectClass'])) {
            $new_attrs['objectClass'][]   = $attrs['objectClass'][$i];
          }
        }

        /* Set info attributes for current object,
         *  or write changes to the ldap database
         */
        if ($only_ldif) {
          $this->users_to_migrate[$key]['before'] = $this->array_to_ldif($attrs);
          $this->users_to_migrate[$key]['after']  = $this->array_to_ldif($new_attrs);
        } else {
          $ldap->cd($attrs['dn']);
          if (!$ldap->modify($new_attrs)) {
            msg_dialog::display(_("Migration error"), sprintf(_("Cannot migrate department '%s':")."<br><br><i>%s</i>", LDAP::fix($attrs['dn']), $ldap->get_error()), ERROR_DIALOG);
            return FALSE;
          }
        }
      }
    }
    return TRUE;
  }

  /* Check Acls if there is at least one object with acls defined */
  function check_adminAccount(&$checkobj)
  {
    global $config;

    /* Reset settings */
    $FD_1_0_8_found                 = FALSE;
    $this->migrate_users            = array();
    $this->acl_migrate_dialog       = FALSE;
    $this->migrate_acl_base_entry   = "";
    $valid_admin                    = FALSE;

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
                  if (!$valid_admin) {
                    $valid_admin = $member;
                  }
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
                        if (!$valid_admin) {
                          $valid_admin = $rusers[$group_attrs['memberUid'][$e]];
                        }
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

      /* Try to find an old FD 1.0.7 administrator account that may be migrated */
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
                    if (!$valid_admin) {
                      $valid_admin = $member;
                    }
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
                          if (!$valid_admin) {
                            $valid_admin = $rusers[$group_attrs['memberUid'][$e]];
                          }
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
                        if (!$valid_admin) {
                          $valid_admin = $member;
                        }
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
                              if (!$valid_admin) {
                                $valid_admin = $rusers[$group_attrs['memberUid'][$e]];
                              }
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
        $this->valid_admin = $valid_admin;
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
     * Get all gosaDepartments to be able to
     *  validate correct ldap tree position of every single user
     ***********/
    $valid_deps = array();
    $valid_deps['/'] = $config->current['BASE'];
    $ldap->search("(&(objectClass=gosaDepartment)(ou=*))", array("dn","ou"));
    while ($attrs = $ldap->fetch()) {
      $valid_deps[] = $attrs['dn'];
    }

    /***********
     * Search for all users
     ***********/
    $res = $ldap->search("(&(objectClass=gosaAccount)(!(uid=*$)))", array("dn"));
    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    }

    /***********
     * Check if returned users are within a valid GOsa department. (peopleou,gosaDepartment,base)
     ***********/
    $this->outside_users = array();
    $people_ou = trim(get_ou('userRDN'));

    while ($attrs = $ldap->fetch()) {
      $people_db_base = preg_replace("/^[^,]+,".preg_quote($people_ou, '/')."/i", "", $attrs['dn']);

      /* Check if entry is not an addressbook only user
       *  and verify that he is in a valid department
       */
      if ( !preg_match("/dc=addressbook,/", $people_db_base) &&
          !in_array($people_db_base, $valid_deps)
         ) {
        $attrs['selected'] = FALSE;
        $attrs['ldif']     = "";
        $this->outside_users[base64_encode($attrs['dn'])] = $attrs;
      }
    }

    if (count($this->outside_users)) {
      throw new CheckFailedException(
        "<div style='color:#F0A500'>"._("Warning")."</div>",
        sprintf(_('Found %s user(s) outside the configured tree "%s".'), count($this->outside_users), $people_ou).
        $checkobj->submit()
      );
    } else {
      return '';
    }
  }

  /* Search for groups outside the group ou */
  function check_outsideGroups(&$checkobj)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $group_ou = get_ou('groupRDN');
    $ldap->cd($config->current['BASE']);

    /***********
     * Get all gosaDepartments to be able to
     *  validate correct ldap tree position of every single user
     ***********/
    $valid_deps = array();
    $valid_deps['/'] = $config->current['BASE'];
    $ldap->search("(&(objectClass=gosaDepartment)(ou=*))", array("dn","ou"));
    while ($attrs = $ldap->fetch()) {
      $valid_deps[] = $attrs['dn'];
    }

    /***********
     * Get all groups
     ***********/
    $res = $ldap->search("(objectClass=posixGroup)", array("dn"));
    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    }

    $this->outside_groups = array();
    $this->groups_list = array();;
    while ($attrs = $ldap->fetch()) {
      $group_db_base = preg_replace("/^[^,]+,".preg_quote($group_ou, '/')."/i", "", $attrs['dn']);

      /* Check if entry is not an addressbook only user
       *  and verify that he is in a valid department
       */
      if ( !preg_match("/".preg_quote("dc=addressbook,", '/')."/", $group_db_base) &&
          !in_array($group_db_base, $valid_deps)
        ) {
        $attrs['selected'] = FALSE;
        $attrs['ldif']     = "";
        $this->outside_groups[base64_encode($attrs['dn'])] = $attrs;
      }
      $this->group_list[] = $attrs['dn'];
    }

    if (count($this->outside_groups)) {
      throw new CheckFailedException(
        "<div style='color:#F0A500'>"._("Warning")."</div>",
        sprintf(_("Found %s groups outside the configured tree '%s'."), count($this->outside_groups), $group_ou).
        '&nbsp;'.$checkobj->submit()
      );
    } else {
      return '';
    }
  }

  /* Check if there are invisible organizational Units */
  function check_organizationalUnits(&$checkobj)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $old                    = $this->deps_to_migrate;
    $this->deps_to_migrate  = array();

    /* TODO use objectTypes and ous for this!!! */
    /* Skip FusionDirectory internal departments */
    $skip_dns = array("/".get_ou('userRDN')."/","/".get_ou('groupRDN')."/","/".get_ou('aclRoleRDN')."/",
        "/^ou=people,/","/^ou=groups,/","/^ou=sudoers,/",
        "/(,|)ou=configs,/","/(,|)ou=systems,/","/(,|)ou=tokens,/",
        "/(,|)ou=apps,/","/(,|)ou=mime,/","/(,|)ou=devices/",
        "/ou=snapshots,/","/(,|)dc=addressbook,/","/^(,|)ou=machineaccounts,/","/^ou=opsi,/","/^ou=structures,/",
        "/(,|)ou=winstations,/","/^ou=hosts,/","/^ou=computers,/","/^ou=idmap,/","/^ou=Idmap,/","/(,|)ou=roles,/");

    /* Get all invisible departments */
    $ldap->cd($config->current['BASE']);
    $res = $ldap->search("(&(objectClass=organizationalUnit)(!(objectClass=gosaDepartment)))", array("ou","description","dn"));
    while ($attrs = $ldap->fetch()) {
      $attrs['checked'] = FALSE;
      $attrs['before']  = "";
      $attrs['after']   = "";

      /* Set objects to selected, that were selected before reload */
      if (isset($old[base64_encode($attrs['dn'])])) {
        $attrs['checked'] = $old[base64_encode($attrs['dn'])]['checked'];
      }
      $this->deps_to_migrate[base64_encode($attrs['dn'])] = $attrs;
    }

    /* Filter returned list of departments and ensure that
     *  FusionDirectory internal departments will not be listed
     */
    foreach ($this->deps_to_migrate as $key => $attrs) {
      $dn = $attrs['dn'];
      $skip = FALSE;

      /* Check if this object is an application release object
          e.g. groups-> application menus.
       */
      if (preg_match("/^.*,[ ]*cn=/", $dn)) {
        $cn_dn = preg_replace("/^.*,[ ]*cn=/", "cn=", $dn);
        if (in_array($cn_dn, $this->group_list)) {
          $skip = TRUE;
        }
      }

      foreach ($skip_dns as $skip_dn) {
        if (preg_match($skip_dn, $dn)) {
          $skip = TRUE;
        }
      }
      if ($skip) {
        unset($this->deps_to_migrate[$key]);
      }
    }

    /* If we have no invisible departments found
     *  tell the user that everything is ok
     */
    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    } elseif (count($this->deps_to_migrate) == 0 ) {
      return '';
    } else {
      throw new CheckFailedException(
        '<font style="color:#FFA500">'._("Warning").'</font>',
        sprintf(_("Found %s department(s) that will not be visible in FusionDirectory."), count($this->deps_to_migrate)).
        '&nbsp;'.$checkobj->submit()
      );
      /* TODO: maybe warnings should be an other kind of exception, saying status to true anyway? */
    }
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

class Step_Migrate_old extends setup_step
{
  var $header_image   = "geticon.php?context=applications&icon=utilities-system-monitor&size=48";
  var $languages      = array();
  var $attributes     = array('valid_admin');
  var $checks         = array();

  /* Department migration attributes */
  var $dep_migration_dialog = FALSE;
  var $deps_to_migrate      = array();
  var $show_details         = FALSE;

  /* Department migration attributes */
  var $users_migration_dialog = FALSE;
  var $users_to_migrate       = array();

  /* Create Acl attributes */
  var $acl_create_dialog    = FALSE;
  var $acl_create_selected  = ""; // Currently selected element, that should receive admin rights
  var $acl_create_changes   = ""; // Contains ldif information about changes
  var $acl_create_confirmed = FALSE;

  /* Checks initialised ? */
  var $checks_initialised = FALSE;

  /* Users outside to people ou */
  var $outside_users        = array();
  var $outside_users_dialog = FALSE;

  /* Users outside to groups ou */
  var $outside_groups        = array();
  var $outside_groups_dialog = FALSE;

  /* check for multiple use of same uidNumber */
  var $check_uidNumbers        = array();
  var $check_uidNumbers_dialog = FALSE;

  /* check for multiple use of same gidNumber */
  var $check_gidNumbers        = array();
  var $check_gidNumbers_dialog = FALSE;

  var $group_list              = array();

  /* Migrable users */
  var $migrate_users = array();
  var $acl_migrate_dialog      = FALSE;
  var $migrate_acl_base_entry  = "";

  /* Root object classes */
  var $rootOC_migrate_dialog = FALSE;
  var $rootOC_details = array();

  /* One valid admin dn */
  var $valid_admin = FALSE;

  /* Defaults ACL roles */
  var $defaultRoles;

  /* Search for groups outside the group ou */
  function search_outside_groups()
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $group_ou = get_ou('groupRDN');
    $ldap->cd($config->current['BASE']);

    /***********
     * Get all gosaDepartments to be able to
     *  validate correct ldap tree position of every single user
     ***********/
    $valid_deps = array();
    $valid_deps['/'] = $config->current['BASE'];
    $ldap->search("(&(objectClass=gosaDepartment)(ou=*))", array("dn","ou"));
    while ($attrs = $ldap->fetch()) {
      $valid_deps[] = $attrs['dn'];
    }

    /***********
     * Get all groups
     ***********/
    $res = $ldap->search("(objectClass=posixGroup)", array("dn"));
    if (!$res) {
      $this->checks['outside_groups']['STATUS']     = FALSE;
      $this->checks['outside_groups']['STATUS_MSG'] = _("LDAP query failed");
      $this->checks['outside_groups']['ERROR_MSG']  = _("Possibly the 'root object' is missing.");
      return FALSE;
    }

    $this->outside_groups = array();
    $this->groups_list = array();;
    while ($attrs = $ldap->fetch()) {
      $group_db_base = preg_replace("/^[^,]+,".preg_quote($group_ou, '/')."/i", "", $attrs['dn']);

      /* Check if entry is not an addressbook only user
       *  and verify that he is in a valid department
       */
      if ( !preg_match("/".preg_quote("dc=addressbook,", '/')."/", $group_db_base) &&
          !in_array($group_db_base, $valid_deps)
        ) {
        $attrs['selected'] = FALSE;
        $attrs['ldif']     = "";
        $this->outside_groups[base64_encode($attrs['dn'])] = $attrs;
      }
      $this->group_list[] = $attrs['dn'];
    }

    if (count($this->outside_groups)) {
      $this->checks['outside_groups']['STATUS']     = FALSE;
      $this->checks['outside_groups']['STATUS_MSG'] = "<div style='color:#F0A500'>"._("Warning")."</div>";
      $this->checks['outside_groups']['ERROR_MSG']  =
        sprintf(_("Found %s groups outside the configured tree '%s'."), count($this->outside_groups), $group_ou);
      $this->checks['outside_groups']['ERROR_MSG']  .= "&nbsp;<input type='submit' name='outside_groups_dialog' value='"._("Move")."...'>";
      return FALSE;
    } else {
      $this->checks['outside_groups']['STATUS']     = TRUE;
      $this->checks['outside_groups']['STATUS_MSG'] = _("Ok");
      $this->checks['outside_groups']['ERROR_MSG']  = "";
      return TRUE;
    }
  }

  /* Search for users outside the people ou */
  function search_outside_users()
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $ldap->cd($config->current['BASE']);

    /***********
     * Get all gosaDepartments to be able to
     *  validate correct ldap tree position of every single user
     ***********/
    $valid_deps = array();
    $valid_deps['/'] = $config->current['BASE'];
    $ldap->search("(&(objectClass=gosaDepartment)(ou=*))", array("dn","ou"));
    while ($attrs = $ldap->fetch()) {
      $valid_deps[] = $attrs['dn'];
    }

    /***********
     * Search for all users
     ***********/
    $res = $ldap->search("(&(objectClass=gosaAccount)(!(uid=*$)))", array("dn"));
    if (!$res) {
      $this->checks['outside_users']['STATUS']      = FALSE;
      $this->checks['outside_users']['STATUS_MSG']  = _("LDAP query failed");
      $this->checks['outside_users']['ERROR_MSG']   = _("Possibly the 'root object' is missing.");
      return FALSE;
    }

    /***********
     * Check if returned users are within a valid GOsa department. (peopleou,gosaDepartment,base)
     ***********/
    $this->outside_users = array();
    $people_ou = trim(get_ou('userRDN'));

    while ($attrs = $ldap->fetch()) {
      $people_db_base = preg_replace("/^[^,]+,".preg_quote($people_ou, '/')."/i", "", $attrs['dn']);

      /* Check if entry is not an addressbook only user
       *  and verify that he is in a valid department
       */
      if ( !preg_match("/dc=addressbook,/", $people_db_base) &&
          !in_array($people_db_base, $valid_deps)
         ) {
        $attrs['selected'] = FALSE;
        $attrs['ldif']     = "";
        $this->outside_users[base64_encode($attrs['dn'])] = $attrs;
      }
    }

    if (count($this->outside_users)) {
      $this->checks['outside_users']['STATUS']      = FALSE;
      $this->checks['outside_users']['STATUS_MSG']  = "<div style='color:#F0A500'>"._("Warning")."</div>";
      $this->checks['outside_users']['ERROR_MSG']   =
        sprintf(_("Found %s user(s) outside the configured tree '%s'."), count($this->outside_users), $people_ou);
      $this->checks['outside_users']['ERROR_MSG']   .= "<input type='submit' name='outside_users_dialog' value='"._("Move")."...'>";
      return FALSE;
    } else {
      $this->checks['outside_users']['STATUS']      = TRUE;
      $this->checks['outside_users']['STATUS_MSG']  = _("Ok");
      $this->checks['outside_users']['ERROR_MSG']   = "";
      return TRUE;
    }
  }

  /* Check if there are invisible organizational Units */
  function check_organizationalUnits()
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $old                    = $this->deps_to_migrate;
    $this->deps_to_migrate  = array();

    /* Skip FusionDirectory internal departments */
    $skip_dns = array("/".get_ou('userRDN')."/","/".get_ou('groupRDN')."/","/".get_ou('aclRoleRDN')."/",
        "/^ou=people,/","/^ou=groups,/","/^ou=sudoers,/",
        "/(,|)ou=configs,/","/(,|)ou=systems,/","/(,|)ou=tokens,/",
        "/(,|)ou=apps,/","/(,|)ou=mime,/","/(,|)ou=devices/",
        "/ou=snapshots,/","/(,|)dc=addressbook,/","/^(,|)ou=machineaccounts,/","/^ou=opsi,/","/^ou=structures,/",
        "/(,|)ou=winstations,/","/^ou=hosts,/","/^ou=computers,/","/^ou=idmap,/","/^ou=Idmap,/","/(,|)ou=roles,/");

    /* Get all invisible departments */
    $ldap->cd($config->current['BASE']);
    $res = $ldap->search("(&(objectClass=organizationalUnit)(!(objectClass=gosaDepartment)))", array("ou","description","dn"));
    while ($attrs = $ldap->fetch()) {
      $attrs['checked'] = FALSE;
      $attrs['before']  = "";
      $attrs['after']   = "";

      /* Set objects to selected, that were selected before reload */
      if (isset($old[base64_encode($attrs['dn'])])) {
        $attrs['checked'] = $old[base64_encode($attrs['dn'])]['checked'];
      }
      $this->deps_to_migrate[base64_encode($attrs['dn'])] = $attrs;
    }

    /* Filter returned list of departments and ensure that
     *  FusionDirectory internal departments will not be listed
     */
    foreach ($this->deps_to_migrate as $key => $attrs) {
      $dn = $attrs['dn'];
      $skip = FALSE;

      /* Check if this object is an application release object
          e.g. groups-> application menus.
       */
      if (preg_match("/^.*,[ ]*cn=/", $dn)) {
        $cn_dn = preg_replace("/^.*,[ ]*cn=/", "cn=", $dn);
        if (in_array($cn_dn, $this->group_list)) {
          $skip = TRUE;
        }
      }

      foreach ($skip_dns as $skip_dn) {
        if (preg_match($skip_dn, $dn)) {
          $skip = TRUE;
        }
      }
      if ($skip) {
        unset($this->deps_to_migrate[$key]);
      }
    }

    /* If we have no invisible departments found
     *  tell the user that everything is ok
     */
    if (!$res) {
      $this->checks['deps_visible']['STATUS']     = FALSE;
      $this->checks['deps_visible']['STATUS_MSG'] = _("LDAP query failed");
      $this->checks['deps_visible']['ERROR_MSG']  = _("Possibly the 'root object' is missing.");
    } elseif (count($this->deps_to_migrate) == 0 ) {
      $this->checks['deps_visible']['STATUS']     = TRUE;
      $this->checks['deps_visible']['STATUS_MSG'] = _("Ok");
      $this->checks['deps_visible']['ERROR_MSG']  = "";
    } else {
      $this->checks['deps_visible']['STATUS']     = TRUE;
      $this->checks['deps_visible']['STATUS_MSG'] = '<font style="color:#FFA500">'._("Warning").'</font>';
      $this->checks['deps_visible']['ERROR_MSG']  = sprintf(_("Found %s department(s) that will not be visible in FusionDirectory."), count($this->deps_to_migrate));
      $this->checks['deps_visible']['ERROR_MSG']  .= "&nbsp;<input type='submit' name='deps_visible_migrate' value='"._("Migrate")."...'>";
    }
  }

  /* Start deparmtment migration */
  function migrate_organizationalUnits($only_ldif = FALSE)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $this->show_details = $only_ldif;

    /* Add gosaDepartment objectClass to each selected entry */
    foreach ($this->deps_to_migrate as $key => $dep) {
      if ($dep['checked']) {

        /* Get current objectClasses */
        $ldap->cat($dep['dn'], array("objectClass","description"));
        $attrs      = $ldap->fetch();

        /* Create new objectClass attribute including gosaDepartment*/
        $new_attrs  = array();
        for ($i = 0; $i < $attrs['objectClass']['count']; $i++) {
          $new_attrs['objectClass'][]   = $attrs['objectClass'][$i];
        }
        $new_attrs['objectClass'][] = "gosaDepartment";

        /* Append description it is missing */
        if (!isset($attrs['description'])) {
          $new_attrs['description'][] = "GOsa department";
        }

        /* Depending on the parameter >only_diff< we save the changes as ldif
         *  or we write our changes directly to the ldap database
         */
        if ($only_ldif) {
          $this->deps_to_migrate[$key]['before'] = $this->array_to_ldif($attrs);
          $this->deps_to_migrate[$key]['after']  = $this->array_to_ldif($new_attrs);
        } else {
          $ldap->cd($attrs['dn']);
          if (!$ldap->modify($new_attrs)) {
            msg_dialog::display(_("Migration error"), sprintf(_("Cannot migrate department '%s':")."<br><br><i>%s</i>", LDAP::fix($attrs['dn']), $ldap->get_error()), ERROR_DIALOG);
            return FALSE;
          }
        }
      }
    }
    return TRUE;
  }

  function create_admin($only_ldif = FALSE)
  {
    global $config;

    /* Reset '' */
    $this->acl_create_changes = "";

    /* Object that should receive admin acls */
    $dn = $this->acl_create_selected;

    /* Get collected configuration settings */
    $ldap = $config->get_ldap_link();

    $ldap->cd($config->current['BASE']);
    $ldap->search("(&(objectClass=gosaRole)(gosaAclTemplate=*:all;cmdrw))", array('dn'));
    if ($attrs = $ldap->fetch()) {
      $roledn = $attrs['dn'];
    } else {
      $roledn = 'cn=admin,'.get_ou('aclRoleRDN').$config->current['BASE'];
      if (!$only_ldif) {
        $ldap->cd($config->current['BASE']);
        $ldap->create_missing_trees(get_ou('aclRoleRDN').$config->current['BASE']);
        $ldap->cd($roledn);
        $attrs_role = array(
          'cn'              => 'admin',
          'description'     => _('Give all rights on all objects'),
          'objectclass'     => array( 'top', 'gosaRole' ),
          'gosaAclTemplate' => '0:all;cmdrw'
        );
        $ldap->add($attrs_role);
        if (!$ldap->success()) {
          msg_dialog::display(_("Migration error"), sprintf(_("Cannot add ACL role '%s':")."<br><br><i>%s</i>", LDAP::fix($roledn), $ldap->get_error()), ERROR_DIALOG);
          return FALSE;
        }
      }
    }

    /* Get current base attributes */
    $ldap->cd($config->current['BASE']);
    $ldap->cat($config->current['BASE'], array("dn","objectClass","gosaAclEntry"));
    $attrs = $ldap->fetch();

    /* Add acls for the selcted user to the base */
    $attrs_new = array();
    $attrs_new['objectClass'] = $attrs['objectClass'];
    unset($attrs_new['objectClass']['count']);
    if (!in_array_ics('gosaAcl', $attrs_new['objectClass'])) {
      $attrs_new['objectClass'][] = 'gosaAcl';
    }

    $acl = "0:subtree:".base64_encode($roledn).':'.base64_encode($dn); //FIXME
    $attrs_new['gosaAclEntry'][] = $acl;
    if (isset($attrs['gosaAclEntry'])) {
      for ($i = 0; $i < $attrs['gosaAclEntry']['count']; $i ++) {

        $prio = preg_replace("/[:].*$/", "", $attrs['gosaAclEntry'][$i]);
        $rest = preg_replace("/^[^:]+/", "", $attrs['gosaAclEntry'][$i]);

        $data = ($prio + 1).$rest;
        $attrs_new['gosaAclEntry'][] = $data;
      }
    }

    if ($only_ldif) {
      $this->acl_create_changes = "\n".($ldap->fix($config->current['BASE']))."\n";
      $this->acl_create_changes .= $this->array_to_ldif($attrs)."\n";
      $this->acl_create_changes .= "\n".($ldap->fix($config->current['BASE']))."\n";
      $this->acl_create_changes .= $this->array_to_ldif($attrs_new);
    } else {
      $ldap->cd($config->current['BASE']);
      $ldap->modify($attrs_new);
      if (!$ldap->success()) {
        msg_dialog::display(_("Migration error"), sprintf(_("Cannot add ACL for user '%s':")."<br><br><i>%s</i>", LDAP::fix($dn), $ldap->get_error()), ERROR_DIALOG);
        return FALSE;
      } else {
        return TRUE;
      }
    }
  }

  function create_admin_user()
  {
    global $config;
    $pw1 = $pw2 = "";
    $uid = "";

    $ldap = $config->get_ldap_link();

    if (isset($_POST['new_user_uid'])) {
      $uid = $_POST['new_user_uid'];
    }
    if (isset($_POST['new_user_password'])) {
      $pw1 = $_POST['new_user_password'];
    }
    if (isset($_POST['new_user_password2'])) {
      $pw2 = $_POST['new_user_password2'];
    }

    $ldap->cd($config->current['BASE']);
    $ldap->search("(uid=".$uid.")");
    if ($ldap->count()) {
      msg_dialog::display(_("Input error"), msgPool::duplicated(_("Uid")), ERROR_DIALOG);
      return FALSE;
    }

    if (empty($pw1) || empty($pw2) | ($pw1 != $pw2)) {
      msg_dialog::display(_("Password error"), _("Provided passwords do not match!"), ERROR_DIALOG);
      return FALSE;
    }

    if (!tests::is_uid($uid) || empty($uid)) {
      msg_dialog::display(_("Input error"), _("Specify a valid user ID!"), ERROR_DIALOG);
      return FALSE;
    }

    /* Get current base attributes */
    $ldap->cd($config->current['BASE']);

    $people_ou = trim(get_ou('userRDN'));

    if ($config->get_cfg_value('accountPrimaryAttribute') == 'cn') {
      $dn = "cn=System Administrator-".$uid.",".$people_ou.$config->current['BASE'];
    } else {
      $dn = "uid=".$uid.",".$people_ou.$config->current['BASE'];
    }

    $hash = passwordMethod::make_hash($pw2, $config->get_cfg_value('passwordDefaultHash', 'ssha'));

    $new_user = array();

    $new_user['objectClass']  = array("top","person","gosaAccount","organizationalPerson","inetOrgPerson");
    $new_user['givenName']    = "System";
    $new_user['sn']           = "Administrator";
    $new_user['cn']           = "System Administrator-".$uid;
    $new_user['uid']          = $uid;
    $new_user['userPassword'] = $hash;

    $ldap->cd($config->current['BASE']);

    $ldap->cat($dn, array("dn"));
    if ($ldap->count()) {
      msg_dialog::display(_("Error"), sprintf(_("Adding an administrator user failed: object '%s' already exists!"), LDAP::fix($dn)), ERROR_DIALOG);
      return FALSE;
    }

    $ldap->create_missing_trees(preg_replace("/^[^,]+,/", "", $dn));
    $ldap->cd($dn);
    $res = $ldap->add($new_user);
    $this->acl_create_selected = $dn;
    $this->create_admin();

    if (!$res) {
      msg_dialog::display(_("LDAP error"), $ldap->get_error(), ERROR_DIALOG);
      return FALSE;
    }

    $this->acl_create_dialog = FALSE;
    $this->check_adminAccount();
    return TRUE;
  }

  function migrate_outside_groups($perform = FALSE)
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);

    /* Check if there was a destination department posted */
    if (isset($_POST['move_group_to'])) {
      $destination_dep = $_POST['move_group_to'];
    } else {
      msg_dialog::display(_("LDAP error"), _("Cannot move users to the requested department!"), ERROR_DIALOG);
      return FALSE;
    }

    foreach ($this->outside_groups as $b_dn => $data) {
      $this->outside_groups[$b_dn]['ldif'] = "";
      if ($data['selected']) {
        $dn = base64_decode($b_dn);
        $d_dn = preg_replace("/,.*$/", ",".base64_decode($destination_dep), $dn);
        if (!$perform) {

          $this->outside_groups[$b_dn]['ldif'] = _("Group will be moved from").":<br>\t".($ldap->fix($dn))."<br>"._("to").":<br>\t".($ldap->fix($d_dn));

          /* Check if there are references to this object */
          $ldap->search("(&(member=".LDAP::prepare4filter($dn).")(|(objectClass=gosaGroupOfNames)(objectClass=groupOfNames)))", array('dn'));
          $refs = "";
          while ($attrs = $ldap->fetch()) {
            $ref_dn = $attrs['dn'];
            $refs .= "<br />\t".$ref_dn;
          }
          if (!empty($refs)) {
            $this->outside_groups[$b_dn]['ldif'] .= "<br /><br /><i>"._("Updating following references too").":</i>".$refs;
          }

        } else {
          $this->move($dn, $d_dn);
        }
      }
    }
  }

  function migrate_outside_users($perform = FALSE)
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);

    /* Check if there was a destination department posted */
    if (isset($_POST['move_user_to'])) {
      $destination_dep = $_POST['move_user_to'];
    } else {
      msg_dialog::display(_("LDAP error"), _("Cannot move users to the requested department!"), ERROR_DIALOG);
      return FALSE;
    }

    foreach ($this->outside_users as $b_dn => $data) {
      $this->outside_users[$b_dn]['ldif'] = "";
      if ($data['selected']) {
        $dn = base64_decode($b_dn);
        $d_dn = preg_replace("/,.*$/", ",".base64_decode($destination_dep), $dn);
        if (!$perform) {
          $this->outside_users[$b_dn]['ldif'] = _("User will be moved from").":<br>\t".($ldap->fix($dn))."<br>"._("to").":<br>\t".($ldap->fix($d_dn));

          /* Check if there are references to this object */
          $ldap->search("(&(member=".LDAP::prepare4filter($dn).")(|(objectClass=gosaGroupOfNames)(objectClass=groupOfNames)))", array('dn'));
          $refs = "";
          while ($attrs = $ldap->fetch()) {
            $ref_dn = $attrs['dn'];
            $refs .= "<br />\t".$ref_dn;
          }
          if (!empty($refs)) {
            $this->outside_users[$b_dn]['ldif'] .= "<br /><br /><i>"._("The following references will be updated").":</i>".$refs;
          }

        } else {
          $this->move($dn, $d_dn);
        }
      }
    }
  }

  function execute()
  {
    global $config;
    /* Initialise checks if this is the first call */
    if (!$this->checks_initialised || isset($_POST['reload'])) {
      $this->initialize_checks();
      $this->checks_initialised = TRUE;
    }

    /*************
     * Groups outside the group ou
     *************/

    if (isset($_POST['outside_groups_dialog_cancel'])) {
      $this->outside_groups_dialog  = FALSE;
      $this->show_details           = FALSE;
      $this->dialog                 = FALSE;
    }

    if (isset($_POST['outside_groups_dialog_whats_done'])) {
      $this->show_details = TRUE;
      $this->migrate_outside_groups(FALSE);
    }

    if (isset($_POST['outside_groups_dialog_refresh'])) {
      $this->show_details = FALSE;
    }

    if (isset($_POST['outside_groups_dialog_perform'])) {
      $this->migrate_outside_groups(TRUE);
      $this->dialog                 = FALSE;
      $this->show_details           = FALSE;
      $this->outside_groups_dialog  = FALSE;
      $this->initialize_checks();
    }

    if (isset($_POST['outside_groups_dialog'])) {
      $this->outside_groups_dialog  = TRUE;
      $this->dialog                 = TRUE;
    }

    if ($this->outside_groups_dialog) {

      /* Fix displayed dn syntax */
      $tmp = $this->outside_groups;
      foreach ($tmp as $key => $data) {
        $tmp[$key]['dn'] = LDAP::fix($data['dn']);
      }

      $smarty = get_smarty();
      $smarty->assign("ous", $this->get_all_group_ous());
      $smarty->assign("method", "outside_groups");
      $smarty->assign("outside_groups", $tmp);
      $smarty->assign("group_details", $this->show_details);
      return $smarty->fetch(get_template_path("setup_migrate.tpl", TRUE, dirname(__FILE__)));
    }

    /*************
     * User outside the people ou
     *************/

    if (isset($_POST['outside_users_dialog_cancel'])) {
      $this->outside_users_dialog = FALSE;
      $this->dialog               = FALSE;
      $this->show_details         = FALSE;
    }

    if (isset($_POST['outside_users_dialog_whats_done'])) {
      $this->show_details = TRUE;
      $this->migrate_outside_users(FALSE);
    }

    if (isset($_POST['outside_users_dialog_perform'])) {
      $this->migrate_outside_users(TRUE);
      $this->initialize_checks();
      $this->dialog               = FALSE;
      $this->show_details         = FALSE;
      $this->outside_users_dialog = FALSE;
    }

    if (isset($_POST['outside_users_dialog_refresh'])) {
      $this->show_details = FALSE;
    }

    if (isset($_POST['outside_users_dialog'])) {
      $this->outside_users_dialog = TRUE;
      $this->dialog               = TRUE;
    }

    if ($this->outside_users_dialog) {

      /* Fix displayed dn syntax */
      $tmp = $this->outside_users;
      foreach ($tmp as $key => $data) {
        $tmp[$key]['dn'] = LDAP::fix($data['dn']);
      }

      $smarty = get_smarty();
      $smarty->assign("ous", $this->get_all_people_ous());
      $smarty->assign("method", "outside_users");
      $smarty->assign("outside_users", $tmp);
      $smarty->assign("user_details", $this->show_details);
      return $smarty->fetch(get_template_path("setup_migrate.tpl", TRUE, dirname(__FILE__)));
    }

    /*************
     * Root object check
     *************/

    if (isset($_POST['retry_root_create'])) {

      $state = $this->checks['root']['STATUS'];
      $this->checkBase(FALSE);
      if ($state != $this->checks['root']['STATUS']) {
        $this->initialize_checks();
      }
    }

    /*************
     * Root object class check
     *************/

    if (isset($_POST['root_add_objectclasses'])) {
      $this->rootOC_migrate_dialog  = TRUE;
      $this->dialog                 = TRUE;
    }
    if (isset($_POST['rootOC_dialog_cancel'])) {
      $this->rootOC_migrate_dialog  = FALSE;
      $this->dialog                 = FALSE;
    }
    if (isset($_POST['rootOC_migrate_start'])) {
      if ($this->checkBaseOC(FALSE)) {
        $this->checkBaseOC(); // Update overview info
        $this->dialog                 = FALSE;
        $this->rootOC_migrate_dialog  = FALSE;
      }
    }

    if ($this->rootOC_migrate_dialog) {
      $smarty = get_smarty();
      $smarty->assign("details", $this->rootOC_details);
      $smarty->assign("method", "rootOC_migrate_dialog");
      return $smarty->fetch(get_template_path("setup_migrate.tpl", TRUE, dirname(__FILE__)));
    }

    /*************
     * Administrator Account -- Migrate/Create
     *************/

    if (isset($_POST['retry_acls'])) {
      $this->check_adminAccount();
    }

    /* Dialog handling */
    if (isset($_POST['create_acls'])) {
      $this->acl_create_dialog  = TRUE;
      $this->dialog             = TRUE;
    }

    if (isset($_POST['migrate_acls'])) {
      $this->acl_migrate_dialog = TRUE;
      $this->dialog             = TRUE;
    }

    if (isset($_POST['create_acls_cancel']) || isset($_POST['migrate_acls_cancel'])) {
      $this->acl_create_dialog  = FALSE;
      $this->acl_migrate_dialog = FALSE;
      $this->dialog             = FALSE;
      $this->show_details       = FALSE;
    }

    /* Account creation */
    if (isset($_POST['create_acls_create'])) {
      $this->create_admin(TRUE);
    }

    if (isset($_POST['create_admin_user'])) {
      if ($this->create_admin_user()) {
        $this->dialog       = FALSE;
        $this->show_details = FALSE;
      }
    }

    if (isset($_POST['root_add_defaultroles'])) {
      $this->insert_defaultRoles();
      $this->check_defaultACLs();
    }

    /* Add admin acls for the selected users to the ldap base */
    if ($this->acl_migrate_dialog && isset($_POST['migrate_admin_user'])) {

      /* Update ldap and reload check infos */
      $this->migrate_selected_admin_users();
      $this->dialog             = FALSE;
      $this->acl_migrate_dialog = FALSE;

    } elseif ($this->acl_migrate_dialog) {

      /* Display admin migration dialog */
      $this->migrate_users();
      $smarty = get_smarty();

      /* Do we have to display the changes */
      $details = isset($_POST['details']) && $_POST['details'];
      if (isset($_POST['migrate_acls_show_changes'])) {
        $details = TRUE;
      } elseif (isset($_POST['migrate_acls_hide_changes'])) {
        $details = FALSE;
      }

      $smarty->assign("migrate_acl_base_entry", $this->migrate_acl_base_entry);
      $smarty->assign("details", $details);
      $smarty->assign("method", "migrate_acls");
      $smarty->assign("migrateable_users", $this->migrate_users);
      return $smarty->fetch(get_template_path("setup_migrate.tpl", TRUE, dirname(__FILE__)));
    }

    if ($this->acl_create_dialog) {
      $smarty = get_smarty();
      $uid = "fd-admin";
      if (isset($_POST['new_user_uid'])) {
        $uid = $_POST['new_user_uid'];
      }
      $smarty->assign("new_user_uid", $uid);
      $smarty->assign("new_user_password", @$_POST['new_user_password']);
      $smarty->assign("new_user_password2", @$_POST['new_user_password2']);
      $smarty->assign("method", "create_acls");
      $smarty->assign("acl_create_selected", $this->acl_create_selected);
      $smarty->assign("what_will_be_done_now", $this->acl_create_changes);
      return $smarty->fetch(get_template_path("setup_migrate.tpl", TRUE, dirname(__FILE__)));
    }

    /*************
     * User Migration handling
     *************/

    /* Refresh list of deparments */
    if (isset($_POST['users_visible_migrate_refresh'])) {
      $this->check_gosaAccounts();
    }

    /* Open migration dialog */
    if (isset($_POST['users_visible_migrate'])) {
      $this->show_details           = FALSE;
      $this->users_migration_dialog = TRUE;
      $this->dialog                 = TRUE;
    }

    /* Close migration dialog */
    if (isset($_POST['users_visible_migrate_close'])) {
      $this->users_migration_dialog = FALSE;
      $this->dialog                 = FALSE;
      $this->show_details           = FALSE;
    }

    /* Start migration */
    if (isset($_POST['users_visible_migrate_migrate'])) {
      if ($this->migrate_gosaAccounts()) {
        $this->initialize_checks();
        $this->dialog                 = FALSE;
        $this->show_details           = FALSE;
        $this->users_migration_dialog = FALSE;
      }
    }

    /* Start migration */
    if (isset($_POST['users_visible_migrate_whatsdone'])) {
      $this->migrate_gosaAccounts(TRUE);
    }

    /* Display migration dialog */
    if ($this->users_migration_dialog) {

      /* Fix displayed dn syntax */
      $tmp = $this->users_to_migrate;
      foreach ($tmp as $key => $data) {
        $tmp[$key]['dn'] = LDAP::fix($data['dn']);
      }

      $smarty = get_smarty();
      $smarty->assign("users_to_migrate", $tmp);
      $smarty->assign("method", "migrate_users");
      $smarty->assign("user_details", $this->show_details);
      return $smarty->fetch(get_template_path("setup_migrate.tpl", TRUE, dirname(__FILE__)));
    }

    /*************
     * Department Migration handling
     *************/

    /* Refresh list of deparments */
    if (isset($_POST['deps_visible_migrate_refresh'])) {
      $this->check_organizationalUnits();
      $this->show_details = FALSE;
    }

    /* Open migration dialog */
    if (isset($_POST['deps_visible_migrate'])) {
      $this->dep_migration_dialog = TRUE;
      $this->dialog               = TRUE;
    }

    /* Close migration dialog */
    if (isset($_POST['deps_visible_migrate_close'])) {
      $this->dep_migration_dialog = FALSE;
      $this->dialog               = FALSE;
      $this->show_details         = FALSE;
    }

    /* Start migration */
    if (isset($_POST['deps_visible_migrate_migrate'])) {
      if ($this->migrate_organizationalUnits()) {
        $this->check_organizationalUnits();
        $this->show_details         = FALSE;
        $this->dialog               = FALSE;
        $this->dep_migration_dialog = FALSE;
      }
    }

    /* Start migration */
    if (isset($_POST['deps_visible_migrate_whatsdone'])) {
      $this->migrate_organizationalUnits(TRUE);
    }

    /* Display migration dialog */
    if ($this->dep_migration_dialog) {
      $smarty = get_smarty();

      /* Fix displayed dn syntax */
      $tmp = $this->deps_to_migrate;
      foreach ($tmp as $key => $data) {
        $tmp[$key]['dn'] = LDAP::fix($data['dn']);
      }

      $smarty->assign("deps_to_migrate", $tmp);
      $smarty->assign("method", "migrate_deps");
      $smarty->assign("deps_details", $this->show_details);
      return $smarty->fetch(get_template_path("setup_migrate.tpl", TRUE, dirname(__FILE__)));
    }

    $smarty = get_smarty();
    $smarty->assign("checks", $this->checks);
    $smarty->assign("method", "default");
    return $smarty->fetch(get_template_path("setup_migrate.tpl", TRUE, dirname(__FILE__)));
  }

  function save_object()
  {
    $this->is_completed = TRUE;

    /* Capture all selected groups from outside_groups_dialog */
    if ($this->outside_groups_dialog) {
      foreach ($this->outside_groups as $dn => $data) {
        if (isset($_POST['select_group_'.$dn])) {
          $this->outside_groups[$dn]['selected'] = TRUE;
        } else {
          $this->outside_groups[$dn]['selected'] = FALSE;
        }
      }
    }

    /* Capture all selected users from outside_users_dialog */
    if ($this->outside_users_dialog) {
      foreach ($this->outside_users as $dn => $data) {
        if (isset($_POST['select_user_'.$dn])) {
          $this->outside_users[$dn]['selected'] = TRUE;
        } else {
          $this->outside_users[$dn]['selected'] = FALSE;
        }
      }
    }

    /* Get "create acl" dialog posts */
    if ($this->acl_create_dialog) {

      if (isset($_POST['create_acls_create_abort'])) {
        $this->acl_create_selected = "";
      }
    }

    /* Get selected departments */
    if ($this->dep_migration_dialog) {
      foreach ($this->deps_to_migrate as $id => $data) {
        if (isset($_POST['migrate_'.$id])) {
          $this->deps_to_migrate[$id]['checked'] = TRUE;
        } else {
          $this->deps_to_migrate[$id]['checked'] = FALSE;
        }
      }
    }

    /* Get selected users */
    if ($this->users_migration_dialog) {
      foreach ($this->users_to_migrate as $id => $data) {
        if (isset($_POST['migrate_'.$id])) {
          $this->users_to_migrate[$id]['checked'] = TRUE;
        } else {
          $this->users_to_migrate[$id]['checked'] = FALSE;
        }
      }
    }
  }

  /* Return ldif information for a
   * given attribute array
   */
  function array_to_ldif($atts)
  {
    $ret = "";
    unset($atts['count']);
    unset($atts['dn']);
    foreach ($atts as $name => $value) {
      if (is_numeric($name)) {
        continue;
      }
      if (is_array($value)) {
        unset($value['count']);
        foreach ($value as $a_val) {
          $ret .= $name.": ". $a_val."\n";
        }
      } else {
        $ret .= $name.": ". $value."\n";
      }
    }
    return preg_replace("/\n$/", "", $ret);
  }

  function get_user_list($class = 'gosaAccount')
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->search("(objectClass=$class)", array("dn"));

    $tmp = array();
    while ($attrs = $ldap->fetch()) {
      $tmp[base64_encode($attrs['dn'])] = LDAP::fix($attrs['dn']);
    }
    return $tmp;
  }

  function get_all_people_ous()
  {
    return $this->get_all_ous('userRDN');
  }

  function get_all_group_ous()
  {
    return $this->get_all_ous('groupRDN');
  }

  function get_all_ous($ou_name)
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ou   = trim(get_ou($ou_name));
    $ou   = preg_replace('/,$/', '', $ou);

    /************
     * If ou is NOT empty
     * Get all valid ous, create one if necessary
     ************/
    if (!empty($ou)) {
      $ldap->cd($config->current['BASE']);
      $ldap->search("($ou)", array('dn'));
      if ($ldap->count() == 0 ) {
        $ldap->create_missing_trees($ou.','.$config->current['BASE']);
      }
      $ldap->search("($ou)", array('dn'));
      $tmp = array();
      while ($attrs = $ldap->fetch()) {
        if (!preg_match('/ou=snapshots,/', $attrs['dn'])) {
          $tmp[base64_encode($attrs['dn'])] = $ldap->fix($attrs['dn']);
        }
      }
    } else {
      /************
       * If ou is empty
       * Get all valid gosaDepartments
       ************/
      $ldap->cd($config->current['BASE']);
      $tmp = array();
      $ldap->search('(&(objectClass=gosaDepartment)(ou=*))', array('dn'));
      $tmp[base64_encode($config->current['BASE'])] = $ldap->fix($config->current['BASE']);
      while ($attrs = $ldap->fetch()) {
        $tmp[base64_encode($attrs['dn'])] = $ldap->fix($attrs['dn']);
      }
    }
    return $tmp;
  }

  function move($source, $destination)
  {
    global $config;
    $ldap = $config->get_ldap_link();

     /* Update object references in gosaGroupOfNames */
    $ogs_to_fix = array();
    $ldap->search('(&(objectClass=gosaGroupOfNames)(member='.@LDAP::prepare4filter($source).'))', array('cn','member'));
    while ($attrs = $ldap->fetch()) {
      $dn = $attrs['dn'];
      $attrs = $this->cleanup_array($attrs);
      $member_new = array($destination);
      foreach ($attrs['member'] as $member) {
        if ($member != $source) {
          $member_new[] = $member;
        }
      }
      $attrs['member'] = $member_new;
      $ogs_to_fix[$dn] = $attrs;
    }

    /* Copy source to destination dn */
    $ldap->cat($source);
    $new_data = $this->cleanup_array($ldap->fetch());
    $ldap->cd($destination);
    $res = $ldap->add($new_data);

    /* Display warning if copy failed */
    if (!$res) {
      msg_dialog::display(_("LDAP error"), sprintf(_("Copy '%s' to '%s' failed:")."<br><br><i>%s</i>", LDAP::fix($source), LDAP::fix($destination), $ldap->get_error()), ERROR_DIALOG);
    } else {
      $res = $ldap->rmDir($source);
      if (!$ldap->success()) {
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $source, LDAP_DEL, get_class()), LDAP_ERROR);
      }

      /* Object is copied, so update its references */
      foreach ($ogs_to_fix as $dn => $data) {
        $ldap->cd($dn);
        $ldap->modify($data);
      }
    }
  }

  /* Cleanup ldap result to be able to write it be to ldap */
  function cleanup_array($attrs)
  {
    foreach ($attrs as $key => $value) {
      if (is_numeric($key) || in_array($key, array("count","dn"))) {
        unset($attrs[$key]);
      }
      if (is_array($value) && isset($value['count'])) {
        unset($attrs[$key]['count']);
      }
    }
    return $attrs;
  }

  function migrate_selected_admin_users()
  {
    global $config;

    /* Updated ui selection */
    $this->migrate_users();

    $ldap = $config->get_ldap_link();

    /* Get current ACL configuration for the ldap base */
    $ldap->cat($config->current['BASE']);
    $base_attrs   = $ldap->fetch();
    $acl_entries  = array();
    $acl_id       = -1;
    if (isset($base_attrs['gosaAclEntry'])) {
      for ($i = 0; $i < $base_attrs['gosaAclEntry']['count']; $i ++) {
        $acl_entries[] = $base_attrs['gosaAclEntry'][$i];
        $cur_id = preg_replace("/^([0-9]*):.*$/", "\\1", $base_attrs['gosaAclEntry'][$i]);
        if ($cur_id > $acl_id) {
          $acl_id = $cur_id;
        }
      }
    }

    /* Append ACLs selected in the migrate admin account dialog */
    foreach ($this->migrate_users as $entry) {
      if ($entry['checked']) {
        $acl_id ++;
        $acl_entries[] = $acl_id.$entry['change'];
      }
    }

    /* Check if the required objectClasses are available */
    $ocs = array();
    for ($i = 0;$i < $base_attrs['objectClass']['count']; $i++) {
      $ocs[] = $base_attrs['objectClass'][$i];
    }
    if (!in_array("gosaACL", $ocs)) {
      $ocs[] = "gosaACL";
    }

    /* Try to write changes */
    if (count($acl_entries)) {
      $new_entry['gosaAclEntry']  = $acl_entries;
      $new_entry['objectClass']   = $ocs;
      $ldap->cd($config->current['BASE']);
      $ldap->modify($new_entry);
      if (!$ldap->success()) {
        $this->checks['acls']['TITLE']      = _("Checking for super administrator");
        $this->checks['acls']['STATUS']     = FALSE;
        $this->checks['acls']['STATUS_MSG'] = _("Failed");
        $this->checks['acls']['ERROR_MSG']  = "<br>".msgPool::ldaperror($config->current['BASE'], $ldap->get_error(), LDAP_MOD);
      } else {
        $this->check_adminAccount();
      }
    }
  }

  /* Collect a list of available FusionDirectory users and groups */
  function migrate_users()
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $ldap->cd($config->current['BASE']);

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

    foreach (array_keys($this->migrate_users) as $id) {
      $this->migrate_users[$id]['checked'] = isset($_POST['migrate_admin_'.$id]);
    }

    /* Try to find an old GOsa 2.5 administrator account that may be migrated */
    if (!count($this->migrate_users)) {
      //FIXME
    }
  }
}
?>
