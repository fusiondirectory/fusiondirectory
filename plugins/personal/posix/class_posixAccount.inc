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

class posixAccount extends plugin
{
  /* Definitions */
  var $plHeadline     = "UNIX";
  var $plDescription  = "Edit users POSIX settings";

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
  function posixAccount (&$config, $dn = NULL, $object = NULL)
  {
    global $class_mapping;

    /* Configuration is fine, allways */
    $this->config= $config;

    /* Load bases attributes */
    plugin::plugin($config, $dn, $object);

    /*  If gotoLastSystemLogin is available read it from ldap and create a readable
        date time string, fallback to sambaLogonTime if available.
     */
    if (isset($this->attrs['gotoLastSystemLogin'][0]) && preg_match("/^[0-9]*$/",$this->attrs['gotoLastSystemLogin'][0])) {
      $this->gotoLastSystemLogin = date("d.m.Y H:i:s", strtotime($this->attrs['gotoLastSystemLogin'][0]));
    } elseif(isset($this->attrs['sambaLogonTime'][0]) && preg_match("/^[0-9]*$/",$this->attrs['sambaLogonTime'][0])) {
      $this->gotoLastSystemLogin = date("d.m.Y H:i:s", $this->attrs['sambaLogonTime'][0]);
    }

    /* Setting uid to default */
    if (isset($this->attrs['uid'][0])) {
      $this->uid = $this->attrs['uid'][0];
    }

    $ldap = $this->config->get_ldap_link();

    if ($dn !== NULL) {

      /* Correct is_account. shadowAccount is not required. */
      if (isset($this->attrs['objectClass']) &&
          in_array ('posixAccount', $this->attrs['objectClass'])) {

        $this->is_account = TRUE;
      }

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

/*      $this->accessTo = array();
      if ($this->is_account && isset($this->attrs['host'])){
        for ($i= 0; $i<$this->attrs['host']['count']; $i++){
          $tmp= $this->attrs['host'][$i];
          $this->accessTo[$tmp]= $tmp;
        }
      }*/
      $this->initially_was_account = $this->is_account;

      // Templates do not have a gidNumber
      if ($this->gidNumber == 2147483647) {
        $this->gidNumber    = "";
        $this->primaryGroup = 0;
      }

      /* Fill group */
      $this->primaryGroup = $this->gidNumber;

      /* Generate status text */
      $current = date("U");

      $current = floor($current / 60 /60 / 24);

      if (($current >= $this->shadowExpire) && $this->shadowExpire) {
        $this->status = _("expired");
        if (($current - $this->shadowExpire) < $this->shadowInactive) {
          $this->status .= ", "._("grace time active");
        }
      } elseif (($this->shadowLastChange + $this->shadowMin) >= $current) {
        $this->status = _("active").", "._("password not changeable");
      } elseif (($this->shadowLastChange + $this->shadowMax) >= $current) {
        $this->status = _("active").", "._("password expired");
      } else {
        $this->status = _("active");
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
      $this->savedUidNumber= $this->uidNumber;
      $this->savedGidNumber= $this->gidNumber;

      // Instanciate SSH object if available
      if (isset($class_mapping["sshPublicKey"])){
        if (empty($this->acl_base)){
          $this->acl_base= $config->current['BASE'];
        }

        $this->sshAcl= $this->getacl("sshPublicKey");
        $this->ssh= new sshPublicKey($this->config, $this->dn, $this->sshAcl);
      }
    }

    /* Adjust shadow checkboxes */
    foreach (array("shadowMin", "shadowMax", "shadowWarning", "shadowInactive",
          "shadowExpire") as $val){

      if ($this->$val != 0){
        $oval= "activate_".$val;
        $this->$oval= "1";
      }
    }

    /* Convert shadowExpire for usage */
    if ($this->shadowExpire == 0){
      $this->shadowExpire= "";
    } else {
      $this->shadowExpire= date('d.m.Y', $this->shadowExpire * 60 * 60 * 24);
    }


    /* Generate shell list from SYSTEMS_DIR./shells */
    if (file_exists(SYSTEMS_DIR.'/shells')){
      $shells = file (SYSTEMS_DIR.'/shells');
      foreach ($shells as $line){
        if (!preg_match ("/^#/", $line)){
          $this->loginShellList[]= trim($line);
        }
      }
    } else {
      if ($this->loginShell == ""){
        $this->loginShellList[]= _("unconfigured");
      }
    }

    /* Insert possibly missing loginShell */
    if ($this->loginShell != "" && !in_array($this->loginShell, $this->loginShellList)){
      $this->loginShellList[]= $this->loginShell;
    }

    /* Generate group list */
    $this->ui = get_userinfo();
    $this->secondaryGroups[0]= "- "._("automatic")." -";
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=posixGroup)", array("cn", "gidNumber"));
    while($attrs = $ldap->fetch()){
      $this->secondaryGroups[$attrs['gidNumber'][0]]= $attrs['cn'][0];
    }
    asort ($this->secondaryGroups);

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

    /* Log view */
    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","users/".get_class($this),$this->dn);
    }

    /* Department has changed? */
    if(isset($_POST['depselect'])){
      session::set('CurrentMainBase',validate($_POST['depselect']));
    }

    if($this->multiple_support_active){
      $this->is_account = TRUE;
    }

