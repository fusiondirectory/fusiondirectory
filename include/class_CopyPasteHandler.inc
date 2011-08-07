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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#define("TEMP_DIR","/var/cache/fusiondirectory/tmp");

class CopyPasteHandler {

  var $config;
  var $current;

  /* This array contains all dns of the currently copyied objects */
  var $queue       = array(); 

  /* Attributes that should be overwritten */
  var $setvar_array= array();

  /* The dn of the last edited object */
  var $lastdn      = "";
  
  var $disallowed_objects = array();
  var $objects_to_fix     = array();
  var $clean_objects      = array();
  var $require_update     = FALSE;



  /* Create CP handler  */
  function CopyPasteHandler(&$config)
  {
    $this->config = &$config;	
    $this->current= NULL;
    $this->queue  = array();
    $this->setvar_array = array();
  }


  /* Entry entry to Copy & Paste queue.
   * A Queue entry is represented as follows.
   *  array['file_name']  - Position on hdd 
   *  array['method']     - copy/cut
   *  array['dn']         - the dn of the object added to the queue 
   *  array['tab_class']  - Tab object that should be used to initialize the new object
   *  array['tab_object'] - Tab object name used to initialize correct object Type like USERTABS
   */
  function add_to_queue($dn,$action,$tab_class,$tab_object,$tab_acl_category,&$parent = NULL)
  {
    if(!class_available($tab_class)){
      trigger_error(sprintf("Specified class object '%s' does not exists.",$tab_class));
      return(FALSE);
    }

    if(!isset($this->config->data['TABS'][$tab_object])){
      trigger_error(sprintf("Specified tab object '%s' does not exists.",$tab_object));
      return(FALSE);
    }

    if(!in_array($action,array("cut","copy"))){
      trigger_error(sprintf("Specified action '%s' does not exists for copy & paste.",$action));
      return(FALSE);
    } 

    if($file_name = $this->save_dn_attributes_to_hdd($dn)){
      $tmp = array();
      $tmp['file_name'] = $file_name;
      $tmp['method']    = $action;  
      $tmp['dn']        = $dn;
      $tmp['tab_class'] = $tab_class;
      $tmp['tab_object']= $tab_object;
      $tmp['tab_acl_category']= $tab_acl_category;
      $tmp['parent']    = $parent;
      $this->queue[]    = $tmp; 
      $this->require_update = TRUE;
    }
  }


  /* This removes all objects from queue.
   * Remove hdd dumps of current entries too.
   * Remove entries older than 24 hours.
   */
  function cleanup_queue()
  {
    $this->current = FALSE;
    $this->require_update = TRUE;
    $this->setvar_array = array();

    /* Remove all entries from queue */  
    foreach($this->queue as $key => $entry){
      @rmdir($entry['file_name']);  
      unset($this->queue[$key]);
    }

    /* Create patch if it doesn't exists */
    if(!is_dir(TEMP_DIR)){
      @mkdir(TEMP_DIR);

      /* Update folder permissions */
      if(!@chmod(TEMP_DIR,0700)){
        $msg= sprintf(_("Copy and paste failed!")."<br><br>"._("Error").": <i>"._("Cannot set permission for '%s'")."</i>" ,TEMP_DIR);
        msg_dialog::display(_("Configuration error"), $msg, ERROR_DIALOG);
        new log("copy","all/all","copy & paste, event queue.",array(), $msg);
        return(FALSE);
      }
    }    
    
    /* check if we are able to create a new file the given directory */
    if(!is_writeable(TEMP_DIR)){
      $msg= _("Copy and paste failed!")."<br><br>"._("Error").": <i>".msgPool::cannotWriteFile(TEMP_DIR)."</i>";
      msg_dialog::display(_("Configuration error"), $msg, ERROR_DIALOG);
      new log("copy","all/all","copy & paste, event queue.",array(), $msg);
      return(FALSE);
    }

    /* Remove entries from hdd that are older than24 hours */
    $fp = opendir(TEMP_DIR);
    while($file = readdir($fp)){
      if(is_file(TEMP_DIR."/".$file) && !preg_match("/^\./",$file)){
        $file_time = fileatime(TEMP_DIR."/".$file);
        if($file_time < (time() - (24* 60 *60))){
          @unlink(TEMP_DIR."/".$file);
        }
      }
    }
  }


