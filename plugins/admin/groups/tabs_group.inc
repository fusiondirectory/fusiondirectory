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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class grouptabs extends tabs
{

	function grouptabs($config, $data, $dn, $cat = "", $hide_refs = FALSE, $hide_acls = FALSE)
	{
		tabs::tabs($config, $data, $dn,"groups", $hide_refs, $hide_acls);
		$this->addSpecialTabs();
	}

	function save_object($save_current= FALSE)
	{
		tabs::save_object($save_current);

		/* Update reference, transfer variables */
		$baseobject= $this->by_object['group'];
		foreach ($this->by_object as $name => $obj){
			/* Don't touch base object */
			if ($name != 'group'){
				$obj->parent= &$this;
				$obj->cn= $baseobject->cn;
				$this->by_object[$name]= $obj;
			}
		}
	}

	function delete()
	{
		/* Put baseobjects 'cn' to mailobjects 'uid' */
		$baseobject= $this->by_object['group'];
		if (isset($this->by_object['mailgroup'])){
			$this->by_object['mailgroup']->uid= $baseobject->cn;
		}
		tabs::delete();
	}

  function save($ignore_account= FALSE)
	{
		$baseobject= $this->by_object['group'];

		/* Check for new 'dn', in order to propagate the
		   'dn' to all plugins */
		$new_dn= 'cn='.$baseobject->cn.','.get_groups_ou().$baseobject->base;

		/* Put baseobjects 'cn' to mailobjects 'uid' */
		if (isset($this->by_object['mailgroup'])){
			$this->by_object['mailgroup']->uid= $baseobject->cn;
		}


		/* Update reference, transfer variables */
		foreach ($this->by_object as $name => $obj){
			/* Transfer attributes for mailgroup account */
			if ($name == 'mailgroup'){
				$this->by_object['mailgroup']->members= $baseobject->memberUid;;
			}
		}

		/* Move group? */
		if ($this->dn != $new_dn){

			/* Write entry on new 'dn' */
			if ($this->dn != "new"){
				$baseobject->update_acls($this->dn,$new_dn);
				$baseobject->move($this->dn, $new_dn);
				$this->by_object['group']= $baseobject;
			}

			/* Happen to use the new one */
			$this->dn= $new_dn;
		}

		$ret= tabs::save();

		return $ret;
	}

	function saveCopyDialog()
	{
		tabs::saveCopyDialog();
		
		/* Update reference, transfer variables */
		$baseobject= $this->by_object['group'];
		foreach ($this->by_object as $name => $obj){
			/* Don't touch base object */
			if ($name != 'group'){
				$obj->parent= &$this;
				$obj->cn= $baseobject->cn;
				$this->by_object[$name]= $obj;
			}
		}
	}
	
}

?>
