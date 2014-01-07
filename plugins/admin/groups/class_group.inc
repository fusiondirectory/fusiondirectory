<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
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

class group extends plugin
{
  /* Group attributes */
  var $cn= "";
  var $description= "";
  var $gidNumber= "";
  var $memberUid= array();
  var $memberUid_used_by_some= array();

  /* Helpers */
  var $base= "";
  var $force_gid= FALSE;
  var $fon_group= FALSE;
  var $smbgroup= FALSE;
  var $groupType= FALSE;
  var $sambaSID= "";
  var $sambaDomainName= "DEFAULT";
  var $SID= "";
  var $ridBase= 0;
  var $members= array();
  var $users= array();
  var $member= array();
  var $allusers= array();
  var $saved_gidNumber= "";
  var $oldgroupType= "";
  var $orig_dn= "";
  var $orig_cn= "";
  var $orig_base= "";
  var $has_mailAccount= FALSE;
  var $userSelect= FALSE;
  var $nagios_group =FALSE;
  var $sambaGroupType;
  var $dialog;
  var $rfc2307bis= FALSE;
  var $OnlyShowFirstEntries =200;
  var $dnMapping= array();
  var $view_logged = FALSE;
  var $allowGroupsWithSameNameInOtherSubtrees = true;
  var $baseSelector;

  /* Trustmodel/AccessTo
   */
  var $accessTo= array();
  var $trustModel= "";
  var $trustSelect= FALSE;
  var $host= array();

  /* attribute list for save action */
  var $attributes= array("cn", "description", "gidNumber","memberUid","sambaGroupType","sambaSID","host");
  var $objectclasses= array("top", "posixGroup");

  var $CopyPasteVars  = array("force_gid","fon_group","smbgroup","groupType","sambaSID","sambaDomainName","SID","nagios_group","sambaGroupType");