  /* To increase performance we save the ldap dump on hdd 
   * This function automatically creates the dumps and returns 
   *  the name of the dumpfile we created 
   */
  function save_dn_attributes_to_hdd($dn)
  {
    $filename = "Should not be returned";
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $res  = $ldap->cat($dn);

    /* Check if given dn is valid and ldap search was succesfull */ 
    if(!$res){
      $msg= sprintf(_("Copy and paste failed!")."<br><br>"._("Error").": <i>"._("'%s' is no vaild LDAP object"), LDAP::fix($dn));
      msg_dialog::display(_("Internal error"), $msg, ERROR_DIALOG);
      new log("copy","all/all",$dn,array(), $msg);
      return(FALSE);
    }

    /* Create data to save given ldap dump on the hdd */
    $filename = "gosa_copy-paste_dump_".preg_replace("/[^0-9]/","",microtime());
    $path     = TEMP_DIR;

    /* Create patch if it doesn't exists */
    if(!is_dir($path)){
      @mkdir($path);
    }    

    /* check if we are able to create a new file the given directory */
    if(!is_writeable($path)){
      $msg= sprintf(_("Copy and paste failed!")."<br><br>"._("Error").": <i>"._("No write permission in '%s'"),TEMP_DIR);
      msg_dialog::display(_("Configuration error"), $msg, ERROR_DIALOG);
      new log("copy","all/all",$dn,array(), $msg);
      return(FALSE);
    }  

    /* Create file handle */
    $fp = @fopen($path."/".$filename,"w+");
    if(!$fp){
      $msg= _("Copy and paste failed!")."<br><br>"._("Error").": <i>".msgPool::cannotWriteFile("$path/$filename")."</i>";
      msg_dialog::display(_("Configuration error"), $msg, ERROR_DIALOG);
      new log("copy","all/all",$dn,array(), $msg);
      return(FALSE);
    }    

    /* Update folder permissions */
    if(!@chmod($path."/".$filename,0700)){
      $msg= sprintf(_("Copy and paste failed!")."<br><br>"._("Error").": <i>"._("Cannot set permission for '%s'"),TEMP_DIR);
      msg_dialog::display(_("Configuration error"), $msg, ERROR_DIALOG);
      new log("copy","all/all","copy & paste, event queue.",array(), $msg);
      return(FALSE);
    }

    $data = serialize($ldap->fetch());
    fwrite($fp,$data,strlen($data));
    fclose($fp);

    /* Only the webserver should be able to read those files */
    @chmod($path."/".$filename,0600); 
    return($path."/".$filename);
  }


  /* Check if there are still entries the object queue */
  function entries_queued()
  {
    return( count($this->queue) >=1 || $this->current != FALSE);
  }


  /* Paste one entry from queue */
  function load_entry_from_queue($entry)
  {
    if(!isset($entry['tab_class'])){
      return(array());
    }


    $tab_c = $entry['tab_class'];
    $tab_o = $entry['tab_object'];
    $tab_a = $entry['tab_acl_category'];
    $parent = $entry['parent'];

    if($entry['method'] == "copy"){
      $entry['object']      = new $tab_c($this->config,$this->config->data['TABS'][$tab_o],"new",$tab_a);
    }else{
      $entry['object']      = new $tab_c($this->config,$this->config->data['TABS'][$tab_o],$entry['dn'],$tab_a);
    }

    if($parent ){
      $entry['object']->parent = $parent;
    }
    $entry['source_data'] = $this->load_attributes_from_hdd($entry['file_name']);

    if($entry['method'] == "copy"){

      /* Prepare each plugin of this tab object to be posted */
      foreach($entry['object']->by_object as $name => $obj){

        /* Prepare every single class, to be copied  */
        $entry['object']->by_object[$name]->PrepareForCopyPaste($entry['source_data']);

        /* handle some special vars */
        foreach(array("is_account") as $attr){
          if(isset($entry['source_data'][$attr])){
            $entry['object']->by_object[$name]->$attr = $entry['source_data'][$attr];
          }
        }
      }
    }
    return($entry);
  }


  /* Load dumped ldap entry specified by $filename and 
   *  return data an unserailized data array
   */
  function load_attributes_from_hdd($filename)
  {
    $fp = @fopen($filename,"r");
    if(is_file($filename) && is_readable($filename) && $fp){
      $data = "";
      while($str = fgets($fp,512)){
        $data .= $str;
      }
      return(unserialize($data));
    }else{
      $msg= sprintf(_("Copy and paste failed!")."<br><br>"._("Error").": <i>".msgPool::cannotReadFile($filename)."</i>");
      msg_dialog::display(_("Internal error"), $msg, ERROR_DIALOG);
      new log("copy","all/all",$dn,array(), $msg);
      return(FALSE);
    }
  }


