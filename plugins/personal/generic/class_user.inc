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
  \brief   user plugin
  \author  Cajus Pollmeier <pollmeier@gonicus.de>
  \version 2.00
  \date    24.07.2003

  This class provides the functionality to read and write all attributes
  relevant for person, organizationalPerson, inetOrgPerson and gosaAccount
  from/to the LDAP. It does syntax checking and displays the formulars required.
 */

class user extends plugin
{
  static $nouid = '{no-template-uid}';

  /* Plugin specific values */
  var $base       = "";
  var $orig_base  = "";
  var $cn         = "";
  var $new_dn     = "";

  var $personalTitle      = "";
  var $academicTitle      = "";
  var $homePostalAddress  = "";
  var $homePhone          = "";
  var $labeledURI         = "";
  var $departmentNumber   = "";
  var $description        = "";

  var $o  = "";
  var $ou = "";

  var $gosaLoginRestriction= array();
  var $gosaLoginRestrictionWidget;

  var $employeeNumber           = "";
  var $employeeType             = "";
  var $roomNumber               = "";
  var $telephoneNumber          = "";
  var $facsimileTelephoneNumber = "";

  var $mobile = "";
  var $pager  = "";
  var $l      = "";
  var $st     = "";

  var $postalAddress  = "";
  var $dateOfBirth;

  var $use_dob            = "0";
  var $gender             = "0";
  var $preferredLanguage  = "0";
  var $baseSelector;

  var $jpegPhoto      = "*removed*";
  var $photoData      = "";
  var $old_jpegPhoto  = "";
  var $old_photoData  = "";
  var $picture_dialog = FALSE;
  var $pwObject       = NULL;

  var $houseIdentifier            = "";
  var $street                     = "";
  var $postalCode                 = "";
  var $vocation                   = "";
  var $ivbbLastDeliveryCollective = "";

  var $gouvernmentOrganizationalUnit            = "";
  var $gouvernmentOrganizationalPersonLocality  = "";
  var $gouvernmentOrganizationalUnitDescription = "";
  var $gouvernmentOrganizationalUnitSubjectArea = "";

  var $functionalTitle  = "";
  var $role             = "";
  var $publicVisible    = "";

  var $orig_dn;
  var $dialog;

  /* variables to trigger password changes */
  var $pw_storage           = "";
  var $last_pw_storage      = "unset";
  var $force_hash           = FALSE;
  var $template_default_pw  = "";

  var $view_logged = FALSE;

  var $manager      = "";
  var $manager_name = "";

  var $passwordClass = NULL;

  /* attribute list for save action */
  var $attributes = array("sn", "givenName", "uid", "personalTitle", "academicTitle",
      "homePostalAddress", "homePhone", "labeledURI", "ou", "o", "dateOfBirth", "gender","preferredLanguage",
      "departmentNumber", "description", "employeeNumber", "employeeType", "l", "st", "jpegPhoto",
      "roomNumber", "telephoneNumber", "mobile", "pager", "cn", "street", "postalCode",
      "postalAddress", "facsimileTelephoneNumber", "gosaLoginRestriction", "manager");

  var $objectclasses = array("top", "person", "organizationalPerson", "inetOrgPerson",
      "gosaAccount");

  /* attributes that are part of the government mode */
  var $govattrs = array("gouvernmentOrganizationalUnit", "houseIdentifier", "vocation",
      "ivbbLastDeliveryCollective", "gouvernmentOrganizationalPersonLocality",
      "gouvernmentOrganizationalUnitDescription","gouvernmentOrganizationalUnitSubjectArea",
      "functionalTitle", "publicVisible", "street", "role",
      "postalCode");

  var $multiple_support = TRUE;

  var $governmentmode = FALSE;

  var $mobiles_available;
  var $phones_available;

