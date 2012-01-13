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

class ogrouptabs extends tabs
{
  var $base= "";
  var $acl_category; 
 
  function reload($dd)
  {
    $objects      = preg_replace('/[\[\]]/', '', $dd);

    /* Check if we have a group with a set different mixed objects.
     */
    $mixed_type = FALSE;
    for($i = 0 ; $i < (strlen($objects) -1 );$i++){
      $mixed_type |= $objects[$i] != $objects[($i+1)];
    }
 
    /* If there is a phonequeue,
     * but there is no user left with goPhoneAccount ... remove it.
     */
    $usePhoneTab = false;
    if(class_available("phonequeue")){

      foreach($this->by_object['ogroup']->memberList as $dn => $val){
        if(isset($this->by_object['ogroup']->objcache[$dn])){
          $obj = $this->by_object['ogroup']->objcache[$dn];
          if(isset($obj['objectClass'])){
            if(in_array("goFonAccount",$obj['objectClass'])){
              $usePhoneTab = true;
            }
          }
        }
      }
      if((!$usePhoneTab && isset($this->by_object['phonequeue']))||
         (!preg_match("/U/",$objects) && isset($this->by_object['phonequeue']))){
        $this->by_object['phonequeue']->remove_from_parent();
        unset($this->by_object['phonequeue']);
        unset($this->by_name['phonequeue']);
      }
    }

    /* Remove mail group if there is no user anymore 
     */
    if(class_available("mailogroup")){
      if(!preg_match("/U/",$objects) && isset($this->by_object['mailogroup'])){
        $this->by_object['mailogroup']->remove_from_parent();
        unset($this->by_object['mailogroup']);
        unset($this->by_name['mailogroup']);
      }
    }

    /* Remove terminal group, if theres no terminal left in the object list 
     */
    if(class_available("termgroup")){
      if(($mixed_type && isset($this->by_object['termgroup'])) ||
          !preg_match("/T/",$objects) && !preg_match("/W/",$objects) && isset($this->by_object['termgroup'])){
        $this->by_object['termgroup']->remove_from_parent();
        unset($this->by_object['termgroup']);
        unset($this->by_name['termgroup']);
      }
    }
    if(class_available("termservice")){
      if(($mixed_type && isset($this->by_object['termservice'])) ||
          !preg_match("/T/",$objects) &&(isset($this->by_object['termservice']))){
        $this->by_object['termservice']->remove_from_parent();
        unset($this->by_object['termservice']);
        unset($this->by_name['termservice']);
      }
    }
    if(class_available("termstartup")){
      if(($mixed_type && isset($this->by_object['termstartup'])) ||
          !preg_match("/T/",$objects)&&(isset($this->by_object['termstartup']))){
        $this->by_object['termstartup']->remove_from_parent();
        unset($this->by_object['termstartup']);
        unset($this->by_name['termstartup']);
      }
    }

    /* Remove ws tabs, if theres no ws left in the object list */
    if(class_available("workservice")){
      if(($mixed_type && isset($this->by_object['workservice'])) ||
          (!preg_match("/W/",$objects))&&(isset($this->by_object['workservice']))){
        $this->by_object['workservice']->remove_from_parent();
        unset($this->by_object['workservice']);
        unset($this->by_name['workservice']);
      }
    }
    if(class_available("workstartup")){
      if(($mixed_type && isset($this->by_object['workstartup'])) ||
          (!preg_match("/S/",$objects) && !preg_match("/W/",$objects))&&(isset($this->by_object['workstartup']))){
        $this->by_object['workstartup']->remove_from_parent();
        unset($this->by_object['workstartup']);
        unset($this->by_name['workstartup']);
        if (isset($this->by_object['faiSummary'])){
          $this->by_object['faiSummary']->remove_from_parent();
          unset($this->by_object['faiSummary']);
          unset($this->by_name['faiSummary']);
        }
      }
    }
  
    /* Create goPhoneAccount if theres an user with goPhoneAccount
     * but only if there is currently no queue enabled.
     */
    if(class_available("phonequeue")){
      if(!isset($this->by_object['phonequeue'])){
        foreach($this->by_object['ogroup']->memberList as $dn => $val){
          if(isset($this->by_object['ogroup']->objcache[$dn])){
            $obj = $this->by_object['ogroup']->objcache[$dn];
            if(isset($obj['objectClass'])){
              if(in_array("goFonAccount",$obj['objectClass'])){
                $this->by_name['phonequeue']= _("Phone queue");
                $this->by_object['phonequeue']= new phonequeue($this->config, $this->dn);
                $this->by_object['phonequeue']->parent= &$this;
                break;
              }
            }
          }
        }
      }
    }

    /* Add mail group tab , if there is curerntly no mail tab defined */ 
    if(class_available("mailogroup")){
      if((preg_match("/U/",$objects))&&(!isset($this->by_object['mailogroup']))){
        if ($this->config->get_cfg_value("mailmethod") == "kolab"){
          $this->by_name['mailogroup']= _("Mail");
          $this->by_object['mailogroup']= new mailogroup($this->config, $this->dn);
          $this->by_object['mailogroup']->parent= &$this;
        }
      }
    }

    /* Add Terminal tab */
    if(class_available("termgroup")){
      if(!$mixed_type && 
          ((preg_match("/T/",$objects)) || (preg_match("/W/",$objects)))&&(!isset($this->by_object['termgroup']))){
        if(!isset($this->by_object['termgroup'])){
          $this->by_name['termgroup']= _("Systems");
          $this->by_object['termgroup']= new termgroup($this->config, $this->dn);
          $this->by_object['termgroup']->inheritTimeServer = false;
          $this->by_object['termgroup']->parent= &$this;
        }
      }
    }
    if(class_available("termstartup")){
      if(!$mixed_type &&
          preg_match("/T/",$objects) &&(!isset($this->by_object['termstartup']))){
        if(!isset($this->by_object['termstartup'])){
          $this->by_name['termstartup']= _("Startup");
          $this->by_object['termstartup']= new termstartup($this->config, $this->dn,$this->by_object['ogroup']);
	  $this->by_object['termstartup']->parent= &$this;
	  $this->by_object['termstartup']->acl = "#all#";
        }
      }
    }
    if(!$mixed_type &&
        class_available("termservice")){
      if(preg_match("/T/",$objects) &&(!isset($this->by_object['termservice']))){
        if(!isset($this->by_object['termservice'])){
          $this->by_name['termservice']= _("Devices");
          $this->by_object['termservice']= new termservice($this->config, $this->dn,$this->by_object['ogroup']);
	  $this->by_object['termservice']->parent= &$this;
	  $this->by_object['termservice']->acl = "#all#";
        }
      }
    }

    /* Add Workstation tabs */
    if(class_available("workstartup")){
      if(!$mixed_type &&
          (preg_match("/S/",$objects) || preg_match("/W/",$objects))&&(!isset($this->by_object['workstartup']))){
        $this->by_name['workstartup']= _("Startup");
        $this->by_object['workstartup']= new workstartup($this->config, $this->dn);
        $this->by_object['workstartup']->parent= &$this;
        $this->by_object['workstartup']->acl = "#all#";
      }
    }
    if(!$mixed_type &&
        class_available("workservice")){
      if((preg_match("/W/",$objects))&&(!isset($this->by_object['workservice']))){
        $this->by_name['workservice']= _("Devices");
        $this->by_object['workservice']= new workservice($this->config, $this->dn);
        $this->by_object['workservice']->inheritTimeServer = false;
        $this->by_object['workservice']->parent= &$this;
        $this->by_object['workservice']->acl = "#all#";
      }
    }
    if(class_available("faiSummary")){
      if(!$mixed_type &&
          (preg_match("/S/",$objects) || preg_match("/W/",$objects))&&(!isset($this->by_object['faiSummary']))){
        $this->by_name['faiSummary']= _("FAI summary");
        $this->by_object['faiSummary']= new faiSummaryTab($this->config, $this->dn);
        $this->by_object['faiSummary']->parent= &$this;
      }
    }

    /* Add environment tab if user or group is member in this object group*/
    if(class_available("environment")){
      if((preg_match("/G/",$objects) || preg_match("/U/",$objects)) && !isset($this->by_name['environment'])){
        $this->by_name['environment']= _("Environment");
        $this->by_object['environment']= new environment($this->config, $this->dn);
        $this->by_object['environment']->acl = "#all#";
        $this->by_object['environment']->parent= &$this;
      }
    }

    /* Remove environment tab if not required any longer */
    if(class_available("environment")){
      if(!preg_match("/G/",$objects) && !preg_match("/U/",$objects) && isset($this->by_name['environment'])){
        $this->by_object['environment']->remove_from_parent();
        unset($this->by_name['environment']);
        unset($this->by_object['environment']);
      }
    }

    /* Add application tab if user or group is member in this object group*/
    if(class_available("appgroup")){
      if((preg_match("/G/",$objects) || preg_match("/U/",$objects)) && !isset($this->by_name['appgroup'])){
        $this->by_name['appgroup']= _("Applications");
        $this->by_object['appgroup']= new appgroup($this->config, $this->dn);
        $this->by_object['appgroup']->acl = "#all#";
        $this->by_object['appgroup']->parent= &$this;
      }
    }

    /* Remove application tab if not required any longer */
    if(class_available("appgroup")){
      if(!preg_match("/G/",$objects) && !preg_match("/U/",$objects) && isset($this->by_name['appgroup'])){
        $this->by_object['appgroup']->remove_from_parent();
        unset($this->by_name['appgroup']);
        unset($this->by_object['appgroup']);
      }
    }

    /* Move reference tab to second position from right */
    if(class_available("acl")){
      if(isset($this->by_name['acl'])){
        $tmp = $this->by_name['acl'];
        unset($this->by_name['acl']);
        $this->by_name['acl'] = $tmp;
      }

      /* Move reference tab to last position*/
      if(class_available("reference")){
        if(isset($this->by_name['reference'])){
          $tmp = $this->by_name['reference'];
          unset($this->by_name['reference']);
          $this->by_name['reference'] = $tmp;
        }
      }

      /* Reset acls */
      if($this->dn == "new"){
        $this->set_acl_base($this->base);
      }else{
        $this->set_acl_base($this->dn);
      }
      foreach($this->by_object as $name => $obj){
        $this->by_object[$name]->set_acl_category($this->acl_category);
      }
    }
  }