    if(!$isCopyPaste && ! $this->multiple_support_active){

      /* Do we need to flip is_account state? */
      if(isset($_POST['modify_state'])){
        if($this->is_account && $this->acl_is_removeable()){
          $this->is_account= FALSE;
        }elseif(!$this->is_account && $this->acl_is_createable()){
          $this->is_account= TRUE;
        }
      }

      /* Do we represent a valid posixAccount? */
      if (!$this->is_account && $this->parent === NULL ){
        $display= "<img alt=\"\" src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".
          msgPool::noValidExtension(_("POSIX"))."</b>";
        $display.= back_to_main();
        return ($display);
      }


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

    // Display dialog to allow selection of groups
    if (isset($_POST['edit_groupmembership'])){
      $this->groupSelect = new groupSelect($this->config,get_userinfo());
      $this->dialog= TRUE;
    }

    // Allow to select trusted machines from a list
    if (isset($_POST["add_ws"])){
      $this->trustSelect= new trustSelect($this->config,get_userinfo());
      $this->dialog= TRUE;
    }

    // Cancel trust and group dialog
    if (isset($_POST['add_groups_cancel']) || isset($_POST['add_ws_cancel'])){
      $this->groupSelect= NULL;
      $this->trustSelect= NULL;
      $this->dialog= FALSE;
    }

    // Add groups selected in groupSelect dialog to ours.
    if (isset($_POST['add_groups_finish']) && $this->groupSelect){
      $groups = $this->groupSelect->detectPostActions();
      if(isset($groups['targets'])){
        $this->addGroup ($groups['targets']);
        $this->is_modified= TRUE;
      }
      $this->groupSelect= NULL;
      $this->dialog= FALSE;
    }

    // Remove groups from currently selected groups.
    if (isset($_POST['delete_groupmembership']) &&
        isset($_POST['group_list']) && count($_POST['group_list'])){

      $this->delGroup ($_POST['group_list']);
    }

    // Add selected machines to trusted ones.
    if (isset($_POST["add_ws_finish"]) &&  $this->trustSelect){
      $trusts = $this->trustSelect->detectPostActions();
      if(isset($trusts['targets'])){

        $headpage = $this->trustSelect->getHeadpage();
        foreach($trusts['targets'] as $id){
          $attrs = $headpage->getEntry($id);
          $this->accessTo[$attrs['cn'][0]]= $attrs['cn'][0];
        }
        ksort($this->accessTo);
        $this->is_modified= TRUE;
      }
      $this->trustSelect= NULL;
      $this->dialog= FALSE;
    }

    // Remove machine from trusted ones.
    if (isset($_POST["delete_ws"]) && isset($_POST['workstation_list'])){
      foreach($_POST['workstation_list'] as $name){
        unset ($this->accessTo[$name]);
      }
      $this->is_modified= TRUE;
    }



    /* Templates now! */
    $smarty= get_smarty();
    $smarty->assign("usePrototype", "true");

    /* Show ws dialog */
    if ($this->trustSelect){

      // Build up blocklist
      session::set('filterBlacklist', array('cn' => array_values($this->accessTo)));
      return($this->trustSelect->execute());
    }

    /* Manage group add dialog */
    if ($this->groupSelect){

      // Build up blocklist
      session::set('filterBlacklist', array('dn' => array_keys($this->groupMembership)));
      return($this->groupSelect->execute());
    }

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

    /* Depending on pwmode, currently hardcoded because there are no other methods */
    if ( 1 == 1 ){
      $smarty->assign("pwmode", dirname(__FILE__)."/posix_shadow");

      $shadowMinACL     =  $this->getacl("shadowMin",$SkipWrite);
      $smarty->assign("shadowmins", sprintf(_("Password can't be changed up to %s days after last change"),
                                              "<input name=\"shadowMin\" size=3 maxlength=4 value=\"".$this->shadowMin."\">"));

      $shadowMaxACL     =  $this->getacl("shadowMax",$SkipWrite);
      $smarty->assign("shadowmaxs", sprintf(_("Password must be changed after %s days"),
                                              "<input name=\"shadowMax\" size=3 maxlength=4 value=\"".$this->shadowMax."\">"));

      $shadowInactiveACL=  $this->getacl("shadowInactive",$SkipWrite);
      $smarty->assign("shadowinactives", sprintf(_("Disable account after %s days of inactivity after password expiry"),
                                              "<input name=\"shadowInactive\" size=3 maxlength=4 value=\"".$this->shadowInactive."\">"));

      $shadowWarningACL =  $this->getacl("shadowWarning",$SkipWrite);
      $smarty->assign("shadowwarnings", sprintf(_("Warn user %s days before password expiry"),
                                              "<input name=\"shadowWarning\" size=3 maxlength=4 value=\"".$this->shadowWarning."\">"));

      foreach( array("activate_shadowMin", "activate_shadowMax",
                     "activate_shadowExpire", "activate_shadowInactive","activate_shadowWarning") as $val){
        if ($this->$val == 1){
          $smarty->assign("$val", "checked");
        } else {
          $smarty->assign("$val", "");
        }
        $smarty->assign("$val"."ACL", $this->getacl($val,$SkipWrite));
      }

      $smarty->assign("mustchangepasswordACL", $this->getacl("mustchangepassword",$SkipWrite));
    }

    // Set last system login
    $smarty->assign("gotoLastSystemLogin",$this->gotoLastSystemLogin);

    /* Fill arrays */
    $smarty->assign("shells", $this->loginShellList);
    $smarty->assign("secondaryGroups", $this->secondaryGroups);
    $smarty->assign("primaryGroup", $this->primaryGroup);
    if(!$this->multiple_support_active){
      if (!count($this->groupMembership)){
        $smarty->assign("groupMembership", array("&nbsp;"));
      } else {
        $smarty->assign("groupMembership", $this->groupMembership);
      }
    }else{
      $smarty->assign("groupMembership", $this->groupMembership);
      $smarty->assign("groupMembership_some", $this->groupMembership_some);
    }
    if (count($this->groupMembership) > 16){
      $smarty->assign("groups", "too_many_for_nfs");
    } else {
      $smarty->assign("groups", "");
    }

    /* Avoid "Undefined index: forceMode" */
    $smarty->assign("forceMode", "");

    /* Checkboxes */
    if ($this->force_ids == 1){
      $smarty->assign("force_ids", "checked");
      if (session::get('js')){
        $smarty->assign("forceMode", "");
      }
    } else {
      if (session::get('js')){
        $smarty->assign("forceMode", "disabled");
      }
      $smarty->assign("force_ids", "");
    }

    /* Create onClick="" action string for the "Force UID/GID" option
     */
    $onClickIDS ="";
    if(preg_match("/w/",$this->getacl("uidNumber",$SkipWrite))){
      $onClickIDS .= "changeState('uidNumber');";
    }
    if(preg_match("/w/",$this->getacl("gidNumber",$SkipWrite))){
      $onClickIDS .= "changeState('gidNumber');";
    }
    $smarty->assign("onClickIDS", $onClickIDS);
    $smarty->assign("force_idsACL", $this->getacl("uidNumber",$SkipWrite).$this->getacl("gidNumber",$SkipWrite));

    foreach(array("primaryGroup","trustmode","activate_shadowWarning","activate_shadowInactive","activate_shadowMin","activate_shadowMax","activate_shadowExpire","mustchangepassword") as $val){
      if(in_array($val,$this->multi_boxes)){
        $smarty->assign("use_".$val,TRUE);
      }else{
        $smarty->assign("use_".$val,FALSE);
      }
    }


    /* Load attributes and acl's */
    foreach($this->attributes as $val){
      if(in_array($val,$this->multi_boxes)){
        $smarty->assign("use_".$val,TRUE);
      }else{
        $smarty->assign("use_".$val,FALSE);
      }

      if((session::get("js"))&&(($val=="uidNumber")||($val=="gidNumber")))
      {
        $smarty->assign("$val"."ACL",$this->getacl($val,$SkipWrite));
        $smarty->assign("$val", $this->$val);
        continue;
      }
      $smarty->assign("$val", $this->$val);
      $smarty->assign("$val"."ACL", $this->getacl($val,$SkipWrite));
    }
    if($SkipWrite){
      $smarty->assign("groupMembershipACL","r");
    }else{
      $smarty->assign("groupMembershipACL","rw");
    }
    $smarty->assign("status", $this->status);

    /* Work on trust modes */
    $smarty->assign("trusthide", " disabled ");
    $smarty->assign("trustmodeACL",  $this->getacl("trustModel",$SkipWrite));
    if ($this->trustModel == "fullaccess"){
      $trustmode= 1;
      // pervent double disable tag in html code, this will disturb our clean w3c html
      $smarty->assign("trustmode",  $this->getacl("trustModel",$SkipWrite));

    } elseif ($this->trustModel == "byhost"){
      $trustmode= 2;
      $smarty->assign("trusthide", "");
    } else {
      // pervent double disable tag in html code, this will disturb our clean w3c html
      $smarty->assign("trustmode",  $this->getacl("trustModel",$SkipWrite));
      $trustmode= 0;
    }
    $smarty->assign("trustmode", $trustmode);
    $smarty->assign("trustmodes", array( 0 => _("disabled"), 1 => _("full access"),
          2 => _("allow access to these hosts")));



    if((count($this->accessTo))==0)
      $smarty->assign("emptyArrAccess",true);
    else
      $smarty->assign("emptyArrAccess",false);

      if($this->mustchangepassword){
        $smarty->assign("mustchangepassword", " checked ");
      } else {
        $smarty->assign("mustchangepassword", "");
      }

    $smarty->assign("workstations", $this->accessTo);

    // Add SSH button if available
    $smarty->assign("sshPublicKey", $this->ssh?1:0);

    $smarty->assign("apply", apply_filter());
    $smarty->assign("multiple_support" , $this->multiple_support_active);
    $display.= $smarty->fetch (get_template_path('generic.tpl', TRUE, dirname(__FILE__)));
    return($display);
  }


