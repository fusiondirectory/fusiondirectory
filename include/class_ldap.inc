<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2003 Alejandro Escanero Blanco <aescanero@chaosdimension.org>
  Copyright (C) 1998  Eric Kilfoil <eric@ipass.net>
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

define("ALREADY_EXISTING_ENTRY",-10001);
define("UNKNOWN_TOKEN_IN_LDIF_FILE",-10002);
define("NO_FILE_UPLOADED",10003);
define("INSERT_OK",10000);
define("SPECIALS_OVERRIDE", TRUE);

class LDAP{

  var $hascon   =false;
  var $reconnect=false;
  var $tls      = false;
  var $cid;
  var $hasres   = array();
  var $sr       = array();
  var $re       = array();
  var $basedn   ="";
  var $start    = array(); // 0 if we are fetching the first entry, otherwise 1
  var $error    = ""; // Any error messages to be returned can be put here
  var $srp      = 0;
  var $objectClasses = array(); // Information read from slapd.oc.conf
  var $binddn   = "";
  var $bindpw   = "";
  var $hostname = "";
  var $follow_referral = FALSE;
  var $referrals= array();
  var $max_ldap_query_time = 0;   // 0, empty or negative values will disable this check 

  function LDAP($binddn,$bindpw, $hostname, $follow_referral= FALSE, $tls= FALSE)
  {
    global $config;
    $this->follow_referral= $follow_referral;
    $this->tls=$tls;
    $this->binddn=LDAP::convert($binddn);

    $this->bindpw=$bindpw;
    $this->hostname=$hostname;

    /* Check if MAX_LDAP_QUERY_TIME is defined */ 
    if(is_object($config) && $config->get_cfg_value("ldapMaxQueryTime") != ""){
      $str = $config->get_cfg_value("ldapMaxQueryTime");
      $this->max_ldap_query_time = (float)($str);
    }

    $this->connect();
  }


  function getSearchResource()
  {
    $this->sr[$this->srp]= NULL;
    $this->start[$this->srp]= 0;
    $this->hasres[$this->srp]= false;
    return $this->srp++;
  }


  /* Function to replace all problematic characters inside a DN by \001XX, where
     \001 is decoded to chr(1) [ctrl+a]. It is not impossible, but very unlikely
     that this character is inside a DN.

     Currently used codes:
     ,   => CO
     \2C => CO
     (   => OB
     )   => CB
     /   => SL                                                                  
     \22 => DQ                                                                  */
  static function convert($dn)
  {
    if (SPECIALS_OVERRIDE == TRUE){
      $tmp= preg_replace(array("/\\\\,/", "/\\\\2C/", "/\(/", "/\)/", "/\//", "/\\\\22/", '/\\\\"/'),
          array("\001CO", "\001CO", "\001OB", "\001CB", "\001SL", "\001DQ", "\001DQ"),
          $dn);
      return (preg_replace('/,\s+/', ',', $tmp));
    } else {
      return ($dn);
    }
  }


  /* Function to fix all problematic characters inside a DN by replacing \001XX
     codes to their original values. See "convert" for mor information. 
     ',' characters are always expanded to \, (not \2C), since all tested LDAP
     servers seem to take it the correct way.                                  */
  static function fix($dn)
  {
    if (SPECIALS_OVERRIDE == TRUE){
      return (preg_replace(array("/\001CO/", "/\001OB/", "/\001CB/", "/\001SL/", "/\001DQ/"),
            array("\,", "(", ")", "/", '\"'),
            $dn));
    } else {
      return ($dn);
    }
  }

  /* Function to fix problematic characters in DN's that are used for search
     requests. I.e. member=....                                               */
  static function prepare4filter($dn)
  {
    $fixed= normalizeLdap(str_replace('\\\\', '\\\\\\', LDAP::fix($dn)));
    return str_replace('\\,', '\\\\,', $fixed);
  }


  function connect()
  {
    $this->hascon=false;
    $this->reconnect=false;
    if ($this->cid= @ldap_connect($this->hostname)) {
      @ldap_set_option($this->cid, LDAP_OPT_PROTOCOL_VERSION, 3);
      if (function_exists("ldap_set_rebind_proc") && $this->follow_referral) {
        @ldap_set_option($this->cid, LDAP_OPT_REFERRALS, 1);
        @ldap_set_rebind_proc($this->cid, array(&$this, "rebind"));
      }
      if (function_exists("ldap_start_tls") && $this->tls){
        @ldap_start_tls($this->cid);
      }

      $this->error = "No Error";
      if ($bid = @ldap_bind($this->cid, LDAP::fix($this->binddn), $this->bindpw)) {
        $this->error = "Success";
        $this->hascon=true;
      } else {
        if ($this->reconnect){
          if ($this->error != "Success"){
            $this->error = "Could not rebind to " . $this->binddn;
          }
        } else {
          $this->error = "Could not bind to " . $this->binddn;
        }
      }
    } else {
      $this->error = "Could not connect to LDAP server";
    }
  }

  function rebind($ldap, $referral)
  {
    $credentials= $this->get_credentials($referral);
    if (@ldap_bind($ldap, LDAP::fix($credentials['ADMINDN']), $credentials['ADMINPASSWORD'])) {
      $this->error = "Success";
      $this->hascon=true;
      $this->reconnect= true;
      return (0);
    } else {
      $this->error = "Could not bind to " . $credentials['ADMINDN'];
      return NULL;
    }
  }