  function execute(){
    $str = "";
    $this->by_object['ogroup']->AddDelMembership();
    $this->reload($this->by_object['ogroup']->gosaGroupObjects);
    $str .= tabs::execute();
    return ( $str);
  }  

  function ogrouptabs($config, $data, $dn,$category ="ogroups",$hide_refs = FALSE, $hide_acls = FALSE)
  {

    tabs::tabs($config, $data, $dn, $category,$hide_refs, $hide_acls);
    $this->base= $this->by_object['ogroup']->base;
    $this->acl_category = $category;

    /* Add references/acls/snapshots */
    $this->reload($this->by_object['ogroup']->gosaGroupObjects);
    $this->addSpecialTabs();
  }


  function check($ignore_account= FALSE)
  {
    return (tabs::check(FALSE));
  }


  function save_object($save_current= FALSE)
  {
    tabs::save_object($save_current);

    /* Update reference, transfer variables */
    $baseobject= $this->by_object['ogroup'];
    foreach ($this->by_object as $name => $obj){

      /* Don't touch base object */
      if ($name != 'ogroup'){
        $obj->parent    = &$this;
        $obj->uid       = $baseobject->uid;
        $obj->cn        = $baseobject->cn;
        $obj->sn        = $baseobject->uid;
        $obj->givenName = $baseobject->uid;
        $this->by_object[$name]= $obj;
      }

      /* Update parent in base object */
      $this->by_object['ogroup']->parent= &$this;
    }
  }


  function save($ignore_account= FALSE)
  {
    $baseobject= $this->by_object['ogroup'];

    /* Check for new 'dn', in order to propagate the
       'dn' to all plugins */
    $new_dn= 'cn='.$baseobject->cn.','.get_ou('ogroupRDN').$baseobject->base;

    /* Move group? */
    if (LDAP::fix($this->dn) != LDAP::fix($new_dn)){

      /* Write entry on new 'dn' */
      if ($this->dn != "new"){
        $baseobject->move($this->dn, $new_dn);
        $this->by_object['ogroup']= $baseobject;
      }

      /* Happen to use the new one */
      $this->dn= $new_dn;
    }

    if ($this->dn == "new"){
      $this->dn= 'cn='.$baseobject->cn.','.get_ou('ogroupRDN').$baseobject->base;
    }

    tabs::save();
  }

  function getCopyDialog()
  {
    $this->reload($this->by_object['ogroup']->gosaGroupObjects); 
    return(tabs::getCopyDialog());
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