  function group (&$config, $dn= NULL)
  {
     /* Set rfc2307bis flag */
     if ($config->get_cfg_value("rfc2307bis") == "TRUE"){
       $this->rfc2307bis= TRUE;
       $this->attributes[]= "member";
       $this->objectclasses[]= "groupOfNames";
     }

    parent::__construct ($config, $dn);

    /* Load attributes depending on the samba version */
    $this->orig_dn= $dn;
    $this->orig_cn= $this->cn;

    /* Get member list */
    if (isset($this->attrs['memberUid'][0])){
      $tmp= array();
      for ($i= 0; $i<$this->attrs['memberUid']['count']; $i++){
        $tmp[$this->attrs['memberUid'][$i]]= $this->attrs['memberUid'][$i];
      }
      $this->memberUid= $tmp;
      ksort ($this->memberUid);
    }

    /* Save gidNumber for later use */
    if (isset($this->attrs['gidNumber'])){
      $this->saved_gidNumber= $this->attrs['gidNumber'][0];
    }

    /* Is a samba group? */
    if (isset($this->attrs['objectClass'])){
      if (array_search ('sambaGroupMapping', $this->attrs['objectClass']) == FALSE ){
        $this->smbgroup= FALSE;
      } else {
        $this->smbgroup= TRUE;
        if (isset($this->attrs['sambaSID'])){
          $this->sambaSID= $this->attrs['sambaSID'][0];
        }
      }
      if (array_search ('goFonPickupGroup', $this->attrs['objectClass']) == FALSE ){
        $this->fon_group= FALSE;
      } else {
        $this->fon_group= TRUE;
      }
      if (array_search ('nagiosContactGroup', $this->attrs['objectClass']) == FALSE ){
        $this->nagios_group= FALSE;
      } else {
        $this->nagios_group= TRUE;
      }
    }

    /* Set mail flag */
    if (isset($this->attrs['objectClass']) && in_array('gosaMailAccount', $this->attrs['objectClass'])){
      $this->has_mailAccount= TRUE;
    }

    /* Get samba Domain in case of samba 3 */
    if (($this->sambaSID != "") && (class_available('sambaAccount'))) {
      $this->SID = preg_replace ("/-[^-]+$/", "", $this->sambaSID);
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search ("(&(objectClass=sambaDomain)(sambaSID=$this->SID))", array("sambaAlgorithmicRidBase"));
      if ($ldap->count() != 0) {
        $attrs = $ldap->fetch();
        if (isset($attrs['sambaAlgorithmicRidBase'])) {
          $this->ridBase = $attrs['sambaAlgorithmicRidBase'][0];
        } else {
          $this->ridBase = $this->config->get_cfg_value("sambaRidBase");
        }

        /* Get domain name for SID */
        $this->sambaDomainName = "DEFAULT";
        foreach ($this->config->data['SERVERS']['SAMBA'] as $key => $val) {
          if ($val['SID'] == $this->SID) {
            $this->sambaDomainName = $key;
            break;
          }
        }
      } else {
        if ($this->config->get_cfg_value("sambaRidBase") != ""){
          $this->sambaDomainName= "DEFAULT";
          $this->ridBase= $this->config->get_cfg_value("sambaRidBase");
          $this->SID= $this->config->get_cfg_value("sid");
        } else {
          msg_dialog::display(_("Configuration error"), _("Cannot find group SID in your configuration!"), ERROR_DIALOG);
        }
      }

      /* Get group type */
      $this->groupType= (int)substr(strrchr($this->sambaSID, "-"), 1);
      if ($this->groupType < 500 || $this->groupType > 553){
        $this->groupType= 0;
      }
      $this->oldgroupType= $this->groupType;
    }

    /* Get global filter config */
    if (!session::is_set("gufilter")) {
      $ui= get_userinfo();
      $base= get_base_from_people($ui->dn);
      $gufilter= array( "dselect"       => $base,
          "regex"           => "*");
      session::set("gufilter", $gufilter);
    }
    $gufilter= session::get('gufilter');
    $gufilter['SubSearchGroup'] = false;
    session::set('gufilter',$gufilter);

    if ($this->dn == "new"){
      if(session::is_set('CurrentMainBase')){
        $this->base = session::get('CurrentMainBase');
      }else{
        $ui= get_userinfo();
        $this->base= dn2base($ui->dn);
      }
    } else {

      /* Get object base */
      $this->base =preg_replace ("/^[^,]+,".preg_quote(get_groups_ou(), '/')."/i","",$this->dn);
    }
    $this->orig_base = $this->base;

    /* Is this account a trustAccount? */
    if ($this->is_account && isset($this->attrs['host'])){
      if ($this->attrs['host'][0] == "*") {
        $this->trustModel= "fullaccess";
      } else {
        $this->trustModel= "byhost";
        $this->accessTo = array();
          if ($this->is_account && isset($this->attrs['host'])){
            for ($i= 0; $i<$this->attrs['host']['count']; $i++){
              $tmp= $this->attrs['host'][$i];
              $this->accessTo[$tmp]= $tmp;
            }
          }
        }
        $this->was_trust_account= TRUE;
      } else {
        $this->was_trust_account= FALSE;
        $this->trustModel= "";
      }

    /* Get global filter config */
    if (!session::is_set("sysfilter")){
      $ui= get_userinfo();
      $base= get_base_from_people($ui->dn);
      $sysfilter= array( "depselect"       => $base,
          "regex"           => "*");
      session::set("sysfilter", $sysfilter);
    }

    /* This is always an account */
    $this->is_account = TRUE;

    /* Instanciate base selector */
    $this->baseSelector = new baseSelector($this->get_allowed_bases(), $this->base);
    $this->baseSelector->setSubmitButton(FALSE);
    $this->baseSelector->setHeight(300);
    $this->baseSelector->update(TRUE);

    $this->reload(TRUE);
  }

  function resetCopyInfos()
  {
    parent::resetCopyInfos();

    $this->force_gid        = FALSE;
    $this->gidNumber        = "";
    $this->saved_gidNumber  = "";
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* Log view */
    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","group/".get_class($this),$this->dn);
    }

    /* Do we represent a valid group? */
    if (!$this->is_account && $this->parent === NULL){
      $display= "<img alt=\"\" src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".msgPool::noValidExtension()."</b>";
      return ($display);
    }

    /* Delete user from group */
    if (isset($_POST['del_users']) && isset($_POST['members']) && preg_match("/w/",$this->getacl("memberUid"))){
      foreach ($_POST['members'] as $value){
        unset ($this->members["$value"]);
        $this->removeUser($value);
      }
      $this->reload();
    }

    /* Add objects? */
    if (isset($_POST["edit_membership"]) && preg_match("/w/",$this->getacl("memberUid"))){
      $this->userSelect= new userSelect($this->config, get_userinfo());
      $this->dialog= TRUE;
    }

    /* Add objects finished? */
    if (isset($_POST["add_cancel"])){
      $this->userSelect= NULL;
      $this->dialog= FALSE;
    }

