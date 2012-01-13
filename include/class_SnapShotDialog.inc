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


/* Snap shot dialog class */
class SnapShotDialog extends plugin 
{
	var $config;
	var $attributes= array("CurrentDescription");
	var $CurrentDescription= "";
	var $parent= NULL;
	var $display_restore_dialog= false;		      /* Defines the restore mode */
	var $display_all_removed_objects= false; 		/* Specifies which objects will be listed, all
                                                 snapshots for a single entry or all snapshots
                                                 of already deleted objects  */	
	var $dialog= true;
	var $del_dn= "";
	var $ui;
	var $acl;
  var $dns = array();
  var $snap_shot_bases = array();
  var $last_list = array();

	
	function SnapShotDialog(&$config, $dn, &$parent)
	{
    plugin::plugin($config,$dn);
    $this->parent 	= &$parent;
		$this->ui 		  = get_userinfo();
	}

  
  /* Show deleted snapshots from these bases */
  function set_snapshot_bases($bases)
  {
    $this->snap_shot_bases = $bases;
  }
	
	
	/* Display snapshot dialog */
	function execute()
	{
		plugin::execute();
		$smarty = get_smarty();

    $ui = get_userinfo();
		$once = true;
		foreach($_POST as $name => $value){
			if((preg_match("/^RemoveSnapShot_/",$name)) && ($once)){
				$once = false;
        $entry = preg_replace("/^RemoveSnapShot_/","",$name);
        $entry = base64_decode(preg_replace("/_[xy]$/","",$entry));
        $found = false;
        foreach($this->last_list as $t_stamp => $obj){
          if($obj['dn'] == $entry){
            $found = true;
            break;
          }
        }

        if($found){
          $this->del_dn 	= $entry;
          $smarty= get_smarty();
          $smarty->assign("intro", sprintf(_("You're about to delete the snapshot '%s'."), LDAP::fix($this->del_dn)));
          return($smarty->fetch (get_template_path('remove.tpl')));
        }
      }
    }

    /* Remove snapshot */
    if(isset($_POST['delete_confirm']) && !empty($this->del_dn)){
      $this->remove_snapshot($this->del_dn);
      $this->del_dn = "";
		}

		/* We must restore a snapshot */

		if($this->display_restore_dialog){

			/* Should we only display all snapshots of already deleted objects 
                or the snapshots for the given object dn */
			$res = array();
			$tmp = array();
			if($this->display_all_removed_objects){
				if(count($this->snap_shot_bases)){
					foreach($this->snap_shot_bases as $dn){
						$tmp = array_merge($tmp,$this->getAllDeletedSnapshots($dn,true));
					}
				}else{
					$tmp = $this->getAllDeletedSnapshots($this->snap_shot_bases,true);
				}
			}else{
				$tmp = $this->Available_SnapsShots($this->dn,true);
			}

			$DivListSnapShots = new divSelectBox("SnapShotRestore");
			$DivListSnapShots->SetHeight(180);

			$list_of_elements = array();			
			
			/* Walk through all entries and setup the display text */
			foreach($tmp as $key => $entry){
    
        /* Check permissions */
        $TimeStamp = $entry['gosaSnapshotTimestamp'][0];
        $list_of_elements[$TimeStamp] = $entry;
			}

			/* Sort generated list */
			krsort($list_of_elements);
	
			/* Add Elements to divlist */	
      $this->last_list = $list_of_elements;
			foreach($list_of_elements as $entry){

        $actions= "<input type='image' src='images/lists/restore.png' name='RestoreSnapShot_%KEY' 
          class='center' title='"._("Restore snapshot")."'>&nbsp;";
        $actions.= "<input type='image' src='images/lists/trash.png' name='RemoveSnapShot_%KEY' 
          class='center' title='"._("Remove snapshot")."'>&nbsp;";

				$time_stamp 	= date(_("Y-m-d, H:i:s"),preg_replace("/\-.*$/","",$entry['gosaSnapshotTimestamp'][0]));
				$display_data 	= $entry['description'][0];

				if($this->display_all_removed_objects){
					$display_data.= " - ".$entry['gosaSnapshotDN'][0];
				}

				$field0 = array("string"=> $time_stamp , "attach"=> "style='vertical-align:top;width:120px;'");
				$field1 = array("string"=> htmlentities (utf8_decode(LDAP::fix($display_data))), "attach"=> "");
				$field2 = array("string"=> str_replace("%KEY",base64_encode($entry['dn']),$actions) , 
								"attach"=> "style='border-right:0px;vertical-align:top;width:40px;text-align:right;'");
				$DivListSnapShots->AddEntry(array($field0,$field1,$field2));
			}		

			$smarty->assign("SnapShotDivlist",$DivListSnapShots->DrawList());	
			$smarty->assign("CountSnapShots",count($list_of_elements));
		}

		$smarty->assign("restore_deleted",$this->display_all_removed_objects);
		$smarty->assign("RestoreMode",$this->display_restore_dialog);
		$smarty->assign("CurrentDate",date(_("Y-m-d, H:i:s")));
		$smarty->assign("CurrentDN",LDAP::fix($this->dn));
		$smarty->assign("CurrentDescription",$this->CurrentDescription);
		return($smarty->fetch(get_template_path("snapshotdialog.tpl")));
	}


	function check()
	{
		$message = plugin::check();
		if(!$this->display_restore_dialog){
			if(empty($this->CurrentDescription)){
        $message[]= msgPool::invalid(_("Description"));
			}
		}
		return($message);
	}


	function save_object()
	{	
		plugin::save_object();
		foreach($this->attributes as $name){
			if(isset($_POST[$name])){
				$this->$name = stripslashes($_POST[$name]);
			}
		}
	}
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
