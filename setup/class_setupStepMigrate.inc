<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007  Fabian Hickert
  Copyright (C) 2011-2019  FusionDirectory

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

setupStepMigrate                - Constructor.
update_strings              - Used to update the displayed step information.
initialize_checks           - Initialize migration steps.
check_ldap_permissions      - Check if the used admin account has full access to the ldap database.
check_accounts              - Check if there are users without the required objectClasses.
migrate_accounts            - Migrate selected users to FusionDirectory user accounts.
check_orgUnits              - Check if there are departments, that are not visible for FusionDirectory
migrate_orgUnits            - Migrate selected departments
check_adminAccount          - Check if there is at least one acl entry available
checkBase                   - Check if there is a root object available

get_user_list               - Get list of available users

create_admin
create_admin_user

readPost                    - Save posts
update                      - Update state
render                      - Generate html output of this plugin
array_to_ldif               - Create ldif output of an ldap result array

 ****************/

class CheckFailedException extends FusionDirectoryException
{
  private $error;

  public function __construct ($msg, $error)
  {
    parent::__construct($msg);
    $this->error = $error;
  }

  public function getError ()
  {
    return $this->error;
  }
}

class StepMigrateDialog implements FusionDirectoryDialog
{
  protected $post_cancel = 'dialog_cancel';
  protected $post_finish = 'dialog_confirm';

  private $infos;
  private $tplfile;
  private $check;

  public function __construct (&$check, $tpl, $infos)
  {
    $this->attribute  = NULL;
    $this->dialog     = NULL;
    $this->infos      = $infos;
    $this->tplfile    = $tpl;
    $this->check      = $check;
  }

  public function readPost ()
  {
    if (isset($_POST[$this->post_cancel])) {
      $this->handleCancel();
    } elseif (isset($_POST[$this->post_finish]) || isset($_GET[$this->post_finish])) {
      $this->handleFinish();
    } elseif (
      isset($_POST['dialog_showchanges']) ||
      isset($_POST['dialog_hidechanges']) ||
      isset($_POST['dialog_refresh'])) {
      $this->infos = $this->check->dialog_refresh();
    }
  }

  public function update(): bool
  {
    if (!isset($this->check)) {
      return FALSE;
    }
  }

  public function render(): string
  {
    $smarty = get_smarty();
    $smarty->assign('infos', $this->infos);
    return $smarty->fetch(get_template_path($this->tplfile, TRUE, dirname(__FILE__)));
  }

  protected function handleFinish ()
  {
    if ($this->check->migrate_confirm()) {
      unset($this->check);
    }
  }

  protected function handleCancel ()
  {
    unset($this->check);
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

  public function __construct ($step, $name, $title)
  {
    $this->name   = $name;
    $this->title  = $title;
    $this->fnc    = 'check_'.$name;
    $this->step   = $step;
  }

  public function run ($fnc = NULL)
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

  public function readPost ()
  {
    if (isset($_POST[$this->name.'_create'])) {
      $createFnc = $this->fnc.'_create';
      $this->step->$createFnc($this);
    } elseif (isset($_POST[$this->name.'_migrate'])) {
      $migrateFnc = $this->fnc.'_migrate';
      $this->step->$migrateFnc($this);
    }
  }

  public function submit ($value = NULL, $id = 'migrate')
  {
    if ($value === NULL) {
      $value = _('Migrate');
    }
    return '<input type="submit" name="'.$this->name.'_'.$id.'" value="'.$value.'"/>';
  }

  public function migrate_confirm ()
  {
    $migrateConfirmFnc = $this->fnc.'_migrate'.'_confirm';
    $res = $this->step->$migrateConfirmFnc($this);
    if ($res) {
      $this->run();
    }
    return $res;
  }

  public function dialog_refresh ()
  {
    $migrateRefreshFnc = $this->fnc.'_migrate'.'_refresh';
    return $this->step->$migrateRefreshFnc($this);
  }
}

class setupStepMigrate extends setupStep
{
  var $header_image   = "geticon.php?context=applications&icon=utilities-system-monitor&size=48";

  /* Root object classes */
  var $rootOC_details = [];

  /* Entries needing migration */
  protected $orgUnits_toMigrate           = [];
  protected $accounts_toMigrate           = [];
  protected $outsideUsers_toMigrate       = [];
  protected $outsideOGroups_toMigrate     = [];
  protected $outsidePosixGroups_toMigrate = [];

  /* check for multiple use of same uidNumber */
  var $check_uidNumber = [];

  /* check for multiple use of same gidNumber */
  var $check_gidNumber = [];

  /* Defaults ACL roles */
  var $defaultRoles;

  /* Limit of objects to check/migrate at once to avoid timeouts or memory overflow */
  static protected $objectNumberLimit = 5000;

