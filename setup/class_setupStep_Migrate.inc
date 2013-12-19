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
check_organizationalUnits   - Check if there are departments, that are not visible for FusionDirectory
migrate_organizationalUnits - Migrate selected departments
check_administrativeAccount - Check if there is at least one acl entry available
checkBase                   - Check if there is a root object available

get_user_list               - Get list of available users
get_group_list              - Get list of groups

create_admin
create_admin_user

execute                     - Generate html output of this plugin
save_object                 - Save posts
array_to_ldif               - Create ldif output of an ldap result array

 ****************/

class Step_Migrate extends setup_step
{
  var $languages      = array();
  var $attributes     = array('valid_admin');
  var $header_image   = "images/setup/migrate.png";
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

  /* Device migration */
  var $device_dialog         = FALSE;
  var $device                = array();

  /* Service migration */
  var $service_dialog         = FALSE;
  var $service                = array();

  /* Group menus */
  var $menu_dialog           = FALSE;
  var $menu                  = array();

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

  function __construct()
  {
    $this->update_strings();
  }

  function update_strings()
  {
    $this->s_title      = _("LDAP inspection");
    $this->s_title_long = _("LDAP inspection");
    $this->s_info       = _("Analyze your current LDAP for FusionDirectory compatibility");
  }

  function initialize_checks()
  {
    $this->checks = array();
    $this->checks['root']['TITLE']      = _("Checking for root object");
    $this->checks['root']['STATUS']     = FALSE;
    $this->checks['root']['STATUS_MSG'] = "";
    $this->checks['root']['ERROR_MSG']  = "";
    $this->checkBase();

    $this->checks['rootOC']['TITLE']      = _("Inspecting object classes in root object");
    $this->checks['rootOC']['STATUS']     = FALSE;
    $this->checks['rootOC']['STATUS_MSG'] = "";
    $this->checks['rootOC']['ERROR_MSG']  = "";
    $this->checkBaseOC();

    $this->checks['permissions']['TITLE']       = _("Checking permission for LDAP database");
    $this->checks['permissions']['STATUS']      = FALSE;
    $this->checks['permissions']['STATUS_MSG']  = "";
    $this->checks['permissions']['ERROR_MSG']   = "";
    $this->check_ldap_permissions();

    $this->checks['deps_visible']['TITLE']      = _("Checking for invisible departments");
    $this->checks['deps_visible']['STATUS']     = FALSE;
    $this->checks['deps_visible']['STATUS_MSG'] = "";
    $this->checks['deps_visible']['ERROR_MSG']  = "";

    $this->checks['users_visible']['TITLE']       = _("Checking for invisible users");
    $this->checks['users_visible']['STATUS']      = FALSE;
    $this->checks['users_visible']['STATUS_MSG']  = "";
    $this->checks['users_visible']['ERROR_MSG']   = "";
    $this->check_gosaAccounts();

    $this->migrate_users = array();
    $this->checks['acls']['TITLE']      = _("Checking for super administrator");
    $this->checks['acls']['STATUS']     = FALSE;
    $this->checks['acls']['STATUS_MSG'] = "";
    $this->checks['acls']['ERROR_MSG']  = "";
    $this->check_administrativeAccount();

    $this->checks['outside_users']['TITLE']       = _("Checking for users outside the people tree");
    $this->checks['outside_users']['STATUS']      = FALSE;
    $this->checks['outside_users']['STATUS_MSG']  = "";
    $this->checks['outside_users']['ERROR_MSG']   = "";
    $this->search_outside_users();

    $this->checks['outside_groups']['TITLE']      = _("Checking for groups outside the groups tree");
    $this->checks['outside_groups']['STATUS']     = FALSE;
    $this->checks['outside_groups']['STATUS_MSG'] = "";
    $this->checks['outside_groups']['ERROR_MSG']  = "";
    $this->search_outside_groups();
    $this->check_organizationalUnits();

    $this->checks['uidNumber_usage']['TITLE']       = _("Checking for duplicated UID numbers");
    $this->checks['uidNumber_usage']['STATUS']      = FALSE;
    $this->checks['uidNumber_usage']['STATUS_MSG']  = "";
    $this->checks['uidNumber_usage']['ERROR_MSG']   = "";
    $this->check_uidNumber();

    $this->checks['gidNumber_usage']['TITLE']       = _("Checking for duplicate GID numbers");
    $this->checks['gidNumber_usage']['STATUS']      = FALSE;
    $this->checks['gidNumber_usage']['STATUS_MSG']  = "";
    $this->checks['gidNumber_usage']['ERROR_MSG']   = "";
    $this->check_gidNumber();

    $this->checks['old_style_devices']['TITLE']       = _("Checking for old style USB devices");
    $this->checks['old_style_devices']['STATUS']      = FALSE;
    $this->checks['old_style_devices']['STATUS_MSG']  = "";
    $this->checks['old_style_devices']['ERROR_MSG']   = "";
    $this->check_usb_devices();

    $this->checks['old_style_services']['TITLE']      = _("Checking for old services that have to be migrated");
    $this->checks['old_style_services']['STATUS']     = FALSE;
    $this->checks['old_style_services']['STATUS_MSG'] = "";
    $this->checks['old_style_services']['ERROR_MSG']  = "";
    $this->check_services();

    $this->checks['old_style_menus']['TITLE']       = _("Checking for old style application menus");
    $this->checks['old_style_menus']['STATUS']      = FALSE;
    $this->checks['old_style_menus']['STATUS_MSG']  = "";
    $this->checks['old_style_menus']['ERROR_MSG']   = "";
    $this->check_menus();
  }


  /* Check if there are uidNumbers which are used more than once */
  function check_uidNumber()
  {
    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();

    $ldap->cd($cv['base']);
    $res = $ldap->search("(&(objectClass=posixAccount)(uidNumber=*))", array("dn","uidNumber"));
    if (!$res) {
      $this->checks['uidNumber_usage']['STATUS']      = FALSE;
      $this->checks['uidNumber_usage']['STATUS_MSG']  = _("LDAP query failed");
      $this->checks['uidNumber_usage']['ERROR_MSG']   = _("Possibly the 'root object' is missing.");
      return FALSE;
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
      $this->checks['uidNumber_usage']['STATUS']      = FALSE;
      $this->checks['uidNumber_usage']['STATUS_MSG']  = "<div style='color:#F0A500'>"._("Warning")."</div>";
      $this->checks['uidNumber_usage']['ERROR_MSG']   =
        sprintf(_("Found %s duplicate values for attribute 'uidNumber'."), count($this->check_uidNumbers));
      return FALSE;
    } else {
      $this->checks['uidNumber_usage']['STATUS']      = TRUE;
      $this->checks['uidNumber_usage']['STATUS_MSG']  = _("Ok");
      $this->checks['uidNumber_usage']['ERROR_MSG']   = "";
      return TRUE;
    }
  }