    /* Add user to group */
    if ((isset($_POST['add_finish']) || isset($_GET['add_finish'])) && $this->userSelect) {

      $users = $this->userSelect->detectPostActions();
      if(isset($users['targets'])){
        $headpage = $this->userSelect->getHeadpage();
        foreach($users['targets'] as $dn){
          $attrs = $headpage->getEntry($dn);
          $value = $attrs['uid'][0];
          $this->addUser($value);
          $this->members["$value"]= $this->allusers[$value];
          $this->reload();
        }
      }
      $this->userSelect= NULL;
      $this->dialog= FALSE;
    }

    /* Dialog handling */
    if(is_object($this->dialog)){
      /* Must be called before save_object */
      $this->dialog->save_object();

      if($this->dialog->isClosed()){
        $this->dialog = false;
      }elseif($this->dialog->isSelected()){

        /* Check if selected base is valid */
        $tmp = $this->get_allowed_bases();
        if(isset($tmp[$this->dialog->isSelected()])){
          $this->base = $this->dialog->isSelected();
        }
        $this->dialog= false;
      }else{
        return($this->dialog->execute());
      }
    }


    /* Add user workstation? */
    if (isset($_POST["add_ws"])){
      $this->trustSelect= new trustSelect($this->config,get_userinfo());
      $this->dialog= TRUE;
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


    /* Add user workstation? */
    if (isset($_POST["add_ws_finish"]) && isset($_POST['wslist'])){
      foreach($_POST['wslist'] as $ws){
        $this->accessTo[$ws]= $ws;
      }
      ksort($this->accessTo);
      $this->is_modified= TRUE;
    }

    /* Remove user workstations? */
    if (isset($_POST["delete_ws"]) && isset($_POST['workstation_list'])){
      foreach($_POST['workstation_list'] as $name){
        unset ($this->accessTo[$name]);
      }
      $this->is_modified= TRUE;
    }

    /* Add user workstation finished? */
    if (isset($_POST["add_ws_cancel"])){
      $this->trustSelect= NULL;
      $this->dialog= FALSE;
    }


    /* Show ws dialog */
    if ($this->trustSelect){

      // Build up blocklist
      session::set('filterBlacklist', array('cn' => array_values($this->accessTo)));
      return($this->trustSelect->execute());
    }

    $smarty= get_smarty();
    $smarty->assign("usePrototype", "true");

    if($this->config->search("nagiosaccount", "CLASS",array('menu'))){
      $smarty->assign("nagios",true);
    }else{
      $smarty->assign("nagios",false);
    }

    if($this->config->search("phoneAccount", "CLASS",array('menu'))){
      $smarty->assign("pickupGroup",true);
    }else{
      $smarty->assign("pickupGroup",false);
    }

    /* Manage object add dialog */
    if ($this->userSelect){
      return($this->userSelect->execute());
    }

    /* Create base acls */
    $smarty->assign("base", $this->baseSelector->render());

    $domains = array();
    if (isset($this->config->data['SERVERS']['SAMBA'])) {
      foreach ($this->config->data['SERVERS']['SAMBA'] as $name => $content) {
        $domains[$name] = $name;
      }
    }
    $smarty->assign("sambaDomains", $domains);
    $smarty->assign("sambaDomainName", $this->sambaDomainName);
    $groupTypes = array(0 => _("Samba group"), 512 => _("Domain admins"), 513 => _("Domain users"),
        514 => _("Domain guests"));

    /* Don't loose special groups! If not key'ed above, add it to
       the combo box... */
    if ($this->groupType >= 500 && $this->groupType <= 553 && !isset($groupTypes[$this->groupType])){
      $groupTypes[$this->groupType]= sprintf(_("Special group (%d)"), $this->groupType);
    }

    $smarty->assign("groupTypes", $groupTypes);
    $smarty->assign("groupType", $this->groupType);


    /* Members and users */
    $smarty->assign("members", $this->members);

    /* Work on trust modes */
    $smarty->assign("trusthide", " disabled ");
    $smarty->assign("trustmodeACL",  $this->getacl("trustModel"));
    if ($this->trustModel == "fullaccess"){
      $trustmode= 1;
      // pervent double disable tag in html code, this will disturb our clean w3c html
      $smarty->assign("trustmode",  $this->getacl("trustModel"));

    } elseif ($this->trustModel == "byhost"){
      $trustmode= 2;
      $smarty->assign("trusthide", "");
    } else {
      // pervent double disable tag in html code, this will disturb our clean w3c html
      $smarty->assign("trustmode",  $this->getacl("trustModel"));
      $trustmode= 0;
    }
    $smarty->assign("trustmode", $trustmode);
    $smarty->assign("trustmodes", array( 0 => _("disabled"), 1 => _("full access"),
          2 => _("allow access to these hosts")));

    if((count($this->accessTo))==0){
      $smarty->assign("emptyArrAccess",true);
    } else{
      $smarty->assign("emptyArrAccess",false);

    }

    $smarty->assign("workstations", $this->accessTo);



    /* Checkboxes */
    foreach (array("force_gid", "smbgroup") as $val){
      if ($this->$val == "1"){
        $smarty->assign("$val", "checked");
      } else {
        $smarty->assign("$val", "");
      }
    }
    if ($this->force_gid != "1"){
      $smarty->assign("forceMode", "disabled");
    }else{
      $smarty->assign("forceMode", "");
    }
    if ($this->fon_group){
      $smarty->assign("fon_group", "checked");
    } else {
      $smarty->assign("fon_group", "");
    }

    if ($this->nagios_group){
      $smarty->assign("nagios_group", "checked");
    } else {
      $smarty->assign("nagios_group", "");
    }

    /* Fields */
    foreach (array("cn", "description", "gidNumber") as $val){
      $smarty->assign("$val", $this->$val);
    }

    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translation){
      $smarty->assign($name."ACL",$this->getacl($name));
    }