  function reconnect()
  {
    if ($this->reconnect){
      @ldap_unbind($this->cid);
      $this->cid = NULL;
    }
  }

  function unbind()
  {
    @ldap_unbind($this->cid);
    $this->cid = NULL;
  }

  function disconnect()
  {
    if($this->hascon){
      @ldap_close($this->cid);
      $this->hascon=false;
    }
  }

  function cd($dir)
  {
    if ($dir == ".."){
      $this->basedn = $this->getParentDir();
    } else {
      $this->basedn = LDAP::convert($dir);
    }
  }

  function getParentDir($basedn = "")
  {
    if ($basedn==""){
      $basedn = $this->basedn;
    } else {
      $basedn = LDAP::convert($basedn);
    }
    return(preg_replace("/[^,]*[,]*[ ]*(.*)/", "$1", $basedn));
  }

  
  function search($srp, $filter, $attrs= array())
  {
    if($this->hascon){
      if ($this->reconnect) $this->connect();

      $start = microtime(true);
      $this->clearResult($srp);
      $this->sr[$srp] = @ldap_search($this->cid, LDAP::fix($this->basedn), $filter, $attrs);
      $this->error = @ldap_error($this->cid);
      $this->resetResult($srp);
      $this->hasres[$srp]=true;
   
      /* Check if query took longer as specified in max_ldap_query_time */
      if($this->max_ldap_query_time){
        $diff = microtime(true) - $start;
        if($diff > $this->max_ldap_query_time){
          msg_dialog::display(_("Performance warning"), sprintf(_("LDAP performance is poor: last query took about %.2fs!"), $diff), WARNING_DIALOG);
        }
      }

      $this->log("LDAP operation: time=".(microtime(true)-$start)." operation=search('".LDAP::fix($this->basedn)."', '$filter')");
      return($this->sr[$srp]);
    }else{
      $this->error = "Could not connect to LDAP server";
      return("");
    }
  }

  function ls($srp, $filter = "(objectclass=*)", $basedn = "",$attrs = array("*"))
  {
    if($this->hascon){
      if ($this->reconnect) $this->connect();

      $this->clearResult($srp);
      if ($basedn == "")
        $basedn = $this->basedn;
      else
        $basedn= LDAP::convert($basedn);
  
      $start = microtime(true);
      $this->sr[$srp] = @ldap_list($this->cid, LDAP::fix($basedn), $filter,$attrs);
      $this->error = @ldap_error($this->cid);
      $this->resetResult($srp);
      $this->hasres[$srp]=true;

       /* Check if query took longer as specified in max_ldap_query_time */
      if($this->max_ldap_query_time){
        $diff = microtime(true) - $start;
        if($diff > $this->max_ldap_query_time){
          msg_dialog::display(_("Performance warning"), sprintf(_("LDAP performance is poor: last query took about %.2fs!"), $diff), WARNING_DIALOG);
        }
      }

      $this->log("LDAP operation: time=".(microtime(true) - $start)." operation=ls('".LDAP::fix($basedn)."', '$filter')");

      return($this->sr[$srp]);
    }else{
      $this->error = "Could not connect to LDAP server";
      return("");
    }
  }

  function cat($srp, $dn,$attrs= array("*"), $filter = "(objectclass=*)")
  {
    if($this->hascon){
      if ($this->reconnect) $this->connect();

      $this->clearResult($srp);
      $this->sr[$srp] = @ldap_read($this->cid, LDAP::fix($dn), $filter,$attrs);
      $this->error = @ldap_error($this->cid);
      $this->resetResult($srp);
      $this->hasres[$srp]=true;
      return($this->sr[$srp]);
    }else{
      $this->error = "Could not connect to LDAP server";
      return("");
    }
  }

  function object_match_filter($dn,$filter)
  {
    if($this->hascon){
      if ($this->reconnect) $this->connect();
      $res =  @ldap_read($this->cid, LDAP::fix($dn), $filter, array("objectClass"));
      $rv =   @ldap_count_entries($this->cid, $res);
      return($rv);
    }else{
      $this->error = "Could not connect to LDAP server";
      return(FALSE);
    }
  }

  function set_size_limit($size)
  {
    /* Ignore zero settings */
    if ($size == 0){
      @ldap_set_option($this->cid, LDAP_OPT_SIZELIMIT, 10000000);
    }
    if($this->hascon){
      @ldap_set_option($this->cid, LDAP_OPT_SIZELIMIT, $size);
    } else {
      $this->error = "Could not connect to LDAP server";
    }
  }

  function fetch($srp)
  {
    $att= array();
    if($this->hascon){
      if($this->hasres[$srp]){
        if ($this->start[$srp] == 0)
        {
          if ($this->sr[$srp]){
            $this->start[$srp] = 1;
            $this->re[$srp]= @ldap_first_entry($this->cid, $this->sr[$srp]);
          } else {
            return array();
          }
        } else {
          $this->re[$srp]= @ldap_next_entry($this->cid, $this->re[$srp]);
        }
        if ($this->re[$srp])
        {
          $att= @ldap_get_attributes($this->cid, $this->re[$srp]);
          $att['dn']= trim(LDAP::convert(@ldap_get_dn($this->cid, $this->re[$srp])));
        }
        $this->error = @ldap_error($this->cid);
        if (!isset($att)){
          $att= array();
        }
        return($att);
      }else{
        $this->error = "Perform a fetch with no search";
        return("");
      }
    }else{
      $this->error = "Could not connect to LDAP server";
      return("");
    }
  }

