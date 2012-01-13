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

class userinfo
{
  var $dn;
  var $ip;
  var $username;
  var $cn;
  var $uid;
  var $restrictions= array();
  var $gidNumber= -1;
  var $language= "";
  var $config;
  var $gosaUnitTag= "";
  var $subtreeACL= array();
  var $ACL= array();
  var $ocMapping= array();
  var $groups= array();
  var $result_cache =array();
  var $ignoreACL = FALSE;
  var $ACLperPath = array();
  var $ACLperPath_usesFilter = array();

  /* get acl's an put them into the userinfo object
     attr subtreeACL (userdn:components, userdn:component1#sub1#sub2,component2,...) */
  function userinfo(&$config, $userdn){
    $this->config= &$config;
    $ldap= $this->config->get_ldap_link();
    $ldap->cat($userdn,array('sn', 'givenName', 'uid', 'gidNumber', 'preferredLanguage', 'gosaUnitTag', 'gosaLoginRestriction'));
    $attrs= $ldap->fetch();

    if (isset($attrs['givenName'][0]) && isset($attrs['sn'][0])){
      $this->cn= $attrs['givenName'][0]." ".$attrs['sn'][0];
    } else {
      $this->cn= $attrs['uid'][0];
    }
    if (isset($attrs['gidNumber'][0])){
      $this->gidNumber= $attrs['gidNumber'][0];
    }

    /* Restrictions? */
    if (isset($attrs['gosaLoginRestriction'])){
      $this->restrictions= $attrs['gosaLoginRestriction'];
      unset($this->restrictions['count']);
    }

    /* Assign user language */
    if (isset($attrs['preferredLanguage'][0])){
      $this->language= $attrs['preferredLanguage'][0];
    }

    if (isset($attrs['gosaUnitTag'][0])){
      $this->gosaUnitTag= $attrs['gosaUnitTag'][0];
    }

    $this->dn= $userdn;
    $this->uid= $attrs['uid'][0];
    $this->ip= $_SERVER['REMOTE_ADDR'];

    $this->ignoreACL = ($this->config->get_cfg_value("ignoreAcl") == $this->dn);

    /* Initialize ACL_CACHE */
    $this->reset_acl_cache();
  }


  public function reset_acl_cache()
  {
    /* Initialize ACL_CACHE */
    session::global_set('ACL_CACHE',array());
  }

  function loadACL()
  {
    $this->ACL= array();    
    $this->groups= array();    
    $this->result_cache =array();
    $this->reset_acl_cache();
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);

    /* Get member groups... */
    $ldap->search("(&(objectClass=posixGroup)(memberUid=".$this->uid."))", array('dn'));
    while ($attrs= $ldap->fetch()){
      $this->groups[$attrs['dn']]= $attrs['dn'];
    }

    /* Crawl through ACLs and move relevant to the tree */
    $ldap->search("(objectClass=gosaACL)", array('dn', 'gosaAclEntry'));
    $aclp= array();
    $aclc= array();
    while ($attrs= $ldap->fetch()){

      /* Insert links in ACL array */
      $aclp[$attrs['dn']]= substr_count($attrs['dn'], ',');
      $aclc[$attrs['dn']]= array();
      $ol= array();
      for($i= 0; $i<$attrs['gosaAclEntry']['count']; $i++){
        $ol= array_merge($ol, @acl::explodeAcl($attrs['gosaAclEntry'][$i]));
      }
      $aclc[$attrs['dn']]= $ol;
    }

    /* Resolve roles here. 
     */
    foreach($aclc as $dn => $data){
      foreach($data as $prio => $aclc_value)  {
        if($aclc_value['type'] == "role"){

          unset($aclc[$dn][$prio]);

          $ldap->cat($aclc_value['acl'],array("gosaAclTemplate"));
          $attrs = $ldap->fetch();

          if(isset($attrs['gosaAclTemplate'])){
            for($i= 0; $i<$attrs['gosaAclTemplate']['count']; $i++){
              $tmp = @acl::explodeAcl($attrs['gosaAclTemplate'][$i]);  

              foreach($tmp as $new_acl){

                /* Keep non role attributes here! */
                $new_acl['filter'] = $aclc_value['filter'];
                $new_acl['members'] = $aclc_value['members'];
                $aclc[$dn][] =$new_acl;
              }
            }      
          }
        }
      }
    }

