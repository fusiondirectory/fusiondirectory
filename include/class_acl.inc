<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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
 * \file class_acl.inc
 * Source code for Class ACL
 */

/*!
 * \brief This class contains all the function needed to manage acl
 * \see class_plugin
 */
class acl extends plugin
{
  /* attribute list for save action */
  var $attributes= array('gosaAclEntry');
  var $objectclasses= array('gosaAcl');

  /* Helpers */
  var $dialogState= "head";
  var $gosaAclEntry= array();
  var $aclType= "";
  var $aclObject= "";
  var $aclContents= array();
  var $target= "group";
  var $aclTypes= array();
  var $aclObjects= array();
  var $aclFilter= "";
  var $aclMyObjects= array();
  var $users= array();
  var $roles= array();
  var $groups= array();
  var $recipients= array();
  var $isContainer= FALSE;
  var $currentIndex= 0;
  var $wasNewEntry= FALSE;
  var $ocMapping= array();
  var $savedAclContents= array();
  var $myAclObjects = array();
  var $acl_category = "acl/";

  /*!
   * \brief Acl contructor
   *
   * \param String $config Configuration file for ACL
   *
   * \param String $parent
   *
   * \param String $dn The DN
   */
  function acl (&$config, $dn = NULL, $baseobject = NULL)
  {
    /* Include config object */
    plugin::plugin($config, $dn, $baseobject);

    /* Load ACL's */
    $this->gosaAclEntry = array();
    if (isset($this->attrs['gosaAclEntry'])) {
      for ($i = 0; $i < $this->attrs['gosaAclEntry']['count']; $i++) {
        $acl = $this->attrs['gosaAclEntry'][$i];
        $this->gosaAclEntry = array_merge($this->gosaAclEntry, acl::explodeACL($acl));
      }
    }
    ksort($this->gosaAclEntry);

    /* Save parent - we've to know more about it than other plugins... */
    if (($baseobject !== NULL) && isset($baseobject->parent)) {
      $this->parent =& $baseobject->parent;
    }

    /* Container? */
    if (preg_match('/^(o|ou|c|l|dc)=/i', $dn)) {
      $this->isContainer = TRUE;
    }

    /* Users */
    $ui   = get_userinfo();
    $tag  = $ui->gosaUnitTag;
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    if ($tag == "") {
      $ldap->search('(objectClass=gosaAccount)', array('uid', 'cn'));
    } else {
      $ldap->search('(&(objectClass=gosaAccount)(gosaUnitTag='.$tag.'))', array('uid', 'cn'));
    }

    while ($attrs = $ldap->fetch()) {
      // Allow objects without cn to be listed without causing an error.
      if (!isset($attrs['cn'][0]) && isset($attrs['uid'][0])) {
        $this->users['U:'.$attrs['dn']] =  $attrs['uid'][0];
      } elseif (!isset($attrs['uid'][0]) && isset($attrs['cn'][0])) {
        $this->users['U:'.$attrs['dn']] =  $attrs['cn'][0];
      } elseif (!isset($attrs['uid'][0]) && !isset($attrs['cn'][0])) {
        $this->users['U:'.$attrs['dn']] = $attrs['dn'];
      } else {
        $this->users['U:'.$attrs['dn']] = $attrs['cn'][0].' ['.$attrs['uid'][0].']';
      }
    }
    ksort($this->users);

    /* Groups */
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=posixGroup)', array('cn', 'description'));
    while ($attrs = $ldap->fetch()) {
      $dsc = "";
      if (isset($attrs['description'][0])) {
        $dsc = $attrs['description'][0];
      }
      $this->groups['G:'.$attrs['dn']] = $attrs['cn'][0].' ['.$dsc.']';
    }
    $this->groups['G:*'] = _("All users");
    ksort($this->groups);