  function resetResult($srp)
  {
    $this->start[$srp] = 0;
  }

  function clearResult($srp)
  {
    if($this->hasres[$srp]){
      $this->hasres[$srp] = false;
      @ldap_free_result($this->sr[$srp]);
    }
  }

  function getDN($srp)
  {
    if($this->hascon){
      if($this->hasres[$srp]){

        if(!$this->re[$srp])
          {
          $this->error = "Perform a Fetch with no valid Result";
          }
          else
          {
          $rv = @ldap_get_dn($this->cid, $this->re[$srp]);
        
          $this->error = @ldap_error($this->cid);
          return(trim(LDAP::convert($rv)));
           }
      }else{
        $this->error = "Perform a Fetch with no Search";
        return("");
      }
    }else{
      $this->error = "Could not connect to LDAP server";
      return("");
    }
  }

  function count($srp)
  {
    if($this->hascon){
      if($this->hasres[$srp]){
        $rv = @ldap_count_entries($this->cid, $this->sr[$srp]);
        $this->error = @ldap_error($this->cid);
        return($rv);
      }else{
        $this->error = "Perform a Fetch with no Search";
        return("");
      }
    }else{
      $this->error = "Could not connect to LDAP server";
      return("");
    }
  }

  function rm($attrs = "", $dn = "")
  {
    if($this->hascon){
      if ($this->reconnect) $this->connect();
      if ($dn == "")
        $dn = $this->basedn;

      $r = ldap_mod_del($this->cid, LDAP::fix($dn), $attrs);
      $this->error = @ldap_error($this->cid);
      return($r);
    }else{
      $this->error = "Could not connect to LDAP server";
      return("");
    }
  }

  function mod_add($attrs = "", $dn = "")
  {
    if($this->hascon){
      if ($this->reconnect) $this->connect();
      if ($dn == "")
        $dn = $this->basedn;

      $r = @ldap_mod_add($this->cid, LDAP::fix($dn), $attrs);
      $this->error = @ldap_error($this->cid);
      return($r);
    }else{
      $this->error = "Could not connect to LDAP server";
      return("");
    }
  }

  function rename($attrs, $dn = "")
  {
    if($this->hascon){
      if ($this->reconnect) $this->connect();
      if ($dn == "")
        $dn = $this->basedn;

      $r = @ldap_mod_replace($this->cid, LDAP::fix($dn), $attrs);
      $this->error = @ldap_error($this->cid);
      return($r);
    }else{
      $this->error = "Could not connect to LDAP server";
      return("");
    }
  }

  function rmdir($deletedn)
  {
    if($this->hascon){
      if ($this->reconnect) $this->connect();
      $r = @ldap_delete($this->cid, LDAP::fix($deletedn));
      $this->error = @ldap_error($this->cid);
      return($r ? $r : 0);
    }else{
      $this->error = "Could not connect to LDAP server";
      return("");
    }
  }


  /*! \brief Move the given Ldap entry from $source to $dest
      @param  String  $source The source dn.
      @param  String  $dest   The destination dn.
      @return Boolean TRUE on success else FALSE.
   */
  function rename_dn($source,$dest)
  {
    /* Check if source and destination are the same entry */
    if(strtolower($source) == strtolower($dest)){
      trigger_error("Source and destination can't be the same entry.");
      $this->error = "Source and destination can't be the same entry.";
      return(FALSE);
    }

    /* Check if destination entry exists */    
    if($this->dn_exists($dest)){
      trigger_error("Destination '$dest' already exists.");
      $this->error = "Destination '$dest' already exists.";
      return(FALSE);
    }

    /* Extract the name and the parent part out ouf source dn.
        e.g.  cn=herbert,ou=department,dc=... 
         parent   =>  ou=department,dc=...
         dest_rdn =>  cn=herbert
     */
    $parent   = preg_replace("/^[^,]+,/","", $dest);
    $dest_rdn = preg_replace("/,.*$/","",$dest);

    if($this->hascon){
      if ($this->reconnect) $this->connect();
      $r= ldap_rename($this->cid,@LDAP::fix($source), @LDAP::fix($dest_rdn),@LDAP::fix($parent),TRUE); 
      $this->error = ldap_error($this->cid);

      /* Check if destination dn exists, if not the 
          server may not support this operation */
      $r &= is_resource($this->dn_exists($dest));
      return($r);
    }else{
      $this->error = "Could not connect to LDAP server";
      return(FALSE);
    }
  }