    /* ACL's read, sort for tree depth */
    asort($aclp);

    /* Sort in tree order */
    foreach ($aclp as $dn => $acl){
      /* Check if we need to keep this ACL */
      foreach($aclc[$dn] as $idx => $type){
        $interresting= FALSE;
        
        /* No members? This ACL rule is deactivated ... */
        if (!count($type['members'])){
          $interresting= FALSE; 
        } else {

          /* Inspect members... */
          foreach ($type['members'] as $grp => $grpdsc){

            /* Some group inside the members that is relevant for us? */
            if (in_array_ics(@LDAP::convert(preg_replace('/^G:/', '', $grp)), $this->groups)){
              $interresting= TRUE;
            }

            /* User inside the members? */
            if (mb_strtoupper(preg_replace('/^U:/', '', $grp)) == mb_strtoupper($this->dn)){
              $interresting= TRUE;
            }

            /* Wildcard? */
            if (preg_match('/^G:\*/',  $grp)){
              $interresting= TRUE;
            }
          }
        }

        if ($interresting){
          if (!isset($this->ACL[$dn])){
            $this->ACL[$dn]= array();
          }
          $this->ACL[$dn][$idx]= $type;
        }
      }
    }

    /* Create an array which represenet all relevant permissions settings 
        per dn.

      The array will look like this:
      
      .     ['ou=base']        ['ou=base']          = array(ACLs);
      .     
      .     ['ou=dep1,ou=base']['ou=dep1,ou=base']  = array(ACLs);
      .                        ['ou=base']          = array(ACLs);


      For object located in 'ou=dep1,ou=base' we have to both ACLs,
       for objects in 'ou=base' we only have to apply on ACL.
     */
    $without_self_acl = $all_acl = array();
    foreach($this->ACL as $dn => $acl){
      $sdn =$dn;
      $first= TRUE; // Run at least once 
      while(strpos($dn,",") !== FALSE || $first){
        $first = FALSE;
        if(isset($this->ACL[$dn])){
          $all_acl[$sdn][$dn] = $this->ACL[$dn];
          $without_self_acl[$sdn][$dn] = $this->ACL[$dn]; 
          foreach($without_self_acl[$sdn][$dn] as $acl_id => $acl_set){
  
            /* Remember which ACL set has speicial user filter 
             */
            if(isset($acl_set['filter']{1})){
              $this->ACLperPath_usesFilter[$sdn] = TRUE;
            }
          
            /* Remove all acl entries which are especially for the current user (self acl)
             */
            foreach($acl_set['acl'] as $object => $object_acls){
              if(isset($object_acls[0]) && strpos($object_acls[0],"s") !== FALSE){
                unset($without_self_acl[$sdn][$dn][$acl_id]['acl'][$object]);
              }
            }
          }
        }
        $dn = preg_replace("/^[^,]*+,/","",$dn);
      }
    } 
    $this->ACLperPath =$without_self_acl;