  /* Check if there are duplicated gidNumbers present in ldap */
  function check_gidNumber()
  {
    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();

    $ldap->cd($cv['base']);
    $res = $ldap->search("(&(objectClass=posixGroup)(gidNumber=*))", array("dn","gidNumber"));
    if (!$res) {
      $this->checks['gidNumber_usage']['STATUS']      = FALSE;
      $this->checks['gidNumber_usage']['STATUS_MSG']  = _("LDAP query failed");
      $this->checks['gidNumber_usage']['ERROR_MSG']   = _("Possibly the 'root object' is missing.");
      return FALSE;
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
      $this->checks['gidNumber_usage']['STATUS']      = FALSE;
      $this->checks['gidNumber_usage']['STATUS_MSG']  = "<div style='color:#F0A500'>"._("Warning")."</div>";
      $this->checks['gidNumber_usage']['ERROR_MSG']   =
        sprintf(_("Found %s duplicate values for attribute 'gidNumber'."), count($this->check_gidNumbers));
      return FALSE;
    } else {
      $this->checks['gidNumber_usage']['STATUS']      = TRUE;
      $this->checks['gidNumber_usage']['STATUS_MSG']  = _("Ok");
      $this->checks['gidNumber_usage']['ERROR_MSG']   = "";
      return TRUE;
    }
  }