  /**
  *  Function rmdir_recursive
  *
  *  Description: Based in recursive_remove, adding two thing: full subtree remove, and delete own node.
  *  Parameters:  The dn to delete
  *  GiveBack:    True on sucessfull , 0 in error, and "" when we don't get a ldap conection
  *
  */
  function rmdir_recursive($srp, $deletedn)
  {
    if($this->hascon){
      if ($this->reconnect) $this->connect();
      $delarray= array();
        
      /* Get sorted list of dn's to delete */
      $this->ls ($srp, "(objectClass=*)",$deletedn);
      while ($this->fetch($srp)){
        $deldn= $this->getDN($srp);
        $delarray[$deldn]= strlen($deldn);
      }
      arsort ($delarray);
      reset ($delarray);

      /* Really Delete ALL dn's in subtree */
      foreach ($delarray as $key => $value){
        $this->rmdir_recursive($srp, $key);
      }
      
      /* Finally Delete own Node */
      $r = @ldap_delete($this->cid, LDAP::fix($deletedn));
      $this->error = @ldap_error($this->cid);
      return($r ? $r : 0);
    }else{
      $this->error = "Could not connect to LDAP server";
      return("");
    }
  }

  function makeReadableErrors($error,$attrs)
  { 
    global $config;

    if($this->success()) return("");

    $str = "";
    if(preg_match("/^objectClass: value #([0-9]*) invalid per syntax$/", $this->get_additional_error())){
      $oc = preg_replace("/^objectClass: value #([0-9]*) invalid per syntax$/","\\1", $this->get_additional_error());
      if(isset($attrs['objectClass'][$oc])){
        $str.= " - <b>objectClass: ".$attrs['objectClass'][$oc]."</b>";
      }
    }
    if($error == "Undefined attribute type"){
      $str = " - <b>attribute: ".preg_replace("/:.*$/","",$this->get_additional_error())."</b>";
    } 

    @DEBUG(DEBUG_LDAP,__LINE__,__FUNCTION__,__FILE__,$attrs,"Erroneous data");

    return($str);
  }

  function modify($attrs)
  {
    if(count($attrs) == 0){
      return (0);
    }
    if($this->hascon){
      if ($this->reconnect) $this->connect();
      $r = @ldap_modify($this->cid, LDAP::fix($this->basedn), $attrs);
      $this->error = @ldap_error($this->cid);
      if(!$this->success()){
        $this->error.= $this->makeReadableErrors($this->error,$attrs);
      }
      return($r ? $r : 0);
    }else{
      $this->error = "Could not connect to LDAP server";
      return("");
    }
  }

  function add($attrs)
  {
    if($this->hascon){
      if ($this->reconnect) $this->connect();
      $r = @ldap_add($this->cid, LDAP::fix($this->basedn), $attrs);
      $this->error = @ldap_error($this->cid);
      if(!$this->success()){
        $this->error.= $this->makeReadableErrors($this->error,$attrs);
      }
      return($r ? $r : 0);
    }else{
      $this->error = "Could not connect to LDAP server";
      return("");
    }
  }

  function create_missing_trees($srp, $target)
  {
    global $config;

    $real_path= substr($target, 0, strlen($target) - strlen($this->basedn) -1 );

    if ($target == $this->basedn){
      $l= array("dummy");
    } else {
      $l= array_reverse(gosa_ldap_explode_dn($real_path));
    }
    unset($l['count']);
    $cdn= $this->basedn;
    $tag= "";

    /* Load schema if available... */
    $classes= $this->get_objectclasses();

    foreach ($l as $part){
      if ($part != "dummy"){
        $cdn= "$part,$cdn";
      }

      /* Ignore referrals */
      $found= false;
      foreach($this->referrals as $ref){
        $base= preg_replace('!^[^:]+://[^/]+/([^?]+).*$!', '\\1', $ref['URI']);
        if ($base == $cdn){
          $found= true;
          break;
        }
      }
      if ($found){
        continue;
      }

      $this->cat ($srp, $cdn);
      $attrs= $this->fetch($srp);

      /* Create missing entry? */
      if (count ($attrs)){
      
        /* Catch the tag - if present */
        if (isset($attrs['gosaUnitTag'][0])){
          $tag= $attrs['gosaUnitTag'][0];
        }

      } else {
        $type= preg_replace('/^([^=]+)=.*$/', '\\1', $cdn);
        $param= LDAP::fix(preg_replace('/^[^=]+=([^,]+).*$/', '\\1', $cdn));
        $param=preg_replace(array('/\\\\,/','/\\\\"/'),array(',','"'),$param);

        $na= array();

        /* Automatic or traditional? */
        if(count($classes)){

          /* Get name of first matching objectClass */
          $ocname= "";
          foreach($classes as $class){
            if (isset($class['MUST']) && in_array($type, $class['MUST'])){

              /* Look for first classes that is structural... */
              if (isset($class['STRUCTURAL'])){
                $ocname= $class['NAME'];
                break;
              }

              /* Look for classes that are auxiliary... */
              if (isset($class['AUXILIARY'])){
                $ocname= $class['NAME'];
              }
            }
          }

          /* Bail out, if we've nothing to do... */
          if ($ocname == ""){
            msg_dialog::display(_("Internal error"), sprintf(_("Cannot automatically create subtrees with RDN '%s': no object class found!"),$type), FATAL_ERROR_DIALOG);
            exit();
          }

          /* Assemble_entry */
          if ($tag != ""){
            $na['objectClass']= array($ocname, "gosaAdministrativeUnitTag");
            $na["gosaUnitTag"]= $tag;
          } else {
            $na['objectClass']= array($ocname);
          }
          if (isset($classes[$ocname]['AUXILIARY'])){
            $na['objectClass'][]= $classes[$ocname]['SUP'];
          }
          if ($type == "dc"){
            /* This is bad actually, but - tell me a better way? */
            $na['objectClass'][]= 'locality';
          }
          $na[$type]= $param;

          // Fill in MUST values - but do not overwrite existing ones.
          if (is_array($classes[$ocname]['MUST'])){
            foreach($classes[$ocname]['MUST'] as $attr){
              if(isset($na[$attr]) && !empty($na[$attr])) continue;
              $na[$attr]= "filled";
            }
          }

        } else {

          /* Use alternative add... */
          switch ($type){
            case 'ou':
              if ($tag != ""){
                $na["objectClass"]= array("organizationalUnit", "gosaAdministrativeUnitTag");
                $na["gosaUnitTag"]= $tag;
              } else {
                $na["objectClass"]= "organizationalUnit";
              }
              $na["ou"]= $param;
              break;
            case 'dc':
              if ($tag != ""){
                $na["objectClass"]= array("dcObject", "top", "locality", "gosaAdministrativeUnitTag");
                $na["gosaUnitTag"]= $tag;
              } else {
                $na["objectClass"]= array("dcObject", "top", "locality");
              }
              $na["dc"]= $param;
              break;
            default:
              msg_dialog::display(_("Internal error"), sprintf(_("Cannot automatically create subtrees with RDN '%s': not supported"),$type), FATAL_ERROR_DIALOG);
              exit();
          }

        }
        $this->cd($cdn);
        $this->add($na);
    
        if (!$this->success()){

          print_a(array($cdn,$na));

          msg_dialog::display(_("LDAP error"), msgPool::ldaperror($this->get_error(), $cdn, LDAP_ADD, get_class()));
          return FALSE;
        }
      }
    }

    return TRUE;
  }


