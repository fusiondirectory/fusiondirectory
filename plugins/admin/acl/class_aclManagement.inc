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

class aclManagement extends management
{
  var $plHeadline     = "Acl";
  var $plDescription  = "Manage access control lists";
  var $plIcon  = "plugins/acl/images/plugin.png";

  // Tab definition
  protected $tabClass = "aclroletab";
  protected $tabType = "ACLROLETAB";
  protected $aclCategory = "acl";
  protected $aclPlugin   = "acl";
  protected $objectName   = "acl";

  function __construct(&$config,$ui)
  {
    $this->config = $config;
    $this->ui = $ui;

    $this->storagePoints = array(get_ou("aclRoleRDN")); // ACLs are attached to department containers

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")){
      $filter= session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path("acl-filter.xml", true));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("acl-list.xml", true));
    $headpage->registerElementFilter("filterLabel", "aclManagement::filterLabel");
    $headpage->setFilter($filter);

    // Add copy&paste and snapshot handler.
    if ($this->config->boolValueIsTrue("main", "copyPaste")){
      $this->cpHandler = new CopyPasteHandler($this->config);
    }
    if($this->config->get_cfg_value("enableSnapshots") == "TRUE"){
      $this->snapHandler = new SnapshotHandler($this->config);
    }

    parent::__construct($this->config, $ui, "acl", $headpage);
  }

  function removeEntryConfirmed($action="",$target=array(),$all=array(),
      $altTabClass="",$altTabType="",$altAclCategory="")
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$target,"Entry removel confirmed!");

    $headpage = $this->getHeadpage();
    foreach($this->dns as $key => $dn){

      // Check permissions, are we allowed to remove this object?
      $acl = $this->ui->get_permissions($dn, $this->aclCategory."/".$this->aclPlugin);
      if(preg_match("/d/",$acl)){

        if($headpage->getType($dn) == "gosaRole"){
          $tabClass = "aclroletab";
          $tabType  = "ACLROLETAB";
        }else{
          $tabClass = "acltab";
          $tabType  = "ACLTAB";
        }

        // Delete the object
        $this->dn = $dn;
        $this->tabObject= new $tabClass($this->config,$this->config->data['TABS'][$tabType], $this->dn, $aclCategory, true, true);
        $this->tabObject->set_acl_base($this->dn);
        $this->tabObject->delete ();
        $this->tabObject->parent = &$this;

        // Remove the lock for the current object.
        del_lock($this->dn);
      } else {
        msg_dialog::display(_("Permission error"), msgPool::permDelete(), ERROR_DIALOG);
        new log("security","groups/".get_class($this),$dn,array(),"Tried to trick deletion.");
      }
    }

    // Cleanup
    $this->remove_lock();
    $this->closeDialogs();
  }


  function newEntry($action="",$target=array(),$all=array(), $altTabClass ="", $altTabType = "", $altAclCategory="")
  {
    $this->skipFooter = TRUE;
    $altTabClass = "aclroletab";
    $altTabType = "ACLROLETAB";
    return(management::newEntry($action,$target,$all,$altTabClass,$altTabType,$altAclCategory));
  }

  function editEntry($action="",$target=array(),$all=array(), $altTabClass ="", $altTabType = "", $altAclCategory="")
  {
    $this->skipFooter = TRUE;
    if(count($target) == 1){

      // Set dummy tab object...
      $this->dn = array_pop($target);
      $headpage = $this->getHeadpage();
      if($headpage->getType($this->dn) == "gosaRole"){
        $altTabClass = "aclroletab";
        $altTabType  = "ACLROLETAB";
      }else{
        $altTabClass = "acltab";
        $altTabType  = "ACLTAB";
      }
      return(management::editEntry($action,array($this->dn),$all,$altTabClass,$altTabType,$altAclCategory));
    }
  }


  function detectPostActions()
  {
    $action= management::detectPostActions();
    if(isset($_POST['edit_acl'])) $action['action'] = "edit_acl";
    if(isset($_POST['edit_role'])) $action['action'] = "edit_role";
    return($action);
  }


  // A filter which allows to open a department by clicking on the departments name.
  static function filterLabel($row,$dn,$ou= array(),$pid=0,$base="")
  {
    $ou = $ou[0];
    if($dn == $base){
      $ou =" . ";
    }
    if(!preg_match("/^cn=/",$dn)){
      $ou.="  &nbsp; ["._("ACL Assignment")."]";
    }

    $dn= LDAP::fix(func_get_arg(1));
    return("<a href='?plug=".$_GET['plug']."&amp;PID=$pid&amp;act=listing_edit_$row' title='$dn'>$ou</a>");
  }


}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