  /* constructor, if 'dn' is set, the node loads the given
     'dn' from LDAP */
  function user (&$config, $dn = NULL, $object = NULL)
  {
    global $lang;

    $this->config = $config;
    /* Configuration is fine, allways */
    if ($this->config->get_cfg_value("honourIvbbAttributes") == "TRUE") {
      $this->governmentmode = TRUE;
      $this->attributes     = array_merge($this->attributes, $this->govattrs);
    }

    /* Load base attributes */
    plugin::plugin ($config, $dn, $object);

    $this->orig_dn  = $this->dn;
    $this->new_dn   = $dn;

    if ($this->governmentmode) {
      /* Fix public visible attribute if unset */
      if (!isset($this->attrs['publicVisible'])) {
        $this->publicVisible == "nein";
      }
    }

    /* Load government mode attributes */
    if ($this->governmentmode) {
      /* Copy all attributs */
      foreach ($this->govattrs as $val) {
        if (isset($this->attrs["$val"][0])) {
          $this->$val= $this->attrs["$val"][0];
        }
      }
    }

    /* Create me for new accounts */
    if ($dn == "new") {
      $this->is_account = TRUE;
    }

    /* Make hash default to md5 if not set in config */
    $this->pw_storage = $this->config->get_cfg_value("passwordDefaultHash", "crypt/md5");
    if ($this->config->get_cfg_value("forcePasswordDefaultHash", "FALSE") === "TRUE") {
      $this->force_hash = $this->pw_storage;
    }

    /* Load data from LDAP? */
    if ($dn !== NULL) {
      /* Do base conversation */
      if ($this->dn == "new") {
        $ui         = get_userinfo();
        $this->base = dn2base(session::global_is_set("CurrentMainBase")?"cn=dummy,".session::global_get("CurrentMainBase"):$ui->dn);
      } else {
        $this->base = dn2base($dn);
      }

      /* get password storage type */
      if (isset($this->attrs['userPassword'][0])) {
        /* Initialize local array */
        $matches = array();
        if (preg_match ("/^{[^}]+}/", $this->attrs['userPassword'][0])) {
          $userPassword = $this->attrs['userPassword'][0];
          if ($this->is_template && preg_match ('/\|/', $userPassword)) {
            list ($userPassword, $default_pw) = explode('|', $userPassword);
            $this->template_default_pw        = $default_pw;
          }
          $tmp = passwordMethod::get_method($userPassword);
          if (is_object($tmp)) {
            $this->pw_storage = $tmp->get_hash();
          }
        } else {
          if ($this->attrs['userPassword'][0] != "") {
            $this->pw_storage = "clear";
          }
        }
      }

      if ($this->uid == self::$nouid) {
        $this->uid = '';
      }

      /* Load extra attributes: picture */
      $this->load_picture();
    }

    /* Reset password storage indicator, used by password_change_needed() */
    if ($dn == "new") {
      $this->last_pw_storage = "unset";
    } else {
      $this->last_pw_storage = $this->pw_storage;
    }

    if ($this->force_hash !== FALSE) {
      $this->pw_storage = $this->force_hash;
    }

    /* Generate dateOfBirth entry */
    if (isset ($this->attrs['dateOfBirth'])) {
      /* This entry is ISO 8601 conform */
      list($year, $month, $day)= explode("-", $this->attrs['dateOfBirth'][0], 3);

      #TODO: use $lang to convert date
      $this->dateOfBirth= "$day.$month.$year";
    } else {
      $this->dateOfBirth= "";
    }

    /* Put gender attribute to upper case */
    if (isset ($this->attrs['gender'])){
      $this->gender= strtoupper($this->attrs['gender'][0]);
    }

    // Get login restrictions
    if(isset($this->attrs['gosaLoginRestriction'])){
      $this->gosaLoginRestriction  =array();
      for($i =0;$i < $this->attrs['gosaLoginRestriction']['count']; $i++){
        $this->gosaLoginRestriction[] = $this->attrs['gosaLoginRestriction'][$i];
      }
    }
    $this->gosaLoginRestrictionWidget= new sortableListing($this->gosaLoginRestriction);
    $this->gosaLoginRestrictionWidget->setDeleteable(true);
    $this->gosaLoginRestrictionWidget->setColspecs(array('*'));
    $this->gosaLoginRestrictionWidget->setWidth("100%");
    $this->gosaLoginRestrictionWidget->setHeight("70px");

    $this->orig_base = $this->base;
    $this->baseSelector= new baseSelector($this->allowedBasesToMoveTo(), $this->base);
    $this->baseSelector->setSubmitButton(false);
    $this->baseSelector->setHeight(300);
    $this->baseSelector->update(true);


    // Detect the managers name
    $this->manager_name = "";
    if (!empty($this->manager)) {
      $ldap = $this->config->get_ldap_link();
      $ldap->cat($this->manager, array('cn'));
      if ($ldap->count()) {
        $attrs = $ldap->fetch();
        $this->manager_name = $attrs['cn'][0];
      } else {
        $this->manager_name = "("._("Unknown")."!): ".$this->manager;
      }
    }

    $this->phones_available   = class_available('phoneGeneric');
    $this->mobiles_available  = class_available('mobilePhoneGeneric');
  }


  /* execute generates the html output for this node */
  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* Set list ACL */
    $restrict_writeable = $this->acl_is_writeable('gosaLoginRestriction', (!is_object($this->parent) && !session::is_set('edit')));
    $this->gosaLoginRestrictionWidget->setAcl($this->getacl('gosaLoginRestriction', (!is_object($this->parent) && !session::is_set('edit'))));
    $this->gosaLoginRestrictionWidget->update();

    /* Handle add/delete for restriction mode */
    if (isset($_POST['add_res']) && isset($_POST['res']) && $restrict_writeable) {
      $val= validate($_POST['res']);
      if (preg_match('/^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$/', $val) ||
          preg_match('/^([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)\/([0-9]+)$/', $val) ||
          preg_match('/^([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)\/([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)$/', $val)) {
        $this->gosaLoginRestrictionWidget->addEntry($val);
      } else {
        msg_dialog::display(_("Error"), _("Please add a single IP address or a network/netmask combination!"), ERROR_DIALOG);
      }
    }