    /* Append Self entry */
    $dn = $this->dn;
    while(strpos($dn,",") && !isset($all_acl[$dn])){
      $dn = preg_replace("/^[^,]*+,/","",$dn);
    }
    if(isset($all_acl[$dn])){
      $this->ACLperPath[$this->dn] = $all_acl[$dn];
    }
  }


  /* Returns an array containing all target objects we've permssions on.
   */
  function get_acl_target_objects()
  {
    return(array_keys($this->ACLperPath));
  }
  

  function get_category_permissions($dn, $category, $any_acl = FALSE)
  {
    return(@$this->get_permissions($dn,$category.'/0',""));
  }

  
  /*! \brief Check if the given object (dn) is copyable
      @param  String The object dn 
      @param  String The acl  category (e.g. users) 
      @param  String The acl  class (e.g. user) 
      @return Boolean   TRUE if the given object is copyable else FALSE 
  */
  function is_copyable($dn, $object, $class)
  {
    return(preg_match("/r/",$this->has_complete_category_acls($dn, $object)));
  }


  /*! \brief Check if the given object (dn) is cutable
      @param  String The object dn 
      @param  String The acl  category (e.g. users) 
      @param  String The acl  class (e.g. user) 
      @return Boolean   TRUE if the given object is cutable else FALSE 
  */
  function is_cutable($dn, $object, $class)
  {
    $remove = preg_match("/d/",$this->get_permissions($dn,$object."/".$class));
    $read   = preg_match("/r/",$this->has_complete_category_acls($dn, $object));
    return($remove && $read);
  }


  /*! \brief  Checks if we are allowed to paste an object to the given destination ($dn)
      @param  String The destination dn 
      @param  String The acl  category (e.g. users) 
      @param  String The acl  class (e.g. user) 
      @return Boolean   TRUE if we are allowed to paste an object.
  */
  function is_pasteable($dn, $object)
  {
    return(preg_match("/w/",$this->has_complete_category_acls($dn, $object)));
  }


  /*! \brief  Checks if we are allowed to restore a snapshot for the given dn.
      @param  String The destination dn 
      @param  String The acl  category (e.g. users) 
      @return Boolean   TRUE if we are allowed to restore a snapshot.
  */
  function allow_snapshot_restore($dn, $object)
  {
    if(!is_array($object)){
      $object = array($object);
    }
    $r = $w = TRUE;
    foreach($object as $category){
      $w &= preg_match("/w/",$this->has_complete_category_acls($dn, $category));
      $r &= preg_match("/r/",$this->has_complete_category_acls($dn, $category));
    }
    return($r && $w ); 
  }  


  /*! \brief  Checks if we are allowed to create a snapshot of the given dn.
      @param  String The source dn 
      @param  String The acl category (e.g. users) 
      @return Boolean   TRUE if we are allowed to restore a snapshot.
  */
  function allow_snapshot_create($dn, $object)
  {
    if(!is_array($object)){
      $object = array($object);
    }
    $r = TRUE;
    foreach($object as $category){
      $r &= preg_match("/r/",$this->has_complete_category_acls($dn, $category));
    }
    return($r) ; 
  }  


  function get_permissions($dn, $object, $attribute= "", $skip_write= FALSE)
  {
    /* If we are forced to skip ACLs checks for the current user 
        then return all permissions.
     */
    if($this->ignore_acl_for_current_user()){
      if($skip_write){
        return("rcdm");
      }
      return("rwcdm");
    }

    /* Push cache answer? */
    $ACL_CACHE = &session::global_get('ACL_CACHE');
    if (isset($ACL_CACHE["$dn+$object+$attribute"])){
      $ret = $ACL_CACHE["$dn+$object+$attribute"];
      if($skip_write){
        $ret = str_replace(array('w','c','d','m'), '',$ret);
      }
      return($ret);
    }

    /* Detect the set of ACLs we have to check for this object 
     */
    $adn = $dn;
    while(!isset($this->ACLperPath[$adn]) && strpos($adn,",") !== FALSE){
      $adn = preg_replace("/^[^,]*+,/","",$adn);
    }
    if(isset($this->ACLperPath[$adn])){
      $ACL = $this->ACLperPath[$adn];
    }else{
      $ACL_CACHE["$dn+$object+$attribute"] = "";
      return("");
    }
 
    /* If we do not need to respect any user-filter settings 
        we can skip the per object ACL checks.
     */
    $orig_dn= $dn;
    if(!isset($this->ACLperPath_usesFilter[$adn])){
      $dn = $adn;
      if (isset($ACL_CACHE["$dn+$object+$attribute"])){
        $ret = $ACL_CACHE["$dn+$object+$attribute"];
        if(!isset($ACL_CACHE["$orig_dn+$object+$attribute"])){
          $ACL_CACHE["$orig_dn+$object+$attribute"] = $ACL_CACHE["$dn+$object+$attribute"];
        }
        if($skip_write){
          $ret = str_replace('w','',$ret);
        }
        return($ret);
      }
    }
 
    /* Get ldap object, for later filter checks 
     */
    $ldap = $this->config->get_ldap_link();

    $acl= array("r" => "", "w" => "", "c" => "", "d" => "", "m" => "", "a" => "");

    /* Build dn array */
    $path= explode(',', $dn);
    $path= array_reverse($path);

    /* Walk along the path to evaluate the acl */
    $cpath= "";
    foreach ($path as $element){

      /* Clean potential ACLs for each level */
			if(isset($this->config->idepartments[$cpath])){
        $acl= $this->cleanACL($acl);
      }

      if ($cpath == ""){
        $cpath= $element;
      } else {
        $cpath= $element.','.$cpath;
      }

      if (isset($ACL[$cpath])){

        /* Inspect this ACL, place the result into ACL */
        foreach ($ACL[$cpath] as $subacl){

          /* Reset? Just clean the ACL and turn over to the next one... */
          if ($subacl['type'] == 'reset'){
            $acl= $this->cleanACL($acl, TRUE);
            continue;
          }

          if($subacl['type'] == "role") {
            echo "role skipped";
            continue;
          }

          /* With user filter */
          if (isset($subacl['filter']) && !empty($subacl['filter'])){
            $id = $dn."-".$subacl['filter'];
            if(!isset($ACL_CACHE['FILTER'][$id])){
              $ACL_CACHE['FILTER'][$id] = $ldap->object_match_filter($dn,$subacl['filter']);
            }
            if(!$ACL_CACHE['FILTER'][$id]){
              continue;
            }
          }

          /* Self ACLs? 
           */
          if($dn != $this->dn && isset($subacl['acl'][$object][0]) && (strpos($subacl['acl'][$object][0],"s") !== FALSE)){
            continue;
          }

          /* If attribute is "", we want to know, if we've *any* permissions here... 
              Merge global class ACLs [0] with attributes specific ACLs [attribute].
           */
          if ($attribute == "" && isset($subacl['acl'][$object])){
            foreach($subacl['acl'][$object] as $attr => $dummy){
              $acl= $this->mergeACL($acl, $subacl['type'], $subacl['acl'][$object][$attr]);
            }
            continue;
          }

          /* Per attribute ACL? */
          if (isset($subacl['acl'][$object][$attribute])){
            $acl= $this->mergeACL($acl, $subacl['type'], $subacl['acl'][$object][$attribute]);
            continue;
          }

          /* Per object ACL? */
          if (isset($subacl['acl'][$object][0])){
            $acl= $this->mergeACL($acl, $subacl['type'], $subacl['acl'][$object][0]);
            continue;
          }

          /* Global ACL? */
          if (isset($subacl['acl']['all'][0])){
            $acl= $this->mergeACL($acl, $subacl['type'], $subacl['acl']['all'][0]);
            continue;
          }

          /* Category ACLs    (e.g. $object = "user/0")
           */
          if(strstr($object,"/0")){
            $ocs = preg_replace("/\/0$/","",$object);
            if(isset($this->ocMapping[$ocs])){

              /* if $attribute is "", then check every single attribute for this object.
                 if it is 0, then just check the object category ACL.
               */
              if($attribute == ""){    
                foreach($this->ocMapping[$ocs] as $oc){
                  if (isset($subacl['acl'][$ocs.'/'.$oc])){

                      // Skip ACLs wich are defined for ourselfs only - if not checking against ($ui->dn)
                      if(isset($subacl['acl'][$ocs.'/'.$oc][0]) && 
                              $dn != $this->dn && 
                              strpos($subacl['acl'][$ocs.'/'.$oc][0],"s") !== FALSE) continue;

                    foreach($subacl['acl'][$ocs.'/'.$oc] as $attr => $dummy){
                      $acl= $this->mergeACL($acl, $subacl['type'], $subacl['acl'][$ocs.'/'.$oc][$attr]);
                    }
                    continue;
                  }
                }
              }else{
                if(isset($subacl['acl'][$ocs.'/'.$oc][0])){
                  if($dn != $this->dn && strpos($subacl['acl'][$ocs.'/'.$oc][0],"s") !== FALSE) continue;
                  $acl= $this->mergeACL($acl, $subacl['type'], $subacl['acl'][$ocs.'/'.$oc][0]);
                }
              }
            }
            continue;
          }
        }
      }
    }

    /* If the requested ACL is for a container object, then alter 
        ACLs by applying cleanACL a last time.
     */
    if(isset($this->config->idepartments[$dn])){
      $acl = $this->cleanACL($acl);
    }

    /* Assemble string */
    $ret= "";
    foreach ($acl as $key => $value){
      if ($value !== ""){
        $ret.= $key;
      }
    }

    $ACL_CACHE["$dn+$object+$attribute"]= $ret;
    $ACL_CACHE["$orig_dn+$object+$attribute"]= $ret;

    /* Remove write if needed */
    if ($skip_write){
      $ret = str_replace(array('w','c','d','m'), '',$ret);
    }
    return ($ret);
  }


  /* Extract all departments that are accessible (direct or 'on the way' to an
     accessible department) */
  function get_module_departments($module, $skip_self_acls = FALSE )
  {
    /* If we are forced to skip ACLs checks for the current user 
        then return all departments as valid.
     */
    if($this->ignore_acl_for_current_user()){
      return(array_keys($this->config->idepartments));
    }

    /* Use cached results if possilbe */
    $ACL_CACHE = &session::global_get('ACL_CACHE');

    if(!is_array($module)){
      $module = array($module);
    }

    global $plist;
    $res = array();
    foreach($module as $mod){
      if(isset($ACL_CACHE['MODULE_DEPARTMENTS'][$mod])){
        $res = array_merge($res,$ACL_CACHE['MODULE_DEPARTMENTS'][$mod]);
        continue;
      }

      $deps = array();

      /* Search for per object ACLs */
      foreach($this->ACL as $dn => $infos){
        foreach($infos as $info){
          $found = FALSE;
          foreach($info['acl'] as $cat => $data){

            /* Skip self acls? */
            if($skip_self_acls && isset($data['0']) && (strpos($data['0'], "s") !== FALSE)) continue;
            if(preg_match("/^".preg_quote($mod, '/')."/",$cat)){
              $found =TRUE;
              break;
            }
          } 

          if($found && !isset($this->config->idepartments[$dn])){
            while(!isset($this->config->idepartments[$dn]) && strpos($dn, ",")){
              $dn = preg_replace("/^[^,]+,/","",$dn);
            }
            if(isset($this->config->idepartments[$dn])){
              $deps[$dn] = $dn;
            }
          }
        }
      }

      /* For all gosaDepartments */
      foreach ($this->config->departments as $dn){
        if(isset($deps[$dn])) continue;
        $acl = "";
        if(strpos($mod, '/')){
          $acl.=  $this->get_permissions($dn,$mod);
        }else{
          $acl.=  $this->get_category_permissions($dn,$mod,TRUE);
        }
        if(!empty($acl)) {
          $deps[$dn] = $dn;
        }
      }

      $ACL_CACHE['MODULE_DEPARTMENTS'][$mod] = $deps;
      $res = array_merge($res,$deps);
    }

    return (array_values($res));
  }


  function mergeACL($acl, $type, $newACL)
  {
		$at= array("psub" => "p", "sub" => "s", "one" => "1");

    if (strpos($newACL, 'w') !== FALSE && strpos($newACL, 'r') === FALSE){
      $newACL .= "r";
    }

		/* Ignore invalid characters */
		$newACL= preg_replace('/[^rwcdm]/', '', $newACL);

    foreach(str_split($newACL) as $char){

      /* Skip "self" ACLs without combination of rwcdm, they have no effect.
         -self flag without read/write/create/...
       */
      if(empty($char)) continue;

      /* Skip permanent and subtree entries */
      if (preg_match('/[sp]/', $acl[$char])){
        continue;
      }

			if ($type == "base" && $acl[$char] != 1) {
				$acl[$char]= 0;
			} else {
        $acl[$char]= $at[$type];
			}
    }

    return ($acl);
  }


  function cleanACL($acl, $reset= FALSE)
  {
    foreach ($acl as $key => $value){

      /* Continue, if value is empty or permanent */
      if ($value == "" || $value == "p") {
	    continue;
      }

      /* Reset removes everything but 'p' */
      if ($reset && $value != 'p'){
        $acl[$key]= "";
        continue;
      }

      /* Decrease tree level */
      if (is_int($value)){
        if ($value){
          $acl[$key]--;
        } else {
          $acl[$key]= "";
        }
      }
    }

    return ($acl);
  }


  /* #FIXME This could be logical wrong or could be optimized in the future
     Return combined acls for a given category. 
     All acls will be combined like boolean AND 
      As example ('rwcdm' + 'rcd' + 'wrm'= 'r') 
    
     Results will be cached in $this->result_cache.
      $this->result_cache will be resetted if load_acls is called.
  */
  function has_complete_category_acls($dn,$category)
  {
    $acl    = "rwcdm";
    $types  = "rwcdm";

    if(!is_string($category)){
      trigger_error("category must be string");   
      $acl = "";
    }else{
      if(!isset($this->result_cache['has_complete_category_acls'][$dn][$category]))   {
        if (isset($this->ocMapping[$category])){
          foreach($this->ocMapping[$category] as $oc){

            /* Skip objectClass '0' (e.g. users/0) get_permissions will ever return '' ??  */
            if($oc == "0") continue;
            $tmp =  $this->get_permissions($dn, $category."/".$oc);
            for($i = 0, $l= strlen($types); $i < $l; $i++) {
              if(!preg_match("/".$types[$i]."/",$tmp)){ 
                $acl = preg_replace("/".$types[$i]."/","",$acl);
              }
            }
          }
        }else{
          $acl = "";
        }
        $this->result_cache['has_complete_category_acls'][$dn][$category] = $acl;
      }else{
        $acl = $this->result_cache['has_complete_category_acls'][$dn][$category];
      }
    }
    return($acl);
  }

 
  /*! \brief  Returns TRUE if the current user is configured in IGNORE_ACL=".." in your fusiondirectory.conf 
      @param  Return Boolean TRUE if we have to skip ACL checks else FALSE.
   */ 
  function ignore_acl_for_current_user()
  {
    return($this->ignoreACL);
  }


  function loginAllowed()
  {
    // Need to check restrictions?
    if (count($this->restrictions)){

      // We have restrictions but cannot check them
      if (!isset($_SERVER['REMOTE_ADDR'])){
        return false;
      }

      // Move to binary...
      $source= $_SERVER['REMOTE_ADDR'];
      foreach ($this->restrictions as $restriction) {

        // Single IP
        if (preg_match('/^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$/', $restriction)) {
           if ($source == $restriction){
             return true;
           }
        }

        // Match with short netmask
        if (preg_match('/^([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)\/([0-9]+)$/', $restriction, $matches)) {
          if (isIpInNet($source, $matches[1], long2ip(~(pow(2, (32-$matches[2]))-1)))) {
            return true;
          }
        }

        // Match with long netmask
        if (preg_match('/^([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)\/([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)$/', $restriction, $matches)) {
          if (isIpInNet($source, $matches[1], $matches[2])) {
            return true;
          }
        }

      }

      return false;
    }

    return true;
  }

}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