  /* remove object from parent */
  function remove_from_parent()
  {
    /* Cancel if there's nothing to do here */
    if ((!$this->initially_was_account) || (!$this->acl_is_removeable())){
      return;
    }


    /* Remove and write to LDAP */
    plugin::remove_from_parent();

    /* Zero out array */
    $this->attrs['gosaHostACL']= array();

    /* Keep uid, because we need it for authentification! */
    unset($this->attrs['uid']);
    unset($this->attrs['trustModel']); /* FIXME should be host ?? */

    @DEBUG (DEBUG_LDAP, __LINE__, __FUNCTION__, __FILE__,
    /* include global link_info */
     $this->attributes, "Save");
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->dn);
    $this->cleanup();
    $ldap->modify ($this->attrs);

    new log("remove","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());

    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
    }

    /* Delete group only if cn is uid and there are no other
       members inside */
    $ldap->cd ($this->config->current['BASE']);
    $ldap->search ("(&(objectClass=posixGroup)(gidNumber=".$this->gidNumber."))", array("cn", "memberUid"));
    if ($ldap->count() != 0){
      $attrs= $ldap->fetch();
      if ($attrs['cn'][0] == $this->uid &&
          !isset($this->attrs['memberUid'])){

        $ldap->rmDir($ldap->getDN());
      }
    }