  function recursive_remove($srp)
  {
    $delarray= array();

    /* Get sorted list of dn's to delete */
    $this->search ($srp, "(objectClass=*)");
    while ($this->fetch($srp)){
      $deldn= $this->getDN($srp);
      $delarray[$deldn]= strlen($deldn);
    }
    arsort ($delarray);
    reset ($delarray);

    /* Delete all dn's in subtree */
    foreach ($delarray as $key => $value){
      $this->rmdir($key);
    }
  }


  function get_attribute($dn, $name,$r_array=0)
  {
    $data= "";
    if ($this->reconnect) $this->connect();
    $sr= @ldap_read($this->cid, LDAP::fix($dn), "objectClass=*", array("$name"));

    /* fill data from LDAP */
    if ($sr) {
      $ei= @ldap_first_entry($this->cid, $sr);
      if ($ei) {
        if ($info= @ldap_get_values_len($this->cid, $ei, "$name")){
          $data= $info[0];
        }
      }
    }
    if($r_array==0) {
      return ($data);
    } else {
      return ($info);
    }
  }
 


  function get_additional_error()
  {
    $error= "";
    @ldap_get_option ($this->cid, LDAP_OPT_ERROR_STRING, $error);
    return ($error);
  }


  function success()
  {
    return (preg_match('/Success/i', $this->error));
  }


  function get_error()
  {
    if ($this->error == 'Success'){
      return $this->error;
    } else {
      $adderror= $this->get_additional_error();
      if ($adderror != ""){
        $error= $this->error." (".$this->get_additional_error().", ".sprintf(_("while operating on '%s' using LDAP server '%s'"), $this->basedn, $this->hostname).")";
      } else {
        $error= $this->error." (".sprintf(_("while operating on LDAP server %s"), $this->hostname).")";
      }
      return $error;
    }
  }

  function get_credentials($url, $referrals= NULL)
  {
    $ret= array();
    $url= preg_replace('!\?\?.*$!', '', $url);
    $server= preg_replace('!^([^:]+://[^/]+)/.*$!', '\\1', $url);

    if ($referrals === NULL){
      $referrals= $this->referrals;
    }

    if (isset($referrals[$server])){
      return ($referrals[$server]);
    } else {
      $ret['ADMINDN']= LDAP::fix($this->binddn);
      $ret['ADMINPASSWORD']= $this->bindpw;
    }

    return ($ret);
  }


