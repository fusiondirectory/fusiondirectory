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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class aclrole extends acl
{
  /* Definitions */
  var $plHeadline= "Access control roles";
  var $plDescription= "Edit AC roles";

  /* attribute list for save action */
  var $attributes= array('gosaAclTemplate',"cn","description");
  var $objectclasses= array('top','gosaRole');

  /* Helpers */
  var $dialogState= "head";
  var $aclType= "";
  var $aclObject= "";
  var $aclContents= array();
  var $target= "group";
  var $aclTypes= array();
  var $aclObjects= array();
  var $recipients= array();
  var $isContainer= TRUE;
  var $currentIndex= 0;
  var $wasNewEntry= FALSE;
  var $ocMapping= array();
  var $savedAclContents= array();
  var $myAclObjects = array();

  /* Role attributes */
  var $gosaAclTemplate= "";
  var $cn = "";
  var $orig_cn = "";
  var $description = "";
  var $orig_dn;
  var $orig_base;
  var $base ="";
  var $baseSelector;

  function aclrole (&$config,  $dn= NULL)
  {
    /* Include config object */
    plugin::plugin($config, $dn);

    if($this->dn == "new"){
      $ui = get_userinfo();
      $this->base= dn2base(session::global_is_set("CurrentMainBase")?"cn=test,".session::global_get("CurrentMainBase"):$ui->dn);
    }else{
      $this->base = preg_replace("/^[^,]+,[^,]+,/","",$this->dn);
      new log("view","acl/".get_class($this),$this->dn);
    }
   
    /* Load ACL's */
    $this->gosaAclTemplate= array();
    if (isset($this->attrs["gosaAclTemplate"])){
      for ($i= 0; $i<$this->attrs["gosaAclTemplate"]['count']; $i++){
        $acl= $this->attrs["gosaAclTemplate"][$i];
        $this->gosaAclTemplate= array_merge($this->gosaAclTemplate, $this->explodeACL($acl));
      }
    }
    ksort($this->gosaAclTemplate);

    /* Extract available categories from plugin info list */
    $tmp= session::get('plist');
    $plist= $tmp->info;
    $oc = array();
    foreach ($plist as $class => $acls){

      /* Only feed categories */
      if (isset($acls['plCategory'])){

        /* Walk through supplied list and feed only translated categories */
        foreach($acls['plCategory'] as $idx => $data){

          /* Non numeric index means -> base object containing more informations */
          if (preg_match('/^[0-9]+$/', $idx)){
            if (!isset($this->ocMapping[$data])){
              $this->ocMapping[$data]= array();
              $this->ocMapping[$data][]= '0';
            }
            $this->ocMapping[$data][]= $class;
          } else {
            if (!isset($this->ocMapping[$idx])){
              $this->ocMapping[$idx]= array();
              $this->ocMapping[$idx][]= '0';
            }
            $this->ocMapping[$idx][]= $class;
            $this->aclObjects[$idx]= $data['description'];

            /* Additionally filter the classes we're interested in in "self edit" mode */
            if (is_array($data['objectClass'])){
              foreach($data['objectClass'] as $objectClass){
                if (in_array_ics($objectClass, $oc)){
                  $this->myAclObjects[$idx.'/'.$class]= $acls['plDescription'];
                  break;
                }
              }
            } else {
              if (in_array_ics($data['objectClass'], $oc)){
                $this->myAclObjects[$idx.'/'.$class]= $acls['plDescription'];
              }
            }
          }

        }
      }
    }
    $this->aclObjects['all']= '*&nbsp;'._("All categories");
    $this->ocMapping['all']= array('0' => 'all');

    /* Sort categories */
    asort($this->aclObjects);

    /* Fill acl types */
    $this->aclTypes= array( "reset" => _("Reset ACL"),
                             "one" => _("One level"),
                             "base" => _("Current object"),
                             "sub" => _("Complete subtree"),
                             "psub" => _("Complete subtree (permanent)"));
    asort($this->aclTypes);

    /* Finally - we want to get saved... */
    $this->is_account= TRUE;
    $this->orig_base = $this->base;
    $this->orig_dn = $this->dn;
    $this->orig_cn = $this->cn;

    /* Instanciate base selector */
    $this->baseSelector= new baseSelector($this->get_allowed_bases(), $this->base);
    $this->baseSelector->setSubmitButton(false);
    $this->baseSelector->setHeight(300);
    $this->baseSelector->update(true);
  }


  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    $tmp= session::get('plist');
    $plist= $tmp->info;

    /* Handle posts */
    if (isset($_POST['new_acl']) && $this->acl_is_writeable("gosaAclEntry")){
      $this->dialogState= 'create';
      $this->dialog= TRUE;
      $this->currentIndex= count($this->gosaAclTemplate);
      $this->loadAclEntry(TRUE);
    }

    $new_acl= array();
    $aclDialog= FALSE;
    $firstedit= FALSE;

    /* Act on HTML post and gets here.
     */
    if(isset($_GET['id']) && isset($_GET['act']) && $_GET['act'] == "edit"){
      $id = trim($_GET['id']);
      $this->dialogState= 'create';
      $firstedit= TRUE;
      $this->dialog= TRUE;
      $this->currentIndex= $id;
      $this->loadAclEntry();
    }

    foreach($_POST as $name => $post){

      /* Actions... */
      if (preg_match('/^acl_edit_.*_x/', $name)){
        $this->dialogState= 'create';
        $firstedit= TRUE;
        $this->dialog= TRUE;
        $this->currentIndex= preg_replace('/^acl_edit_([0-9]+).*$/', '\1', $name);
        $this->loadAclEntry();
        continue;
      }
      if (preg_match('/^cat_edit_.*_x/', $name)){
        $this->aclObject= preg_replace('/^cat_edit_([^_]+)_.*$/', '\1', $name);
        $this->dialogState= 'edit';
        foreach ($this->ocMapping[$this->aclObject] as $oc){
          if (isset($this->aclContents[$oc])){
            $this->savedAclContents[$oc]= $this->aclContents[$oc];
          }
        }
        continue;
      }

      if(!$this->acl_is_writeable("gosaAclEntry")){
        continue;
      }

      if (preg_match('/^acl_del_.*_x/', $name) && $this->acl_is_writeable("gosaAclEntry")){
        unset($this->gosaAclTemplate[preg_replace('/^acl_del_([0-9]+).*$/', '\1', $name)]);
        continue;
      }

      if (preg_match('/^cat_del_.*_x/', $name) && $this->acl_is_writeable("gosaAclEntry")){
        $idx= preg_replace('/^cat_del_([^_]+)_.*$/', '\1', $name);
        foreach ($this->ocMapping[$idx] as $key){
          unset($this->aclContents["$idx/$key"]);
        }
        continue;
      }

      /* Sorting... */
      if (preg_match('/^sortup_.*_x/', $name) && $this->acl_is_writeable("gosaAclEntry")){
        $index= preg_replace('/^sortup_([0-9]+).*$/', '\1', $name);
        if ($index > 0){
          $tmp= $this->gosaAclTemplate[$index];
          $this->gosaAclTemplate[$index]= $this->gosaAclTemplate[$index-1];
          $this->gosaAclTemplate[$index-1]= $tmp;
        }
        continue;
      }
      if (preg_match('/^sortdown_.*_x/', $name) && $this->acl_is_writeable("gosaAclEntry")){
        $index= preg_replace('/^sortdown_([0-9]+).*$/', '\1', $name);
        if ($index < count($this->gosaAclTemplate)-1){
          $tmp= $this->gosaAclTemplate[$index];
          $this->gosaAclTemplate[$index]= $this->gosaAclTemplate[$index+1];
          $this->gosaAclTemplate[$index+1]= $tmp;
        }
        continue;
      }

      /* ACL saving... */
      if (preg_match('/^acl_.*_[^xy]$/', $name) && $this->acl_is_writeable("gosaAclEntry")){
        list($dummy, $object, $attribute, $value)= explode('_', $name);

        /* Skip for detection entry */
        if ($object == 'dummy') {
          continue;
        }

        /* Ordinary ACL */
        if (!isset($new_acl[$object])){
          $new_acl[$object]= array();
        }
        if (isset($new_acl[$object][$attribute])){
          $new_acl[$object][$attribute].= $value;
        } else {
          $new_acl[$object][$attribute]= $value;
        }
      }
    }
   
    if(isset($_POST['acl_dummy_0_0_0'])){
      $aclDialog= TRUE;
    }
 
    /* Only be interested in new acl's, if we're in the right _POST place */
    if ($aclDialog && $this->aclObject != "" && is_array($this->ocMapping[$this->aclObject])){

      foreach ($this->ocMapping[$this->aclObject] as $oc){
        unset($this->aclContents[$oc]);
        unset($this->aclContents[$this->aclObject.'/'.$oc]);
        if (isset($new_acl[$oc])){
          $this->aclContents[$oc]= $new_acl[$oc];
        }
        if (isset($new_acl[$this->aclObject.'/'.$oc])){
          $this->aclContents[$this->aclObject.'/'.$oc]= $new_acl[$this->aclObject.'/'.$oc];
        }
      }
    }

    /* Save new acl in case of base edit mode */
    if (1 == 0 && $this->aclType == 'base' && !$firstedit){
      $this->aclContents= $new_acl;
    }

    /* Cancel new acl? */
    if (isset($_POST['cancel_new_acl'])){
      $this->dialogState= 'head';
      $this->dialog= FALSE;
      if ($this->wasNewEntry){
        unset ($this->gosaAclTemplate[$this->currentIndex]);
      }
    }

    /* Store ACL in main object? */
    if (isset($_POST['submit_new_acl']) && $this->acl_is_writeable("gosaAclEntry")){
      $this->gosaAclTemplate[$this->currentIndex]['type']= $this->aclType;
      $this->gosaAclTemplate[$this->currentIndex]['members']= $this->recipients;
      $this->gosaAclTemplate[$this->currentIndex]['acl']= $this->aclContents;
      $this->dialogState= 'head';
      $this->dialog= FALSE;
    }

    /* Cancel edit acl? */
    if (isset($_POST['cancel_edit_acl'])){
      $this->dialogState= 'create';
      foreach ($this->ocMapping[$this->aclObject] as $oc){
        if (isset($this->savedAclContents[$oc])){
          $this->aclContents[$oc]= $this->savedAclContents[$oc];
        }
      }
    }

    /* Save edit acl? */
    if (isset($_POST['submit_edit_acl']) && $this->acl_is_writeable("gosaAclEntry")){
      $this->dialogState= 'create';
    }

    /* Add acl? */
    if (isset($_POST['add_acl']) && $_POST['aclObject'] != "" && $this->acl_is_writeable("gosaAclEntry")){
      $this->dialogState= 'edit';
      $this->savedAclContents= array();
      foreach ($this->ocMapping[$this->aclObject] as $oc){
        if (isset($this->aclContents[$oc])){
          $this->savedAclContents[$oc]= $this->aclContents[$oc];
        }
      }
    }

    /* Save common values */
    foreach (array("aclType", "aclObject", "target") as $key){
      if (isset($_POST[$key]) && $this->acl_is_writeable("gosaAclEntry")){
        $this->$key= validate($_POST[$key]);
      }
    }

    /* Create templating instance */
    $smarty= get_smarty();
    $smarty->assign("usePrototype", "true");

    $smarty->assign("base", $this->baseSelector->render());

    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translation){
      $smarty->assign($name."ACL",$this->getacl($name));
    }

    if ($this->dialogState == 'head'){
      /* Draw list */
      $aclList= new divSelectBox("aclList");
      $aclList->SetHeight(350);

      /* Fill in entries */
      foreach ($this->gosaAclTemplate as $key => $entry){

        if($this->acl_is_readable("")){
          $link = "<a href=?plug=".$_GET['plug']."&amp;id=".$key."&amp;act=edit>".$this->assembleAclSummary($entry)."</a>";
        }else{
          $link = $this->assembleAclSummary($entry);
        }

        $field1= array("string" => $this->aclTypes[$entry['type']], "attach" => "style='width:150px'");
        $field2= array("string" => $link);

        $action ="";
        if($this->acl_is_writeable("gosaAclEntry")){
          $action.= "<input type='image' name='sortup_$key' alt='up' 
            title='"._("Up")."' src='images/lists/sort-up.png' align='top'>";
          $action.= "<input type='image' name='sortdown_$key' alt='down' 
            title='"._("Down")."' src='images/lists/sort-down.png'>";
        }
        if($this->acl_is_readable("gosaAclEntry")){
          $action.= "<input class='center' type='image' src='images/lists/edit.png' alt='"._("Edit")."' name='acl_edit_$key' 
            title='".msgPool::editButton(_("ACL"))."'>";
        }
        if($this->acl_is_writeable("gosaAclEntry")){
          $action.= "<input class='center' type='image' src='images/lists/trash.png' alt='"._("Delete")."' name='acl_del_$key' 
            title='".msgPool::delButton(_("ACL"))."'>";
        }

        $field3= array("string" => $action, "attach" => "style='border-right:0px;width:50px;text-align:right;'");
        $aclList->AddEntry(array($field1, $field2, $field3));
      }

      $smarty->assign("aclList", $aclList->DrawList());
    }

    if ($this->dialogState == 'create'){
      /* Draw list */
      $aclList= new divSelectBox("aclList");
      $aclList->SetHeight(450);

      /* Add settings for all categories to the (permanent) list */
      foreach ($this->aclObjects as $section => $dsc){
        $summary= "";
        foreach($this->ocMapping[$section] as $oc){
          if (isset($this->aclContents[$oc]) && count($this->aclContents[$oc]) && isset($this->aclContents[$oc][0]) &&
              $this->aclContents[$oc][0] != ""){

            $summary.= "$oc, ";
            continue;
          }
          if (isset($this->aclContents["$section/$oc"]) && count($this->aclContents["$section/$oc"])){
            $summary.= "$oc, ";
            continue;
          }
          if (isset($this->aclContents[$oc]) && !isset($this->aclContents[$oc][0]) && count($this->aclContents[$oc])){
            $summary.= "$oc, ";
          }
        }

        /* Set summary... */
        if ($summary == ""){
          $summary= '<i>'._("No ACL settings for this category").'</i>';
        } else {
          $summary= sprintf(_("ACL for these objects: %s"), preg_replace('/, $/', '', $summary));
        }

        $action = "";
        if($this->acl_is_readable("gosaAclEntry")){
          $action.= "<input class='center' type='image' src='images/lists/edit.png' 
            alt='"._("Edit")."' name='cat_edit_$section'   title='"._("Edit category ACL")."'>";
        }
        if($this->acl_is_writeable("gosaAclEntry")){
          $action.= "<input class='center' type='image' src='images/lists/trash.png' 
            alt='"._("Delete")."' name='cat_del_$section' title='"._("Reset category ACL")."'>";
        }

        $field1= array("string" => $dsc, "attach" => "style='width:140px'");
        $field2= array("string" => $summary);
        $field3= array("string" => $action, "attach" => "style='border-right:0px;width:50px'");
        $aclList->AddEntry(array($field1, $field2, $field3));
      }

      $smarty->assign("aclList", $aclList->DrawList());
      $smarty->assign("aclType", $this->aclType);
      $smarty->assign("aclTypes", $this->aclTypes);
      $smarty->assign("target", $this->target);

      if ($this->aclType == 'base'){
        $smarty->assign('aclSelector', $this->buildAclSelector($this->myAclObjects));
      }
    }

    if ($this->dialogState == 'edit'){
      $smarty->assign('headline', sprintf(_("Edit ACL for '%s', scope is '%s'"), $this->aclObjects[$this->aclObject], $this->aclTypes[$this->aclType]));

      /* Collect objects for selected category */
      foreach ($this->ocMapping[$this->aclObject] as $idx => $class){
        if ($idx == 0){
          continue;
        }
        $aclObjects[$this->aclObject.'/'.$class]= $plist[$class]['plDescription'];
      }
      if ($this->aclObject == 'all'){
        $aclObjects['all']= _("All objects in current subtree");
      }
      $smarty->assign('aclSelector', $this->buildAclSelector($aclObjects));
    }

    /* Show main page */
    $smarty->assign("dialogState", $this->dialogState);

    /* Assign cn and decription if this is a role */
    foreach(array("cn","description") as $name){
      $smarty->assign($name,$this->$name);
    }
    return ($smarty->fetch (get_template_path('acl_role.tpl',dirname(__FILE__))));
  }

  function sort_by_priority($list)
  {
    $tmp= session::get('plist');
    $plist= $tmp->info;
    asort($plist);
    $newSort = array();

    foreach($list as $name => $translation){
      $na  =  preg_replace("/^.*\//","",$name);
      if (!isset($plist[$na]['plPriority'])){
        $prio= 0;
      } else {
        $prio=  $plist[$na]['plPriority'] ;
      }

      $newSort[$name] = $prio;
    }

    asort($newSort);

    $ret = array();
    foreach($newSort as $name => $prio){
      $ret[$name] = $list[$name];
    }
    return($ret);
  }

  function loadAclEntry($new= FALSE)
  {
    /* New entry gets presets... */
    if ($new){
      $this->aclType= 'sub';
      $this->recipients= array();
      $this->aclContents= array();
    } else {
      $acl= $this->gosaAclTemplate[$this->currentIndex];
      $this->aclType= $acl['type'];
      $this->recipients= $acl['members'];
      $this->aclContents= $acl['acl'];
    }

    $this->wasNewEntry= $new;
  }


  function aclPostHandler()
  {
    if (isset($_POST['save_acl']) && $this->acl_is_writeable("gosaAclEntry")){
      $this->save();
      return TRUE;
    }

    return FALSE;
  }


  function save()
  {
    /* Assemble ACL's */
    $tmp_acl= array();
    foreach ($this->gosaAclTemplate as $prio => $entry){
      $final= "";
      $members= "";
      if (isset($entry['members'])){
        foreach ($entry['members'] as $key => $dummy){
          $members.= base64_encode(preg_replace('/^.:/', '', $key)).',';
        }
      }
      $final= $prio.":".$entry['type'].":".preg_replace('/,$/', '', $members);

      /* ACL's if needed */
      if ($entry['type'] != "reset" && $entry['type'] != "role"){
        $acl= ":";
        if (isset($entry['acl'])){
          foreach ($entry['acl'] as $object => $contents){

            /* Only save, if we've some contents in there... */
            if (count($contents)){
              $acl.= $object.";";

              foreach($contents as $attr => $permission){

                /* First entry? Its the one for global settings... */
                if ($attr == '0'){
                  $acl.= $permission;
                } else {
                  $acl.= '#'.$attr.';'.$permission;
                }

              }
              $acl.= ',';
            }
            
          }
        }
        $final.= preg_replace('/,$/', '', $acl);
      }

      $tmp_acl[]= $final;
    } 

    /* Call main method */
    plugin::save();

    /* Finally (re-)assign it... */
    $this->attrs["gosaAclTemplate"]= $tmp_acl;

    /* Remove acl from this entry if it is empty... */
    if (!count($tmp_acl)){
      /* Remove attribute */
      if ($this->initially_was_account){
        $this->attrs["gosaAclTempalte"]= array();
      } else {
        if (isset($this->attrs["gosaAclTemplate"])){
          unset($this->attrs["gosaAclTemplate"]);
        }
      }
    }

    /* Do LDAP modifications */
    $ldap= $this->config->get_ldap_link();

    /* Check if object already exists */
    $ldap->cat($this->dn);
    if($ldap->count()){
      $ldap->cd($this->dn);
      $this->cleanup();
      $ldap->modify ($this->attrs);
      new log("modify","acl/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }else{
      $ldap->cd($this->config->current['BASE']);
      $ldap->create_missing_trees(preg_replace("/^[^,]+,/","",$this->dn));
      $ldap->cd($this->dn);
      $ldap->add($this->attrs);
      new log("create","acl/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }

    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, "", get_class()));
    }

    /* Refresh users ACL */
    $ui= get_userinfo();
    $ui->loadACL();
    session::set('ui',$ui);
  }


  function remove_from_parent()
  {
    $ldap = $this->config->get_ldap_link();
    $serach_for = "*:role:".base64_encode($this->dn).":*"; 
    $ldap->search ("(&(objectClass=gosaACL)(gosaAclEntry=".$serach_for."))",array('dn','cn','sn','givenName','uid'));
    $all_names = "";


    $cnt = 3;
    while(($attrs = $ldap->fetch()) && $cnt){
      $name = $attrs['dn'];
      $name = preg_replace("/[ ]/","&nbsp;",$name);
      $name = "<i>'".$name."'</i>";
      $all_names .= $name.", ";
      $cnt --;
    }

    if(!empty($all_names)){
      $all_names = preg_replace("/, $/","",$all_names);
      if(!$cnt){
        $all_names .= ", ...";
      }
      $all_names = "<span style='text-align:left;'>".$all_names."</span>";
      msg_dialog::display(_("Object in use"), sprintf(_("This role cannot be removed while it is in use by these objects:")."<br><br>%s", $all_names), WARNING_DIALOG);
      return;
    }
        
    $ldap->rmDir($this->dn);
    new log("remove","acl/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, "", get_class()));
    }

    /* Optionally execute a command after we're done */
    $this->handle_post_events("remove");

    /* Delete references to object groups */
    $ldap->cd ($this->config->current['BASE']);
    $ldap->search ("(&(objectClass=gosaGroupOfNames)(member=".LDAP::prepare4filter($this->dn)."))", array("cn"));
    while ($ldap->fetch()){
      $og= new ogroup($this->config, $ldap->getDN());
      unset($og->member[$this->dn]);
      $og->save ();
      if (!$ldap->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $og->dn, "", get_class()));
      }
    }
  }


  function save_object()
  {
    plugin::save_object();
    if(isset($_POST['acl_role_posted'])){
     
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
    }
  }


  function saveCopyDialog()
  {
    if(isset($_POST['cn'])){
      $this->cn = $_POST['cn'];
    }
  }
  

  function getCopyDialog()
  {
    $smarty = get_smarty();
    $smarty->assign("cn",$this->cn);
    $str = $smarty->fetch(get_template_path("paste_role.tpl",TRUE,dirname(__FILE__)));
    $ret = array();
    $ret['string'] = $str;
    $ret['status'] = "";
    return($ret);
  }

  
  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);
    
    $source_o = new aclrole($this->config,$source['dn']);
    $this->gosaAclTemplate = $source_o->gosaAclTemplate;
  }
  

  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return (array(
          "plShortName" => _("Role"),
          "plDescription" => _("Access control roles"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 0,
          "plSection"     => array("administration"),
          "plCategory"    => array("acl"),
          "plProvidedAcls"    => array(
            "cn"                => _("Name"),
            "base"              => _("Base"),
            "description"       => _("Description"),
            "gosaAclEntry"      => _("Permissions"))
        ));
  }

  function check()
  {
    $message = plugin::check();

    if(empty($this->cn)){
      $message[] = msgPool::required(_("Name"));
    }
  
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    if($this->cn != $this->orig_cn){
      $ldap->search("(&(objectClass=gosaRole)(cn=".$this->cn."))");
      if($ldap->count()) {
        while($attrs = $ldap->fetch()){
          if($attrs['dn'] != $this->orig_dn){
            $message[] = msgPool::duplicated(_("Name"));
          }
        }
      }
    } 
 
    if(!count($this->gosaAclTemplate)){
      $message[] = msgPool::required(_("ACL"));
    }

    // Check if a wrong base was supplied
    if(!$this->baseSelector->checkLastBaseUpdate()){
      $message[]= msgPool::check_base();;
    }
 
    /* Check if we are allowed to create or move this object
     */
    if($this->orig_dn == "new" && !$this->acl_is_createable($this->base)){
      $message[] = msgPool::permCreate();
    }elseif($this->orig_dn != "new" && $this->base != $this->orig_base && !$this->acl_is_moveable($this->base)){
      $message[] = msgPool::permMove();
    }

    return($message);
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