  static function getAttributesInfo (): array
  {
    return [
      'checks' => [
        'class'     => ['fullwidth'],
        'name'      => _('PHP module and extension checks'),
        'template'  => get_template_path("setup_migrate.tpl", TRUE, dirname(__FILE__)),
        'attrs'     => [
          new FakeAttribute('checks')
        ]
      ],
    ];
  }

  function __construct ($parent)
  {
    parent::__construct($parent);
    $this->fill_defaultRoles();
  }

  function update_strings ()
  {
    $this->s_short_name   = _('LDAP inspection');
    $this->s_title        = _('LDAP inspection');
    $this->s_description  = _('Analyze your current LDAP for FusionDirectory compatibility');
  }

  function fill_defaultRoles ()
  {
    $this->defaultRoles = [
      [
        'cn'              => 'manager',
        'description'     => _('Give all rights on users in the given branch'),
        'objectclass'     => ['top', 'gosaRole'],
        'gosaAclTemplate' => '0:user/user;cmdrw,user/posixAccount;cmdrw'
      ],
      [
        'cn'              => 'editowninfos',
        'description'     => _('Allow users to edit their own information (main tab and posix use only on base)'),
        'objectclass'     => ['top', 'gosaRole'],
        'gosaAclTemplate' => '0:user/user;srw,user/posixAccount;srw'
      ],
      [
        'cn'              => 'editownpwd',
        'description'     => _('Allow users to edit their own password (use only on base)'),
        'objectclass'     => ['top', 'gosaRole'],
        'gosaAclTemplate' => '0:user/user;s#userPassword;rw'
      ],
    ];
  }

  function initialize_checks ()
  {
    global $config;
    $config->resetDepartmentCache();

    $checks = [
      'baseOC'          => new StepMigrateCheck($this, 'baseOC',          _('Inspecting object classes in root object')),
      'permissions'     => new StepMigrateCheck($this, 'permissions',     _('Checking permission for LDAP database')),
      'accounts'        => new StepMigrateCheck($this, 'accounts',        _('Checking for invisible users')),
      'adminAccount'    => new StepMigrateCheck($this, 'adminAccount',    _('Checking for super administrator')),
      'defaultACLs'     => new StepMigrateCheck($this, 'defaultACLs',     _('Checking for default ACL roles and groups')),
      'outsideUsers'    => new StepMigrateCheck($this, 'outsideUsers',    _('Checking for users outside the people tree')),
      'outsideOGroups'  => new StepMigrateCheck($this, 'outsideOGroups',  _('Checking for groups outside the groups tree')),
      'orgUnits'        => new StepMigrateCheck($this, 'orgUnits',        _('Checking for invisible departments')),
    ];

    if (class_available('posixAccount')) {
      $checks['outsidePosixGroups'] = new StepMigrateCheck($this, 'outsidePosixGroups', _('Checking for POSIX groups outside the groups tree'));
      $checks['uidNumber']          = new StepMigrateCheck($this, 'uidNumber',          _('Checking for duplicated UID numbers'));
      $checks['gidNumber']          = new StepMigrateCheck($this, 'gidNumber',          _('Checking for duplicated GID numbers'));
    }

    $this->checks = $checks;
  }

  /* Return ldif information for a given attribute array */
  function array_to_ldif ($attrs)
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

  public function readPost ()
  {
    if (ini_get('max_execution_time') < 180) {
      set_time_limit(180);
    }
    $this->is_completed = TRUE;
    parent::readPost();
    if (isset($_POST['reload'])) {
      $this->checks = [];
    }
    foreach ($this->checks as $check) {
      $check->readPost();
    }
  }

  public function update (): bool
  {
    if (ini_get('max_execution_time') < 180) {
      set_time_limit(180);
    }
    if (empty($this->checks)) {
      $this->initialize_checks();
      foreach ($this->checks as $check) {
        $check->run();
      }
    }
    return parent::update();
  }

  /* Check if the root object includes the required object classes, e.g. gosaDepartment is required for ACLs.
   * If the parameter just_check is TRUE, then just check for the OCs.
   * If the Parameter is FALSE, try to add the required object classes.
   */
  function check_baseOC (&$checkobj)
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

    $attrs = $ldap->fetch(TRUE);