  /*! \brief  Generates an ldif for all entries matching the filter settings, scope and limit.
   *  @param  $dn           The entry to export.
   *  @param  $filter       Limit the exported object to those maching this filter.
   *  @param  $scope        'base', 'sub' .. see manpage for 'ldapmodify' for details.
   *  @param  $limit        Limits the result.
   */
  function generateLdif ($dn, $filter= "(objectClass=*)", $attributes= array(),$scope = 'sub', $limit=0)
  {
        
      // Attributes are unused !!
      $attrs = '';

      // Ensure that limit is numeric if not skip here.
      if(!empty($limit) && !is_numeric($limit)){
          trigger_error(sprintf("Invalid parameter for limit '%s', a numeric value is required."), $limit);
          return(NULL);
      }
      $limit = (!$limit)?'':' -z '.$limit;

      // Check scope values
      $scope = trim($scope);
      if(!empty($scope) && !in_array($scope, array('base', 'one', 'sub', 'children'))){
          trigger_error(sprintf("Invalid parameter for scope '%s', please use 'base', 'one', 'sub' or 'children'."), $scope);
          return(NULL);
      }
      $scope = (!empty($scope))?' -s '.$scope: '';

      // First check if we are able to call 'ldapsearch' on the command line.
      $check = shell_exec('which ldapsearch');
      if(empty($check)){
        $this->error = sprintf(_("Missing command line programm '%s'!"), 'ldapsearch');
        return(NULL);
      }

      // Prepare parameters to be valid for shell execution
      $dn = escapeshellarg($dn);
      $pwd = $this->bindpw;
      $host = escapeshellarg($this->hostname);
      $admin = escapeshellarg($this->binddn);
      $filter = escapeshellarg($filter);

        
      $cmd = "ldapsearch -x -LLLL -D {$admin} {$filter} {$limit} {$scope} -H {$host} -b {$dn} -w {$pwd} ";

      // Create list of process pipes
      $descriptorspec = array(
              0 => array("pipe", "r"),  // stdin
              1 => array("pipe", "w"),  // stdout
              2 => array("pipe", "w")); // stderr

      // Try to open the process
      $process = proc_open($cmd, $descriptorspec, $pipes);
      if (is_resource($process)) {

          // Write the password to stdin
          fclose($pipes[0]);

          // Get results from stdout and stderr
          $res = stream_get_contents($pipes[1]);
          $err = stream_get_contents($pipes[2]);
          fclose($pipes[1]);

          // Close the process and check its return value
          if(proc_close($process) != 0){
              $this->error = $err;
              return(NULL);
          }
      }
      return($res);
  }


  function gen_xls ($srp, $dn, $filter= "(objectClass=*)", $attributes= array('*'), $recursive= TRUE,$r_array=0)
  {
    $display= array();

      $this->cd($dn);
      $this->search($srp, "$filter");

      $i=0;
      while ($attrs= $this->fetch($srp)){
        $j=0;

        foreach ($attributes as $at){
          $display[$i][$j]= $this->get_attribute($attrs['dn'], $at,$r_array);
          $j++;
        }

        $i++;
      }

    return ($display);
  }
    

  function dn_exists($dn)
  {
    return @ldap_list($this->cid, LDAP::fix($dn), "(objectClass=*)", array("objectClass"));
  }
  


  /*  This funktion imports ldifs 
        
      If DeleteOldEntries is true, the destination entry will be deleted first. 
      If JustModify is true the destination entry will only be touched by the attributes specified in the ldif.
      if JustMofify id false the destination dn will be overwritten by the new ldif. 
    */

  function import_complete_ldif($srp, $str_attr,$error,$JustModify,$DeleteOldEntries)
  {
    if($this->reconnect) $this->connect();

    /* First we have to split the string into empty lines.
       An empty line indicates an new Entry */
    $entries = preg_split("/\n/",$str_attr);

    $data = "";
    $cnt = 0; 
    $current_line = 0;

    /* FIX ldif */
    $last = "";
    $tmp  = "";
    $i = 0;
    foreach($entries as $entry){
      if(preg_match("/^ /",$entry)){
        $tmp[$i] .= trim($entry);
      }else{
        $i ++;
        $tmp[$i] = trim($entry);
      }
    }

    /* Every single line ... */
    foreach($tmp as $entry) {
      $current_line ++;

      /* Removing Spaces to .. 
         .. test if a new entry begins */
      $tmp  = str_replace(" ","",$data );

      /* .. prevent empty lines in an entry */
      $tmp2 = str_replace(" ","",$entry);

      /* If the Block ends (Empty Line) */
      if((empty($entry))&&(!empty($tmp))) {
        /* Add collected lines as a complete block */
        $all[$cnt] = $data;
        $cnt ++;
        $data ="";
      } else {

        /* Append lines ... */
        if(!empty($tmp2)) {
          /* check if we need base64_decode for this line */
          if(strstr($tmp2, "::") !== false)
          {
            $encoded = explode("::",$entry);
            $attr  = trim($encoded[0]);
            $value = base64_decode(trim($encoded[1]));
            /* Add linenumber */
            $data .= $current_line."#".base64_encode($attr.":".$value)."\n";
          }
          else
          {
            /* Add Linenumber */ 
            $data .= $current_line."#".base64_encode($entry)."\n";
          }
        }
      }
    }

    /* The Data we collected is not in the array all[];
       For example the Data is stored like this..

       all[0] = "1#dn : .... \n 
       2#ObjectType: person \n ...."
       
       Now we check every insertblock and try to insert */
    foreach ( $all as $single) {
      $lineone = preg_split("/\n/",$single);  
      $ndn = explode("#", $lineone[0]);
      $line = base64_decode($ndn[1]);

      $dnn = explode (":",$line,2);
      $current_line = $ndn[0];
      $dn    = $dnn[0];
      $value = $dnn[1];

      /* Every block must begin with a dn */
      if($dn != "dn") {
        $error= sprintf(_("This is not a valid DN: '%s'. A block for import should begin with 'dn: ...' in line %s"), $line, $current_line);
        return -2;  
      }

      /* Should we use Modify instead of Add */
      $usemodify= false;

      /* Delete before insert */
      $usermdir= false;
    
      /* The dn address already exists, Don't delete destination entry, overwrite it */
      if (($this->dn_exists($value))&&((!$JustModify)&&(!$DeleteOldEntries))) {

        $usermdir = $usemodify = false;

      /* Delete old entry first, then add new */
      } elseif(($this->dn_exists($value))&&($DeleteOldEntries)){

        /* Delete first, then add */
        $usermdir = true;        

      } elseif(($this->dn_exists($value))&&($JustModify)) {
        
        /* Modify instead of Add */
        $usemodify = true;
      }
     
      /* If we can't Import, return with a file error */
      if(!$this->import_single_entry($srp, $single,$usemodify,$usermdir) ) {
        $error= sprintf(_("Error while importing dn: '%s', please check your LDIF from line %s on!"), $line,
                        $current_line);
        return UNKNOWN_TOKEN_IN_LDIF_FILE;      }
    }

    return (INSERT_OK);
  }