    /* Log view */
    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","users/".get_class($this),$this->dn);
    }

    // Clear manager attribute if requested
    if(preg_match("/ removeManager/i", " ".implode(array_keys($_POST),' ')." ")){
      $this->manager = "";
      $this->manager_name = "";
    }

    // Allow to select a new inetOrgPersion:manager
    if(preg_match("/ editManager/i", " ".implode(array_keys($_POST),' ')." ")){
      $this->dialog = new singleUserSelect($this->config, get_userinfo());
    }
    if($this->dialog instanceOf singleUserSelect && count($this->dialog->detectPostActions())){
      $users = $this->dialog->detectPostActions();
      if(isset($users['targets']) && count($users['targets'])){

        $headpage = $this->dialog->getHeadpage();
        $dn = $users['targets'][0];
        $attrs = $headpage->getEntry($dn);
        $this->manager = $dn;
        $this->manager_name = $attrs['cn'][0];
        $this->dialog = NULL;
      }
    }
    if(isset($_POST['add_users_cancel'])){
      $this->dialog = NULL;
    }
    if($this->dialog instanceOf singleUserSelect) {
      return($this->dialog->execute());
    }
    if ($this->mobiles_available || $this->phones_available) {
      if (preg_match("/ edit(Phone|Mobile)/i", " ".implode(array_keys($_POST),' '), $m)) {
        $this->dialog = new phoneSelect($this->config, get_userinfo(), ($m[1] == 'Mobile'));
      } elseif (isset($_POST['select_phone_cancel'])) {
        $this->dialog = NULL;
      } elseif ($this->dialog instanceOf phoneSelect) {
        $phone = $this->dialog->detectPostActions();
        if (isset($phone['targets']) && count($phone['targets'])) {
          $attrs = $this->dialog->getHeadpage()->getEntry($phone['targets'][0]);
          if ($this->dialog->mobileDialog) {
            $this->mobile = $attrs['telephoneNumber'][0];
          } else {
            $this->telephoneNumber = $attrs['goFonMSN'][0];
          }
          $this->dialog = NULL;
        }
      }
      if($this->dialog instanceOf phoneSelect) {
        return $this->dialog->execute();
      }
    }


    $smarty = get_smarty();
    $smarty->assign("usePrototype", "true");
    $smarty->assign("gosaLoginRestrictionWidget", $this->gosaLoginRestrictionWidget->render());
    $smarty->assign("phone_dialog_available", $this->phones_available);
    $smarty->assign("mobile_dialog_available", $this->mobiles_available);

    /* Assign sex */
    $sex= array(0 => "&nbsp;", "F" => _("female"), "M" => _("male"));
    $smarty->assign("gender_list", $sex);
    $language= array_merge(array(0 => "&nbsp;") ,get_languages(TRUE));
    $smarty->assign("preferredLanguage_list", $language);

    /* Get random number for pictures */
    srand((double)microtime()*1000000);
    $smarty->assign("rand", rand(0, 10000));


    /* Do we represent a valid gosaAccount? */
    if (!$this->is_account){
      $str = "<img alt=\"\" src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".
        msgPool::noValidExtension("FusionDirectory")."</b>";
      return($str);
    }

    /* Password configure dialog handling */
    if(is_object($this->pwObject) && $this->pwObject->display){
      $output= $this->pwObject->configure();
      if ($output != ""){
        $this->dialog= TRUE;
        return $output;
      }
      $this->dialog= false;
    }

    /* Want password method editing? */
    if ($this->acl_is_writeable("userPassword")){
      if (isset($_POST['edit_pw_method'])){
        if (!is_object($this->pwObject) || $this->pw_storage != $this->pwObject->get_hash_name()){
          $temp= passwordMethod::get_available_methods();
          $this->pwObject= new $temp[$this->pw_storage]($this->config,$this->dn);
        }
        $this->pwObject->display = TRUE;
        $this->dialog= TRUE;
        return ($this->pwObject->configure());
      }
    }

    /* Want picture edit dialog? */
    if($this->acl_is_writeable("userPicture")) {
      if (isset($_POST['edit_picture'])){
        /* Save values for later recovery, in case some presses
           the cancel button. */
        $this->old_jpegPhoto= $this->jpegPhoto;
        $this->old_photoData= $this->photoData;
        $this->picture_dialog= TRUE;
        $this->dialog= TRUE;
      }
    }

    /* Remove picture? */
    if($this->acl_is_writeable("userPicture",(!is_object($this->parent) && !session::is_set('edit'))) ){
      if (isset($_POST['picture_remove'])){
        $this->set_picture ();
        $this->jpegPhoto= "*removed*";
        $this->is_modified= TRUE;
        return($smarty->fetch (get_template_path('generic_picture.tpl', TRUE, dirname(__FILE__))));
      }
    }

    /* Save picture */
    if (isset($_POST['picture_edit_finish'])){

      /* Check for clean upload */
      if ($_FILES['picture_file']['name'] != ""){
        if (!is_uploaded_file($_FILES['picture_file']['tmp_name'])) {
          msg_dialog::display(_("Error"), _("Cannot upload file!"), ERROR_DIALOG);
        }else{
          /* Activate new picture */
          $this->set_picture($_FILES['picture_file']['tmp_name']);
        }
      }
      $this->picture_dialog= FALSE;
      $this->dialog= FALSE;
      $this->is_modified= TRUE;
    }


    /* Cancel picture */
    if (isset($_POST['picture_edit_cancel'])){

      /* Restore values */
      $this->jpegPhoto= $this->old_jpegPhoto;
      $this->photoData= $this->old_photoData;

      /* Update picture */
      session::set('binary',$this->photoData);
      session::set('binarytype',"image/jpeg");
      $this->picture_dialog= FALSE;
      $this->dialog= FALSE;
    }

    /* Display picture dialog */
    if ($this->picture_dialog){
      return($smarty->fetch (get_template_path('generic_picture.tpl', TRUE, dirname(__FILE__))));
    }

    /* Prepare password hashes */
    if ($this->pw_storage == ""){
      $this->pw_storage= $this->config->get_cfg_value("passwordDefaultHash");
    }

    $temp= passwordMethod::get_available_methods();
    $is_configurable= FALSE;
    $hashes = $temp['name'];
    if(isset($temp[$this->pw_storage])){
      $test= new $temp[$this->pw_storage]($this->config);
      $is_configurable= $test->is_configurable();
    }else{
      new msg_dialog(_("Password method"),_("The selected password method is no longer available."),WARNING_DIALOG);
    }


    /* Create password methods array */
    $pwd_methods = array();
    foreach($hashes as $id => $name){
      if(!empty($temp['desc'][$id])){
        $pwd_methods[$name] = $name." (".$temp['desc'][$id].")";
      }else{
        $pwd_methods[$name] = $name;
      }
    }

    /* Load attributes and acl's */
    $ui = get_userinfo();
    foreach($this->attributes as $val){
      $smarty->assign("$val", $this->$val);
    }

    /* Set acls */
    $tmp = $this->plinfo();
    foreach($tmp['plProvidedAcls'] as $val => $translation){
        $smarty->assign("$val"."ACL", $this->getacl($val,(!is_object($this->parent) && !session::is_set('edit'))));
    }

    // Special ACL for gosaLoginRestrictions -
    // In case of multiple edit, we need a readonly ACL for the list.
    $smarty->assign('gosaLoginRestriction_ONLY_R_ACL',
      preg_replace("/[^r]/i","", $this->getacl("gosaLoginRestriction",(!is_object($this->parent) && !session::is_set('edit')))));

    $smarty->assign("pwmode", $pwd_methods);
    $smarty->assign("pwmode_select", $this->pw_storage);
    $smarty->assign("pw_configurable", $is_configurable);
    $smarty->assign("disabled_pw_storage", ($this->force_hash === FALSE?"":" disabled"));
    $smarty->assign("passwordStorageACL", $this->getacl("userPassword",(!is_object($this->parent) && !session::is_set('edit'))));

    $smarty->assign("userPictureACL",   $this->getacl("userPicture",(!is_object($this->parent) && !session::is_set('edit'))));
    $smarty->assign("userPicture_is_readable",   $this->acl_is_readable("userPicture",(!is_object($this->parent) && !session::is_set('edit'))));

    /* Create base acls */
    $smarty->assign("base", $this->baseSelector->render());

    /* Save government mode attributes */
    if($this->governmentmode){
      $smarty->assign("governmentmode", "true");
      $ivbbmodes= array("nein", "ivbv", "testa", "ivbv,testa", "internet",
          "internet,ivbv", "internet,testa", "internet,ivbv,testa");
      $smarty->assign("ivbbmodes", $ivbbmodes);
      foreach ($this->govattrs as $val){
        $smarty->assign("$val", $this->$val);
        $smarty->assign("$val"."ACL", $this->getacl($val,(!is_object($this->parent) && !session::is_set('edit'))));
      }
    } else {
      $smarty->assign("governmentmode", "false");
    }

    /* Special mode for uid */
    $uidACL = $this->getacl("uid", (!is_object($this->parent) && !session::is_set('edit')));
    if (!(isset($this->dn) && ($this->dn == "new")) && !$this->is_template) {
      $uidACL = preg_replace("/w/", "", $uidACL);
    }

    $smarty->assign("uidACL", $uidACL);
    $smarty->assign("is_template", $this->is_template);
    $smarty->assign("use_dob", $this->use_dob);

    if (isset($this->parent)) {
      if (isset($this->parent->by_object['phoneAccount']) &&
          $this->parent->by_object['phoneAccount']->is_account) {
        $smarty->assign("has_phoneaccount", "true");
      } else {
        $smarty->assign("has_phoneaccount", "false");
      }
    } else {
      $smarty->assign("has_phoneaccount", "false");
    }
    $smarty->assign("manager_name",$this->manager_name);

    if ($this->is_template) {
      $smarty->assign('default_pw', $this->template_default_pw);
    }

    return $smarty->fetch (get_template_path('generic.tpl', TRUE, dirname(__FILE__)));
  }


  /* remove object from parent */
  function remove_from_parent()
  {
    /* Only remove valid accounts */
    if(!$this->initially_was_account) return;

    /* Remove password extension */
    $temp= passwordMethod::get_available_methods();

    /* Remove password method from user account */
    if(isset($temp[$this->pw_storage]) && class_available($temp[$this->pw_storage])){
      $this->pwObject= new $temp[$this->pw_storage]($this->config,$this->dn);
      $this->pwObject->remove_from_parent();
    }

    /* Remove user */
    $ldap= $this->config->get_ldap_link();
    $ldap->rmdir ($this->dn);
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
    }

    new log("remove","users/".get_class($this),$this->dn,$this->attributes,$ldap->get_error());

    /* Delete references to groups */
    $ldap->cd ($this->config->current['BASE']);
    $ldap->search ("(&(objectClass=posixGroup)(memberUid=".$this->uid."))", array("uid"));
    while ($ldap->fetch()){
      $g= new group($this->config, $ldap->getDN());
      $g->removeUser($this->uid);
      $g->save ();
    }

    /* Delete references to object groups */
    $ldap->cd ($this->config->current['BASE']);
    $ldap->search ("(&(objectClass=gosaGroupOfNames)(member=".LDAP::prepare4filter($this->dn)."))", array("cn"));
    while ($ldap->fetch()){
      $og= new ogroup($this->config, $ldap->getDN());
      unset($og->member[$this->dn]);
      $og->save ();
    }

    // Update 'manager' attributes from gosaDepartment and inetOrgPerson
    $filter = "(&(objectClass=inetOrgPerson)(manager=".LDAP::prepare4filter($this->dn)."))";
    $ocs = $ldap->get_objectclasses();
    if(isset($ocs['gosaDepartment']['MAY']) && in_array('manager', $ocs['gosaDepartment']['MAY'])){
      $filter = "(|".$filter."(&(objectClass=gosaDepartment)(manager=".LDAP::prepare4filter($this->dn).")))";
    }
    $leaf_deps=  get_list($filter,array("all"),$this->config->current['BASE'],
        array("manager","dn","objectClass"),GL_SUBSEARCH | GL_NO_ACL_CHECK);
    foreach($leaf_deps as $entry){
      $update = array('manager' => array());
      $ldap->cd($entry['dn']);
      $ldap->modify($update);
      if(!$ldap->success()){
        trigger_error(sprintf("Failed to update manager for '%s', error was '%s'", $entry['dn'], $ldap->get_error()));
      }
    }

    /* Delete references to roles */
    $ldap->cd ($this->config->current['BASE']);
    $ldap->search ("(&(objectClass=organizationalRole)(roleOccupant=".LDAP::prepare4filter($this->dn)."))", array("cn"));
    while ($ldap->fetch()){
      $role= new roleGeneric($this->config, $ldap->getDN());
      $key = array_search($this->dn,$role->roleOccupant);
      if($key !== FALSE){
        unset($role->roleOccupant[$key]);
        $role->roleOccupant= array_values($role->roleOccupant);
        $role->save ();
      }
    }

    /* If needed, let the password method do some cleanup */
    $tmp = new passwordMethod($this->config);
    $available = $tmp->get_available_methods();
    if (in_array_ics($this->pw_storage, $available['name'])){
      $test= new $available[$this->pw_storage]($this->config);
      $test->attrs= $this->attrs;
      $test->dn= $this->dn;
      $test->remove_from_parent();
    }

    /* Remove ACL dependencies too */
    acl::remove_acl_for($this->dn);

    /* Optionally execute a command after we're done */
    $this->handle_post_events("remove",array("uid" => $this->uid));
  }


  /* Save data to object */
  function save_object()
  {
    if (isset($_POST['generic'])) {

      /* Make a backup of the current selected base */
      $base_tmp = $this->base;

      /* Parents save function */
      plugin::save_object ();

      /* Refresh base */
      if ($this->acl_is_moveable($this->base) ||
            ($this->dn == "new" && $this->acl_is_createable($this->base))){
        if (!$this->baseSelector->update()) {
          msg_dialog::display(_("Error"), msgPool::permMove(), ERROR_DIALOG);
        }
        if ($this->base != $this->baseSelector->getBase()) {
          $this->base= $this->baseSelector->getBase();
          $this->is_modified= TRUE;
        }
      }

      /* Sync lists */
      $this->gosaLoginRestrictionWidget->save_object();
      if ($this->gosaLoginRestrictionWidget->isModified()) {
        $this->gosaLoginRestriction= array_values($this->gosaLoginRestrictionWidget->getMaintainedData());
      }

      /* Save government mode attributes */
      if ($this->governmentmode){
        foreach ($this->govattrs as $val){
          if ($this->acl_is_writeable($val,(!is_object($this->parent) && !session::is_set('edit'))) && isset($_POST["$val"])){
            $data= stripcslashes($_POST["$val"]);
            if ($data != $this->$val){
              $this->is_modified= TRUE;
            }
            $this->$val= $data;
          }
        }
      }

      /* Get pw_storage mode */
      if (isset($_POST['pw_storage'])) {
        $data = validate($_POST['pw_storage']);
        if ($data != $this->pw_storage) {
          $this->is_modified = TRUE;
        }
        $this->pw_storage = $data;
      }

      if ($this->pw_storage != $this->last_pw_storage && isset($_POST['pw_storage'])) {
        if ($this->acl_is_writeable("userPassword")) {
          $temp = passwordMethod::get_available_methods();
          if (!is_object($this->pwObject) || !($this->pwObject instanceOf $temp[$this->pw_storage])) {
            foreach ($temp as $id => $data) {
              if (isset($data['name']) && $data['name'] == $this->pw_storage && $data['is_configurable']) {
                $this->pwObject = new $temp[$this->pw_storage]($this->config,$this->dn);
                break;
              }
            }
          }
        }
      }

      if ($this->is_template) {
        /* Template mode */
        $this->givenName  = $this->sn;
        $this->cn         = $this->sn;
        if (isset($_POST['default_pw'])) {
          $this->template_default_pw = $_POST['default_pw'];
        }
      } else {
        /* Save current cn */
        $this->cn = $this->givenName." ".$this->sn;
      }
    }
  }

  function rebind($ldap, $referral)
  {
    $credentials= LDAP::get_credentials($referral, $this->config->current['REFERRAL']);
    if (ldap_bind($ldap, $credentials['ADMIN'], $credentials['PASSWORD'])) {
      $this->error = "Success";
      $this->hascon=true;
      $this->reconnect= true;
      return (0);
    } else {
      $this->error = "Could not bind to " . $credentials['ADMIN'];
      return NULL;
    }
  }


  /* Save data to LDAP, depending on is_account we save or delete */
  function save()
  {
    global $lang;

    /* Only force save of changes ....
       If this attributes aren't changed, avoid saving.
     */

    if($this->gender=="0") $this->gender ="";
    if($this->preferredLanguage=="0") $this->preferredLanguage ="";

    /* First use parents methods to do some basic fillup in $this->attrs */
    plugin::save ();

    if ($this->dateOfBirth != ""){
      if(!is_array($this->attrs['dateOfBirth'])) {
        #TODO: use $lang to convert date
        list($day, $month, $year)= explode(".", $this->dateOfBirth);
        $this->attrs['dateOfBirth'] = sprintf("%04d-%02d-%02d", $year, $month, $day);
      }
    }

    /* Remove additional objectClasses */
    $tmp= array();
    foreach ($this->attrs['objectClass'] as $key => $set){
      $found= false;
      foreach (array("ivbbentry", "gosaUserTemplate") as $val){
        if (preg_match ("/^$set$/i", $val)){
          $found= true;
          break;
        }
      }
      if (!$found){
        $tmp[]= $set;
      }
    }

    /* Replace the objectClass array. This is done because of the
       separation into government and normal mode. */
    $this->attrs['objectClass']= $tmp;

    /* Add objectClasss for template mode? */
    if ($this->is_template){
      $this->attrs['objectClass'][]= "gosaUserTemplate";
    }

    /* Hard coded government mode? */
    if ($this->governmentmode){
      $this->attrs['objectClass'][]= "ivbbentry";

      /* Copy standard attributes */
      foreach ($this->govattrs as $val){
        if ($this->$val != ""){
          $this->attrs["$val"]= $this->$val;
        } elseif (!$this->is_new) {
          $this->attrs["$val"]= array();
        }
      }

      /* Remove attribute if set to "nein" */
      if ($this->publicVisible == "nein"){
        $this->attrs['publicVisible']= array();
        if($this->is_new){
          unset($this->attrs['publicVisible']);
        }else{
          $this->attrs['publicVisible']=array();
        }

      }

    }

    /* Special handling for dateOfBirth value */
    if ($this->dateOfBirth == ""){
      if ($this->is_new) {
        unset($this->attrs["dateOfBirth"]);
      } else {
        $this->attrs["dateOfBirth"]= array();
      }
    }
    if (!$this->gender){
      if ($this->is_new) {
        unset($this->attrs["gender"]);
      } else {
        $this->attrs["gender"]= array();
      }
    }
    if (!$this->preferredLanguage){
      if ($this->is_new) {
        unset($this->attrs["preferredLanguage"]);
      } else {
        $this->attrs["preferredLanguage"]= array();
      }
    }

    /* Special handling for attribute jpegPhote needed, scale image via
       image magick to 150x200 pixels and inject resulting data. */
    if ($this->jpegPhoto == "*removed*"){
      /* Reset attribute to avoid writing *removed* as value */
      $this->attrs["jpegPhoto"] = array();
    } else {
      if (class_exists('Imagick')) {
        $width  = 150;
        $height = 200;
        $im = new Imagick();
        $modify = FALSE;
        $im->readImageBlob($this->photoData);

        $size = $im->getImageGeometry();

        if (($size['width'] > 0 && $size['height'] > 0) && (($size['width'] < $width && $size['height'] < $height) || $size['width'] > $width || $size['height'] > $height)) {
          $modify = TRUE;
          $im->resizeImage($width, $height, Imagick::FILTER_GAUSSIAN, 1, TRUE);
        }

        if ($modify || !preg_match('/^jpeg$/i',$im->getImageFormat())) {
          $im->setImageCompression(Imagick::COMPRESSION_JPEG);
          $im->setImageCompressionQuality(90);
          $im->setImageFormat('jpeg');

          /* Save attribute */
          $this->attrs["jpegPhoto"] = $im->getImageBlob();
        } else {
          $this->attrs["jpegPhoto"] = $this->photoData;
        }
      } else {
        msg_dialog::display(_("Error"),
                  _("Cannot save user picture, FusionDirectory requires the package 'php5-imagick' to be installed!"),
                  ERROR_DIALOG);
      }
    }

    /* This only gets called when user is renaming himself */
    $ldap= $this->config->get_ldap_link();
    if ($this->dn != $this->new_dn){

      /* Write entry on new 'dn' */
      $this->update_acls($this->dn,$this->new_dn);
      $this->move($this->dn, $this->new_dn);

      /* Happen to use the new one */
      change_ui_dn($this->dn, $this->new_dn);
      $this->dn= $this->new_dn;
    }


    /* Save data. Using 'modify' implies that the entry is already present, use 'add' for
       new entries. So do a check first... */
    $ldap->cat ($this->dn, array('dn'));
    if ($ldap->fetch()){
      $mode= "modify";
    } else {
      $mode= "add";
      $ldap->cd($this->config->current['BASE']);
      $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
    }

    /* Set password to some junk stuff in case of templates */
    if ($this->is_template){
      $temp = passwordMethod::get_available_methods();
      foreach ($temp as $id => $data) {
        if (isset($data['name']) && ($data['name'] == $this->pw_storage)) {
          $tmp = new $temp[$this->pw_storage]($this->config, $this->dn);
          $tmp->set_hash($this->pw_storage);
          if ($this->template_default_pw != "") {
            $this->attrs['userPassword'] = $tmp->create_template_hash($this->attrs).'|'.$this->template_default_pw;
          } else {
            $this->attrs['userPassword'] = $tmp->create_template_hash($this->attrs);
          }
          break;
        }
      }

      /* Empty uid not allowed by LDAP schema */
      if ($this->uid == '') {
        $this->attrs['uid'] = self::$nouid;
      }
    }

    @DEBUG (DEBUG_LDAP, __LINE__, __FUNCTION__, __FILE__,
        $this->attributes, "Save via $mode");

    /* Finally write data with selected 'mode' */
    $this->cleanup();

    /* Update current locale settings, if we have edited ourselves */
    $ui = session::get('ui');
    if(isset($this->attrs['preferredLanguage']) && $this->dn == $ui->dn){
      $ui->language = $this->preferredLanguage;
      session::set('ui',$ui);
      session::set('Last_init_lang',"update");
    }

    $ldap->cd ($this->dn);
    $ldap->$mode ($this->attrs);
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
      return (1);
    }

    /* Remove ACL dependencies too */
    if ($this->dn != $this->orig_dn && $this->orig_dn != "new") {
      $tmp = new acl($this->config, $this->dn, $this->parent);
      $tmp->update_acl_membership($this->orig_dn, $this->dn);
    }

    if($mode == "modify"){
      new log("modify","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }else{
      new log("create","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }

    /* If needed, let the password method do some cleanup */
    if ($this->pw_storage != $this->last_pw_storage){
      $tmp = new passwordMethod($this->config);
      $available = $tmp->get_available_methods();
      if (in_array_ics($this->last_pw_storage, $available['name'])){
        $test= new $available[$this->last_pw_storage]($this->config,$this->dn);
        $test->attrs= $this->attrs;
        $test->remove_from_parent();
      }
    } elseif (!$this->is_template && ($this->template_default_pw != "")) {
      change_password ($this->dn, $this->template_default_pw, 0, $this->pw_storage);
    }

    /* Maybe the current password method want's to do some changes... */
    if (is_object($this->pwObject)){
      $this->pwObject->save($this->dn);
    }

    /* Optionally execute a command after we're done */
    if ($mode == "add") {
      $this->handle_post_events("add", array("uid" => $this->uid));
    } elseif ($this->is_modified) {
      $this->handle_post_events("modify", array("uid" => $this->uid));
    }

    return 0;
  }


  function create_initial_rdn($pattern)
  {
    // Only generate single RDNs
    if (preg_match('/\+/', $pattern)){
      msg_dialog::display(_("Error"), _("Cannot build RDN: no + allowed to build sub RDN!"), ERROR_DIALOG);
      return "";
    }

    // Extract attribute
    $attribute= preg_replace('/=.*$/', '', $pattern);
    if (!in_array_ics($attribute, $this->attributes)) {
      msg_dialog::display(_("Error"), _("Cannot build RDN: attribute is not defined!"), ERROR_DIALOG);
      return "";
    }

    // Sort attributes for length
    $attrl= array();
    foreach ($this->attributes as $attr) {
      $attrl[$attr]= strlen($attr);
    }
    arsort($attrl);

    // Walk thru sorted attributes and replace them in pattern
    foreach ($attrl as $attr => $dummy) {
      if (!is_array($this->$attr)) {
        $pattern = preg_replace("/%$attr%/", $this->$attr, $pattern);
      } else {
        // Array elements cannot be used for ID generation
        if (preg_match("/%$attr%/", $pattern)) {
          msg_dialog::display(_("Error"), _("Cannot build RDN: invalid attribute parameters!"), ERROR_DIALOG);
          break;
        }
      }
    }

    // Internally assign value
    $this->$attribute = preg_replace('/^[^=]+=/', '', $pattern);

    return $pattern;
  }


  function update_new_dn()
  {
    // Alternative way to handle DN
    $pattern = $this->config->get_cfg_value("accountRDN");
    if ($this->is_template) { // Use cn in dn for user templates
      /* Don't touch dn, if cn hasn't changed */
      if (isset($this->saved_attributes['cn']) && $this->saved_attributes['cn'] == $this->cn &&
          $this->orig_base == $this->base ) {
        $this->new_dn = $this->dn;
      } else {
        $this->new_dn = $this->create_unique_dn('cn', get_people_ou().$this->base);
      }
    } elseif ($pattern != "") {
      $rdn = $this->create_initial_rdn($pattern);
      $attribute = preg_replace('/=.*$/', '', $rdn);
      $value = preg_replace('/^[^=]+=$/', '', $rdn);

      /* Don't touch dn, if $attribute hasn't changed */
      if (isset($this->saved_attributes[$attribute]) && $this->saved_attributes[$attribute] == $this->$attribute &&
            $this->orig_base == $this->base ) {
        $this->new_dn = $this->dn;
      } else {
        $this->new_dn = $this->create_unique_dn2($rdn, get_people_ou().$this->base);
      }

    // Original way to handle DN
    } else {

      $pt = "";
      if ($this->config->get_cfg_value("personalTitleInDN") == "TRUE") {
        if (!empty($this->personalTitle)) {
          $pt = $this->personalTitle." ";
        }
      }

      $this->cn = $pt.$this->givenName." ".$this->sn;

      /* Permissions for that base? */
      if ($this->config->get_cfg_value("accountPrimaryAttribute") == "uid") {
        $this->new_dn = 'uid='.$this->uid.','.get_people_ou().$this->base;
      } else {
        /* Don't touch dn, if cn hasn't changed */
        if (isset($this->saved_attributes['cn']) && $this->saved_attributes['cn'] == $this->cn &&
            $this->orig_base == $this->base ) {
          $this->new_dn = $this->dn;
        } else {
          $this->new_dn = $this->create_unique_dn('cn', get_people_ou().$this->base);
        }
      }
    }
  }


  /* Check formular input */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    /* Configurable password methods should be configured initially.
     */
    if($this->last_pw_storage != $this->pw_storage){
      $temp= passwordMethod::get_available_methods();
      foreach($temp['name'] as $id => $name){
        if($name == $this->pw_storage){
          if($temp['is_configurable'][$id] && !$this->pwObject instanceof $temp[$name] ){
            $message[] = _("The selected password method requires initial configuration!");
          }
          break;
        }
      }
    }

    $this->update_new_dn();

    /* Set the new acl base */
    if($this->dn == "new") {
      $this->set_acl_base($this->base);
    }

    /* Check if we are allowed to create/move this user */
    if($this->orig_dn == "new" && !$this->acl_is_createable($this->base)){
      $message[]= msgPool::permCreate();
    }elseif($this->orig_dn != "new" && $this->new_dn != $this->orig_dn && !$this->acl_is_moveable($this->base)){
      $message[]= msgPool::permMove();
    }

    /* UID already used? */
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(uid=$this->uid)", array("uid"));
    $ldap->fetch();
    if ($ldap->count() != 0 && $this->dn == 'new'){
      $message[]= msgPool::duplicated(_("Login"));
    }

    /* In template mode, the uid and givenName are autogenerated... */
    if ($this->sn == ""){
      $message[]= msgPool::required(_("Name"));
    }

    // Check if a wrong base was supplied
    if(!$this->baseSelector->checkLastBaseUpdate()){
      $message[]= msgPool::check_base();;
    }

    if (!$this->is_template){
      if ($this->givenName == ""){
        $message[]= msgPool::required(_("Given name"));
      }
      if ($this->uid == ""){
        $message[]= msgPool::required(_("Login"));
      }
      if ($this->config->get_cfg_value("accountPrimaryAttribute") != "uid"){
        $ldap->cat($this->new_dn);
        if ($ldap->count() != 0 && $this->dn != $this->new_dn && $this->dn == 'new'){
          $message[]= msgPool::duplicated(_("Name"));
        }
      }

      /* Check for valid input */
      if ($this->is_modified && !tests::is_uid($this->uid)){

        if (strict_uid_mode()){
          $message[]= msgPool::invalid(_("Login"), $this->uid, "/[a-z0-9_-]/");
        } else {
          $message[]= msgPool::invalid(_("Login"), $this->uid, "/[a-z0-9_-]/i");
        }
      }
      if (!tests::is_url($this->labeledURI)){
        $message[]= msgPool::invalid(_("Homepage"), "", "", "http://www.your-domain.com/yourname");
      }

      /* Check phone numbers */
      if (!tests::is_phone_nr($this->telephoneNumber)){
        $message[]= msgPool::invalid(_("Phone"), $this->telephoneNumber, "/[\/0-9 ()+*-]/");
      }
      if (!tests::is_phone_nr($this->facsimileTelephoneNumber)){
        $message[]= msgPool::invalid(_("Fax"), $this->facsimileTelephoneNumber, "/[\/0-9 ()+*-]/");
      }
      if (!tests::is_phone_nr($this->mobile)){
        $message[]= msgPool::invalid(_("Mobile"), $this->mobile, "/[\/0-9 ()+*-]/");
      }
      if (!tests::is_phone_nr($this->pager)){
        $message[]= msgPool::invalid(_("Pager"), $this->pager, "/[\/0-9 ()+*-]/");
      }

      /* Check dates */
      if (!tests::is_date($this->dateOfBirth)){
        $message[]= msgPool::invalid(_("Date of birth"), $this->dateOfBirth,"" ,"23.02.2009");
      }
    }

    /* Check for reserved characers */
    if (preg_match ('/[,+"?\'()=<>;\\\\]/', $this->givenName)){
      $message[]= msgPool::invalid(_("Given name"), $this->givenName, '/[^,+"?\'()=<>;\\\\]/');
    }
    if (preg_match ('/[,+"?\'()=<>;\\\\]/', $this->sn)){
      $message[]= msgPool::invalid(_("Name"), $this->sn, '/[^,+"?\'()=<>;\\\\]/');
    }

    return $message;
  }


  /* Indicate whether a password change is needed or not */
  function password_change_needed()
  {
    return ($this->pw_storage != $this->last_pw_storage && !$this->is_template);
  }


  /* Load a jpegPhoto from LDAP, this is going to be simplified later on */
  function load_picture()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd ($this->dn);
    $data = $ldap->get_attribute($this->dn,"jpegPhoto");

    if((!$data) || ($data == "*removed*")){

      /* In case we don't get an entry, load a default picture */
      $this->set_picture ();
      $this->jpegPhoto= "*removed*";
    }else{

      /* Set picture */
      $this->photoData= $data;
      session::set('binary',$this->photoData);
      session::set('binarytype',"image/jpeg");
      $this->jpegPhoto= "";
    }
  }

  /* Load picture from file to object */
  function set_picture($filename = "")
  {
    if (!is_file($filename) || $filename == "" ) {
      $filename= "./plugins/users/images/default.jpg";
      $this->jpegPhoto= "*removed*";
    }

    clearstatcache();
    $fd = fopen ($filename, "rb");
    $this->photoData= fread ($fd, filesize ($filename));
    session::set('binary',$this->photoData);
    session::set('binarytype',"image/jpeg");
    $this->jpegPhoto= "";

    fclose ($fd);
  }

  /* Adapt from given 'dn' */
  function adapt_from_template($dn, $skip= array())
  {
    plugin::adapt_from_template($dn, $skip);

    /* Get password method from template
     */
    $tmp_array  = explode('|', $this->attrs['userPassword'][0], 2);
    $hash       = $tmp_array[0];
    $default_pw = (isset($tmp_array[1])?$tmp_array[1]:'');
    $tmp = passwordMethod::get_method($hash);
    if (is_object($tmp)) {
      if ($tmp->is_configurable()) {
        $tmp->adapt_from_template($dn);
        $this->pwObject = &$tmp;
      }
      $this->pw_storage= $tmp->get_hash();
    }

    if ($default_pw != '') {
      foreach (array("sn", "givenName", "uid") as $repl) {
        if (preg_match("/%$repl%/i", $default_pw)) {
          $default_pw = preg_replace ("/%$repl%/i", $this->parent->$repl, $default_pw);
        }
      }
      $this->template_default_pw  = $default_pw;
      $this->last_pw_storage      = $this->pw_storage;
    }

    /* Get base */
    $this->base= preg_replace('/^[^,]+,'.preg_quote(get_people_ou(), '/').'/i', '', $dn);
    $this->baseSelector->setBase($this->base);

    if($this->governmentmode){

      /* Walk through govattrs */
      foreach ($this->govattrs as $val){

        if (in_array($val, $skip)){
          continue;
        }

        if (isset($this->attrs["$val"][0])) {

          /* If attribute is set, replace dynamic parts:
             %sn, %givenName and %uid. Fill these in our local variables. */
          $value = $this->attrs["$val"][0];

          foreach (array("sn", "givenName", "uid") as $repl) {
            if (preg_match("/%$repl%/i", $value)) {
              $value = preg_replace ("/%$repl%/i",
                  $this->parent->$repl, $value);
            }
          }
          $this->$val = $value;
        }
      }
    }

    /* Get back uid/sn/givenName - only write if nothing's skipped */
    if ($this->parent !== NULL && count($skip) == 0){
      $this->uid= $this->parent->uid;
      $this->sn= $this->parent->sn;
      $this->givenName= $this->parent->givenName;
    }


    /* Generate dateOfBirth entry */
    if (isset ($this->attrs['dateOfBirth'])){
      /* This entry is ISO 8601 conform */
      list($year, $month, $day)= explode("-", $this->attrs['dateOfBirth'][0], 3);

      #TODO: use $lang to convert date
      $this->dateOfBirth= "$day.$month.$year";
    } else {
      $this->dateOfBirth= "";
    }

  }


  /* This avoids that users move themselves out of their rights.
   */
  function allowedBasesToMoveTo()
  {
    /* Get bases */
    $bases  = $this->get_allowed_bases();
    return($bases);
  }

  /* FIXME : is that useful? */
  function postCopyHook()
  {
    $this->load_picture();
  }


  static function plInfo()
  {
    $govattrs = array(
        "gouvernmentOrganizationalUnit"             =>  _("Unit"),
        "houseIdentifier"                           =>  _("House identifier"),
        "vocation"                                  =>  _("Vocation"),
        "ivbbLastDeliveryCollective"                =>  _("Last delivery"),
        "gouvernmentOrganizationalPersonLocality"   =>  _("Person locality"),
        "gouvernmentOrganizationalUnitDescription"  =>  _("Unit description"),
        "gouvernmentOrganizationalUnitSubjectArea"  =>  _("Subject area"),
        "functionalTitle"                           =>  _("Functional title"),
        "publicVisible"                             =>  _("Public visible"),
        "street"                                    =>  _("Street"),
        "role"                                      =>  _("Role"),
        "postalCode"                                =>  _("Postal code"));

    $ret = array(
        "plShortName"   => _("Generic"),
        "plDescription" => _("Generic user settings"),
        "plSelfModify"  => TRUE,
        "plCategory"    => array("users" => array("description" => _("Users"),
                                                  "objectClass" => "gosaAccount")),
        "plObjectType"  => array("user" => array("name" => _("User"),
                                                  "filter" => "objectClass=gosaAccount")),

        "plProvidedAcls" => array(
          "sn"                => _("Surname"),
          "givenName"         => _("Given name"),
          "uid"               => _("User identification"),
          "personalTitle"     => _("Personal title"),
          "academicTitle"     => _("Academic title"),

          "dateOfBirth"       => _("Date of birth"),
          "gender"            => _("Sex"),
          "preferredLanguage" => _("Preferred language"),
          "base"              => _("Base"),

          "userPicture"       => _("User picture"),

          "gosaLoginRestriction" => _("Login restrictions"),

          "o"                 => _("Organization"),
          "ou"                => _("Department"),
          "departmentNumber"  => _("Department number"),
          "description"       => _("Description"),
          "manager"           => _("Manager"),
          "employeeNumber"    => _("Employee number"),
          "employeeType"      => _("Employee type"),

          "roomNumber"        => _("Room number"),
          "telephoneNumber"   => _("Telefon number"),
          "pager"             => _("Pager number"),
          "mobile"            => _("Mobile number"),
          "facsimileTelephoneNumber"     => _("Fax number"),

          "st"                => _("State"),
          "l"                 => _("Location"),
          "postalAddress"     => _("Postal address"),

          "homePostalAddress" => _("Home postal address"),
          "homePhone"         => _("Home phone number"),
          "labeledURI"        => _("Homepage"),
          "userPassword"      => _("User password method"),
        ));

    /* Append government attributes if required */
    global $config;
    if ($config->get_cfg_value("honourIvbbAttributes") == "TRUE") {
      foreach ($govattrs as $attr => $desc) {
        $ret["plProvidedAcls"][$attr] = $desc;
      }
    }
    return $ret;
  }

  function convertLoginRestriction()
  {
    $all = array_unique(array_merge($this->gosaLoginRestriction,$this->gosaLoginRestriction_some));
    $data = array();
    foreach($all as $ip){
      $data['data'][] = $ip;
      if(!in_array($ip, $this->gosaLoginRestriction)){
        $data['displayData'][] = array('mode' => LIST_MARKED , 'data' => array($ip.' ('._("Entries differ").')'));
      }else{
        $data['displayData'][] = array('mode' => 0 , 'data' => array($ip));
      }
    }
    return($data);
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