    /* Root object doesn't exists */
    if (!in_array("gosaDepartment", $attrs['objectClass'])) {
      $this->rootOC_details = [];
      $mods = [];

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
      /* This allow us to filter it as if it was already migrated */
      $attrs['objectClass'][] = 'gosaDepartment';
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
          $error = new FusionDirectoryLdapError($config->current['BASE'], LDAP_MOD, $ldap->get_error(), $ldap->get_errno());
          $error->display();
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
  function check_permissions (&$checkobj)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    /* Create dummy entry */
    $name       = 'GOsa_setup_text_entry_'.session_id().random_int(0, 999999);
    $dn         = 'ou='.$name.','.$config->current['BASE'];
    $testEntry  = [];

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
    $ldap->cd($dn);
    $res = $ldap->add($testEntry);
    $ldap->cat($dn);
    if (!$ldap->count()) {
      logging::log('error', 'setup/'.get_class($this), $dn, [], $ldap->get_error());
      throw new CheckFailedException(
        _('Failed'),
        sprintf(_('The specified user "%s" does not have full access to your LDAP database.'), $config->current['ADMINDN'])
      );
    }

    /* Try to remove created entry */
    $res = $ldap->rmDir($dn);
    $ldap->cat($dn);
    if ($ldap->count()) {
      logging::log('error', 'setup/'.get_class($this), $dn, [], $ldap->get_error());
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
  function check_accounts (&$checkobj)
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->set_size_limit(static::$objectNumberLimit);

    /* Remember old list of invisible users, to be able to set
     *  the 'html checked' status for the checkboxes again
     */
    $old                      = $this->accounts_toMigrate;
    $this->accounts_toMigrate = [];

    /* Get all invisible users */
    $ldap->cd($config->current['BASE']);
    $res = $ldap->search(
      '(&'.
        '(|'.
          '(objectClass=posixAccount)'.
          '(objectClass=person)'.
          '(objectClass=OpenLDAPperson)'.
        ')'.
        '(!(objectClass=inetOrgPerson))'.
        '(uid=*)'.
        '(!(uid=*$))'.
      ')',
      ['objectClass','sn','givenName','cn','uid']
    );
    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    }
    $sizeLimitHit   = $ldap->hitSizeLimit();
    $accountsCount  = $ldap->count();

    while ($attrs = $ldap->fetch(TRUE)) {
      $base = preg_replace('/^[^,]+,/', '', $attrs['dn']);

      /* Build groupid depending on base and objectClasses */
      $groupid = md5($base.implode('', $attrs['objectClass']));

      if (!isset($this->accounts_toMigrate[$groupid])) {
        $this->accounts_toMigrate[$groupid] = [
          /* Set objects to selected, that were selected before reload */
          'checked' => ($old[$groupid]['checked'] ?? FALSE),
          'objects' => [],
          'base'    => $base,
          'classes' => $attrs['objectClass'],
        ];
      }

      $attrs['before']  = '';
      $attrs['after']   = '';

      $this->accounts_toMigrate[$groupid]['objects'][base64_encode($attrs['dn'])] = $attrs;
    }

    if (count($this->accounts_toMigrate) == 0) {
      /* No invisible */
      return '';
    } else {
      if ($sizeLimitHit) {
        $message = sprintf(
          _('Found more than %d user(s) that will not be visible in FusionDirectory or which are incomplete.'),
          static::$objectNumberLimit
        );
      } else {
        $message = sprintf(
          _('Found %d user(s) that will not be visible in FusionDirectory or which are incomplete.'),
          $accountsCount
        );
      }
      throw new CheckFailedException(
        "<div style='color:#F0A500'>"._("Warning")."</div>",
        $message.$checkobj->submit()
      );
    }
  }

  function check_accounts_migrate (&$checkobj)
  {
    $this->check_multipleGeneric_migrate(
      $checkobj,
      [
        'title'   => _('User migration'),
        'outside' => FALSE,
      ]
    );
  }

  function check_accounts_migrate_refresh (&$checkobj)
  {
    return $this->check_multipleGeneric_migrate_refresh(
      $checkobj,
      [
        'title'   => _('User migration'),
        'outside' => FALSE,
      ]
    );
  }

  function check_accounts_migrate_confirm (&$checkobj, $only_ldif = FALSE)
  {
    return $this->check_multipleGeneric_migrate_confirm(
      $checkobj,
      ['inetOrgPerson','organizationalPerson','person'],
      [],
      $only_ldif
    );
  }

  function check_multipleGeneric_migrate (&$checkobj, $infos)
  {
    $var              = $checkobj->name.'_toMigrate';
    $infos['entries'] = $this->$var;

    $this->openDialog(new StepMigrateDialog($checkobj, 'setup_migrate_accounts.tpl', $infos));
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

    $var              = $checkobj->name.'_toMigrate';
    $infos['entries'] = $this->$var;

    return $infos;
  }