  /* Displays a dialog which allows the user to fix all dependencies of this object.
     Create unique names, ids, or what ever */
  function execute()
  {
    $ui = get_userinfo();
    $type = $this->current['method'];

    /* Check which entries can be pasted directly.
     * Create a list of all entries that can be pasted directly.
     */
    if($this->require_update){
      $this->clean_objects      = array();
      $this->objects_to_fix     = array();
      $this->disallowed_objects = array();

      /* Put each queued object in one of the above arrays 
       */
      foreach($this->queue as $key => $entry){

        /* Update entries on demand 
         */
        if(!isset($entry['object'])){
          $entry = $this->load_entry_from_queue($entry);
          $this->queue[$key] = $entry;
        }
        $entry= $this->_update_vars($entry);
        $msgs = $entry['object']->check();

        /* To copy an object we require full read access to the object category 
         */
        $copy_acl = preg_match("/r/",$ui->has_complete_category_acls($entry['dn'], $entry['tab_acl_category']));

        /* In order to copy an object we require read an delete acls 
         */
        $cut_acl  = preg_match("/d/",$ui->has_complete_category_acls($entry['dn'], $entry['tab_acl_category']));
        $cut_acl &= preg_match("/r/",$ui->has_complete_category_acls($entry['dn'], $entry['tab_acl_category']));

        /* Check permissions */
        if($entry['method'] == "copy" && !$copy_acl){
          $this->disallowed_objects[$key] = $entry;
        }elseif($entry['method'] == "cut" && !$cut_acl){
          $this->disallowed_objects[$key] = $entry;
        }elseif(!count($msgs)){
          $this->clean_objects[$key]  = $entry;
        }else{
          $this->objects_to_fix[$key] = $entry;
        }
      }
      if(count($this->disallowed_objects)){
        $dns = array();
        foreach($this->disallowed_objects as $entry){
          $dns[] = $entry['dn'];
        }
#        msg_dialog::display(_("Permission"),msgPool::permCreate($dns),INFO_DIALOG);
      }
      $this->require_update = FALSE;
    }

    /* Save objects that can be pasted directly 
     */
    if(isset($_POST['PerformCopyPaste']) && count($this->clean_objects)){
      $this->save_object();
      $this->current = FALSE;
      foreach($this->clean_objects as $key => $entry){

        /* Remove from queue -> avoid saving twice */
        unset($this->queue[$key]);
        unset($this->clean_objects[$key]);

        /* Load next queue entry */
        $this->current = $entry;
        $this->lastdn = $this->current['object']->dn;
        $this->current= $this->_update_vars($this->current);
        $this->current['object']->save();
        $this->handleReferences();
        $this->current = FALSE;
      } 
    }

    /* Save edited entry and force loading new one 
     */
    if(isset($this->current['object']) && method_exists($this->current['object'],"saveCopyDialog")) {
      $this->current['object']->saveCopyDialog();
    }

    if(isset($_POST['PerformCopyPaste']) && $this->current){
      $msgs = $this->check();

      /* Load next queue entry */
      if(!count($msgs)){
        $this->current['object']->save();
        $this->handleReferences();
        $this->lastdn = $this->current['object']->dn;
        $this->current = FALSE;
      }else{
        foreach( $msgs as $msg){
          msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
        }
      }
    }
      
    /* Display a list of all pastable entries 
     */
    if(count($this->clean_objects)){
      
      $dns = array();
      foreach($this->clean_objects as $object){
        $dns[] = $object['dn'];
      }

      $smarty = get_smarty();
      $smarty->assign("type","directly");
      $smarty->assign("Complete",false);
      $smarty->assign("AttributesToFix","&nbsp;");
      $smarty->assign("SubDialog","");
      $smarty->assign("message"  , sprintf(_("These objects will be pasted: %s"), "<br>".msgPool::buildList($dns)));
      return($smarty->fetch(get_template_path("copyPasteDialog.tpl",FALSE)));
    }

    /* Display a list of all pastable entries 
     */
    if($this->current || count($this->objects_to_fix)){
      $this->save_object(); 
      if(!$this->current){
        $key = key($this->objects_to_fix);
        if(isset($this->objects_to_fix[$key])){
          $this->current = $this->objects_to_fix[$key];
          $this->current= $this->_update_vars($this->current);
          unset($this->objects_to_fix[$key]);
          unset($this->queue[$key]); 
        }
      }
      if($this->current){
        $smarty = get_smarty();
        $smarty->assign("type","modified");
        $smarty->assign("Complete",false);
        $smarty->assign("AttributesToFix",$this->generateAttributesToFix());
        $smarty->assign("SubDialog",$this->current['object']->SubDialog);
        $smarty->assign("objectDN",$this->current['source_data']['dn']);
        $smarty->assign("message", sprintf(_("This object will be pasted: %s"), "<br><br>".$this->current['source_data']['dn']));
        return($smarty->fetch(get_template_path("copyPasteDialog.tpl",FALSE)));
      }
    }
    return("");
  }