    /* Optionally execute a command after we're done */
    $this->handle_post_events("remove",array("uid" => $this->uid));
  }


  function save_object()
  {
    if (isset($_POST['posixTab'])){
      /* Save values to object */
      plugin::save_object();


      /* Save force GID checkbox */
      if($this->acl_is_writeable("gidNumber") || $this->acl_is_writeable("uidNumber")){
        if (isset ($_POST['force_ids'])){
          $data= 1;
        } else {
          $data= 0;
        }
        if ($this->force_ids != $data){
          $this->is_modified= TRUE;
        }
        $this->force_ids= $data;
      }

      /*Save primary group settings */
      if($this->acl_is_writeable("primaryGroup") && isset($_POST['primaryGroup'])){
        $data= $_POST['primaryGroup'];
        if ($this->primaryGroup != $data){
          $this->is_modified= TRUE;
        }
        $this->primaryGroup= $_POST['primaryGroup'];
      }

      /* Get seelcted shadow checkboxes */
      foreach(array("shadowMin","shadowMax","shadowExpire","shadowInactive","shadowWarning") as $var) {
        if($this->acl_is_writeable($var)){
          $activate_var = "activate_".$var;
          if(isset($_POST['activate_'.$var])){
            $this->$activate_var  = true;
            $this->$var      = $_POST[$var];
          }else{
            $this->$activate_var  = false;
            if ($var != "shadowExpire") {
              $this->$var      = 0;
            }
          }
        }
      }

      /* Force change password ? */
      if(isset($_POST['mustchangepassword'])){
        $this->mustchangepassword = TRUE;
      }else{
        $this->mustchangepassword = FALSE;
      }

      /* Trust mode - special handling */
      if($this->acl_is_writeable("trustModel")){
        if (isset($_POST['trustmode'])){
          $saved= $this->trustModel;
          if ($_POST['trustmode'] == "1"){
            $this->trustModel= "fullaccess";
          } elseif ($_POST['trustmode'] == "2"){
            $this->trustModel= "byhost";
          } else {
            $this->trustModel= "";
          }
          if ($this->trustModel != $saved){
            $this->is_modified= TRUE;
          }
        }
      }
    }

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

    /* Adapt shadow values */
    if (!$this->activate_shadowExpire) {
      $this->shadowExpire = "0";
    } else {
      /* Transform date to days since the beginning */
      list($day, $month, $year) = explode('.', $this->shadowExpire, 3);
      $this->shadowExpire = (int)(mktime(0, 0, 0, $month, $day, $year)/ (60 * 60 * 24)) ;
    }
    if (!$this->activate_shadowMax) {
      $this->shadowMax = "0";
    }
    if ($this->mustchangepassword) {
      $this->shadowLastChange = (int)(date("U") / 86400) - $this->shadowMax - 1;
    } elseif ($this->is_account && !$this->initially_was_account) {
      $this->shadowLastChange = (int)(date("U") / 86400);
    }
    if (!$this->activate_shadowWarning) {
      $this->shadowWarning = "0";
    }

    /* Check what to do with ID's
        Nothing forced, so we may have to generate our own IDs, if not done already.
     */
    if ($this->force_ids == 0) {

      /* Handle uidNumber.
       * - use existing number if possible
       * - if not, try to create a new uniqe one.
       * */
      if ($this->savedUidNumber != "") {
        $this->uidNumber= $this->savedUidNumber;
      } else {

        /* Calculate new id's. We need to place a lock before calling get_next_id
           to get real unique values.
         */
        $wait= 10;
        while (get_lock("uidnumber") != "") {
          sleep (1);

          /* Oups - timed out */
          if ($wait-- == 0) {
            msg_dialog::display(_("Warning"), _("Timeout while waiting for lock. Ignoring lock!"), WARNING_DIALOG);
            break;
          }
        }
        add_lock ("uidnumber", "gosa");
        $this->uidNumber= get_next_id("uidNumber", $this->dn);
      }
    }


    /* Handle gidNumber
     * - If we do not have a primary group selected (automatic), we will check if there
     *    is already a group  with the same name and use this as primary.
     * - .. if we couldn't find a group with the same name, we will create a new one,
     *    using the users uid as cn and a generated uniqe gidNumber.
     * */
    if($this->is_template && !$this->primaryGroup){
      $this->gidNumber = 2147483647;
    }elseif ($this->primaryGroup == 0 || $this->force_ids){

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

        $groupcn = $this->uid;
        $pri_attr = $this->config->get_cfg_value("accountPrimaryAttribute");
        $groupdn= preg_replace ('/^'.preg_quote($pri_attr,'/').'=[^,]+,'.preg_quote(get_people_ou(),'/').'/i',
            'cn='.$groupcn.','.get_groups_ou(), $this->dn);

        /* Request a new and uniqe gidNumber, if required */
        if (!$this->force_ids) {
          $this->gidNumber= get_next_id("gidNumber", $this->dn);
        }

        /* If forced gidNumber could not be found, then check if the given group name already exists
           we do not want to modify the gidNumber of an existing group.
         */
        $cnt= 0;
        while ($ldap->dn_exists($groupdn) && ($cnt < 100)) {
          $cnt ++;
          $groupcn = $this->uid."_".$cnt;
          $groupdn= preg_replace ('/^'.preg_quote($pri_attr,'/').'=[^,]+,'.preg_quote(get_people_ou(),'/').'/i',
              'cn='.$groupcn.','.get_groups_ou(), $this->dn);
        }

        /* Create new primary group and enforce the new gidNumber */
        $g= new group($this->config, $groupdn);
        $g->cn= $groupcn;
        $g->force_gid= 1;
        $g->gidNumber= $this->gidNumber;
        $g->description= _("Group of user")." ".$this->givenName;
        $g->save ();

        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
            sprintf("Primary group '%s' created, using gidNumber '%s'.",$groupcn,$this->gidNumber),"");
      } else {
        $attrs = $ldap->fetch();
        $this->gidNumber = $attrs['gidNumber'][0];
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
            "Found and used: <i>".$attrs['dn']."</i>",
            sprintf("Primary group '%s' exists, gidNumber is '%s'.",$this->uid,$this->gidNumber));
      }
    } else {

      /* Primary group was selected by user
       */
      $this->gidNumber = $this->primaryGroup;
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
          sprintf("Primary group '%s' for user '%s' manually selected.",$this->gidNumber,$this->uid),"");
    }

    if ($this->activate_shadowMin != "1" ) {
      $this->shadowMin = "";
    }

    if (($this->activate_shadowMax != "1") && ($this->mustchangepassword != "1")) {
      $this->shadowMax = "";
    }

    if ($this->activate_shadowWarning != "1" ) {
      $this->shadowWarning = "";
    }

    if ($this->activate_shadowInactive != "1" ) {
      $this->shadowInactive = "";
    }

    if ($this->activate_shadowExpire != "1" ) {
      $this->shadowExpire = "";
    }

    /* Fill gecos */
    if (isset($this->parent) && $this->parent !== NULL) {
      $this->gecos= rewrite($this->parent->by_object['user']->cn);
      if (!preg_match('/^[a-z0-9 -]+$/i', $this->gecos)) {
        $this->gecos= "";
      }
    }

    foreach (array("shadowMin","shadowMax","shadowWarning","shadowInactive","shadowExpire") as $attr) {
      $this->$attr = (int) $this->$attr;
    }
    /* Call parents save to prepare $this->attrs */
    plugin::save();

    /* Trust accounts */
    $objectclasses= array();
    foreach ($this->attrs['objectClass'] as $key => $class) {
      /*if (preg_match('/trustAccount/i', $class)){*/
      if (preg_match('/hostObject/i', $class)) {
        continue;
      }
      $objectclasses[]= $this->attrs['objectClass'][$key];
    }
    $this->attrs['objectClass']= $objectclasses;
    if ($this->trustModel != "") {
      $this->attrs['objectClass'][]= "hostObject";
      $this->attrs['host']= array();

      if ($this->trustModel == "fullaccess") {
        $this->attrs['host'][0]= "*";
      } else {
        if ($this->trustModel == "byhost") {
          foreach ($this->accessTo as $hosts) {
            $this->attrs['host'][]= $hosts;
          }
        }
      }
    } else {
      if ($this->was_trust_account) {
        $this->attrs['host']= array();
      }
    }

    if (empty($this->attrs['gosaDefaultPrinter'])) {
      $thid->attrs['gosaDefaultPrinter']=array();
    }

    /* include global link_info */
    $this->cleanup();

    /* This is just a test, we have had duplicated ids
        in the past when copy & paste was used.
       Normaly this should not happen.
     */
    if (isset($this->attrs['uidNumber']) && !$this->force_ids) {
      $used = $this->get_used_uid_numbers();
      if (isset($used[$this->attrs['uidNumber']]) && $used[$this->attrs['uidNumber']] != $this->dn) {
        msg_dialog::display(_("Warning"),_("A duplicated UID number was written for this user. If this was not intended please verify all used uidNumbers!"), WARNING_DIALOG);
      }
    }

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->dn);
    unset($this->attrs['uid']);
    $ldap->modify ($this->attrs);

    /* Log last action */
    if ($this->initially_was_account) {
      new log("modify","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    } else {
      new log("create","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }

    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));
    }

    /* Remove lock needed for unique id generation */
    del_lock ("uidnumber");

    // Save ssh stuff if needed
    if ($this->ssh) {
      $this->ssh->setDN($this->dn);
      $this->ssh->save();
    }

    /* Take care about groupMembership values: add to groups */
    foreach ($this->groupMembership as $key => $value){
      if (!isset($this->savedGroupMembership[$key])){
        $g= new grouptabs($this->config,$this->config->data['TABS']['GROUPTABS'], $key,"groups");
        $g->set_acl_base($key);
        $g->by_object['group']->addUser($this->uid);
        $g->save();
      }
    }

    /* Remove groups not listed in groupMembership */
    foreach ($this->savedGroupMembership as $key => $value){
      if (!isset($this->groupMembership[$key])){
        $g= new grouptabs($this->config,$this->config->data['TABS']['GROUPTABS'], $key,"groups");
        $g->set_acl_base($key);
        $g->by_object['group']->removeUser ($this->uid);
        $g->save();
      }
    }

    /* Optionally execute a command after we're done */
    if ($this->initially_was_account == $this->is_account){
      if ($this->is_modified){
        $this->handle_post_events("modify",array("uid" => $this->uid));
      }
    } else {
      $this->handle_post_events("add" ,array("uid"=> $this->uid));
    }
  }


  /* Check formular input */
  function check()
  {
    /* Include global link_info */
    $ldap= $this->config->get_ldap_link();

    /* Append groups as memberGroup: to check hook
     */
    $tmp_attributes  = $this->attributes;
    $this->attributes[] = "memberGroup";
    $this->memberGroup = array();
    foreach($this->groupMembership as $dn => $name){
      $this->memberGroup[] = $name;
    }

    /* Call common method to give check the hook */
    $message= plugin::check();
    $this->attributes = $tmp_attributes;

    /* must: homeDirectory */
    if ($this->homeDirectory == ""){
      $message[]= msgPool::required(_("Home directory"));
    }
    if (!tests::is_path($this->homeDirectory)){
      $message[]= msgPool::invalid(_("Home directory"), "", "", "/home/yourname" );
    }

    /* Check ID's if they are forced by user */
    if ($this->force_ids == "1"){

      /* Valid uid/gid? */
      if (!tests::is_id($this->uidNumber)){
        $message[]= msgPool::invalid(_("UID"), $this->uidNumber, "/[0-9]/");
      } else {
        if ($this->uidNumber < $this->config->get_cfg_value("minId")){
          $message[]= msgPool::toosmall(_("UID"), $this->config->get_cfg_value("minId"));
        }
      }
      if (!tests::is_id($this->gidNumber)){
        $message[]= msgPool::invalid(_("GID"), $this->gidNumber, "/[0-9]/");
      } else {
        if ($this->gidNumber < $this->config->get_cfg_value("minId")){
          $message[]= msgPool::toosmall(_("GID"), $this->config->get_cfg_value("minId"));
        }
      }
    }

    /* Check dates */
    if ($this->activate_shadowExpire && ($this->shadowExpire == "" || !tests::is_date($this->shadowExpire))){
      $message[]= msgPool::invalid("shadowExpire", $this->shadowExpire);
    }

    /* Check shadow settings, well I like spaghetties... */
    if ($this->activate_shadowMin){
      if (!tests::is_id($this->shadowMin)){
        $message[]= msgPool::invalid(_("shadowMin"), $this->shadowMin, "/[0-9]/");
      }
    }
    if ($this->activate_shadowMax){
      if (!tests::is_id($this->shadowMax)){
        $message[]= msgPool::invalid(_("shadowMax"), $this->shadowMax, "/[0-9]/");
      }
    }
    if ($this->activate_shadowWarning){
      if (!tests::is_id($this->shadowWarning)){
        $message[]= msgPool::invalid(_("shadowWarning"), $this->shadowWarning, "/[0-9]/");
      }
      if (!$this->activate_shadowMax){
        $message[]= msgPool::depends("shadowWarning", "shadowMax");
      }
      if ($this->shadowWarning > $this->shadowMax){
        $message[]= msgPool::toobig("shadowWarning", "shadowMax");
      }
      if ($this->activate_shadowMin && $this->shadowWarning < $this->shadowMin){
        $message[]= msgPool::toosmall("shadowWarning", "shadowMin");
      }
    }
    if ($this->activate_shadowInactive){
      if (!tests::is_id($this->shadowInactive)){
        $message[]= msgPool::invalid(_("shadowInactive"), $this->shadowInactive, "/[0-9]/");
      }
      if (!$this->activate_shadowMax){
        $message[]= msgPool::depends("shadowInactive", "shadowMax");
      }
    }
    if ($this->activate_shadowMin && $this->activate_shadowMax){
      if ($this->shadowMin > $this->shadowMax){
        $message[]= msgPool::toobig("shadowMin", "shadowMax");
      }
    }

    return ($message);
  }


  function multiple_check()
  {
    $message = plugin::multiple_check();
    if ($this->homeDirectory == "" && in_array("homeDirectory",$this->multi_boxes)){
      $message[]= msgPool::required(_("Home directory"));
    }
    if (!tests::is_path($this->homeDirectory) && in_array("homeDirectory",$this->multi_boxes)){
      $message[]= msgPool::invalid(_("Home directory"), "", "", "/home/yourname" );
    }

    /* Check shadow settings, well I like spaghetties... */
    if ($this->activate_shadowMin && in_array("activate_shadowMin",$this->multi_boxes)){
      if (!tests::is_id($this->shadowMin)){
        $message[]= msgPool::invalid(_("shadowMin"), $this->shadowMin, "/[0-9]/");
      }
    }
    if ($this->activate_shadowMax && in_array("activate_shadowMax",$this->multi_boxes)){
      if (!tests::is_id($this->shadowMax)){
        $message[]= msgPool::invalid(_("shadowMax"), $this->shadowMax, "/[0-9]/");
      }
    }
    if ($this->activate_shadowWarning && in_array("activate_shadowWarning",$this->multi_boxes)){
      if (!tests::is_id($this->shadowWarning)){
        $message[]= msgPool::invalid(_("shadowWarning"), $this->shadowWarning, "/[0-9]/");
      }
      if (!$this->activate_shadowMax && in_array("activate_shadowMax",$this->multi_boxes)){
        $message[]= msgPool::depends("shadowWarning", "shadowMax");
      }
      if ($this->shadowWarning > $this->shadowMax && in_array("activate_shadowWarning",$this->multi_boxes)){
        $message[]= msgPool::toobig("shadowWarning", "shadowMax");
      }
      if ($this->activate_shadowMin && $this->shadowWarning < $this->shadowMin && in_array("activate_shadowMin",$this->multi_boxes)){
        $message[]= msgPool::tosmall("shadowWarning", "shadowMin");
      }
    }
    if ($this->activate_shadowInactive && in_array("activate_shadowInactive",$this->multi_boxes)){
      if (!tests::is_id($this->shadowInactive)){
        $message[]= msgPool::invalid(_("shadowInactive"), $this->shadowInactive, "/[0-9]/");
      }
      if (!$this->activate_shadowMax && in_array("activate_shadowMax",$this->multi_boxes)){
        $message[]= msgPool::depends("shadowInactive", "shadowMax");
      }
    }
    if ($this->activate_shadowMin && $this->activate_shadowMax && in_array("activate_shadowMin",$this->multi_boxes)){
      if ($this->shadowMin > $this->shadowMax){
        $message[]= msgPool::toobig("shadowMin", "shadowMax");
      }
    }

    return($message);
  }


  function addGroup ($groups)
  {
    /* include global link_info */
    $ldap= $this->config->get_ldap_link();

    /* Walk through groups and add the descriptive entry if not exists */
    foreach ($groups as $value){

      if (!array_key_exists($value, $this->groupMembership)){
        $ldap->cat($value, array('cn', 'description', 'dn'));
        $attrs= $ldap->fetch();
        error_reporting (0);
        if (!isset($attrs['description'][0])){
          $entry= $attrs["cn"][0];
        } else {
          $dsc= preg_replace ('/^Group of user/', _("Group of user"), $attrs["description"][0]);
          $entry= $attrs["cn"][0]." [$dsc]";
        }
        error_reporting (E_ALL | E_STRICT);

        if(obj_is_writable($attrs['dn'],"groups/group","memberUid")){
          $this->groupMembership[$attrs['dn']]= $entry;
          if($this->multiple_support_active && isset($this->groupMembership_some[$attrs['dn']])){
            unset($this->groupMembership_some[$attrs['dn']]);
          }
        }
      }
    }

    /* Sort groups */
    asort ($this->groupMembership);
    reset ($this->groupMembership);
  }


  /* Del posix user from some groups */
  function delGroup ($groups)
  {
    $dest= array();
    foreach($groups as $dn_to_del){
      if(isset($this->groupMembership[$dn_to_del]) && obj_is_writable($dn_to_del,"groups/group","memberUid")){
        unset($this->groupMembership[$dn_to_del]);
      }
      if($this->multiple_support_active){
        if(isset($this->groupMembership_some[$dn_to_del]) && obj_is_writable($dn_to_del,"groups/group","memberUid")){
          unset($this->groupMembership_some[$dn_to_del]);
        }
      }
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


  function get_used_uid_numbers()
  {
    $ids= array();
    $ldap= $this->config->get_ldap_link();

    $ldap->cd ($this->config->current['BASE']);
    $ldap->search ("(&(objectClass=posixAccount)(uidNumber=*))", array("uidNumber"));

    /* Get list of ids */
    while ($attrs= $ldap->fetch()){
      $ids[$attrs['uidNumber'][0]] = $attrs['dn'];
    }
    return($ids);
  }


  /* Get posts from copy & paste dialog */
  function saveCopyDialog()
  {
    if(isset($_POST['homeDirectory'])){
      $this->homeDirectory = $_POST['homeDirectory'];
      if (isset ($_POST['force_ids'])){
        $data= 1;
        $this->gidNumber = $_POST['gidNumber'];
        $this->uidNumber = $_POST['uidNumber'];
      } else {
        $data= 0;
      }
      if ($this->force_ids != $data){
        $this->is_modified= TRUE;
      }
      $this->force_ids= $data;
      $data= $_POST['primaryGroup'];
      if ($this->primaryGroup != $data){
        $this->is_modified= TRUE;
      }
      $this->primaryGroup= $_POST['primaryGroup'];
    }
  }


  /* Create the posix dialog part for copy & paste */
  function getCopyDialog()
  {
    /* Skip dialog creation if this is not a valid account*/
    if(!$this->is_account) return("");
    if ($this->force_ids == 1){
      $force_ids = "checked";
      if (session::get('js')){
        $forceMode = "";
      }
    } else {
      if (session::get('js')){
        if($this->acl != "#none#")
          $forceMode ="disabled";
      }
      $force_ids = "";
    }

    $sta = "";

    /* Open group add dialog */
    if(isset($_POST['edit_groupmembership'])){
      $this->groupSelect = new groupSelect($this->config,get_userinfo());
      $sta = "SubDialog";
    }

    /* If the group-add dialog is closed, call execute
       to ensure that the membership is updatd */
    if(isset($_POST['add_groups_finish']) || isset($_POST['add_groups_cancel'])){
      $this->execute();
      $this->groupSelect =NULL;
    }

    if($this->groupSelect){
      $str = $this->execute(true);
      $ret = array();
      $ret['string'] = $str;
      $ret['status'] = $sta;
      return($ret);
    }

    /* If a group member should be deleted, simply call execute */
    if(isset($_POST['delete_groupmembership'])){
      $this->execute();
    }

    /* Assigned informations to smarty */
    $smarty = get_smarty();
    $smarty->assign("homeDirectory",$this->homeDirectory);
    $smarty->assign("secondaryGroups",$this->secondaryGroups);
    $smarty->assign("primaryGroup",$this->primaryGroup);

    $smarty->assign("uidNumber",$this->uidNumber);
    $smarty->assign("gidNumber",$this->gidNumber);
    $smarty->assign("forceMode",$forceMode);
    $smarty->assign("force_ids",$force_ids);
    if (!count($this->groupMembership)){
      $smarty->assign("groupMembership", array("&nbsp;"));
    } else {
      $smarty->assign("groupMembership", $this->groupMembership);
    }

    /* Display wars message if there are more than 16 group members */
    if (count($this->groupMembership) > 16){
      $smarty->assign("groups", "too_many_for_nfs");
    } else {
      $smarty->assign("groups", "");
    }
    $str = $smarty->fetch(get_template_path("paste_generic.tpl",TRUE,dirname(__FILE__)));

    $ret = array();
    $ret['string'] = $str;
    $ret['status'] = $sta;
    return($ret);
  }


  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);

    /* Avoid using the same gid/uid number as source user
        empty numbers to enforce new ones. */
    $this->savedUidNumber = "";
    $this->savedGidNumber = "";

    /* Get group membership */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=posixGroup)(memberUid=".$source['uid'][0]."))", array("cn", "description"));

    while ($attrs= $ldap->fetch()){
      if (!isset($attrs["description"][0])){
        $entry= $attrs["cn"][0];
      } else {
        $entry= $attrs["cn"][0]." [".$attrs["description"][0]."]";
      }
      $this->groupMembership[$ldap->getDN()]= $entry;
    }
    asort($this->groupMembership);
    reset($this->groupMembership);

    /* Fill group */
    if(isset($source['gidNumber'][0])){
      $this->primaryGroup= $source['gidNumber'][0];
    }


    /* Adjust shadow checkboxes */
    foreach (array("shadowMin", "shadowMax", "shadowWarning", "shadowInactive",
                "shadowExpire") as $val){

        if ($this->$val != 0){
            $oval= "activate_".$val;
            $this->$oval= "1";
        }
    }

  }


  function multiple_execute()
  {
    return($this->execute());
  }

  /* Return selected values for multiple edit */
  function get_multi_edit_values()
  {
    $ret = plugin::get_multi_edit_values();
    $ret['groupMembership']     = $this->groupMembership;
    $ret['groupMembership_some']= $this->groupMembership_some;

    if(in_array("primaryGroup",$this->multi_boxes)){
      $ret['primaryGroup'] = $this->primaryGroup;
    }
    if(in_array("trustmode",$this->multi_boxes)){
      $ret['trustModel'] = $this->trustModel;
      $ret['accessTo'] = $this->accessTo;
    }
    foreach(array("shadowWarning","shadowInactive","shadowMin","shadowMax", "shadowExpire") as $entry){
      $active = "activate_".$entry;
      if(in_array($active,$this->multi_boxes)){
        $ret[$entry] = $this->$entry;
        $ret[$active] = $this->$active;
      }
    }
    if(in_array("mustchangepassword",$this->multi_boxes)){
      $ret['mustchangepassword'] = $this->mustchangepassword;
    }
    return($ret);
  }


  /* Save posts for multiple edit
   */
  function multiple_save_object()
  {
    if(isset($_POST['posix_mulitple_edit'])){

      /* Backup expire value */
      $expire_tmp = $this->shadowExpire;

      /* Update all values */
      plugin::multiple_save_object();

      /* Get selected checkboxes */
      foreach(array("primaryGroup","trustmode","mustchangepassword","activate_shadowWarning","activate_shadowInactive","activate_shadowMin", "activate_shadowMax","activate_shadowExpire") as $val){
        if(isset($_POST["use_".$val])){
          $this->multi_boxes[] = $val;
        }
      }

      /* Update special values, checkboxes for posixShadow */
      foreach(array("shadowMin","shadowMax","shadowExpire","shadowInactive","shadowWarning") as $var) {
        if($this->acl_is_writeable($var)){
          $activate_var = "activate_".$var;
          if(in_array($activate_var, $this->multi_boxes)){
            if(isset($_POST['activate_'.$var])){
              $this->$activate_var  = true;
              $this->$var      = $_POST[$var];
            }else{
              $this->$activate_var  = false;
              $this->$var      = 0;
            }
          }
        }
      }

      /* Restore shadow value, if the shadow attribute isn't used */
      if(!in_array("activate_shadowExpire",$this->multi_boxes)){
        $this->shadowExpire = $expire_tmp;
      }

      /* Force change password ? */
      if(isset($_POST['mustchangepassword'])){
        $this->mustchangepassword = TRUE;
      }else{
        $this->mustchangepassword = FALSE;
      }

      /* Trust mode - special handling */
      if($this->acl_is_writeable("trustModel")){
        if (isset($_POST['trustmode'])){
          $saved= $this->trustModel;
          if ($_POST['trustmode'] == "1"){
            $this->trustModel= "fullaccess";
          } elseif ($_POST['trustmode'] == "2"){
            $this->trustModel= "byhost";
          } else {
            $this->trustModel= "";
          }
          if ($this->trustModel != $saved){
            $this->is_modified= TRUE;
          }
        }
      }

      /* Save primary group settings */
      if($this->acl_is_writeable("primaryGroup") && isset($_POST['primaryGroup'])){
        $data= $_POST['primaryGroup'];
        if ($this->primaryGroup != $data){
          $this->is_modified= TRUE;
        }
        $this->primaryGroup= $_POST['primaryGroup'];
      }
    }
  }


  /* Initialize plugin with given atribute arrays
   */
  function init_multiple_support($attrs,$all)
  {
    plugin::init_multiple_support($attrs,$all);

    /* Some dummy values */
    $groups_some = array();
    $groups_all  = array();
    $groups_uid  = array();
    $uids        = array();
    $first       = TRUE;

    /* Get all groups used by currently edited users */
    $uid_filter="";
    for($i =0; $i < $this->multi_attrs_all['uid']['count'] ; $i ++){
      $uid = $this->multi_attrs_all['uid'][$i];
      $uids[] = $uid;
      $uid_filter.= "(memberUid=".$uid.")";
    }
    $uid_filter = "(&(objectClass=posixGroup)(|".$uid_filter."))";
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search($uid_filter,array("dn","cn","memberUid"));
    while($group = $ldap->fetch()){
      $groups_some[$group['dn']] = $group['cn'][0];
      for($i = 0 ; $i < $group['memberUid']['count'] ; $i++){
        $groups_uid[$group['dn']][] = $group['memberUid'][$i];
      }
    }

    /* Create an array, containing all used groups */
    $groups_all = $groups_some;
    foreach($groups_all as $id => $group){
      foreach($uids as $uid){
        if(!in_array($uid,$groups_uid[$id])){
          unset($groups_all[$id]);
          break;
        }
      }
    }

    /* Assign group array */
    $this->groupMembership = $groups_all;

    /* Create an array of all grouops used by all users */
    foreach( $groups_all as $dn => $cn){
      if(isset($groups_some[$dn])){
        unset($groups_some[$dn]);
      }
    }
    $this->groupMembership_some = $groups_some;
    $this->primaryGroup = $this->gidNumber;

    /* Is this account a trustAccount? */
    if (isset($this->multi_attrs['trustModel'])){
      $this->trustModel= $this->multi_attrs['trustModel'][0];
      $this->was_trust_account= TRUE;
      $this->multi_boxes[] = "trustmode";
    } else {
      $this->was_trust_account= FALSE;
      $this->trustModel= "";
    }

    /* Create access informations */
    $this->accessTo = array();
    if (isset($this->multi_attrs['accessTo'])){
      for ($i= 0; $i<$this->multi_attrs['accessTo']['count']; $i++){
        $tmp= $this->multi_attrs['accessTo'][$i];
        $this->accessTo[$tmp]= $tmp;
      }
    }

    /* Adjust shadow checkboxes */
    foreach (array("shadowMin", "shadowMax", "shadowWarning", "shadowInactive",
          "shadowExpire") as $val){
      if ($this->$val != 0){
        $oval= "activate_".$val;
        $this->$oval= "1";
      }
    }

    /* Convert to seconds */
    if(isset($this->multi_attrs['shadowExpire'])){
      $this->shadowExpire = $this->convertToSeconds($this->multi_attrs['shadowExpire'][0]);
    }else{
      $this->activate_shadowExpire = FALSE;
    }
  }


  function set_multi_edit_values($attrs)
  {
    $groups = array();

    /* Update groupMembership, keep optinal group */
    foreach($attrs['groupMembership_some'] as $dn => $cn){
      if(isset($this->groupMembership[$dn])){
        $groups[$dn] = $cn;
      }
    }
    /* Update groupMembership, add forced groups */
    foreach($attrs['groupMembership'] as $dn => $cn){
      $groups[$dn] = $cn;
    }
    plugin::set_multi_edit_values($attrs);
    $this->groupMembership = $groups;
  }

  static function plInfo()
  {
    return array(
      "plShortName"       => _("Unix"),
      "plDescription"     => _("POSIX account"),
      "plSelfModify"      => TRUE,
      "plDepends"         => array("user"),
      "plPriority"        => 2,
      "plCategory"        => array("users"),
      "plObjectType"      => array("user"),

      "plProvidedAcls"  => array(
        "homeDirectory"       =>  _("Home directory"),
        "loginShell"          =>  _("Shell"),
        "uidNumber"           =>  _("User ID"),
        "gidNumber"           =>  _("Group ID"),

        "mustchangepassword"=>  _("Force password change on login"),
        "shadowMin"           =>  _("Shadow min"),
        "shadowMax"           =>  _("Shadow max"),
        "shadowWarning"       =>  _("Shadow warning"),
        "shadowInactive"      =>  _("Shadow inactive"),
        "shadowExpire"        =>  _("Shadow expire"),
        "sshPublickey"        =>  _("Public SSH key"),
        "trustModel"          =>  _("System trust model")
      )
    );
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