  function check_multipleGeneric_migrate_confirm (&$checkobj, $oc, $mandatory, $only_ldif)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    /* Add objectClasses to the selected entries */
    $var = $checkobj->name.'_toMigrate';
    foreach ($this->$var as $key => &$entry) {
      $entry['checked'] = isset($_POST['migrate_'.$key]);
      if ($entry['checked']) {
        if (isset($entry['objects'])) {
          $objects =& $entry['objects'];
        } else {
          $objects = [&$entry];
        }
        foreach ($objects as &$object) {
          /* Get old objectClasses */
          $ldap->cat($object['dn'], array_merge(['objectClass'], array_keys($mandatory)));
          $attrs = $ldap->fetch(TRUE);

          /* Create new objectClass array */
          $new_attrs  = [];
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
            $object['before'] = $this->array_to_ldif($attrs);
            $object['after']  = $this->array_to_ldif($new_attrs);
          } else {
            $ldap->cd($attrs['dn']);
            if (!$ldap->modify($new_attrs)) {
              $error = new FusionDirectoryError(
                nl2br(sprintf(
                  htmlescape(_("Cannot migrate entry \"%s\":\n\n%s")),
                  htmlescape($attrs['dn']), '<i>'.htmlescape($ldap->get_error()).'</i>'
                ))
              );
              $error->display();
              return FALSE;
            }
          }
        }
        unset($object);
        unset($objects);
      }
    }
    unset($entry);
    return TRUE;
  }

  /* Check Acls if there is at least one object with acls defined */
  function check_adminAccount (&$checkobj)
  {
    global $config;

    /* Establish ldap connection */
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $res = $ldap->cat($config->current['BASE'], ['gosaAclEntry']);

    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    } else {
      $FD_admin_found = FALSE;

      $attrs = $ldap->fetch();

      /* Check if a valid FusionDirectory admin exists
          -> gosaAclEntry for an existing and accessible user.
       */
      $valid_users  = [];
      $valid_groups = [];
      $valid_roles  = [];
      if (isset($attrs['gosaAclEntry'])) {
        $acls = $attrs['gosaAclEntry'];
        for ($i = 0; $i < $acls['count']; $i++) {
          $acl = $acls[$i];
          $tmp = explode(':', $acl);

          if ($tmp[1] == 'subtree') {
            /* Check if acl owner is a valid FusionDirectory user account */
            $ldap->cat(base64_decode($tmp[2]), ['gosaAclTemplate'], '(gosaAclTemplate=*:all;cmdrw)');
            if ($ldap->count()) {
              $members = explode(',', $tmp[3]);
              foreach ($members as $member) {
                $member = base64_decode($member);

                $ldap->cat($member, ['dn','uid','cn','memberUid','roleOccupant','objectClass']);
                if ($member_attrs = $ldap->fetch()) {
                  if (in_array('inetOrgPerson', $member_attrs['objectClass'])) {
                    $valid_users[]  = htmlescape(($member_attrs['uid'][0] ?? $member_attrs['dn']));
                    $FD_admin_found = TRUE;
                  } elseif (in_array('posixGroup', $member_attrs['objectClass'])) {
                    $val_users = [];
                    if (isset($member_attrs['memberUid'])) {
                      for ($e = 0; $e < $member_attrs['memberUid']['count']; $e++) {
                        $ldap->search('(&(objectClass=inetOrgPerson)(uid='.ldap_escape_f($member_attrs['memberUid'][$e]).'))', ['uid','dn']);
                        if ($user_attrs = $ldap->fetch()) {
                          $val_users[] = htmlescape(($user_attrs['uid'][0] ?? $user_attrs['dn']));
                        }
                      }
                    }
                    if (!empty($val_users)) {
                      $valid_groups[] = htmlescape($member_attrs['cn'][0]).'(<i>'.implode(', ', $val_users).'</i>)';
                      $FD_admin_found = TRUE;
                    }
                  } elseif (in_array('organizationalRole', $member_attrs['objectClass'])) {
                    $val_users = [];
                    if (isset($member_attrs['roleOccupant'])) {
                      for ($e = 0; $e < $member_attrs['roleOccupant']['count']; $e ++) {
                        $ldap->cat($member_attrs['roleOccupant'][$e], ['uid','dn'], '(objectClass=inetOrgPerson)');
                        if ($user_attrs = $ldap->fetch()) {
                          $val_users[] = htmlescape(($user_attrs['uid'][0] ?? $user_attrs['dn']));
                        }
                      }
                    }
                    if (!empty($val_users)) {
                      $valid_roles[]  = htmlescape($member_attrs['cn'][0]).'(<i>'.implode(', ', $val_users).'</i>)';
                      $FD_admin_found = TRUE;
                    }
                  }
                }
              }
            }
          }
        }
      }

      /* Print out results */
      if ($FD_admin_found) {
        $str = '';
        if (!empty($valid_users)) {
          $str .= '<b>'._('Users').'</b>:&nbsp;'.implode(', ', $valid_users).'<br/>';
        }
        if (!empty($valid_groups)) {
          $str .= '<b>'._('Groups').'</b>:&nbsp;'.implode(', ', $valid_groups).'<br/>';
        }
        if (!empty($valid_roles)) {
          $str .= '<b>'._('Roles').'</b>:&nbsp;'.implode(', ', $valid_roles).'<br/>';
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

  function check_adminAccount_create (&$checkobj)
  {
    $infos = [
      'uid'       => 'fd-admin',
      'password'  => '',
      'password2' => '',
    ];
    $this->openDialog(new StepMigrateDialog($checkobj, 'setup_migrate_adminAccount.tpl', $infos));
  }

  function check_adminAccount_migrate_confirm (&$checkobj)
  {
    global $config, $ui;

    $ui->setCurrentBase($config->current['BASE']);

    /* Creating role */
    $ldap = $config->get_ldap_link();

    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=gosaRole)(gosaAclTemplate=*:all;cmdrw))', ['dn']);
    if ($attrs = $ldap->fetch()) {
      $roledn = $attrs['dn'];
    } else {
      $tabObject  = objects::create('aclRole');
      $baseObject = $tabObject->getBaseObject();

      $baseObject->cn               = 'admin';
      $baseObject->description      = _('Gives all rights on all objects');
      $baseObject->gosaAclTemplate  = [['all' => ['0' => 'cmdrw']]];

      $tabObject->save();
      $roledn = $tabObject->dn;
    }

    $_POST['dialog_refresh'] = TRUE;

    /* Creating user */
    $tabObject = objects::create('user');
    $tabObject->givenName                 = 'System';
    $tabObject->sn                        = 'Administrator';
    $tabObject->update();
    $errors = $tabObject->save();
    if (!empty($errors)) {
      msg_dialog::displayChecks($errors);
      return FALSE;
    }
    $admindn = $tabObject->dn;

    /* Assigning role */
    $tabObject  = objects::open($config->current['BASE'], 'aclAssignment');
    $baseObject = $tabObject->getBaseObject();

    $assignments = $baseObject->gosaAclEntry;
    array_unshift(
      $assignments,
      [
        'scope'   => 'subtree',
        'role'    => $roledn,
        'members' => [$admindn],
      ]
    );
    $baseObject->gosaAclEntry = $assignments;
    $tabObject->save();

    return TRUE;
  }

  function check_adminAccount_migrate_refresh (&$checkobj)
  {
    return [
      'uid'       => $_POST['uid'],
      'password'  => $_POST['userPassword_password'],
      'password2' => $_POST['userPassword_password2'],
    ];
  }

  /* Check if default roles and groupes have been inserted */
  function check_defaultACLs (&$checkobj)
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
      $ldap->cat($dn, ['dn']);
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

  function check_defaultACLs_migrate (&$checkobj)
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);

    foreach ($this->defaultRoles as $role) {
      $dn = 'cn='.$role['cn'].','.get_ou('aclRoleRDN').$config->current['BASE'];
      $ldap->cat($dn);
      if ($ldap->count() == 0) {
        $ldap->cd($config->current['BASE']);
        try {
          $ldap->create_missing_trees(get_ou('aclRoleRDN').$config->current['BASE']);
        } catch (FusionDirectoryError $error) {
          $error->display();
        }
        $ldap->cd($dn);
        $ldap->add($role);
        if (!$ldap->success()) {
          $error = new FusionDirectoryError(
            nl2br(sprintf(
              htmlescape(_("Cannot add ACL role \"%s\":\n\n%s")),
              htmlescape($dn), '<i>'.htmlescape($ldap->get_error()).'</i>'
            ))
          );
          $error->display();
          return FALSE;
        }
      }
    }
    $checkobj->run();
    return TRUE;
  }

  /* Search for users outside the people ou */
  function check_outsideUsers (&$checkobj)
  {
    list($sizeLimitHit,$count) = $this->check_outsideObjects_generic($checkobj, '(&(objectClass=inetOrgPerson)(!(uid=*$)))', 'userRDN');

    if ($count > 0) {
      if ($sizeLimitHit) {
        $message = sprintf(_('Found more than %d user(s) outside the configured tree "%s".'), static::$objectNumberLimit, trim(get_ou('userRDN')));
      } else {
        $message = sprintf(_('Found %d user(s) outside the configured tree "%s".'), $count, trim(get_ou('userRDN')));
      }
      throw new CheckFailedException(
        "<div style='color:#F0A500'>"._("Warning")."</div>",
        $message.
        $checkobj->submit()
      );
    }
  }

  function check_outsideObjects_generic (&$checkobj, $filter, $ou)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $ldap->cd($config->current['BASE']);

    /***********
     * Check if objects are within a valid department. (peopleou,gosaDepartment,base)
     ***********/
    $sizeLimitHit = FALSE;
    $var          = $checkobj->name.'_toMigrate';
    $this->$var   = [];
    $objects_ou   = trim(get_ou($ou));
    $cookie       = '';
    $count        = 0;

    do {
      if (version_compare(PHP_VERSION, '7.3.0') >= 0) {
        /* 7.3 and newer, use pagination control */
        $res = $ldap->search($filter, ['dn','objectClass'], 'subtree',
          [['oid' => LDAP_CONTROL_PAGEDRESULTS, 'value' => ['size' => 500, 'cookie' => $cookie]]]
        );
        if (!$res) {
          throw new CheckFailedException(
            _('LDAP query failed'),
            _('Possibly the "root object" is missing.')
          );
        }
        try {
          list($errcode, $matcheddn, $errmsg, $referrals, $controls) = $ldap->parse_result();
        } catch (FusionDirectoryException $e) {
          throw new CheckFailedException(
            _('LDAP result parsing failed'),
            $e->getMessage()
          );
        }
        if ($errcode !== 0) {
          throw new CheckFailedException(
            _('LDAP error'),
            $errcode.' - '.ldap_err2str($errcode).(!empty($errmsg) ? ' ('.$errmsg.')' : '')
          );
        }
        if (isset($controls[LDAP_CONTROL_PAGEDRESULTS]['value']['cookie'])) {
          // You need to pass the cookie from the last call to the next one
          $cookie = $controls[LDAP_CONTROL_PAGEDRESULTS]['value']['cookie'];
        } else {
          $cookie = '';
        }
      } else {
        /* fallback to search without pagination */
        $ldap->set_size_limit(static::$objectNumberLimit);
        $res = $ldap->search($filter, ['dn','objectClass']);
        if (!$res) {
          throw new CheckFailedException(
            _('LDAP query failed'),
            _('Possibly the "root object" is missing.')
          );
        }
        if ($ldap->hitSizeLimit()) {
          throw new CheckFailedException(
            _('Sizelimit hit'),
            sprintf(_('Sizelimit of %d hit. Please check this manually'), static::$objectNumberLimit)
          );
        }
      }

      while ($attrs = $ldap->fetch(TRUE)) {
        $object_base  = preg_replace('/^[^,]+,'.preg_quote($objects_ou, '/').'/i', '', $attrs['dn'], 1, $pregCount);
        $base         = preg_replace('/^[^,]+,/', '', $attrs['dn']);

        /* Check if entry is in a valid department */
        if (($pregCount === 0) || !in_array($object_base, $config->getDepartmentList())) {
          /* Build groupid depending on base and objectClasses */
          $groupid = md5($base.implode('', $attrs['objectClass']));

          if (!isset($this->$var[$groupid])) {
            $this->$var[$groupid] = [
              'checked' => FALSE,
              'objects' => [],
              'base'    => $base,
              'classes' => $attrs['objectClass'],
            ];
          }

          $attrs['ldif'] = '';
          $this->$var[$groupid]['objects'][base64_encode($attrs['dn'])] = $attrs;
          if (++$count >= static::$objectNumberLimit) {
            $sizeLimitHit = TRUE;
            break 2;
          }
        }
      }
      // Empty cookie means last page
    } while (!empty($cookie));

    return [$sizeLimitHit, $count];
  }

  function check_outsideUsers_migrate (&$checkobj)
  {
    global $config;
    $this->check_multipleGeneric_migrate(
      $checkobj,
      [
        'title'       => _('Move users into configured user tree'),
        'outside'     => TRUE,
        'ous'         => $config->getDepartmentList(),
        'destination' => (isset($_POST['destination']) ? $_POST['destination'] : ''),
      ]
    );
  }

  function check_outsideUsers_migrate_refresh (&$checkobj)
  {
    global $config;
    return $this->check_multipleGeneric_migrate_refresh(
      $checkobj,
      [
        'title'       => _('Move users into configured user tree'),
        'outside'     => TRUE,
        'ous'         => $config->getDepartmentList(),
        'destination' => (isset($_POST['destination']) ? $_POST['destination'] : ''),
      ]
    );
  }

  function check_outsideUsers_migrate_confirm (&$checkobj, $only_ldif = FALSE, $ou = 'userRDN')
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);

    /* Check if there was a destination department posted */
    if (isset($_POST['destination'])) {
      $destination_dep = get_ou($ou).$_POST['destination'];
    } else {
      $error = new FusionDirectoryError(htmlescape(_('Cannot move entries to the requested department!')));
      $error->display();
      return FALSE;
    }

    $var = $checkobj->name.'_toMigrate';
    foreach ($this->$var as $b_dn => &$entry) {
      $entry['checked'] = isset($_POST['migrate_'.$b_dn]);
      if ($entry['checked']) {
        foreach ($entry['objects'] as &$object) {
          $dn   = $object['dn'];
          $d_dn = preg_replace('/,.*$/', ','.$destination_dep, $dn);
          if ($only_ldif) {
            $object['ldif'] = nl2br(htmlescape(sprintf(_("Entry will be moved from:\n\t%s\nto:\n\t%s"), $dn, $d_dn)));

            /* Check if there are references to this object */
            $ldap->search('(&(member='.ldap_escape_f($dn).')(|(objectClass=gosaGroupOfNames)(objectClass=groupOfNames)))', ['dn']);
            $refs = '';
            while ($attrs = $ldap->fetch()) {
              $ref_dn = $attrs['dn'];
              $refs .= "<br/>\t".$ref_dn;
            }
            if (!empty($refs)) {
              $entry['ldif'] .= '<br/><br/><i>'._('The following references will be updated').':</i>'.$refs;
            }
          } else {
            $object['ldif']    = '';
            $this->move($dn, $d_dn);
          }
        }
        unset($object);
      }
    }
    unset($entry);

    return TRUE;
  }

  /* Search for groups outside the group ou */
  function check_outsideOGroups (&$checkobj)
  {
    list($sizeLimitHit,$count) = $this->check_outsideObjects_generic($checkobj, '(objectClass=groupOfNames)', 'ogroupRDN');

    if ($count > 0) {
      if ($sizeLimitHit) {
        $message = sprintf(_('Found more than %d groups outside the configured tree "%s".'), static::$objectNumberLimit, trim(get_ou('ogroupRDN')));
      } else {
        $message = sprintf(_('Found %d groups outside the configured tree "%s".'), $count, trim(get_ou('ogroupRDN')));
      }
      throw new CheckFailedException(
        '<div style="color:#F0A500">'._('Warning').'</div>',
        $message.
        $checkobj->submit()
      );
    }
  }

  function check_outsideOGroups_migrate (&$checkobj)
  {
    global $config;
    $this->check_multipleGeneric_migrate(
      $checkobj,
      [
        'title'       => _('Move groups into configured groups tree'),
        'outside'     => TRUE,
        'ous'         => $config->getDepartmentList(),
        'destination' => (isset($_POST['destination']) ? $_POST['destination'] : ''),
      ]
    );
  }

  function check_outsideOGroups_migrate_refresh (&$checkobj)
  {
    global $config;
    return $this->check_multipleGeneric_migrate_refresh(
      $checkobj,
      [
        'title'       => _('Move groups into configured groups tree'),
        'outside'     => TRUE,
        'ous'         => $config->getDepartmentList(),
        'destination' => (isset($_POST['destination']) ? $_POST['destination'] : ''),
      ]
    );
  }

  function check_outsideOGroups_migrate_confirm (&$checkobj, $only_ldif = FALSE)
  {
    return $this->check_outsideUsers_migrate_confirm($checkobj, $only_ldif, 'ogroupRDN');
  }

  /* Search for POSIX groups outside the group ou */
  function check_outsidePosixGroups (&$checkobj)
  {
    list($sizeLimitHit,$count) = $this->check_outsideObjects_generic($checkobj, '(objectClass=posixGroup)', 'groupRDN');

    if ($count > 0) {
      if ($sizeLimitHit) {
        $message = sprintf(_('Found more than %d POSIX groups outside the configured tree "%s".'), static::$objectNumberLimit, trim(get_ou('groupRDN')));
      } else {
        $message = sprintf(_('Found %d POSIX groups outside the configured tree "%s".'), $count, trim(get_ou('groupRDN')));
      }
      throw new CheckFailedException(
        '<div style="color:#F0A500">'._('Warning').'</div>',
        $message.
        $checkobj->submit()
      );
    }
  }

  function check_outsidePosixGroups_migrate (&$checkobj)
  {
    global $config;
    $this->check_multipleGeneric_migrate(
      $checkobj,
      [
        'title'       => _('Move POSIX groups into configured groups tree'),
        'outside'     => TRUE,
        'ous'         => $config->getDepartmentList(),
        'destination' => (isset($_POST['destination']) ? $_POST['destination'] : ''),
      ]
    );
  }

  function check_outsidePosixGroups_migrate_refresh (&$checkobj)
  {
    global $config;
    return $this->check_multipleGeneric_migrate_refresh(
      $checkobj,
      [
        'title'       => _('Move POSIX groups into configured groups tree'),
        'outside'     => TRUE,
        'ous'         => $config->getDepartmentList(),
        'destination' => (isset($_POST['destination']) ? $_POST['destination'] : ''),
      ]
    );
  }

  function check_outsidePosixGroups_migrate_confirm (&$checkobj, $only_ldif = FALSE)
  {
    return $this->check_outsideUsers_migrate_confirm($checkobj, $only_ldif, 'groupRDN');
  }

  /* Check if there are invisible organizational Units */
  function check_orgUnits (&$checkobj)
  {
    global $config;
    $ldap = $config->get_ldap_link();
    $ldap->set_size_limit(static::$objectNumberLimit);

    $old                      = $this->orgUnits_toMigrate;
    $this->orgUnits_toMigrate = [];

    /* Skip FusionDirectory internal departments */
    $skip_dns = [
      '/ou=fusiondirectory,'.preg_quote($config->current['BASE']).'$/',
      '/^ou=systems,/',
      '/ou=snapshots,/'
    ];
    foreach (objects::types() as $type) {
      $infos = objects::infos($type);
      if (isset($infos['ou']) && ($infos['ou'] != '')) {
        $skip_dns[] = '/^'.preg_quote($infos['ou'], '/').'/';
      }
    }

    /* Get all invisible departments */
    $ldap->cd($config->current['BASE']);
    $res = $ldap->search('(&(objectClass=organizationalUnit)(!(objectClass=gosaDepartment)))', ['ou','description','dn']);
    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    }
    if ($ldap->hitSizeLimit()) {
      throw new CheckFailedException(
        _('Sizelimit hit'),
        sprintf(_('Sizelimit of %d hit. Please check this manually'), static::$objectNumberLimit)
      );
    }
    $sizeLimitHit = FALSE;

    while ($attrs = $ldap->fetch(TRUE)) {
      foreach ($skip_dns as $skip_dn) {
        /* Filter out FusionDirectory internal departments */
        if (preg_match($skip_dn, $attrs['dn'])) {
          continue 2;
        }
      }

      $attrs['checked'] = FALSE;
      $attrs['before']  = '';
      $attrs['after']   = '';

      /* Set objects to selected, that were selected before reload */
      if (isset($old[base64_encode($attrs['dn'])])) {
        $attrs['checked'] = $old[base64_encode($attrs['dn'])]['checked'];
      }
      $this->orgUnits_toMigrate[base64_encode($attrs['dn'])] = $attrs;
      if (count($this->orgUnits_toMigrate) >= static::$objectNumberLimit) {
        $sizeLimitHit = TRUE;
        break;
      }
    }

    /* If we have no invisible departments found
     *  tell the user that everything is ok
     */
    if (count($this->orgUnits_toMigrate) == 0) {
      return '';
    } else {
      if ($sizeLimitHit) {
        $message = sprintf(_('Found more than %d department(s) that will not be visible in FusionDirectory.'), static::$objectNumberLimit);
      } else {
        $message = sprintf(_('Found %d department(s) that will not be visible in FusionDirectory.'), count($this->orgUnits_toMigrate));
      }
      throw new CheckFailedException(
        '<font style="color:#FFA500">'._('Warning').'</font>',
        $message.
        $checkobj->submit()
      );
    }
  }

  function check_orgUnits_migrate (&$checkobj)
  {
    $this->check_multipleGeneric_migrate(
      $checkobj,
      [
        'title'   => _('Department migration'),
        'outside' => FALSE,
      ]
    );
  }

  function check_orgUnits_migrate_refresh (&$checkobj)
  {
    return $this->check_multipleGeneric_migrate_refresh(
      $checkobj,
      [
        'title'   => _('Department migration'),
        'outside' => FALSE,
      ]
    );
  }

  function check_orgUnits_migrate_confirm (&$checkobj, $only_ldif = FALSE)
  {
    return $this->check_multipleGeneric_migrate_confirm(
      $checkobj,
      ['gosaDepartment'],
      ['description' => 'FusionDirectory department'],
      $only_ldif
    );
  }

  /* Check if there are uidNumbers which are used more than once */
  function check_uidNumber (&$checkobj)
  {
    $this->check_duplicatesGeneric($checkobj, 'posixAccount');
  }

  /* Check if there are duplicated gidNumbers present in ldap */
  function check_gidNumber (&$checkobj)
  {
    $this->check_duplicatesGeneric($checkobj, 'posixGroup');
  }

  /* Generic function to check duplicated values */
  function check_duplicatesGeneric (&$checkobj, $oc)
  {
    global $config;
    $ldap = $config->get_ldap_link();

    $attribute  = $checkobj->name;
    $duplicates = 'check_'.$checkobj->name;

    $ldap->cd($config->current['BASE']);
    $res = $ldap->search('(&(objectClass='.$oc.')('.$attribute.'=*))', ['dn',$attribute]);
    if (!$res) {
      throw new CheckFailedException(
        _('LDAP query failed'),
        _('Possibly the "root object" is missing.')
      );
    }

    $this->$duplicates = [];
    $tmp = [];
    while ($attrs = $ldap->fetch()) {
      $tmp[$attrs[$attribute][0]][] = $attrs['dn'];
    }

    foreach ($tmp as $value => $dns) {
      if (count($dns) > 1) {
        foreach ($dns as $dn) {
          $this->$duplicates[$dn] = $value;
        }
      }
    }

    if (count($this->$duplicates) == 0) {
      return '';
    } else {
      $list = '<ul>';
      foreach ($this->$duplicates as $dn => $value) {
        $list .= '<li>'.$dn.' ('.$value.')</li>';
      }
      $list .= '</ul>';
      throw new CheckFailedException(
        '<div style="color:#F0A500">'._('Warning').'</div>',
        sprintf(_('Found %d duplicate values for attribute "'.$attribute.'":%s'), count($this->$duplicates), $list)
      );
    }
  }
}