    /* Roles */
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=gosaRole)', array('cn', 'description','gosaAclTemplate','dn'));
    while ($attrs= $ldap->fetch()) {
      $dsc= "";
      if (isset($attrs['description'][0])){
        $dsc= $attrs['description'][0];
      }

      $role_id = $attrs['dn'];

      $this->roles[$role_id]['acls'] =array();
      for ($i= 0; $i < $attrs['gosaAclTemplate']['count']; $i++){
        $acl= $attrs['gosaAclTemplate'][$i];
        $this->roles[$role_id]['acls'] = array_merge($this->roles[$role_id]['acls'],acl::explodeACL($acl));
      }
      $this->roles[$role_id]['description'] = $dsc;
      $this->roles[$role_id]['cn'] = $attrs['cn'][0];
    }

    /* Objects */
    $tmp    = session::global_get('plist');
    $plist  = $tmp->info;
    $cats   = array();
    if (isset($this->parent) && $this->parent !== NULL) {
      $oc = array();
      foreach ($this->parent->by_object as $key => $obj) {
        $oc = array_merge($oc, $obj->objectclasses);
        if (isset($obj->acl_category)) {
          $tmp = str_replace("/", "", $obj->acl_category);
          $cats[$tmp] = $tmp;
        }
      }
      if (in_array_ics('organizationalUnit', $oc)) {
        $this->isContainer = TRUE;
      }
    } else {
      $oc = $this->attrs['objectClass'];
    }

    /* Extract available categories from plugin info list */
    foreach ($plist as $class => $acls) {

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

            if(isset($cats[$data])){
              $this->myAclObjects[$data.'/'.$class]= $acls['plShortName'];
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
                  $this->myAclObjects[$idx.'/'.$class]= $acls['plShortName'];
                  break;
                }
              }
            } else {
              if (in_array_ics($data['objectClass'], $oc)){
                $this->myAclObjects[$idx.'/'.$class]= $acls['plShortName'];
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
    if ($this->isContainer){
      $this->aclTypes= array("reset" => _("Reset ACLs"),
                             "one" => _("One level"),
                             "base" => _("Current object"),
                             "sub" => _("Complete subtree"),
                             "psub" => _("Complete subtree (permanent)"),
                             "role" => _("Use ACL defined in role"));
    } else {
      $this->aclTypes= array("base" => _("Current object"),
          "role" => _("Use ACL defined in role"));
    }
    asort($this->aclTypes);
    $this->targets= array("user" => _("Users"), "group" => _("Groups"));
    asort($this->targets);

    /* Finally - we want to get saved... */
    $this->is_account= TRUE;
  }

  /*!
   * \brief Generates the html output for this node
   */
  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    $tmp= session::global_get('plist');
    $plist= $tmp->info;

    /* Handle posts */
    if (isset($_POST['new_acl'])){
      $this->dialogState= 'create';
      $this->dialog= TRUE;
      $this->currentIndex= count($this->gosaAclEntry);
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

      /* Only handle posts, if we allowed to modify ACLs */
      if(!$this->acl_is_writeable("")){
        continue;
      }

      if (preg_match('/^acl_del_.*_x/', $name)){
        unset($this->gosaAclEntry[preg_replace('/^acl_del_([0-9]+).*$/', '\1', $name)]);
        continue;
      }

      if (preg_match('/^cat_del_.*_x/', $name)){
        $idx= preg_replace('/^cat_del_([^_]+)_.*$/', '\1', $name);
        foreach ($this->ocMapping[$idx] as $key){
          unset($this->aclContents["$idx/$key"]);
        }
        continue;
      }

      /* Sorting... */
      if (preg_match('/^sortup_.*_x/', $name)){
        $index= preg_replace('/^sortup_([0-9]+).*$/', '\1', $name);
        if ($index > 0){
          $tmp= $this->gosaAclEntry[$index];
          $this->gosaAclEntry[$index]= $this->gosaAclEntry[$index-1];
          $this->gosaAclEntry[$index-1]= $tmp;
        }
        continue;
      }
      if (preg_match('/^sortdown_.*_x/', $name)){
        $index= preg_replace('/^sortdown_([0-9]+).*$/', '\1', $name);
        if ($index < count($this->gosaAclEntry)-1){
          $tmp= $this->gosaAclEntry[$index];
          $this->gosaAclEntry[$index]= $this->gosaAclEntry[$index+1];
          $this->gosaAclEntry[$index+1]= $tmp;
        }
        continue;
      }

      /* ACL saving... */
      if (preg_match('/^acl_.*_[^xy]$/', $name)){
        list($dummy, $object, $attribute, $value)= explode('_', $name);

        /* Skip for detection entry */
        if ($object == 'dummy') {
          continue;
        }

        /* Ordinary ACLs */
        if (!isset($new_acl[$object])){
          $new_acl[$object]= array();
        }
        if (isset($new_acl[$object][$attribute])){
          $new_acl[$object][$attribute].= $value;
        } else {
          $new_acl[$object][$attribute]= $value;
        }
      }

      // Remember the selected ACL role.
      if(isset($_POST['selected_role']) && $_POST['aclType'] == 'role'){
        $this->aclContents = "";
        $this->aclContents = base64_decode($_POST['selected_role']);
      }
    }

    if(isset($_POST['acl_dummy_0_0_0'])){
      $aclDialog= TRUE;
    }

    if($this->acl_is_writeable("")){

      /* Only be interested in new acl's, if we're in the right _POST place */
      if ($aclDialog && $this->aclObject != "" && is_array($this->ocMapping[$this->aclObject])){

        foreach ($this->ocMapping[$this->aclObject] as $oc){

          if(isset($this->aclContents[$oc]) && is_array($this->aclContents)){
            unset($this->aclContents[$oc]);
          }elseif(isset($this->aclContents[$this->aclObject.'/'.$oc]) && is_array($this->aclContents)){
            unset($this->aclContents[$this->aclObject.'/'.$oc]);
          }else{
#          trigger_error("Huhm?");
          }
          if (isset($new_acl[$oc]) && is_array($new_acl)){
            $this->aclContents[$oc]= $new_acl[$oc];
          }
          if (isset($new_acl[$this->aclObject.'/'.$oc]) && is_array($new_acl)){
            $this->aclContents[$this->aclObject.'/'.$oc]= $new_acl[$this->aclObject.'/'.$oc];
          }
        }
      }

      /* Save new acl in case of base edit mode */
      if ($this->aclType == 'base' && !$firstedit){
        $this->aclContents= $new_acl;
      }
    }

    /* Cancel new acl? */
    if (isset($_POST['cancel_new_acl'])){
      $this->dialogState= 'head';
      $this->dialog= FALSE;
      if ($this->wasNewEntry){
        unset ($this->gosaAclEntry[$this->currentIndex]);
      }
    }

    /* Save common values */
    if($this->acl_is_writeable("")){
      foreach (array("aclType","aclFilter", "aclObject", "target") as $key){
        if (isset($_POST[$key])){
          $this->$key= validate($_POST[$key]);
        }
      }
    }

    /* Store ACL in main object? */
    if (isset($_POST['submit_new_acl'])){
      $this->gosaAclEntry[$this->currentIndex]['type']= $this->aclType;
      $this->gosaAclEntry[$this->currentIndex]['members']= $this->recipients;
      $this->gosaAclEntry[$this->currentIndex]['acl']= $this->aclContents;
      $this->gosaAclEntry[$this->currentIndex]['filter']= $this->aclFilter;
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
    if (isset($_POST['submit_edit_acl'])){
      $this->dialogState= 'create';
    }

    /* Add acl? */
    if (isset($_POST['add_acl']) && $_POST['aclObject'] != ""){
      $this->dialogState= 'edit';
      $this->savedAclContents= array();
      foreach ($this->ocMapping[$this->aclObject] as $oc){
        if (isset($this->aclContents[$oc])){
          $this->savedAclContents[$oc]= $this->aclContents[$oc];
        }
      }
    }

    /* Add to list? */
    if (isset($_POST['add']) && isset($_POST['source'])){
      foreach ($_POST['source'] as $key){
        if ($this->target == 'user'){
          $this->recipients[$key]= $this->users[$key];
        }
        if ($this->target == 'group'){
          $this->recipients[$key]= $this->groups[$key];
        }
      }
      ksort($this->recipients);
    }

    /* Remove from list? */
    if (isset($_POST['del']) && isset($_POST['recipient'])){
      foreach ($_POST['recipient'] as $key){
          unset($this->recipients[$key]);
      }
    }

    /* Create templating instance */
    $smarty= get_smarty();
    $smarty->assign("acl_readable",$this->acl_is_readable(""));
    if(!$this->acl_is_readable("")){
      return ($smarty->fetch (get_template_path('acl.tpl')));
    }

    if ($this->dialogState == 'head') {
      /* Draw list */
      $aclList= new divSelectBox("aclList");
      $aclList->SetHeight(450);

      /* Fill in entries */
      foreach ($this->gosaAclEntry as $key => $entry) {
        if(!$this->acl_is_readable("")) continue;

        $action ="";

        if ($this->acl_is_readable("")) {
          $link = "<a href=?plug=".$_GET['plug']."&amp;id=".$key."&amp;act=edit>".$this->assembleAclSummary($entry)."</a>";
        } else {
          $link = $this->assembleAclSummary($entry);
        }

        $field1= array("string" => $this->aclTypes[$entry['type']], "attach" => "style='width:150px'");
        $field2= array("string" => $link);

        if($this->acl_is_writeable("")){
          $action.= "<input type='image' name='sortup_$key' alt='up'
            title='"._("Up")."' src='images/lists/sort-up.png' align='top'>";
          $action.= "<input type='image' name='sortdown_$key' alt='down'
            title='"._("Down")."' src='images/lists/sort-down.png'>";
        }

        if($this->acl_is_readable("")){
          $action.= "<input class='center' type='image' src='images/lists/edit.png'
            alt='"._("Edit")."' name='acl_edit_$key' title='".msgPool::editButton(_("ACL"))."'>";
        }
        if($this->acl_is_removeable("")){
          $action.= "<input class='center' type='image' src='images/lists/trash.png'
            alt='"._("Delete")."' name='acl_del_$key' title='".msgPool::delButton(_("ACL"))."'>";
        }

        $field3= array("string" => $action, "attach" => "style='border-right:0px;width:50px;text-align:right;'");
        $aclList->AddEntry(array($field1, $field2, $field3));
      }

      $smarty->assign("aclList", $aclList->DrawList());
    }

    if ($this->dialogState == 'create') {
      /* Draw list */
      $aclList= new divSelectBox("aclList");
      $aclList->SetHeight(150);

      /* Add settings for all categories to the (permanent) list */
      foreach ($this->aclObjects as $section => $dsc) {
        $summary= "";
        foreach ($this->ocMapping[$section] as $oc) {
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
        if ($summary == "") {
          $summary= '<i>'._("No ACL settings for this category!").'</i>';
        } else {
          $summary= sprintf(_("Contains ACLs for these objects: %s"), preg_replace('/, $/', '', $summary));
        }

        $actions ="";
        if ($this->acl_is_readable("")) {
          $actions= "<input class='center' type='image' src='images/lists/edit.png'
            alt='"._("Edit")."' name='cat_edit_$section' title='".msgPool::editButton(_("category ACL"))."'>";
        }
        if ($this->acl_is_removeable()) {
          $actions.= "<input class='center' type='image' src='images/lists/trash.png'
            alt='"._("Delete")."' name='cat_del_$section' title='".msgPool::delButton(_("category ACL"))."'>";
        }

        $field1= array("string" => $dsc, "attach" => "style='width:100px'");
        $field2= array("string" => $summary);
        $field3= array("string" => $actions, "attach" => "style='border-right:0px;width:50px'");
        $aclList->AddEntry(array($field1, $field2, $field3));
      }

      $smarty->assign("aclList", $aclList->DrawList());
      $smarty->assign("aclType", $this->aclType);
      $smarty->assign("aclFilter", $this->aclFilter);
      $smarty->assign("aclTypes", $this->aclTypes);
      $smarty->assign("target", $this->target);
      $smarty->assign("targets", $this->targets);

      /* Assign possible target types */
      $smarty->assign("targets", $this->targets);
      foreach ($this->attributes as $attr){
        $smarty->assign($attr, $this->$attr);
      }


      /* Generate list */
      $tmp= array();
      if ($this->target == "group" && !isset($this->recipients["G:*"])) {
        $tmp["G:*"]= _("All users");
      }
      foreach (array("user" => "users", "group" => "groups") as $field => $arr) {
        if ($this->target == $field){
          foreach ($this->$arr as $key => $value){
            if (!isset($this->recipients[$key])){
              $tmp[$key]= $value;
            }
          }
        }
      }
      $smarty->assign('sources', $tmp);
      $smarty->assign('recipients', $this->recipients);

      /* Acl selector if scope is base */
      if ($this->aclType == 'base') {
        $smarty->assign('aclSelector', $this->buildAclSelector($this->myAclObjects));
      }

      /* Role selector if scope is base */
      if ($this->aclType == 'role') {
        $smarty->assign('roleSelector', $this->buildRoleSelector($this->roles));
      }
    }

    if ($this->dialogState == 'edit') {
      $smarty->assign('headline', sprintf(_("Edit ACL for '%s' - scope is '%s'"), $this->aclObjects[$this->aclObject], $this->aclTypes[$this->aclType]));

      /* Collect objects for selected category */
      foreach ($this->ocMapping[$this->aclObject] as $idx => $class) {
        if ($idx == 0) {
          continue;
        }
        $aclObjects[$this->aclObject.'/'.$class]= $plist[$class]['plShortName'];
      }
      if ($this->aclObject == 'all') {
        $aclObjects['all']= _("All objects in current subtree");
      }

      /* Role selector if scope is base */
      if ($this->aclType == 'role') {
        $smarty->assign('roleSelector', $this->buildRoleSelector($this->roles));
      } else {
        $smarty->assign('aclSelector', $this->buildAclSelector($aclObjects));
      }
    }

    /* Show main page */
    $smarty->assign("dialogState", $this->dialogState);

    /* Assign acls */
    $smarty->assign("acl_createable", $this->acl_is_createable());
    $smarty->assign("acl_writeable",  $this->acl_is_writeable(""));
    $smarty->assign("acl_readable",   $this->acl_is_readable(""));
    $smarty->assign("acl_removeable", $this->acl_is_removeable());

    return $smarty->fetch(get_template_path('acl.tpl'));
  }

  /*!
   *  \brief Function sort an array by elements priority
   *
   *  \param Array $list Array to be sorted
   */
  function sort_by_priority($list)
  {
    $tmp= session::global_get('plist');
    $plist= $tmp->info;
    asort($plist);
    $newSort = array();

    foreach($list as $name => $translation){
      $na  =  preg_replace("/^.*\//","",$name);
      $prio = 0;
      if(isset($plist[$na]['plPriority'])){
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

  /*!
   * \brief Build a role selector from a list
   *
   * \param Array $list
   */
  function buildRoleSelector($list)
  {
    $D_List =new divSelectBox("Acl_Roles");

    $selected = $this->aclContents;
    if(!is_string($this->aclContents) || !isset($list[$this->aclContents])){
      $selected = key($list);
    }

    $str ="";
    foreach($list as $dn => $values){

      if($dn == $selected){
        $option = "<input type='radio' name='selected_role' value='".base64_encode($dn)."' checked>";
      }else{
        $option = "<input type='radio' name='selected_role' value='".base64_encode($dn)."'>";
      }

      $field1 = array("string" => $option) ;
      $field2 = array("string" => $values['cn'], "attach" => "style='width:200px;'") ;
      $field3 = array("string" => $values['description'],"attach" => "style='border-right:0px;'") ;

      $D_List->AddEntry(array($field1,$field2,$field3));
    }
    return($D_List->DrawList());
  }

  /*!
   * \brief Build an acl selector form
   *
   * \param Array $list
   *
   * \return the acl selector form
   */
  function buildAclSelector($list)
  {
    $display= "<input type='hidden' name='acl_dummy_0_0_0' value='1'>";
    $cols= 3;
    $tmp= session::global_get('plist');
    $plist= $tmp->info;
    asort($plist);

    /* Add select all/none buttons */
    $style = "style='width:100px;'";

    if($this->acl_is_writeable("")){
      $display .= "<input ".$style." type='button' name='toggle_all_create' onClick=\"acl_toggle_all('_0_c$');\" value='Toggle C'>";
      $display .= "<input ".$style." type='button' name='toggle_all_move'   onClick=\"acl_toggle_all('_0_m$');\" value='Toggle M'>";
      $display .= "<input ".$style." type='button' name='toggle_all_remove' onClick=\"acl_toggle_all('_0_d$');\" value='Toggle D'> - ";
      $display .= "<input ".$style." type='button' name='toggle_all_read'   onClick=\"acl_toggle_all('_0_r$');\" value='Toggle R'>";
      $display .= "<input ".$style." type='button' name='toggle_all_write'  onClick=\"acl_toggle_all('_0_w$');\" value='Toggle W'> - ";

      $display .= "<input ".$style." type='button' name='toggle_all_sub_read'  onClick=\"acl_toggle_all('[^0]_r$');\" value='R+'>";
      $display .= "<input ".$style." type='button' name='toggle_all_sub_write'  onClick=\"acl_toggle_all('[^0]_w$');\" value='W+'>";

      $display .= "<br>";

      $style = "style='width:50px;'";
      $display .= "<input ".$style." type='button' name='set_true_all_create' onClick=\"acl_set_all('_0_c$',true);\" value='C+'>";
      $display .= "<input ".$style." type='button' name='set_false_all_create' onClick=\"acl_set_all('_0_c$',false);\" value='C-'>";
      $display .= "<input ".$style." type='button' name='set_true_all_move' onClick=\"acl_set_all('_0_m$',true);\" value='M+'>";
      $display .= "<input ".$style." type='button' name='set_false_all_move' onClick=\"acl_set_all('_0_m$',false);\" value='M-'>";
      $display .= "<input ".$style." type='button' name='set_true_all_remove' onClick=\"acl_set_all('_0_d$',true);\" value='D+'>";
      $display .= "<input ".$style." type='button' name='set_false_all_remove' onClick=\"acl_set_all('_0_d$',false);\" value='D-'> - ";
      $display .= "<input ".$style." type='button' name='set_true_all_read' onClick=\"acl_set_all('_0_r$',true);\" value='R+'>";
      $display .= "<input ".$style." type='button' name='set_false_all_read' onClick=\"acl_set_all('_0_r$',false);\" value='R-'>";
      $display .= "<input ".$style." type='button' name='set_true_all_write' onClick=\"acl_set_all('_0_w$',true);\" value='W+'>";
      $display .= "<input ".$style." type='button' name='set_false_all_write' onClick=\"acl_set_all('_0_w$',false);\" value='W-'> - ";

      $display .= "<input ".$style." type='button' name='set_true_all_read' onClick=\"acl_set_all('[^0]_r$',true);\" value='R+'>";
      $display .= "<input ".$style." type='button' name='set_false_all_read' onClick=\"acl_set_all('[^0]_r$',false);\" value='R-'>";
      $display .= "<input ".$style." type='button' name='set_true_all_write' onClick=\"acl_set_all('[^0]_w$',true);\" value='W+'>";
      $display .= "<input ".$style." type='button' name='set_false_all_write' onClick=\"acl_set_all('[^0]_w$',false);\" value='W-'>";
    }

    /* Build general objects */
    $list =$this->sort_by_priority($list);
    foreach ($list as $key => $name){

      /* Create sub acl if it does not exist */
      if (!isset($this->aclContents[$key])){
        $this->aclContents[$key]= array();
      }
      if(!isset($this->aclContents[$key][0])){
        $this->aclContents[$key][0]= '';
      }

      $currentAcl= $this->aclContents[$key];

      /* Get the overall plugin acls
       */
      $overall_acl ="";
      if(isset($currentAcl[0])){
        $overall_acl = $currentAcl[0];
      }

      // Detect configured plugins
      $expand = count($currentAcl) > 1 || $currentAcl[0] != "";

      /* Object header */
      $tname= preg_replace("/[^a-z0-9]/i","_",$name);

      if($expand){
        $back_color = "#C8C8FF";
      }else{
        $back_color = "#C8C8C8";
      }

      if(session::global_get('js')) {
        if(isset($_SERVER['HTTP_USER_AGENT']) &&
             (preg_match("/gecko/i",$_SERVER['HTTP_USER_AGENT'])) ||
             (preg_match("/presto/i",$_SERVER['HTTP_USER_AGENT']))) {
          $display.= "\n<table style='width:100%;border:1px solid #A0A0A0' cellspacing=0 cellpadding=2>".
                     "\n  <tr>".
                     "\n    <td style='background-color:{$back_color};height:1.8em;' colspan=".($cols-1)."><b>"._("Object").": $name</b></td>".
                     "\n    <td align='right' style='background-color:{$back_color};height:1.8em;'>".
                     "\n    <input type='button' onclick=\"$('{$tname}').toggle();\" value='"._("Show/hide advanced settings")."' /></td>".
                     "\n  </tr>";
        } else if (isset($_SERVER['HTTP_USER_AGENT']) && preg_match("/ie/i",$_SERVER['HTTP_USER_AGENT'])) {
          $display.= "\n<table style='width:100%;border:1px solid #A0A0A0' cellspacing=0 cellpadding=2>".
                     "\n  <tr>".
                     "\n    <td style='background-color:#C8C8C8;height:1.8em;' colspan=".($cols-1)."><b>"._("Object").": $name</b></td>".
                     "\n    <td align='right' style='background-color:#C8C8C8;height:1.8em;'>".
                     "\n    <input type='button' onclick=\"$('{$tname}').toggle();\" value='"._("Show/hide advanced settings")."' /></td>".
                     "\n  </tr>";
        } else {
          $display.= "\n<table style='width:100%;border:1px solid #A0A0A0' cellspacing=0 cellpadding=2>".
                     "\n  <tr>".
                     "\n    <td style='background-color:#C8C8C8;height:1.8em;' colspan=$cols><b>"._("Object").": $name</b></td>".
                     "\n  </tr>";
        }
      } else {
          $display.= "\n<table style='width:100%;border:1px solid #A0A0A0' cellspacing=0 cellpadding=2>".
                     "\n  <tr>".
                     "\n    <td style='background-color:#C8C8C8;height:1.8em;' colspan=$cols><b>"._("Object").": $name</b></td>".
                     "\n  </tr>";
      }

      /* Generate options */
      $spc= "&nbsp;&nbsp;";
      $options= $this->mkchkbx($key."_0_c",  _("Create objects"), preg_match('/c/', $overall_acl)).$spc;
      $options.= $this->mkchkbx($key."_0_m", _("Move objects"), preg_match('/m/', $overall_acl)).$spc;
      $options.= $this->mkchkbx($key."_0_d", _("Remove objects"), preg_match('/d/', $overall_acl)).$spc;
      if ($plist[preg_replace('%^.*/%', '', $key)]['plSelfModify']){
        $options.= $this->mkchkbx($key."_0_s", _("Grant permission to owner"), preg_match('/s/', $overall_acl)).$spc;
      }

      /* Global options */
      $more_options= $this->mkchkbx($key."_0_r",  _("read"), preg_match('/r/', $overall_acl)).$spc;
      $more_options.= $this->mkchkbx($key."_0_w", _("write"), preg_match('/w/', $overall_acl));

      $display.= "\n  <tr>".
                 "\n    <td style='background-color:#E0E0E0' colspan=".($cols-1).">$options</td>".
                 "\n    <td style='background-color:#D4D4D4'>&nbsp;"._("Complete object").": $more_options</td>".
                 "\n  </tr>";

      /* Walk through the list of attributes */
      $cnt= 1;
      $splist= $plist[preg_replace('%^.*/%', '', $key)]['plProvidedAcls'];
      if(session::global_get('js')) {
        if(isset($_SERVER['HTTP_USER_AGENT']) &&
            (preg_match("/gecko/i",$_SERVER['HTTP_USER_AGENT'])) || (preg_match("/presto/i",$_SERVER['HTTP_USER_AGENT']))) {
          $display.= "\n  <tr id='tr_$tname' style='vertical-align:top;height:0px;'>".
                     "\n    <td colspan=".$cols.">".
                     "\n      <div id='$tname' style='overflow:hidden; display:none;vertical-align:top;width:100%;'>".
                     "\n        <table style='width:100%;'>";
        } else if (isset($_SERVER['HTTP_USER_AGENT']) && preg_match("/ie/i",$_SERVER['HTTP_USER_AGENT'])) {
          $display.= "\n  <tr id='tr_$tname' style='vertical-align:top;height:0px;'>".
                     "\n    <td colspan=".$cols.">".
                     "\n      <div id='$tname' style='position:absolute;overflow:hidden;display:none;;vertical-align:top;width:100%;'>".
                     "\n        <table style='width:100%;'>";
        }else{
        }
      }


      foreach($splist as $attr => $dsc){

        /* Skip pl* attributes, they are internal... */
        if (preg_match('/^pl[A-Z]+.*$/', $attr)){
          continue;
        }

        /* Open table row */
        if ($cnt == 1){
          $display.= "\n  <tr>";
        }

        /* Close table row */
        if ($cnt == $cols){
          $cnt= 1;
          $rb= "";
          $end= "\n  </tr>";
        } else {
          $cnt++;
          $rb= "border-right:1px solid #A0A0A0;";
          $end= "";
        }

        /* Collect list of attributes */
        $state= "";
        if (isset($currentAcl[$attr])){
          $state= $currentAcl[$attr];
        }
        $display.= "\n    <td style='border-top:1px solid #A0A0A0;${rb}width:".(int)(100/$cols)."%'>".
                   "\n      <b>$dsc</b> ($attr)<br>".$this->mkrwbx($key."_".$attr, $state)."</td>$end";
      }

      /* Fill missing td's if needed */
      if (--$cnt != $cols && $cnt != 0){
       $display.= str_repeat("\n    <td style='border-top:1px solid #A0A0A0; width:".(int)(100/$cols)."%'>&nbsp;</td>", $cols-$cnt);
      }

      if(session::global_get('js')) {
        if(isset($_SERVER['HTTP_USER_AGENT']) &&
            (preg_match("/gecko/i",$_SERVER['HTTP_USER_AGENT'])) ||
            (preg_match("/presto/i",$_SERVER['HTTP_USER_AGENT'])) ||
            (preg_match("/ie/i",$_SERVER['HTTP_USER_AGENT']))) {
          $display.= "\n        </table>".
                     "\n      </div>".
                     "\n    </td>".
                     "\n  </tr>";
        }
      }

      $display.= "\n</table><br />\n";
    }

    return ($display);
  }

  /*!
   * \brief Create a checkbox
   *
   * \param String $name Name of the acl checkbox
   *
   * \param String $text Label for the checkbox
   *
   * \param boolean $state false
   *
   * \return String containing checkbox
   */
  function mkchkbx($name, $text, $state= FALSE)
  {
    $state= $state?"checked":"";
    if($this->acl_is_writeable("")){
      $tname= preg_replace("/[^a-z0-9]/i","_",$name);
      return "\n      <input id='acl_$tname' type=checkbox name='acl_$name' $state>".
        "\n      <label for='acl_$tname'>$text</label>";
    }else{
      return "\n <input type='checkbox' disabled name='dummy_".microtime(1)."' $state>$text";
    }
  }

  /*!
   * \brief Make a read and write box
   *
   * \param String $name Name of the box
   *
   * \param boolean $state false
   *
   * \return String containing checkbox
   */
  function mkrwbx($name, $state= "")
  {
    $rstate= preg_match('/r/', $state)?'checked':'';
    $wstate= preg_match('/w/', $state)?'checked':'';
    $tname= preg_replace("/[^a-z0-9]/i","_",$name);

    if($this->acl_is_writeable("")){
      return ("\n      <input id='acl_".$tname."_r' type=checkbox name='acl_${name}_r' $rstate>".
          "\n      <label for='acl_".$tname."_r'>"._("read")."</label>".
          "\n      <input id='acl_".$tname."_w' type=checkbox name='acl_${name}_w' $wstate>".
          "\n      <label for='acl_".$tname."_w'>"._("write")."</label>");
    }else{
      return ("\n      <input disabled type=checkbox name='dummy_".microtime(1)."' $rstate>"._("read").
          "\n      <input disabled type=checkbox name='dummy_".microtime(1)."' $wstate>"._("write"));
    }
  }

  /*!
   * \brief Explode an acl
   *
   * \param string $acl ACL to be exploded
   */
  static function explodeACL($acl)
  {

    $list= explode(':', $acl);
    if(count($list) == 5){
      list($index, $type,$member,$permission,$filter)= $list;
      $filter = base64_decode($filter);
    }else{
      $filter = "";
      list($index, $type,$member,$permission)= $list;
    }

    $a= array( $index => array("type" => $type,
                               "filter"=> $filter,
                               "members" => acl::extractMembers($acl,$type == "role")));

    /* Handle different types */
    switch ($type){

      case 'psub':
      case 'sub':
      case 'one':
      case 'base':
        $a[$index]['acl']= acl::extractACL($acl);
        break;

      case 'role':
        $a[$index]['acl']= base64_decode(preg_replace('/^[^:]+:[^:]+:([^:]+).*$/', '\1', $acl));
        break;

      case 'reset':
        break;

      default:
        msg_dialog::display(_("Internal error"), sprintf(_("Unkown ACL type '%s'!"), $type), ERROR_DIALOG);
        $a= array();
    }
    return ($a);
  }

  /*!
   * \brief Extract members of an acl
   *
   * \param $acl The acl to be extracted
   *
   * \param $role Boolean false
   *
   * \return an array with members
   */
  static function extractMembers($acl,$role = FALSE)
  {
    global $config;
    $a= array();

    /* Rip acl off the string, seperate by ',' and place it in an array */
    if($role){
      $ms= preg_replace('/^[^:]+:[^:]+:[^:]+:([^:]+).*$/', '\1', $acl);
    }else{
      $ms= preg_replace('/^[^:]+:[^:]+:([^:]+).*$/', '\1', $acl);
    }
    if ($ms == $acl){
      return $a;
    }
    $ma= explode(',', $ms);

    /* Decode dn's, fill with informations from LDAP */
    $ldap= $config->get_ldap_link();
    foreach ($ma as $memberdn){
      // Check for wildcard here
      $dn= base64_decode($memberdn);
      if ($dn != "*") {
        $ldap->cat($dn, array('cn', 'objectClass', 'description', 'uid'));

        /* Found entry... */
        if ($ldap->count()){
          $attrs= $ldap->fetch();
          if (in_array_ics('gosaAccount', $attrs['objectClass'])){
            $a['U:'.$dn]= $attrs['cn'][0]." [".$attrs['uid'][0]."]";
          } else {
            $a['G:'.$dn]= $attrs['cn'][0];
            if (isset($attrs['description'][0])){
              $a['G:'.$dn].= " [".$attrs['description'][0]."]";
            }
          }

        /* ... or not */
        } else {
          $a['U:'.$dn]= sprintf(_("Unknown entry '%s'!"), $dn);
        }

      } else {
        $a['G:*']= sprintf(_("All users"));
      }
    }

    return ($a);
  }

  /*!
   * \brief Extract an acl
   *
   * \param string $acl The acl to be extracted
   */
  static function extractACL($acl)
  {
    /* Rip acl off the string, seperate by ',' and place it in an array */
    $as= preg_replace('/^[^:]+:[^:]+:[^:]*:([^:]*).*$/', '\1', $acl);
    $aa= explode(',', $as);
    $a= array();

    /* Dis-assemble single ACLs */
    foreach($aa as $sacl){

      /* Dis-assemble field ACLs */
      $ao= explode('#', $sacl);
      $gobject= "";
      foreach($ao as $idx => $ssacl){

        /* First is department with global acl */
        $object= preg_replace('/^([^;]+);.*$/', '\1', $ssacl);
        $gacl=   preg_replace('/^[^;]+;(.*)$/', '\1', $ssacl);
        if ($idx == 0){
          /* Create hash for this object */
          $gobject= $object;
          $a[$gobject]= array();

          /* Append ACL if set */
          if ($gacl != ""){
            $a[$gobject]= array($gacl);
          }
        } else {

          /* All other entries get appended... */
          list($field, $facl)= explode(';', $ssacl);
          $a[$gobject][$field]= $facl;
        }

      }
    }

    return ($a);
  }

  /*!
   * \brief Summarize ACL and members
   *
   * \param Array $entry
   */
  function assembleAclSummary($entry)
  {
    $summary= "";

    /* Summarize ACL */
    if (isset($entry['acl'])){
      $acl= "";

      if($entry['type'] == "role"){

        if(isset($this->roles[$entry['acl']])){
          $summary.= sprintf(_("Role: %s"), $this->roles[$entry['acl']]['cn']);
        }else{
          $summary.= sprintf(_("Role: %s"), "<i>"._("unknown role")."</i>");
        }
      }else{
        foreach ($entry['acl'] as $name => $object){
          if (count($object)){
            $acl.= "$name, ";
          }
        }
        $summary.= sprintf(_("Contains settings for these objects: %s"), preg_replace('/, $/', '', $acl));
      }
    }


    /* Summarize members */
    if(!($this instanceOf aclrole)){
      if ($summary != ""){
        $summary.= ", ";
      }
      if (count($entry['members'])){
        $summary.= _("Members").": ";
        foreach ($entry['members'] as $cn){
          $cn= preg_replace('/ \[.*$/', '', $cn);
          $summary.= $cn.", ";
        }
      } else {
        $summary.= "<font color='red'><i>"._("inactive")."&nbsp;-&nbsp;"._("No members")."</i></font>";
      }
    }
    return (preg_replace('/, $/', '', $summary));
  }

  /*!
   * \brief Load an ACL
   *
   * \param boolean $new false
   */
  function loadAclEntry($new= FALSE)
  {
    /* New entry gets presets... */
    if ($new){
      $this->aclType= 'base';
      $this->aclFilter= "";
      $this->recipients= array();
      $this->aclContents= array();
    } else {
      $acl= $this->gosaAclEntry[$this->currentIndex];
      $this->aclType= $acl['type'];
      $this->recipients= $acl['members'];
      $this->aclContents= $acl['acl'];
      $this->aclFilter= $acl['filter'];
    }

    $this->wasNewEntry= $new;
  }

  /*!
   * \brief Save an ACL after it's handle
   *
   * \return boolean true if the acl is saved, else return false
   */
  function aclPostHandler()
  {
    if (isset($_POST['save_acl'])){
      $this->save();
      return TRUE;
    }

    return FALSE;
  }

  /*!
   * \brief Prepare for Copy & Paste
   *
   * \see plugin::PrepareForCopyPaste($source)
   *
   * \param string $source Source to prepare for copy and paste
   */
  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);

    $dn = $source['dn'];
    $acl_c = new acl($this->config, $dn, $this->parent);
    $this->gosaAclEntry = $acl_c->gosaAclEntry;
  }

  /*!
   *  \brief Save data to LDAP, depending on is_account we save or delete
   */
  function save()
  {
    /* Assemble ACL's */
    $tmp_acl= array();

    foreach ($this->gosaAclEntry as $prio => $entry){
      $final= "";
      $members= "";
      if (isset($entry['members'])){
        foreach ($entry['members'] as $key => $dummy){
          $members.= base64_encode(preg_replace('/^.:/', '', $key)).',';
        }
      }

      if($entry['type'] != "role"){
        $final= $prio.":".$entry['type'].":".preg_replace('/,$/', '', $members);
      }else{
        $final= $prio.":".$entry['type'].":".base64_encode($entry['acl']).":".preg_replace('/,$/', '', $members);
      }

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

      /* Append additional filter options
       */
      if(!empty($entry['filter'])){
        $final .= ":".base64_encode($entry['filter']);
      }

      $tmp_acl[]= $final;
    }

    /* Call main method */
    plugin::save();

    /* Finally (re-)assign it... */
    $this->attrs['gosaAclEntry']= $tmp_acl;

    /* Remove acl from this entry if it is empty... */
    if (!count($tmp_acl)){
      /* Remove attribute */
      if ($this->initially_was_account){
        $this->attrs['gosaAclEntry']= array();
      } else {
        if (isset($this->attrs['gosaAclEntry'])){
          unset($this->attrs['gosaAclEntry']);
        }
      }

      /* Remove object class */
      $this->attrs['objectClass']= array_remove_entries(array('gosaAcl'), $this->attrs['objectClass']);
    }

    /* Do LDAP modifications */
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->dn);
    $this->cleanup();
    $ldap->modify ($this->attrs);

    if(count($this->attrs)){
      new log("modify","acls/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }

    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()), ERROR_DIALOG);
    }

    /* Refresh users ACLs */
    $ui= get_userinfo();
    $ui->loadACL();
    session::global_set('ui',$ui);
  }

  /*!
   * \brief Removes object from parent
   */
  function remove_from_parent()
  {
    plugin::remove_from_parent();

    /* include global link_info */
    $ldap= $this->config->get_ldap_link();

    $ldap->cd($this->dn);
    $this->cleanup();
    $ldap->modify ($this->attrs);

    new log("remove","acls/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());

    /* Optionally execute a command after we're done */
    $this->handle_post_events("remove",array("uid" => $this->uid));
  }


  /*
   * \brief Return plugin informations for acl handling
   */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("ACL"),
      "plDescription" => _("Manage access control lists"),
      "plSelfModify"  => FALSE,
      "plCategory"    => array("acl" => array("description"  => _("ACL")."&nbsp;&amp;&nbsp;"._("ACL roles"),
                                                      "objectClass"  => array("gosaAcl","gosaRole"))),
      "plObjectType"  => array("acl" => array("name"  => _("ACL"),
                                              "filter"  => "objectClass=gosaAcl")),

      "plProvidedAcls"=> array(
//        "cn"          => _("Role name"),
//        "description" => _("Role description")
      )
    );
  }


  /*!
   * \brief Remove acls defined for $src
   */
  function remove_acl()
  {
    acl::remove_acl_for($this->dn);
  }


  /*!
   * \brief Remove acls defined for $src
   *
   * \param String $dn The DN
   */
  static function remove_acl_for($dn)
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search("(&(objectClass=gosaAcl)(gosaAclEntry=*".base64_encode($dn)."*))",array("gosaAclEntry","dn"));
    $new_entries= array();
    while($attrs = $ldap->fetch()){
      if (!isset($attrs['gosaAclEntry'])) {
        continue;
      }
      unset($attrs['gosaAclEntry']['count']);

      // Remove entry directly
      foreach($attrs['gosaAclEntry'] as $id => $entry){
        $parts= explode(':',$entry);
        $members= explode(',',$parts[2]);
        $new_members= array();
        foreach($members as $member) {
          if (base64_decode($member) != $dn) {
            $new_members[]= $member;
          } else {
            fusiondirectory_log("modify","users/acl",$attrs['dn'],array(),sprintf("Removed acl for %s on object %s.",$dn,$attrs['dn']));
          }
        }

        /* We can completely remove the entry if there are no members anymore */
        if (count($new_members)) {
          $parts[2]= implode(",", $new_members);
          $new_entries[]= implode(":", $parts);
        }
      }

      // There should be a modification, so write it back
      $ldap->cd($attrs['dn']);
      $new_attrs= array("gosaAclEntry" => $new_entries);
      $ldap->modify($new_attrs);
      if (!$ldap->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $dn, LDAP_MOD, get_class()), ERROR_DIALOG);
      }
    }
  }

  /*!
   * \brief Update the acl membership
   *
   * \param String $src The source
   *
   * \param String $dst The destination
   */
  function update_acl_membership($src,$dst)
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=gosaAcl)(gosaAclEntry=*".base64_encode($src)."*))",array("gosaAclEntry","dn"));
    while($attrs = $ldap->fetch()){
      $acl = new acl($this->config, $attrs['dn'], $this->parent);
      foreach($acl->gosaAclEntry as $id => $entry){
        foreach($entry['members'] as $m_id => $member){
          if($m_id == "U:".$src){
            unset($acl->gosaAclEntry[$id]['members'][$m_id]);
            $new = "U:".$dst;
            $acl->gosaAclEntry[$id]['members'][$new] = $new;
            fusiondirectory_log("modify","users/acl",$attrs['dn'],array(),sprintf("Updated acl for user %s on object %s.",$src,$attrs['dn']));
          }
          if($m_id == "G:".$src){
            unset($acl->gosaAclEntry[$id]['members'][$m_id]);
            $new = "G:".$dst;
            $acl->gosaAclEntry[$id]['members'][$new] = $new;
            fusiondirectory_log("modify","groups/acl",$attrs['dn'],array(),sprintf("Updated acl for group %s on object %s.",$src,$attrs['dn']));
          }
        }
      }
      $acl -> save();
    }
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