    if($this->acl_is_writeable("base")){
      $smarty->assign("baseSelect",true);
    }else{
      $smarty->assign("baseSelect",false);
    }

    /* Show main page */
    $smarty->assign("alphabet", generate_alphabet(10));
    $smarty->assign("search_image", get_template_path('images/lists/search.png'));
    $smarty->assign("launchimage", get_template_path('images/lists/action.png'));
    $smarty->assign("tree_image", get_template_path('images/lists/search-subtree.png'));
    $smarty->assign("deplist", $this->config->idepartments);

    $smarty->assign("memberUid_All",$this->memberUid);
    $smarty->assign("memberUid_Some",$this->memberUid_used_by_some);

    return $smarty->fetch (get_template_path('generic.tpl', TRUE));
  }

  function addUser($uid)
  {
    /* Ensure that the requested object is known to the group class
     */
    if(!isset($this->dnMapping[$uid])){
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(&(objectClass=gosaAccount)(uid=".$uid."))",array("dn", "uid","sn","givenName"));
      if($ldap->count() == 0 ){
        msg_dialog::display(_("Error"),
            sprintf(_("Adding UID '%s' to group '%s' failed: cannot find user object!"),
              $uid,$this->cn),
            ERROR_DIALOG);
        return;
      }elseif($ldap->count() >= 2){
        msg_dialog::display(_("Error"),
            sprintf(_("Add UID '%s' to group '%s' failed: UID is used more than once!"),
              $uid,$this->cn),
            ERROR_DIALOG);
        return;
      }else{
        while($attrs = $ldap->fetch()){
          $this->dnMapping[$attrs['uid'][0]] = $attrs['dn'];
          $this->members[$attrs['uid'][0]] = $this->createResultName($attrs);
          $this->allusers[$attrs['uid'][0]]= $this->createResultName($attrs);
        }
      }
    }

    $this->memberUid[$uid]= $uid;
  }


  function removeUser($uid)
  {
    $temp= array();
    if(isset($this->memberUid[$uid])){
      unset($this->memberUid[$uid]);
    }
  }

  /* Reload data */
  function reload($silent = FALSE)
  {
    /* Fix regex string */
    $gufilter = session::get("gufilter");
    $regex    = normalizeLdap($gufilter['regex']);
    $MaxUser  = $this->OnlyShowFirstEntries;

    /* Prepare ldap link */
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($gufilter['dselect']);


    /* Resolve still unresolved memberuids to fill the list with sn/giveName attributes
        (Store gathered sn/givenName informations in $this->allusers too,
         to be prepared when adding/deleting users)
     */
    $filter = "";
    if ($this->config->get_cfg_value("ldapFilterNestingLimit") == "" ||
        count($this->memberUid) < $this->config->get_cfg_value("ldapFilterNestingLimit")){
      foreach ($this->memberUid as $value){
        if(!isset($this->members[$value])){
          $filter .= "(uid=".normalizeLdap($value).")";
        }
      }
    }

    if(!empty($filter)){
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(&(objectClass=gosaAccount)(|".$filter."))",array("dn", "uid","sn","givenName"));
      while($attrs = $ldap->fetch()){
        $this->dnMapping[$attrs['uid'][0]] = $attrs['dn'];
        $this->members[$attrs['uid'][0]] = $this->createResultName($attrs);
        $this->allusers[$attrs['uid'][0]]= $this->createResultName($attrs);
      }
    }

    /* check if all uids are resolved */
    if ($this->config->get_cfg_value("ldapFilterNestingLimit") == "" ||
        count($this->memberUid) < $this->config->get_cfg_value("ldapFilterNestingLimit")){
      foreach ($this->memberUid as $value){
        if(!isset($this->members[$value])){
          $this->members[$value] =  _("! unknown UID")." [".$value."]";
        }
      }
    }else{
      foreach ($this->memberUid as $value){
        $this->members[$value] = $value;
      }
    }

    /* Create display list of users matching regex & filter
     */
    $this->displayUsers = array();
    $filter = "(&(objectClass=gosaAccount)(!(objectClass=gosaUserTemplate))(!(uid=*$))(|(uid=".$regex.")(sn=".$regex.")(givenName=".$regex.")))";

    /* Search in current tree or within subtrees depending on the checkbox from filter section */
    if($gufilter['SubSearchGroup']){
      $flag = GL_SIZELIMIT | GL_SUBSEARCH;
      $base = $gufilter['dselect'];
    }else{
      $flag = GL_SIZELIMIT ;
      $base = get_people_ou().$gufilter['dselect'];
    }
    $i = 0;


    $res = get_list($filter,"user",$base,array("dn", "uid", "sn", "givenName"),$flag);

    /* Fetch all users and skip already used users */
    foreach($res as $attrs){
      if(in_array($attrs['uid'][0], $this->memberUid)) {
        continue;
      }
      $i ++;
      if($i > $MaxUser) {
        break;
      }
      $this->dnMapping[$attrs['uid'][0]]= $attrs["dn"];
      $this->allusers[$attrs['uid'][0]]     = $this->createResultName($attrs);
      $this->displayUsers[$attrs['uid'][0]] = $this->createResultName($attrs);
    }

    /* If more than max users are found, display a message to warn the user */
    if($i == $MaxUser && !$silent){
      msg_dialog::display(_("Configuration error"), sprintf(_("Search returned too many results. Not displaying more than %s entries!"), $MaxUser), ERROR_DIALOG);
    }

    /* Sort lists */
    natcasesort($this->members);
    reset($this->members);
    natcasesort ($this->displayUsers);
    reset ($this->displayUsers);
  }


  /* Create display name, this was used so often that it is excluded into a seperate function */
  function createResultName($attrs)
  {
    if (isset($attrs["givenName"][0]) && isset($attrs["sn"][0])){
      $ret =  $attrs["sn"][0].", ".$attrs["givenName"][0]." [".$attrs["uid"][0]."]";
    } else {
      $ret= $attrs['uid'][0];
    }
    return($ret);
  }

  protected function post_save()
  {
    del_lock("gidnumber");
    parent::post_save();
  }


  function remove_from_parent()
  {
    plugin::remove_from_parent();

    $ldap= $this->config->get_ldap_link();
    $ldap->rmdir($this->dn);
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()), LDAP_ERROR);
    }

    new log("remove","group/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());

    /* Remove ACL dependencies */
    acl::remove_acl_for($this->dn);

    /* Send signal to the world that we've done */
    $this->handle_post_events("remove");
  }


  /* Save data to object */
  function save_object()
  {
    /* Save additional values for possible next step */
    if (isset($_POST['groupedit'])){

      /* Create a base backup and reset the
          base directly after calling plugin::save_object();
         Base will be set seperatly a few lines below */
      $base_tmp = $this->base;
      plugin::save_object();
      $this->base = $base_tmp;

      /* Refresh base */
      if ($this->acl_is_moveable($this->base)){
        if (!$this->baseSelector->update()) {
          msg_dialog::display(_("Error"), msgPool::permMove(), ERROR_DIALOG);
        }
        if ($this->base != $this->baseSelector->getBase()) {
          $this->base= $this->baseSelector->getBase();
          $this->is_modified= TRUE;
        }
      }

      $this->force_gid = FALSE;

      /* Only reset sambagroup flag if we are able to write this flag */
      if($this->acl_is_writeable("sambaGroupType")){
        $this->smbgroup = 0;
      }

      foreach (array(
            "force_gid"  => "gidNumber",
            "smbgroup"   => "sambaGroupType") as $val => $aclname) {
        if ($this->acl_is_writeable($aclname)  && isset($_POST["$val"])){
          $this->$val= $_POST["$val"];
        }
      }

      /* Save sambaDomain attribute */
      if ($this->acl_is_writeable("sambaDomainName") && isset ($_POST['sambaDomainName'])){
        $this->sambaDomainName= $_POST['sambaDomainName'];
        $this->groupType= $_POST['groupType'];
      }

      /* Save fon attribute */
      if ($this->acl_is_writeable("fonGroup")){
        if (isset ($_POST['fon_group'])){
          $this->fon_group= TRUE;
        } else {
          $this->fon_group= FALSE;
        }
      }
      if ($this->acl_is_writeable("nagiosGroup")){
        if (isset ($_POST['nagios_group'])){
          $this->nagios_group= TRUE;
        } else {
          $this->nagios_group= FALSE;
        }
      }
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


  /* Save to LDAP */
  function save()
  {

    /* ID handling */
    if (!$this->force_gid) {
      if ($this->saved_gidNumber != "") {
        $this->gidNumber= $this->saved_gidNumber;
      } else {
        /* Calculate new, lock uids */
        $wait= 10;
        while (get_lock("gidnumber") != ""){
          sleep (1);

          /* timed out? */
          if ($wait-- == 0){
            break;
          }
        }
        add_lock ("gidnumber", "gosa");
        $this->gidNumber= get_next_id("gidNumber", $this->dn);
      }
    }

    plugin::save();

 /* Trust accounts */
    $objectclasses= array();
    foreach ($this->attrs['objectClass'] as $key => $class){
      if (preg_match('/hostObject/i', $class)){
        continue;
      }
      $objectclasses[]= $this->attrs['objectClass'][$key];
    }
    $this->attrs['objectClass']= $objectclasses;
    if ($this->trustModel != ""){
      $this->attrs['objectClass'][]= "hostObject";
      $this->attrs['host']= array();
      if ($this->trustModel == "fullaccess"){
        $this->attrs['host'][0]= "*";
      } else {
        if ($this->trustModel == "byhost"){
          foreach ($this->accessTo as $hosts){
            $this->attrs['host'][]= $hosts;
          }
        }
      }
    } else {
      if ($this->was_trust_account){
        $this->attrs['host']= array();
      }
    }

    /* Remove objectClass for samba/phone support */
    $tmp= array();
    for ($i= 0; $i<count($this->attrs["objectClass"]); $i++){
      if ($this->attrs['objectClass'][$i] != 'sambaGroupMapping' &&
          $this->attrs['objectClass'][$i] != 'sambaIdmapEntry' &&
          $this->attrs['objectClass'][$i] != 'goFonPickupGroup' &&
   $this->attrs['objectClass'][$i] != 'nagiosContactGroup'){
        $tmp[]= $this->attrs['objectClass'][$i];
      }
    }
    $this->attrs['objectClass']= $tmp;
    $ldap= $this->config->get_ldap_link();

    /* Add samba group functionality */
    if ($this->smbgroup) {

      /* Fixed undefined index ...
       */
      $this->SID = $this->ridBase = "";
      if (isset($this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['SID'])) {
        $this->SID    = $this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['SID'];
      } else {
        msg_dialog::display(_("Error"), sprintf(_("Cannot find any SID for '%s'!"), $this->sambaDomainName), ERROR_DIALOG);
      }
      if (isset($this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['RIDBASE'])) {
        $this->ridBase = $this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['RIDBASE'];
      } else {
        msg_dialog::display(_("Error"), sprintf(_("Cannot find any RIDBASE for '%s'!"), $this->sambaDomainName), ERROR_DIALOG);
      }

      $this->attrs['objectClass'][]   = 'sambaGroupMapping';
      $this->attrs['sambaGroupType']  = "2";

      /* Check if we need to create a special entry */
      if ($this->groupType == 0) {
        if (($this->sambaSID == "") || ($this->oldgroupType != $this->groupType)) {
          $sid = $this->getSambaSID();
          $this->attrs['sambaSID'] = $sid;
          $this->sambaSID = $sid;
        }
      } else {
        $this->attrs['sambaSID']=$this->SID."-".$this->groupType;
      }

      /* User wants me to fake the idMappings? This is useful for
         making winbind resolve the group names in a reasonable amount
         of time in combination with larger databases. */
      if ($this->config->get_cfg_value("sambaidmapping") == "TRUE"){
        $this->attrs['objectClass'][]= "sambaIdmapEntry";
      }

    }

    /* Add phone functionality */
    if ($this->fon_group){
      $this->attrs['objectClass'][]= "goFonPickupGroup";
    }

    /* Add nagios functionality */
    if ($this->nagios_group){
      $this->attrs['objectClass'][]= "nagiosContactGroup";
    }

    /* Take members array */
    if (count ($this->memberUid)){
      $this->attrs['memberUid']= array_values(array_unique($this->memberUid));
    }

    /* New accounts need proper 'dn', propagate it to remaining objects */
    if ($this->dn == 'new'){
      $this->dn= 'cn='.$this->cn.','.get_groups_ou().$this->base;
    }

    /* Add member dn's for RFC2307bis Support */
    if ($this->rfc2307bis){
      $this->attrs['member'] = array();
      if (count($this->memberUid)){
        foreach($this->attrs['memberUid'] as $uid) {

          if(isset($this->dnMapping[$uid])){
            $this->attrs['member'][]= $this->dnMapping[$uid];
          }
        }
      } else {
        $this->attrs['member'][]= $this->config->current['ADMINDN'];
      }
    }

    /* Save data. Using 'modify' implies that the entry is already present, use 'add' for
       new entries. So do a check first... */
    $ldap->cat ($this->dn, array('dn'));
    if ($ldap->fetch()){
      /* Modify needs array() to remove values :-( */
      if (!count ($this->memberUid)){
        $this->attrs['memberUid']= array();
      }
      if (!$this->smbgroup){
        $this->attrs['sambaGroupType']= array();
        $this->attrs['sambaSID']= array();
      }

      $mode= "modify";
    } else {
      $mode= "add";
      $ldap->cd($this->config->current['BASE']);
      $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
    }


    /* Check generated gidNumber, it may be used by another group.
     */
    if($this->gidNumber != ""){
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(&(!(cn=".$this->orig_cn."))(objectClass=posixGroup)(gidNumber=".$this->gidNumber."))",array("cn"));
      if($ldap->count()){
        $cns = "";
        while($attrs = $ldap->fetch()){
          $cns .= $attrs['cn'][0].", ";
        }
        $cns = rtrim($cns,", ");
        msg_dialog::display(_("Warning"),sprintf(_("The gidNumber '%s' is already in use by %s!"),$this->gidNumber,$cns) , WARNING_DIALOG );
      }
    }

    /* Write back to ldap */
    $ldap->cd($this->dn);
    $this->cleanup();
    $ldap->$mode($this->attrs);

    /* Remove ACL dependencies too,
     */
    if($this->dn != $this->orig_dn && $this->orig_dn != "new"){
      acl::update_acl_membership($this->orig_dn,$this->dn);
    }

    if($this->initially_was_account){
      new log("modify","group/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }else{
      new log("create","group/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }

    $ret= 0;
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()), LDAP_ERROR);
      $ret= 1;
    }

    /* Remove uid lock */
    del_lock ("gidnumber");

    /* Post that we've done*/
    $this->handle_post_events($mode);

    return ($ret);
  }

  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    /* Permissions for that base? */
    if ($this->base != ""){
      $new_dn= 'cn='.$this->cn.','.get_groups_ou().$this->base;
    } else {
      $new_dn= $this->dn;
    }

    /* must: cn */
    if ($this->cn == "" && $this->acl_is_writeable("cn")){
      $message[]= msgPool::required(_("Name"));
    }

    // Check if a wrong base was supplied
    if(!$this->baseSelector->checkLastBaseUpdate()){
      $message[]= msgPool::check_base();;
    }

    /* Check for valid input */
    if (!tests::is_uid($this->cn)){
      if (strict_uid_mode()){
        $message[]= msgPool::invalid(_("Name"), $this->cn, "/[a-z0-9_-]/");
      } else {
        $message[]= msgPool::invalid(_("Name"), $this->cn, "/[a-z0-9_-]/i");
      }
    }

    // Check if a wrong base was supplied
    if(!$this->baseSelector->checkLastBaseUpdate()){
      $message[]= msgPool::check_base();;
    }

    if($this->allowGroupsWithSameNameInOtherSubtrees == true){

      /* Check for used 'cn' */
      $ldap= $this->config->get_ldap_link();
      if(($this->cn  != $this->orig_cn) || ($this->orig_dn == "new") || ($new_dn != $this->orig_dn)){
        $ldap->cd(get_groups_ou().$this->base);
        $ldap->ls("(&(|(objectClass=gosaGroupOfNames)(objectClass=posixGroup))(cn=$this->cn))",get_groups_ou().$this->base,array("cn"));
        if ($ldap->count() != 0){
          $message[]= msgPool::duplicated(_("Name"));
        }
      }

    }else{

      /* Check for used 'cn' */
      $ldap= $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(&(|(objectClass=gosaGroupOfNames)(objectClass=posixGroup))(cn=$this->cn))",array("cn"));
      if ($ldap->count() != 0){

        /* New entry? */
        if ($this->dn == 'new'){
          $message[]= msgPool::duplicated(_("Name"));
        }

        /* Moved? */
        elseif ($new_dn != $this->orig_dn){
          $ldap->fetch();
          if ($ldap->getDN() != $this->orig_dn){
            $message[]= msgPool::duplicated(_("Name"));
          }
        }
      }
    }

    /* Check ID */
    if ($this->force_gid) {
      if (!tests::is_id($this->gidNumber)) {
        $message[]= msgPool::invalid(_("GID"), $this->gidNumber, "/[0-9]/");
      } else {
        if ($this->gidNumber < $this->config->get_cfg_value("minId")) {
          $message[]= msgPool::toosmall(_("GID"), $this->config->get_cfg_value("minId"));
        }

      }
    }

    /* Check if we are allowed to create or move this object
     */
    if(!$this->orig_dn == "new" ||
        $this->orig_base != $this->base ||
        $this->cn != $this->orig_cn){

      if($this->orig_dn == "new" && !$this->acl_is_createable($this->base)){
        $message[] = msgPool::permCreate();
      }elseif($this->orig_dn != "new" && !$this->acl_is_moveable($this->base)){
        $message[] = msgPool::permMove();
      }
    }

    return ($message);
  }


  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Generic"),
      "plDescription" => _("Generic group settings"),
      "plSelfModify"  => FALSE,
      "plObjectType"  => array("group" => array(
        "description" => _("Groups"),
        "name"        => _("Group"),
        'ou'          => get_ou('groupRDN'),
        'filter'      => 'objectClass=posixGroup',
        'icon'        => 'plugins/groups/images/iconMini.png',
        'mainAttr'    => 'cn',
      )),
      'plForeignKeys'  => array(
        'memberUid' => array('user','uid')
      ),


      "plProvidedAcls"    => array(
        "cn"                => _("Name"),
        "description"       => _("Description"),
        "base"              => _("Base"),

        "gidNumber"         => _("GID"),

        "sambaGroupType"    => _("Samba group type"),
        "sambaDomainName"   => _("Samba domain name"),
        "trustModel"        => _("System trust"),
        "fonGroup"          => _("Phone pickup group"),
        "nagiosGroup"       => _("Nagios group"),

        "memberUid"         => _("Group member")
      )
    );
  }

  function foreignKeyUpdate ($field, $oldvalue, $newvalue, $source)
  {
    if (($field == 'memberUid') && ($source['CLASS'] == 'user') && ($source['FIELD'] == 'uid')) {
      if (isset($this->memberUid[$oldvalue])) {
        if ($source['MODE'] != 'copy') {
          $this->removeUser($oldvalue);
        }
        if ($newvalue !== NULL) {
          $this->addUser($newvalue);
        }
      }
    } else {
      return parent::foreignKeyUpdate($field, $oldvalue, $newvalue, $source);
    }
  }

  function foreignKeyCheck ($field, $value, $source)
  {
    if (($field == 'memberUid') && ($source['CLASS'] == 'user') && ($source['FIELD'] == 'uid')) {
      return isset($this->memberUid[$value]);
    } else {
      return parent::foreignKeyCheck($field, $value, $source);
    }
  }

  /*! \brief Get a new SambaSID for a group */
  function getSambaSID()
  {
    $ldap = $this->config->get_ldap_link();
    $gidNumber= $this->gidNumber;
    while(TRUE){
      $sid= $this->SID."-".($gidNumber*2 + $this->ridBase+1);
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(sambaSID=$sid)",array("sambaSID"));
      if ($ldap->count() == 0){
        break;
      }
      $gidNumber++;
    }

    return $sid;
  }
}
?>