  /* Return the dn of the last edited entry */
  function last_entry()
  {
    return($this->lastdn);
  }


  /* Save new values posted by copy & paste dialog */
  function save_object()
  {
    if(isset($_POST['abort_current_cut-copy_operation'])){
      $this->current = FALSE;
    }

    if(isset($_POST['abort_all_cut-copy_operations'])){
      $this->cleanup_queue();
      $this->current = FALSE;
    }
  }


  /* Create dialog which asks unique attributes/values ... 
   *  call tabs -> getCopyDialog() 
   *    which calls tab -> getCopyDialog()  */
  function generateAttributesToFix()
  {
    if($this->current){
      return($this->current['object']->getCopyDialog());  
    }
  }


  /* Set a single attribute to specified value
   *  example :   ("base", $newBase );    */
  function SetVar($name,$value)
  {
    $this->setvar_array[$name]=$value; 
  }


  /* Update current object attributes, collected via SetVar */
  function _update_vars($entry)
  {
    /* Update all attributes specified with SetVar */
    foreach($this->setvar_array as $name => $value){
      if(isset($entry['object']->$name)){
        $entry['object']->$name = $value;
      }
    }

    /* Walk through tabs */
    foreach($entry['object']->by_object as $key => $obj){

      /* Update all attributes specified with SetVar */
      foreach($this->setvar_array as $name => $value){

        /* Do not update parent for plugins, this may break things */
        if($name == "parent") continue;

        if(isset($entry['object']->by_object[$key]->$name)){
          $entry['object']->by_object[$key]->$name = $value;
        }
      }
    }
    return($entry);
  }


  /* Returns errors from including tabs. */
  function check()
  {
    $ret = array();
    foreach($this->current['object']->by_object as $obj){
      if($obj->is_account || $obj->ignore_account){
        $ret = array_merge($ret , $obj->check());
      }
    }
    return($ret);
  }


  function handleReferences()
  {
    $dst_dn = $this->current['object']->dn;
    $src_dn = $this->current['dn'];

    // Only copy references if required 
    if($this->current['method'] != 'copy') return;

    // Migrate objectgroups
    $ogroups = get_sub_list("(&(objectClass=gosaGroupOfNames)(member=".LDAP::prepare4filter(LDAP::fix($src_dn))."))",
            "ogroups", array(get_ou("ogroupRDN")),$this->config->current['BASE'],array("dn"), GL_SUBSEARCH | GL_NO_ACL_CHECK);

    // Walk through all objectGroups
    foreach($ogroups as $ogroup){
        $o_ogroup= new ogroup($this->config,$ogroup['dn']);
        $o_ogroup->member[$dst_dn]= $dst_dn;
        $o_ogroup->save();
    }

    // Update roles 
    $roles = get_sub_list("(&(objectClass=organizationalRole)(roleOccupant=".LDAP::prepare4filter(LDAP::fix($src_dn))."))",
            "roles", array(get_ou("roleRDN")),$this->config->current['BASE'],array("dn"), GL_SUBSEARCH | GL_NO_ACL_CHECK);

    // Walk through all roles
    foreach($roles as $role){
        $role = new roleGeneric($this->config,$role['dn']);
        $role->roleOccupant[] = $dst_dn;
        $role->save();
    }

    // Update groups
    if(isset($this->current['object']->uid) && !empty($this->current['object']->uid)){

        $ldap = $this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        $ldap->cat($src_dn);
        $attrs = $ldap->fetch();
        if(isset($attrs['uid'][0])){
            $suid = $attrs['uid'][0];

            $uid = $this->current['object']->uid;
            $groups = get_sub_list("(&(objectClass=posixGroup)(memberUid={$suid}))",
                    "groups",array(get_ou("groupRDN")),$this->config->current['BASE'],array("dn"), GL_SUBSEARCH | GL_NO_ACL_CHECK);

            // Walk through all POSIX groups
            foreach($groups as $group){
                $o_group= new group($this->config,$group['dn']);
                $o_group->addUser($uid);
                $o_group->save();
            }
        }
    }
  }

  /* returns the paste icon for headpages */ 
  function generatePasteIcon()
  {
    $Copy_Paste= "&nbsp;<img class='center' src='images/lists/seperator.png' align='middle' alt='' height='16' width='1'>&nbsp;";
    if($this->entries_queued()){
      $img= "images/lists/paste.png";
      $Copy_Paste.= "<input type='image' name='editPaste' class='center'
        src='".$img."' alt='"._("Paste")."'>&nbsp;";
    }else{
      $Copy_Paste.= "<img class='center' src='images/lists/paste-grey.png' alt=\""._("Cannot paste")."\">&nbsp;";
    }
    return ($Copy_Paste);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