  /* Imports a single entry 
      If $delete is true;  The old entry will be deleted if it exists.
      if $modify is true;  All variables that are not touched by the new ldif will be kept.
      if $modify is false; The new ldif overwrites the old entry, and all untouched attributes get lost.
  */
  function import_single_entry($srp, $str_attr,$modify,$delete)
  {
    global $config;

    if(!$config){
      trigger_error("Can't import ldif, can't read config object.");
    }
  

    if($this->reconnect) $this->connect();

    $ret = false;
    $rows= preg_split("/\n/",$str_attr);
    $data= false;

    foreach($rows as $row) {
      
      /* Check if we use Linenumbers (when import_complete_ldif is called we use
         Linenumbers) Linenumbers are use like this 123#attribute : value */
      if(!empty($row)) {
        if(strpos($row,"#")!=FALSE) {

          /* We are using line numbers 
             Because there is a # before a : */
          $tmp1= explode("#",$row);
          $current_line= $tmp1[0];
          $row= base64_decode($tmp1[1]);
        }

        /* Split the line into  attribute  and value */
        $attr   = explode(":", $row,2);
        $attr[0]= trim($attr[0]);  /* attribute */
        $attr[1]= $attr[1];  /* value */

        /* Check :: was used to indicate base64_encoded strings */
        if($attr[1][0] == ":"){
          $attr[1]=trim(preg_replace("/^:/","",$attr[1]));
          $attr[1]=base64_decode($attr[1]);
        }

        $attr[1] = trim($attr[1]);

        /* Check for attributes that are used more than once */
        if(!isset($data[$attr[0]])) {
          $data[$attr[0]]=$attr[1];
        } else {
          $tmp = $data[$attr[0]];

          if(!is_array($tmp)) {
            $new[0]=$tmp;
            $new[1]=$attr[1];
            $datas[$attr[0]]['count']=1;             
            $data[$attr[0]]=$new;
          } else {
            $cnt = $datas[$attr[0]]['count'];           
            $cnt ++;
            $data[$attr[0]][$cnt]=$attr[1];
            $datas[$attr[0]]['count'] = $cnt;
          }
        }
      }
    }

    /* If dn is an index of data, we should try to insert the data */
    if(isset($data['dn'])) {

      /* Fix dn */
      $tmp = gosa_ldap_explode_dn($data['dn']);
      unset($tmp['count']);
      $newdn ="";
      foreach($tmp as $tm){
        $newdn.= trim($tm).",";
      }
      $newdn = preg_replace("/,$/","",$newdn);
      $data['dn'] = $newdn;
   
      /* Creating Entry */
      $this->cd($data['dn']);

      /* Delete existing entry */
      if($delete){
        $this->rmdir_recursive($srp, $data['dn']);
      }
     
      /* Create missing trees */
      $this->cd ($this->basedn);
      $this->cd($config->current['BASE']);
      $this->create_missing_trees($srp, preg_replace("/^[^,]+,/","",$data['dn']));
      $this->cd($data['dn']);

      $dn = $data['dn'];
      unset($data['dn']);
      
      if(!$modify){

        $this->cat($srp, $dn);
        if($this->count($srp)){
        
          /* The destination entry exists, overwrite it with the new entry */
          $attrs = $this->fetch($srp);
          foreach($attrs as $name => $value ){
            if(!is_numeric($name)){
              if(in_array($name,array("dn","count"))) continue;
              if(!isset($data[$name])){
                $data[$name] = array();
              }
            }
          }
          $ret = $this->modify($data);
    
        }else{
    
          /* The destination entry doesn't exists, create it */
          $ret = $this->add($data);
        }

      } else {
        
        /* Keep all vars that aren't touched by this ldif */
        $ret = $this->modify($data);
      }
    }

    if (!$this->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($this->get_error(), $dn, "", get_class()));
    }