  /* Search for groups outside the group ou */
  function search_outside_groups()
  {
    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();

    $group_ou = $cv['groupou'];
    $ldap->cd($cv['base']);

    /***********
     * Get all gosaDepartments to be able to
     *  validate correct ldap tree position of every single user
     ***********/
    $valid_deps = array();
    $valid_deps['/'] = $cv['base'];
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
      $group_db_base = preg_replace("/^[^,]+,".preg_quote($group_ou, '/')."+,/i", "", $attrs['dn']);

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
    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();
    $ldap->cd($cv['base']);

    /***********
     * Get all gosaDepartments to be able to
     *  validate correct ldap tree position of every single user
     ***********/
    $valid_deps = array();
    $valid_deps['/'] = $cv['base'];
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
    $people_ou = trim($cv['peopleou']);
    if (!empty($people_ou)) {
      $people_ou = $people_ou.",";
    }

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


  /* Check ldap accessibility
   * Create and remove a dummy object,
   *  to ensure that we have the necessary permissions
   */
  function check_ldap_permissions()
  {
    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();

    /* Create dummy entry */
    $name       = "GOsa_setup_text_entry_".session_id().rand(0, 999999);
    $dn         = "ou=".$name.",".$cv['base'];
    $testEntry  = array();

    $testEntry['objectClass'][] = "top";
    $testEntry['objectClass'][] = "organizationalUnit";
    $testEntry['objectClass'][] = "gosaDepartment";
    $testEntry['description']   = "Created by FusionDirectory setup, this object can be removed.";
    $testEntry['ou']            = $name;

    /* check if simple ldap cat will be successful */
    $res = $ldap->cat($cv['base']);
    if (!$res) {
      $this->checks['permissions']['STATUS']      = FALSE;
      $this->checks['permissions']['STATUS_MSG']  = _("LDAP query failed");
      $this->checks['permissions']['ERROR_MSG']   = _("Possibly the 'root object' is missing.");
      return FALSE;
    }

    /* Try to create dummy object */
    $ldap->cd ($dn);
    $res = $ldap->add($testEntry);
    $ldap->cat($dn);
    if (!$ldap->count()) {
      new log("view", "setup/".get_class($this), $dn, array(), $ldap->get_error());

      $this->checks['permissions']['STATUS']      = FALSE;
      $this->checks['permissions']['STATUS_MSG']  = _("Failed");
      $this->checks['permissions']['ERROR_MSG']   =
        sprintf(_("The specified user '%s' does not have full access to your ldap database."), $cv['admin']);
      return FALSE;
    }

    /* Try to remove created entry */
    $res = $ldap->rmDir($dn);
    $ldap->cat($dn);
    if ($ldap->count()) {
      new log("view", "setup/".get_class($this), $dn, array(), $ldap->get_error());
      $this->checks['permissions']['STATUS']      = FALSE;
      $this->checks['permissions']['STATUS_MSG']  = _("Failed");
      $this->checks['permissions']['ERROR_MSG']   =
        sprintf(_("The specified user '%s' does not have full access to your ldap database."), $cv['admin']);
      return FALSE;
    }

    /* Create & remove of dummy object was successful */
    $this->checks['permissions']['STATUS']      = TRUE;
    $this->checks['permissions']['STATUS_MSG']  = _("Ok");
    $this->checks['permissions']['ERROR_MSG']   = "";
    return TRUE;
  }


  /* Check if there are users which will
   *  be invisible for FusionDirectory
   */
  function check_gosaAccounts()
  {
    /* Remember old list of invisible users, to be able to set
     *  the 'html checked' status for the checkboxes again
     */
    $old    = $this->users_to_migrate;
    $this->users_to_migrate = array();

    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();

    /* Get all invisible users */
    $ldap->cd($cv['base']);
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

    /* No invisible */
    if (!$res) {
      $this->checks['users_visible']['STATUS']      = FALSE;
      $this->checks['users_visible']['STATUS_MSG']  = _("LDAP query failed");
      $this->checks['users_visible']['ERROR_MSG']   = _("Possibly the 'root object' is missing.");
    } elseif (count($this->users_to_migrate) == 0) {
      $this->checks['users_visible']['STATUS']      = TRUE;
      $this->checks['users_visible']['STATUS_MSG']  = _("Ok");
      $this->checks['users_visible']['ERROR_MSG']   = "";
    } else {
      $this->checks['users_visible']['STATUS']      = FALSE;
      $this->checks['users_visible']['STATUS_MSG']  = "<div style='color:#F0A500'>"._("Warning")."</div>";
      $this->checks['users_visible']['ERROR_MSG']   = sprintf(_("Found %s user(s) that will not be visible in FusionDirectory or which are incomplete."),
          count($this->users_to_migrate));
      $this->checks['users_visible']['ERROR_MSG']   .= "<input type='submit' name='users_visible_migrate' value='"._("Migrate")."...'>";
    }
  }


  /* Start user account migration */
  function migrate_gosaAccounts($only_ldif = FALSE)
  {
    $this->show_details = $only_ldif;

    /* Establish ldap connection */
    $ldap = $this->get_ldap_link();

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


  /* Check if there are invisible organizational Units */
  function check_organizationalUnits()
  {
    $old                    = $this->deps_to_migrate;
    $this->deps_to_migrate  = array();

    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();

    /* Skip FusionDirectory internal departments */
    $skip_dns = array("/".$cv['peopleou']."/","/".$cv['groupou']."/","/".$cv['aclroleou']."/",
        "/^ou=people,/","/^ou=groups,/","/^ou=sudoers,/",
        "/(,|)ou=configs,/","/(,|)ou=systems,/","/(,|)ou=tokens,/",
        "/(,|)ou=apps,/","/(,|)ou=mime,/","/(,|)ou=devices/",
        "/ou=snapshots,/","/(,|)dc=addressbook,/","/^(,|)ou=machineaccounts,/",
        "/(,|)ou=winstations,/","/^ou=hosts,/","/^ou=computers,/","/^ou=idmap,/","/^ou=Idmap,/","/(,|)ou=roles,/");

    /* Get all invisible departments */
    $ldap->cd($cv['base']);
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
    $this->show_details = $only_ldif;

    /* Establish ldap connection */
    $ldap = $this->get_ldap_link();

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


  /* Check Acls if there is at least one object with acls defined */
  function check_administrativeAccount()
  {
    /* Reset settings */
    $FD_1_0_8_found = FALSE;
    $this->migrate_users = array();
    $this->acl_migrate_dialog = FALSE;
    $this->migrate_acl_base_entry  = "";
    $valid_admin = FALSE;

    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();
    $ldap->cd($cv['base']);
    $res = $ldap->cat($cv['base']);

    if (!$res) {
      $this->checks['acls']['STATUS']     = FALSE;
      $this->checks['acls']['STATUS_MSG'] = _("LDAP query failed");
      $this->checks['acls']['ERROR_MSG']  = _("Possibly the 'root object' is missing.");
    } else {
      $FD_1_0_8_found = FALSE; // GOsa 2.6 Account found
      $FD_1_0_7_found = FALSE; // GOsa 2.5 Account found, allow migration

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
        if (!empty($valid_groups)) {
          $str .= "<i>".sprintf(_("FD 1.0.7 administrative accounts found: %s"), trim($valid_groups, ", "))."</i><br>";
        }
        $this->checks['acls']['STATUS']     = FALSE;
        $this->checks['acls']['STATUS_MSG'] = _("Failed");
        $this->checks['acls']['ERROR_MSG']  = $str;
        $this->checks['acls']['ERROR_MSG']  .= _("There is no valid FusionDirectory 1.0.8 administrator account inside your LDAP.")."&nbsp;";
        $this->checks['acls']['ERROR_MSG']  .= "<input type='submit' name='migrate_acls' value='"._("Migrate")."'>";
        $this->checks['acls']['ERROR_MSG']  .= "<input type='submit' name='create_acls' value='"._("Create")."'>";
      } elseif ($FD_1_0_8_found) {
        $str = "";
        if (!empty($valid_users)) {
          $str .= "<b>"._("Users")."</b>:&nbsp;".trim($valid_users, ", ")."<br>";
        }
        if (!empty($valid_groups)) {
          $str .= "<b>"._("Groups")."</b>:&nbsp;".trim($valid_groups, ", ")."<br>";
        }
        $this->checks['acls']['STATUS']     = TRUE;
        $this->checks['acls']['STATUS_MSG'] = _("Ok");
        $this->checks['acls']['ERROR_MSG']  = $str;
        $this->valid_admin = $valid_admin;
      } else {
        $this->checks['acls']['STATUS']     = FALSE;
        $this->checks['acls']['STATUS_MSG'] = _("Failed");
        $this->checks['acls']['ERROR_MSG']  = _("There is no FusionDirectory administrator account inside your LDAP.")."&nbsp;";
        $this->checks['acls']['ERROR_MSG']  .= "<input type='submit' name='create_acls' value='"._("Create")."'>";
      }
    }

    // Reload base OC
    $this->checkBaseOC();
    return $FD_1_0_8_found;
  }



  function create_admin($only_ldif = FALSE)
  {
    /* Reset '' */
    $this->acl_create_changes = "";

    /* Object that should receive admin acls */
    $dn = $this->acl_create_selected;

    /* Get collected configuration settings */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();

    $ldap->search("(&(objectClass=gosaRole)(gosaAclTemplate=*:all;cmdrw))", array('dn'));
    if ($attrs = $ldap->fetch()) {
      $roledn = $attrs['dn'];
    } else {
      $roledn = 'cn=admin,'.$cv['aclroleou'].$cv['base'];
      if (!$only_ldif) {
        $ldap->create_missing_trees($cv['aclroleou'].$cv['base']);
        $ldap->cd($roledn);
        $attrs_role = array(
          'cn'              => 'admin',
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
    $ldap->cd($cv['base']);
    $ldap->cat($cv['base'], array("dn","objectClass","gosaAclEntry"));
    $attrs = $ldap->fetch();

    /* Add acls for the selcted user to the base */
    $attrs_new = array();
    $attrs_new['objectClass'] = $attrs['objectClass'];
    if (!in_array_ics('gosaAcl', $attrs_new['objectClass'])) {
      $attrs_new['objectClass'][] = 'gosaAcl';
    }

    $acl = "0:subtree:".base64_encode($roledn).':'.base64_encode($dn); //FIXME
    $attrs_new['gosaAclEntry'][] = $acl;
    if (isset($attrs['gosaAclEntry'])) {
      for ($i = 0; $i < $attrs['gosaAclEntry']['count']; $i ++) {

        $prio = preg_replace("/[:].*$/", "", $attrs['gosaAclEntry'][$i]);
        $rest = preg_replace("/^[^:]/", "", $attrs['gosaAclEntry'][$i]);

        $data = ($prio + 1).$rest;
        $attrs_new['gosaAclEntry'][] = $data;
      }
    }

    if ($only_ldif) {
      $this->acl_create_changes = "\n".($ldap->fix($cv['base']))."\n";
      $this->acl_create_changes .= $this->array_to_ldif($attrs)."\n";
      $this->acl_create_changes .= "\n".($ldap->fix($cv['base']))."\n";
      $this->acl_create_changes .= $this->array_to_ldif($attrs_new);
    } else {
      $ldap->cd($cv['base']);
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
    $pw1 = $pw2 = "";
    $uid = "";

    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();

    if (isset($_POST['new_user_uid'])) {
      $uid = $_POST['new_user_uid'];
    }
    if (isset($_POST['new_user_password'])) {
      $pw1 = $_POST['new_user_password'];
    }
    if (isset($_POST['new_user_password2'])) {
      $pw2 = $_POST['new_user_password2'];
    }

    $ldap->cd($cv['base']);
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
    $ldap->cd($cv['base']);

    $people_ou = trim($cv['peopleou']);
    if (!empty($people_ou)) {
      $people_ou = trim($people_ou).",";
    }

    if ($cv['peopledn'] == "cn") {
      $dn = "cn=System Administrator-".$uid.",".$people_ou.$cv['base'];
    } else {
      $dn = "uid=".$uid.",".$people_ou.$cv['base'];
    }

    $hash = passwordMethod::make_hash($pw2, $cv['encryption']);

    $new_user = array();

    $new_user['objectClass']  = array("top","person","gosaAccount","organizationalPerson","inetOrgPerson");
    $new_user['givenName']    = "System";
    $new_user['sn']           = "Administrator";
    $new_user['cn']           = "System Administrator-".$uid;
    $new_user['uid']          = $uid;
    $new_user['userPassword'] = $hash;

    $ldap->cd($cv['base']);

    $ldap->cat($dn, array("dn"));
    if ($ldap->count()) {
      msg_dialog::display(_("Error"), sprintf(_("Adding an administrative user failed: object '%s' already exists!"), LDAP::fix($dn)), ERROR_DIALOG);
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
    $this->check_administrativeAccount();
    return TRUE;
  }

  function migrate_outside_groups($perform = FALSE)
  {
    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();
    $ldap->cd($cv['base']);

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
    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();
    $ldap->cd($cv['base']);

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
     * Administrative Account -- Migrate/Create
     *************/

    if (isset($_POST['retry_acls'])) {
      $this->check_administrativeAccount();
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

    /*************
     * Device migration
     *************/

    if ($this->device_dialog) {
      $this->check_device_posts();
    }

    if (isset($_POST['device_dialog_cancel'])) {
      $this->device_dialog  = FALSE;
      $this->show_details   = FALSE;
      $this->dialog         = FALSE;
    }

    if (isset($_POST['device_dialog_whats_done'])) {
      $this->show_details = TRUE;
    }

    if (isset($_POST['device_dialog_refresh'])) {
      $this->show_details = FALSE;
    }

    if (isset($_POST['migrate_devices'])) {
      $this->migrate_usb_devices();
    }

    if (isset($_POST['device_dialog'])) {
      $this->device_dialog  = TRUE;
      $this->dialog         = TRUE;
    }

    if ($this->device_dialog) {
      $smarty = get_smarty();
      $smarty->assign("method", "devices");
      $smarty->assign("devices", $this->device);
      $smarty->assign("device_details", $this->show_details);
      return $smarty->fetch(get_template_path("setup_migrate.tpl", TRUE, dirname(__FILE__)));
    }

    /*************
     * Service migration
     *************/

    if ($this->service_dialog) {
      $this->check_service_posts();
    }

    if (isset($_POST['service_dialog_cancel'])) {
      $this->service_dialog = FALSE;
      $this->show_details   = FALSE;
      $this->dialog         = FALSE;
    }

    if (isset($_POST['service_dialog_whats_done'])) {
      $this->show_details = TRUE;
    }

    if (isset($_POST['service_dialog_refresh'])) {
      $this->show_details = FALSE;
    }

    if (isset($_POST['migrate_services'])) {
      $this->migrate_services();
    }

    if (isset($_POST['service_dialog'])) {
      $this->service_dialog = TRUE;
      $this->dialog         = TRUE;
    }

    if ($this->service_dialog) {
      $smarty = get_smarty();
      $smarty->assign("method", "services");
      $smarty->assign("services", $this->service);
      $smarty->assign("service_details", $this->show_details);
      return $smarty->fetch(get_template_path("setup_migrate.tpl", TRUE, dirname(__FILE__)));
    }

    /*************
     * Menu migration
     *************/

    if ($this->menu_dialog) {
      $this->check_menu_posts();
    }

    if (isset($_POST['menu_dialog_cancel'])) {
      $this->menu_dialog  = FALSE;
      $this->show_details = FALSE;
      $this->dialog       = FALSE;
    }

    if (isset($_POST['menu_dialog_whats_done'])) {
      $this->show_details = TRUE;
    }

    if (isset($_POST['menu_dialog_refresh'])) {
      $this->show_details = FALSE;
    }

    if (isset($_POST['migrate_menus'])) {
      $this->migrate_menus();
    }

    if (isset($_POST['menu_dialog'])) {
      $this->menu_dialog  = TRUE;
      $this->dialog       = TRUE;
    }

    if ($this->menu_dialog) {
      $smarty = get_smarty();
      $smarty->assign("method", "menus");
      $smarty->assign("menus", $this->menu);
      $smarty->assign("menu_details", $this->show_details);
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


  /* Check if the root object exists.
   * If the parameter just_check is TRUE, then just check if the
   *  root object is missing and update the info messages.
   * If the Parameter is FALSE, try to create a new root object.
   */
  function checkBase($just_check = TRUE)
  {
    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();

    /* Check if root object exists */
    $ldap->cd($cv['base']);
    $ldap->set_size_limit(1);
    $res = $ldap->search("(objectClass=*)");
    $ldap->set_size_limit(0);
    $err = ldap_errno($ldap->cid);

    if ( !$res ||
        $err == 0x20 || // LDAP_NO_SUCH_OBJECT
        $err == 0x40) { // LDAP_NAMING_VIOLATION

      /* Root object doesn't exists */
      if ($just_check) {
        $this->checks['root']['STATUS']     = FALSE;
        $this->checks['root']['STATUS_MSG'] = _("Failed");
        $this->checks['root']['ERROR_MSG']  = _("The LDAP root object is missing. It is required to use your LDAP service.").'&nbsp;';
        $this->checks['root']['ERROR_MSG']  .= "<input type='submit' name='retry_root_create' value='"._("Try to create root object")."'>";
        return FALSE;
      } else {

        /* Add root object */
        $ldap->cd($cv['base']);
        $res = $ldap->create_missing_trees($cv['base']);

        /* If adding failed, tell the user */
        if (!$res) {
          $this->checks['root']['STATUS']     = FALSE;
          $this->checks['root']['STATUS_MSG'] = _("Failed");
          $this->checks['root']['ERROR_MSG']  = _("Root object couldn't be created, you should try it on your own.");
          $this->checks['root']['ERROR_MSG']  .= "&nbsp;<input type='submit' name='retry_root_create' value='"._("Try to create root object")."'>";
          return $res;
        }
      }
    }

    /* Create & remove of dummy object was successful */
    $this->checks['root']['STATUS']     = TRUE;
    $this->checks['root']['STATUS_MSG'] = _("Ok");
  }


  /* Check if the root object includes the required object classes, e.g. gosaDepartment is required for ACLs.
   * If the parameter just_check is TRUE, then just check for the OCs.
   * If the Parameter is FALSE, try to add the required object classes.
   */
  function checkBaseOC($just_check = TRUE)
  {
    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();

    /* Check if root object exists */
    $ldap->cd($cv['base']);
    $ldap->cat($cv['base']);
    if (!$ldap->count()) {
      $this->checks['rootOC']['STATUS']     = FALSE;
      $this->checks['rootOC']['STATUS_MSG'] = _("LDAP query failed");
      $this->checks['rootOC']['ERROR_MSG']  = _("Possibly the 'root object' is missing.");
      return;
    }

    $attrs = $ldap->fetch();

    /* Root object doesn't exists */
    if (!in_array("gosaDepartment", $attrs['objectClass'])) {
      if ($just_check) {

        $this->rootOC_details = array();
        $mods = array();

        /* Get list of possible container objects, to be able to detect naming
         *  attributes and missing attribute types.
         */
        if (!class_available("departmentManagement")) {
          $this->checks['rootOC']['STATUS']     = FALSE;
          $this->checks['rootOC']['STATUS_MSG'] = _("Failed");
          $this->checks['rootOC']['ERROR_MSG']  = sprintf(_("Missing FusionDirectory object class '%s'!"), "departmentManagement").
            "&nbsp;"._("Please check your installation.");
          return;
        }

        /* Create a minimal config object for objectType infos */
        global $config, $plist;
        $config = new config("");
        load_all_classes();
        $plist  = new pluglist($config, $ui);
        $config->loadPlist($plist);

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
          $this->checks['rootOC']['STATUS']     = FALSE;
          $this->checks['rootOC']['STATUS_MSG'] = _("Failed");
          $this->checks['rootOC']['ERROR_MSG']  =
            sprintf(_("Cannot handle the structural object type of your root object. Please try to add the object class '%s' manually."), "gosaDepartment");
          return;
        }
        $dep_infos = objects::infos($dep_type);

        /* Create 'current' and 'target' object properties, to be able to display
         *  a set of modifications required to create a valid FusionDirectory department.
         */
        $str = "dn: ".$cv['base']."\n";
        for ($i = 0; $i < $attrs['objectClass']['count']; $i++) {
          $str .= "objectClass: ".$attrs['objectClass'][$i]."\n";
        }
        $this->rootOC_details['current'] = $str;

        /* Create target infos */
        $str = "dn: ".$cv['base']."\n";
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
        $this->checks['rootOC']['STATUS']     = FALSE;
        $this->checks['rootOC']['STATUS_MSG'] = _("Failed");
        $this->checks['rootOC']['ERROR_MSG']  = "&nbsp;<input type='submit'
          name='root_add_objectclasses' value='"._("Migrate")."'>";

        return FALSE;
      } else {

        /* Add root object */
        $ldap->cd($cv['base']);
        if (isset($this->rootOC_details['mods'])) {
          $res  = $ldap->modify($this->rootOC_details['mods']);
          if (!$res) {
            msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $cv['base'], LDAP_MOD, get_class()));
          }
          $this->checkBaseOC();
          $this->check_administrativeAccount();
          return $res;
        } else {
          trigger_error("No modifications to make... ");
        }
      }
      return TRUE;
    }

    /* Create & remove of dummy object was successful */
    $this->checks['rootOC']['STATUS']     = TRUE;
    $this->checks['rootOC']['STATUS_MSG'] = _("Ok");
    $this->checks['rootOC']['ERROR_MSG']  = "";
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
    /* Establish ldap connection */
    $ldap = $this->get_ldap_link();
    $ldap->search("(objectClass=$class)", array("dn"));

    $tmp = array();
    while ($attrs = $ldap->fetch()) {
      $tmp[base64_encode($attrs['dn'])] = LDAP::fix($attrs['dn']);
    }
    return $tmp;
  }


  function get_group_list()
  {
    return $this->get_user_list('posixGroup');
  }


  function get_all_people_ous()
  {
    return $this->get_all_ous('peopleou');
  }

  function get_all_group_ous()
  {
    return $this->get_all_ous('groupou');
  }

  function get_all_ous($cv_key)
  {
    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();
    $ou   = trim($cv[$cv_key]);

    /************
     * If ou is NOT empty
     * Get all valid ous, create one if necessary
     ************/
    if (!empty($ou)) {
      $ldap->search("($ou)", array('dn'));
      if ($ldap->count() == 0 ) {
        $ldap->create_missing_trees($ou.$cv['base']);
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
      $ldap->cd($cv['base']);
      $tmp = array();
      $ldap->search('(&(objectClass=gosaDepartment)(ou=*))', array('dn'));
      $tmp[base64_encode($cv['base'])] = $ldap->fix($cv['base']);
      while ($attrs = $ldap->fetch()) {
        $tmp[base64_encode($attrs['dn'])] = $ldap->fix($attrs['dn']);;
      }
    }
    return $tmp;
  }


  function move($source, $destination)
  {
    /* Establish ldap connection */
    $ldap = $this->get_ldap_link();

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
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $source, LDAP_DEL, get_class()));
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


  /*! \brief  Act in posts from the device migration dialog
   */
  function check_device_posts()
  {
    foreach (array_keys($this->device) as $key) {
      if (isset($_POST["migrate_".$key])) {
        $this->device[$key]['DETAILS'] = TRUE;
      } else {
        $this->device[$key]['DETAILS'] = FALSE;
      }
    }
  }


  /*! \brief  Check for old style (gosa-2.5) devices.
              Save readable informations and a list of migratable devices
               in $this->devices.
   */
  function check_usb_devices ()
  {
    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();
    $ldap->cd($cv['base']);
    $res = $ldap->search("(&(|(objectClass=posixAccount)(objectClass=posixGroup))(gotoHotplugDevice=*))",
        array("cn","gotoHotplugDevice"));

    if (!$res) {
      $this->checks['old_style_devices']['STATUS']      = FALSE;
      $this->checks['old_style_devices']['STATUS_MSG']  = _("LDAP query failed");
      $this->checks['old_style_devices']['ERROR_MSG']   = _("Possibly the 'root object' is missing.");
      return;
    }

    /* If adding failed, tell the user */
    if ($ldap->count()) {

      $this->device = array();
      while ($attrs = $ldap->fetch()) {

        for ($j = 0; $j < $attrs['gotoHotplugDevice']['count']; $j++) {

          $after    = "";
          $current  = "";

          $entry = $attrs['gotoHotplugDevice'][$j];

          @list($name, $desc, $serial, $vendor, $product) = explode('|', $entry);

          $add      = 1;
          $new_name = $name;
          while (isset($dest[$new_name])) {
            $new_name = $name."_".$add;
            $add ++;
          }
          $name   = $new_name;
          $newdn  = "cn=$name,ou=devices,".preg_replace('/^[^,]+,/', '', $attrs['dn']);

          if (!isset($dest[$name])) {
            $dest[$name] = $newdn;

            $current .= "dn: ".$attrs['dn']."\n";

            for ($c = 0; $c < $attrs['gotoHotplugDevice']['count']; $c++) {
              if ($c == $j) {
                $current .= "<b>gotoHotplugDevice: ".$attrs['gotoHotplugDevice'][$c]."</b>\n";
              } else {
                $current .= "gotoHotplugDevice: ".$attrs['gotoHotplugDevice'][$c]."\n";
              }
            }

            $after .= "dn: $newdn\n";
            $after .= "changetype: add\n";
            $after .= "objectClass: top\n";
            $after .= "objectClass: gotoDevice\n";
            $after .= "cn: $name\n";
            $after .= "gotoHotplugDevice: $desc|$serial|$vendor|$product\n\n";

            $this->device[] = array(
                'CURRENT'     => $current,
                'AFTER'       => $after,
                'OLD_DEVICE'  => $entry,
                'DN'          => $attrs['dn'],
                'NEW_DN'      => $newdn,
                'DEVICE_NAME' => $name,
                'DETAILS'     => FALSE);
          }
        }
      }

      $this->checks['old_style_devices']['STATUS']      = FALSE;
      $this->checks['old_style_devices']['STATUS_MSG']  = "<div style='color:#F0A500'>"._("Warning")."</div>";
      $this->checks['old_style_devices']['ERROR_MSG']   =
        sprintf(_("There are %s devices that need to be migrated."), count($this->device)).
          "<input type='submit' name='device_dialog' value='"._("Migrate")."'>";
    } else {
      $this->checks['old_style_devices']['STATUS']      = TRUE;
      $this->checks['old_style_devices']['STATUS_MSG']  = _("Ok");
      $this->checks['old_style_devices']['ERROR_MSG']   = "";
    }
  }


  /*! \brief  Migrate all selected devices.
              Execute all required ldap actions to migrate the
               selected devices.
   */
  function migrate_usb_devices ()
  {
    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();

    /* Walk through migrateable devices and initiate migration for all
        devices that are checked (DETAILS==TRUE)
     */
    foreach ($this->device as $key => $device) {
      if ($device['DETAILS']) {

        /* Get source object and verify that the specified device is a
            member attribute of it.
         */
        $ldap->cd($cv['base']);
        $ldap->cat($device['DN']);
        $attrs = $ldap->fetch();
        if (in_array($device['OLD_DEVICE'], $attrs['gotoHotplugDevice'])) {

          /* Create new hotplug device object 'gotoDevice' */
          @list($name, $desc, $serial, $vendor, $product) = explode('|', $device['OLD_DEVICE']);
          $newdn = $device['NEW_DN'];
          $new_attr = array();
          $new_attr['cn'] = $device['DEVICE_NAME'];
          $new_attr['objectClass'] = array('top','gotoDevice');
          $new_attr['gotoHotplugDevice'] = "$desc|$serial|$vendor|$product";

          /* Add new object */
          $ldap->cd($cv['base']);
          $ldap->create_missing_trees(preg_replace("/^[^,]+,/", "", $newdn));
          $ldap->cd($newdn);
          $ldap->add($new_attr);

          /* Throw an error message if the action failed */
          if (!$ldap->success()) {
            msg_dialog::display(_("LDAP error"),
                sprintf(_("Adding '%s' to the LDAP failed: %s"),
                  "<b>".LDAP::fix($newdn)."</b>",
                  "<br><br><i>".$ldap->get_error()."</i>"), ERROR_DIALOG);
          } else {

            /* Remove old style device definition from source object */
            $update['gotoHotplugDevice'] = array();
            for ($i = 0; $i < $attrs['gotoHotplugDevice']['count']; $i++) {
              if ($attrs['gotoHotplugDevice'][$i] == $device['OLD_DEVICE']) {
                 continue;
              }
              $update['gotoHotplugDevice'][] = $attrs['gotoHotplugDevice'][$i];
            }

            $ldap->cd($device['DN']);
            $ldap->modify($update);
            $ldap->cat($device['DN'], array("gotoHotplugDevice"));
            if (!$ldap->success()) {
              msg_dialog::display(_("LDAP error"),
                  sprintf(_("Updating '%s' failed: %s"),
                    "<b>".LDAP::fix($device['DN'])."</b>",
                    "<br><br><i>".$ldap->get_error()."</b>"), ERROR_DIALOG);
            } else {
              unset($this->device[$key]);
            }
          }
        }
      }
    }
    $this->check_usb_devices();
  }


  /*! \brief  Check for old style (FD < 1.0.5) services that have to be migrated
               to be useable in FD >= 1.0.5.
              All required changes are stored in $this->service, also some
               readable informations describing the actions required
               to migrate the service
   */
  function check_services()
  {
    /* Establish ldap connection */
    $cv     = $this->parent->captured_values;
    $ldap_l = new LDAP(
      $cv['admin'], $cv['password'],
      $cv['connection'], FALSE,
      $cv['tls']
    );

    $ldap = new ldapMultiplexer($ldap_l);
    $this->service = array();

    /* Check for Ldap services that must be migrated */
    $ldap->cd($cv['base']);
    $res = $ldap->search('(objectClass=goLdapServer)');

    /* Check if we were able to query the ldap server */
    if (!$res) {
      $this->checks['old_style_services']['STATUS']     = FALSE;
      $this->checks['old_style_services']['STATUS_MSG'] = _("LDAP query failed");
      $this->checks['old_style_services']['ERROR_MSG']  = _("Possibly the 'root object' is missing.");
      return;
    }

    /* Walk through each configured ldap server
        and check if it is configured correctly.
     */
    while ($attrs = $ldap->fetch()) {
      if (!isset($attrs['goLdapURI'])) {
        $dn   = $attrs['dn'];
        $uri  = $attrs['goLdapBase'][0];
        if (preg_match("!^(ldaps?://[^/]+)/(.+)$!", $uri, $m)) {
          $this->service[] = array(
            'TYPE'    => 'modify' ,
            'DN'      => $dn,
            'DETAILS' => FALSE,
            'ATTRS'   => array('goLdapBase' => $m[2], 'goLdapURI' => $m[1]),
            'CURRENT' => 'goLdapBase: '.$uri,
            'AFTER'   => 'goLdapBase: '.$m[2].', goLdapURI: '.$m[1]
          );
        }
      }
    }

    /* Other sevices following here later ...maybe */

    /*  Update status message */
    if (count($this->service)) {
      $this->checks['old_style_services']['STATUS']     = FALSE;
      $this->checks['old_style_services']['STATUS_MSG'] = "<div style='color:#F0A500'>"._("Warning")."</div>";
      $this->checks['old_style_services']['ERROR_MSG']  =
        sprintf(_("There are %s services that need to be migrated."),
            count($this->service)).
        "<input type='submit' name='service_dialog' value='"._("Migrate")."'>";
    } else {
      $this->checks['old_style_services']['STATUS']     = TRUE;
      $this->checks['old_style_services']['STATUS_MSG'] = _("Ok");
      $this->checks['old_style_services']['ERROR_MSG']  = "";
    }
  }



  /*! \brief  Migrate selected services.
              This function executes the commands collected by the
               service_check() function.
   */
  function migrate_services()
  {
    /* Establish ldap connection */
    $ldap = $this->get_ldap_link();

    /* Handle each service */
    foreach ($this->service as $key => $service) {
      if ($service['DETAILS']) {
        /* Handle modify requests */
        if ($service['TYPE'] == "modify") {
          $ldap->cd($service['DN']);
          $ldap->modify($service['ATTRS']);

          /* Check if everything done was successful */
          if (!$ldap->success()) {
            msg_dialog::display(_("LDAP error"),
                sprintf(_("Updating '%s' failed: %s"),
                  "<b>".LDAP::fix($service['DN'])."</b>",
                  "<br><br><i>".$ldap->get_error()."</b>"), ERROR_DIALOG);
          } else {
            /* Remove action from list */
            unset($this->service[$key]);
          }
        }
      }
    }

    /* Update the service migration status */
    $this->check_services();
  }


  /*! \brief  Ensure that posts made on the service migration dialog
               are processed.
   */
  function check_service_posts()
  {
    foreach (array_keys($this->service) as $key) {
      if (isset($_POST["migrate_".$key])) {
        $this->service[$key]['DETAILS'] = TRUE;
      } else {
        $this->service[$key]['DETAILS'] = FALSE;
      }
    }
  }


  /*! \brief  This function checks the given ldap for old style (gosa-2.5)
               menu entries and will prepare a list of actions that are required
               to migrate them to gosa-2.6.
              All required actions and some readable informations are stored in
               $this->menu.
   */
  function check_menus()
  {
    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();

    /* First detect all release names */
    $ldap->cd($cv['base']);
    $res = $ldap->search("(&(objectClass=organizational)(objectClass=FAIbranch))", array("ou","objectClass"));

    /* Check if we were able to query the ldap server */
    if (!$res) {
      $this->checks['old_style_menus']['STATUS']      = FALSE;
      $this->checks['old_style_menus']['STATUS_MSG']  = _("LDAP query failed");
      $this->checks['old_style_menus']['ERROR_MSG']   = _("Possibly the 'root object' is missing.");
      return;
    }

    /* Create application -> parameter mapping, used later to detect
        which configured parameter belongs to which application entry.
     */
    $amap = array();
    $todo = array();
    $ldap->cd($cv['base']);
    $ldap->search("(objectClass=gosaApplication)", array("cn", "gosaApplicationParameter"));
    while ($info = $ldap->fetch()) {
      if (isset($info['gosaApplicationParameter']['count'])) {
        for ($j = 0; $j < $info['gosaApplicationParameter']['count']; $j++) {
          $p = preg_replace("/^([^:]+):.*$/", "$1", $info['gosaApplicationParameter'][$j]);

          if (!isset($amap[$info['cn'][0]]) || !in_array($p, $amap[$info['cn'][0]])) {
            $amap[$info['cn'][0]][] = $p;
          }
        }
      } else {
        $amap[$info['cn'][0]] = array();
      }
    }

    /* Search for all groups that have an old style application menu configured */
    $ldap->cd($cv['base']);
    $ldap->search("(&(objectClass=gosaApplicationGroup)(objectClass=posixGroup)(FAIrelease=*))",
        array("gosaMemberApplication","gosaApplicationParameter","FAIrelease","objectClass"));

    /* Create readable prefix for "What will be done" infos */
    $s_add = "<i>"._("Add")."</i>\t";
    $s_del = "<i>"._("Remove")."</i>\t";

    /* Walk through all found old-style menu configurations.
        -Prepare ldap update list     $data
        -Prepare printable changes    $after/$current
     */
    while ($info = $ldap->fetch()) {

      $data     = array();
      $current  = "";
      $after    = "";

      /* Collect application parameter for this group */
      $params = array();
      if (isset($info['gosaApplicationParameter'])) {
        for ($i = 0; $i < $info['gosaApplicationParameter']['count']; $i++) {
          $name = preg_replace("/^([^:]+):.*$/", "$1", $info['gosaApplicationParameter'][$i]);
          $params[$name] = $info['gosaApplicationParameter'][$i];
        }
      }

      /* Create release container for each release/subrelease.
         eg.   "sisa/1.0.0":
         .       "ou=siga, ..."
         .       "ou=1.0.0,ou=siga, .."
       */
      $release  = "";
      $r        = $info['FAIrelease'][0];
      $z        = explode("/", $r);
      foreach ($z as $part) {

        if (!empty($part)) {
          $release = "ou=".$part.",".$release;

          /* Append release department information to "What will be done" info */
          $release_dn = $release.$info['dn'];
          $after      .= $s_add."dn: $release_dn\n";
          $after      .= $s_add."objectClass: top\n";
          $after      .= $s_add."objectClass: FAIbranch\n";
          $after      .= $s_add."objectClass: organizationalUnit\n";

          $after   .= $s_add."ou: $part\n";

          /* Append release data to ldap actions */
          $d = array();
          $d['objectClass'] = array("top","FAIbranch","organizationalUnit");
          $d['ou'] = $part;

          $data['ADD'][$release_dn] = $d;
        }
      }

      /* Add member applications to the array */
      $current .= "dn: ".$info['dn']."\n";
      $menu_structure = array();
      for ($i = 0; $i < $info['gosaMemberApplication']['count']; $i++) {
        list($name, $location, $priority) = explode("|", $info['gosaMemberApplication'][$i]);

        /* Create location dn */
        $location_dn = "";
        if (!empty($location)) {
          $location_dn = "cn=".$location.",";
        }

        /* Append old style element to current detail informations */
        $current .= $s_del."gosaMemberApplication: ".$info['gosaMemberApplication'][$i]."\n";

        /* Append ldap update action to remove the old menu entry attributes */
        unset($info['objectClass']['count']);
        $d = array();
        $d['gosaMemberApplication']      = array();
        $d['gosaApplicationParameter']  = array();
        if (isset($info['FAIrelease'])) {
          $d['FAIrelease'] = array();
        }
        $d['objectClass']               = array_remove_entries(array("gosaApplicationGroup","FAIreleaseTag"), $info['objectClass']);
        $data['MODIFY'][$info['dn']]    = $d;

        /* Create new application menu structure */
        if (isset($amap[$name])) {

          /* Append missing menu structure to "What is done info" */
          if (!isset($menu_structure[$location]) && !empty($location)) {
            $menu_structure[$location] = TRUE;

            $after .= "\n";
            $after .= $s_add."dn: $location_dn$release_dn\n";
            $after .= $s_add."objectClass: gotoSubmenuEntry\n";

            $after .= $s_add."cn: $location\n";

            /* Create ldap entry to append */
            $d = array();
            $d['cn'] = $location;
            $d['objectClass'] = array("gotoSubmenuEntry");
            $data['ADD'][$location_dn.$release_dn] = $d;
          }

          /* Append missing menu entry for "What is done info" */
          if (!empty($name)) {
            $after .= "\n";
            $after .= $s_add."dn: cn=$name,$location_dn$release_dn\n";
            $after .= $s_add."objectClass: gotoMenuEntry\n";
            $after .= $s_add."cn: $name\n";
            $after .= $s_add."gosaApplicationPriority: $priority\n";

            /* Create ldap entry */
            $d = array();
            $d['objectClass'] = array("gotoMenuEntry");
            $d['cn']                      = $name;
            $d['gosaApplicationPriority'] = $priority;

            foreach ($amap[$name] as $n) {
              if (isset($params[$n])) {
                $after .= $s_add."gosaApplicationParameter: ".$params[$n]."\n";

                $d['gosaApplicationParameter'][] = $params[$n];
              }
            }
            $data['ADD']["cn=$name,$location_dn$release_dn"] = $d;
          }
        }
      }

      /* Updated todo list */
      $todo[] = array(
        "DETAILS" => FALSE,
        "DN"      => $info['dn'],
        "AFTER"   => $after,
        "CURRENT" => $current,
        "TODO"    => $data
      );
    }

    /* Remember checks */
    $this->menu = $todo;

    /* Check if we were able to query the ldap server */
    if (count($this->menu)) {
      $this->checks['old_style_menus']['STATUS']      = FALSE;
      $this->checks['old_style_menus']['STATUS_MSG']  = "<div style='color:#F0A500'>"._("Warning")."</div>";
      $this->checks['old_style_menus']['ERROR_MSG']   = sprintf(_("There are %s application menus which have to be migrated."),
                                                      count($this->menu))."<input type='submit' name='menu_dialog' value='"._("Migrate")."'>";
    } else {
      $this->checks['old_style_menus']['STATUS']      = TRUE;
      $this->checks['old_style_menus']['STATUS_MSG']  = _("Ok");
      $this->checks['old_style_menus']['ERROR_MSG']   = "";
    }
  }


  /*! \brief  Handle posts for the menu_dialog
              Ensure that checked checkboxes stay checked.
   */
  function check_menu_posts()
  {
    foreach (array_keys($this->menu) as $key) {
      if (isset($_POST["migrate_".$key])) {
        $this->menu[$key]['DETAILS'] = TRUE;
      } else {
        $this->menu[$key]['DETAILS'] = FALSE;
      }
    }
  }


  /*! \brief  This function updates old-style application menus to
               valid 2.6 application menus.
              All selected menus will be converted (DETAILS = TRUE).
              The ldap actions collected by check_menus() will be executed.
   */
  function migrate_menus()
  {
    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();
    $ldap->cd($cv['base']);

    /* Walk through menus and detect selected menu */
    foreach ($this->menu as $key => $menu) {
      if ($menu['DETAILS']) {

        /* Excute all LDAP-ADD actions */
        $success = TRUE;
        foreach ($menu['TODO']['ADD'] as $dn => $data) {
          $ldap->cd($cv['base']);
          if (!$ldap->dn_exists($dn)) {
            $ldap->cd($dn);
            $ldap->add($data);
            if (!$ldap->success()) {
              msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_ADD, get_class()));
              $success = FALSE;
            }
          }
        }

        /* Execute all LDAP-MODIFY actions */
        foreach ($menu['TODO']['MODIFY'] as $dn => $data) {
          $ldap->cd($cv['base']);
          if ($ldap->dn_exists($dn)) {
            $ldap->cd($dn);
            $ldap->modify($data);
            if (!$ldap->success()) {
              msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_MOD, get_class()));
              $success = FALSE;
            }
          }
        }

        /* If every action was successful, remove this entry from the list */
        if ($success) {
          unset($this->menu[$key]);
        }
      }
    }

    /* Udpate migration status for application menus */
    $this->check_menus();
  }


  function migrate_selected_admin_users()
  {
    /* Updated ui selection */
    $this->migrate_users();

    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();

    /* Get current ACL configuration for the ldap base */
    $ldap->cat($cv['base']);
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
      $ldap->cd($cv['base']);
      $ldap->modify($new_entry);
      if (!$ldap->success()) {
        $this->checks['acls']['TITLE']      = _("Checking for super administrator");
        $this->checks['acls']['STATUS']     = FALSE;
        $this->checks['acls']['STATUS_MSG'] = _("Failed");
        $this->checks['acls']['ERROR_MSG']  = "<br>".msgPool::ldaperror($cv['base'], $ldap->get_error(), LDAP_MOD);
      } else {
        $this->check_administrativeAccount();
      }
    }
  }


  function migrate_users()
  {
    /* Collect a list of available FusionDirectory users and groups */

    /* Establish ldap connection */
    $cv   = $this->parent->captured_values;
    $ldap = $this->get_ldap_link();
    $ldap->cd($cv['base']);

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

    /* Try to find an old GOsa 2.5 administrative account that may be migrated */
    if (!count($this->migrate_users)) {
      //FIXME
    }
  }
}
?>