    return($ret);
  }

  
  function importcsv($str)
  {
    $lines = preg_split("/\n/",$str);
    foreach($lines as $line)
    {
      /* continue if theres a comment */
      if(substr(trim($line),0,1)=="#"){
        continue;
      }

      $line= str_replace ("\t\t","\t",$line);
      $line= str_replace ("\t"  ,"," ,$line);
      echo $line;

      $cells = explode(",",$line )  ;
      $linet= str_replace ("\t\t",",",$line);
      $cells = preg_split("/\t/",$line);
      $count = count($cells);  
    }

  }
  
  function get_objectclasses( $force_reload = FALSE)
  {
    $objectclasses = array();
    global $config;

    /* Only read schema if it is allowed */
    if(isset($config) && preg_match("/config/i",get_class($config))){
      if ($config->get_cfg_value("schemaCheck") != "true"){
        return($objectclasses);
      } 
    }

    /* Return the cached results. */
    if(class_available('session') && session::global_is_set("LDAP_CACHE::get_objectclasses") && !$force_reload){
      $objectclasses = session::global_get("LDAP_CACHE::get_objectclasses");
      return($objectclasses);
    }
	
	  # Get base to look for schema 
	  $sr = @ldap_read ($this->cid, "", "objectClass=*", array("subschemaSubentry"));
	  $attr = @ldap_get_entries($this->cid,$sr);
	  if (!isset($attr[0]['subschemasubentry'][0])){
	    return array();
	  }
	
	  /* Get list of objectclasses and fill array */
	  $nb= $attr[0]['subschemasubentry'][0];
	  $objectclasses= array();
	  $sr= ldap_read ($this->cid, $nb, "objectClass=*", array("objectclasses"));
	  $attrs= ldap_get_entries($this->cid,$sr);
	  if (!isset($attrs[0])){
	    return array();
	  }
	  foreach ($attrs[0]['objectclasses'] as $val){
      if (preg_match('/^[0-9]+$/', $val)){
        continue;
      }
      $name= "OID";
      $pattern= explode(' ', $val);
      $ocname= preg_replace("/^.* NAME\s+\(*\s*'([^']+)'\s*\)*.*$/", '\\1', $val);
      $objectclasses[$ocname]= array();

      foreach($pattern as $chunk){
        switch($chunk){

          case '(':
                    $value= "";
                    break;

          case ')': if ($name != ""){
                      $v = $this->value2container($value);
                      if(in_array($name, array('MUST', 'MAY')) && !is_array($v)){
                        $v = array($v);
                      }
                      $objectclasses[$ocname][$name]= $v;
                    }
                    $name= "";
                    $value= "";
                    break;

          case 'NAME':
          case 'DESC':
          case 'SUP':
          case 'STRUCTURAL':
          case 'ABSTRACT':
          case 'AUXILIARY':
          case 'MUST':
          case 'MAY':
                    if ($name != ""){
                      $v = $this->value2container($value);
                      if(in_array($name, array('MUST', 'MAY')) && !is_array($v)){
                        $v = array($v);
                      }
                      $objectclasses[$ocname][$name]= $v;
                    }
                    $name= $chunk;
                    $value= "";
                    break;

          default:  $value.= $chunk." ";
        }
      }

	  }
    if(class_available("session")){
      session::global_set("LDAP_CACHE::get_objectclasses",$objectclasses);
    }

	  return $objectclasses;
  }


  function value2container($value)
  {
    /* Set emtpy values to "true" only */
    if (preg_match('/^\s*$/', $value)){
      return true;
    }

    /* Remove ' and " if needed */
    $value= preg_replace('/^[\'"]/', '', $value);
    $value= preg_replace('/[\'"] *$/', '', $value);

    /* Convert to array if $ is inside... */
    if (preg_match('/\$/', $value)){
      $container= preg_split('/\s*\$\s*/', $value);
    } else {
      $container= chop($value);
    }

    return ($container);
  }


  function log($string)
  {
    if (session::global_is_set('config')){
      $cfg = session::global_get('config');
      if (isset($cfg->current['LDAPSTATS']) && preg_match('/true/i', $cfg->current['LDAPSTATS'])){
        syslog (LOG_INFO, $string);
      }
    }
  }

  /* added by Guido Serra aka Zeph <zeph@purotesto.it> */
  function getCn($dn){
    $simple= explode(",", $dn);

    foreach($simple as $piece) {
      $partial= explode("=", $piece);

      if($partial[0] == "cn"){
        return $partial[1];
      }
    }
  }


  function get_naming_contexts($server, $admin= "", $password= "")
  {
    /* Build LDAP connection */
    $ds= ldap_connect ($server);
    if (!$ds) {
      die ("Can't bind to LDAP. No check possible!");
    }
    ldap_set_option($ds, LDAP_OPT_PROTOCOL_VERSION, 3);
    $r= ldap_bind ($ds, $admin, $password);

    /* Get base to look for naming contexts */
    $sr  = @ldap_read ($ds, "", "objectClass=*", array("+"));
    $attr= @ldap_get_entries($ds,$sr);

    return ($attr[0]['namingcontexts']);
  }


  function get_root_dse($server, $admin= "", $password= "")
  {
    /* Build LDAP connection */
    $ds= ldap_connect ($server);
    if (!$ds) {
      die ("Can't bind to LDAP. No check possible!");
    }
    ldap_set_option($ds, LDAP_OPT_PROTOCOL_VERSION, 3);
    $r= ldap_bind ($ds, $admin, $password);

    /* Get base to look for naming contexts */
    $sr  = @ldap_read ($ds, "", "objectClass=*", array("+"));
    $attr= @ldap_get_entries($ds,$sr);
   
    /* Return empty array, if nothing was set */
    if (!isset($attr[0])){
      return array();
    }

    /* Rework array... */
    $result= array();
    for ($i= 0; $i<$attr[0]['count']; $i++){
      $result[$attr[0][$i]]= $attr[0][$attr[0][$i]];
      unset($result[$attr[0][$i]]['count']);
    }

    return ($result);
  }

}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
