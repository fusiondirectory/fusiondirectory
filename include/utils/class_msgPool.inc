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


define ("LDAP_READ",   1);
define ("LDAP_ADD",    2);
define ("LDAP_MOD",    3);
define ("LDAP_DEL",    4);
define ("LDAP_SEARCH", 5);
define ("LDAP_AUTH",   6);

/*! \brief Some common needed messages */ 
class msgPool
{

  public static function selectToView($type,$o_type = "")
  {
    if($o_type == ""){
      return(sprintf(_("Select to list objects of type '%s'."),$type));
    }elseif($o_type == "contains"){
      return(sprintf(_("Select to list objects containig '%s'."),$type));
    }elseif($o_type == "enabled"){
      return(sprintf(_("Select to list objects that have '%s' enabled"),$type));
    }elseif($o_type == "subsearch"){
      return(_("Select to search within subtrees"));
    }elseif($o_type == "subsearch_small"){
      return(_("Search in subtrees"));
    }

  }


  public static function deleteInfo($name= "",$type = "")
  {
    if ($name == "") { 
      if($type == ""){
        return (_("This object will be deleted!"));
      }else{
        return (sprintf(_("This '%s' object will be deleted!"), $type));
      }
    }
    if (!is_array($name)){
      if($type == ""){
        return (sprintf(_("This object will be deleted: %s"), "<br><br><i>$name</i>"));
      }else{
        return (sprintf(_("This '%s' object will be deleted: %s"),$type, "<br><br><i>$name</i>"));
      }
    }
    if (count($name) == 1){
      if($type == ""){
        return (_("This object will be deleted:")."<br>".msgPool::buildList($name));
      }else{
        return (sprintf(_("This '%s' object will be deleted:"),$type). "<br>".msgPool::buildList($name));
      }
    }
    if($type == ""){
      return (sprintf(_("These objects will be deleted: %s"), "<br>".msgPool::buildList($name)));
    }else{
      return (sprintf(_("These '%s' objects will be deleted: %s"),$type, "<br>".msgPool::buildList($name)));
    }
  }


  public static function permDelete($name= "")
  {
    if ($name == "") { 
      return (_("You have no permission to delete this object!"));
    }

    if (!is_array($name)){
      return (_("You have no permission to delete the object:")."<br><br><i>$name</i>");
    }

    if (count($name) == 1){
      return (_("You have no permission to delete the object:")."<br>".msgPool::buildList($name));
    }

    return (_("You have no permission to delete these objects:")."<br>".msgPool::buildList($name));
  }


  public static function permCreate($name= "")
  {
    if ($name == "") { 
      return (_("You have no permission to create this object!"));
    }

    if (!is_array($name)){
      return (_("You have no permission to create the object:")."<br><br><i>$name</i>");
    }

    if (count($name) == 1){
      return (_("You have no permission to create the object:")."<br>".msgPool::buildList($name));
    }

    return (_("You have no permission to create these objects:")."<br>".msgPool::buildList($name));
  }


  public static function permModify($name= "")
  {
    if ($name == "") { 
      return (_("You have no permission to modify this object!"));
    }

    if (!is_array($name)){
      return (_("You have no permission to modify the object:")."<br><br><i>$name</i>");
    }

    if (count($name) == 1){
      return (_("You have no permission to modify the object:")."<br>".msgPool::buildList($name));
    }

    return (_("You have no permission to modify these objects:")."<br>".msgPool::buildList($name));
  }


  public static function permView($name= "")
  {
    if ($name == "") { 
      return (_("You have no permission to view this object!"));
    }

    if (!is_array($name)){
      return (_("You have no permission to view the object:")."<br><br><i>$name</i>");
    }

    if (count($name) == 1){
      return (_("You have no permission to view the object:")."<br>".msgPool::buildList($name));
    }

    return (_("You have no permission to view these objects:")."<br>".msgPool::buildList($name));
  }


  public static function permMove($name= "")
  {
    if ($name == "") { 
      return (_("You have no permission to move this object!"));
    }

    if (!is_array($name)){
      return (_("You have no permission to move the object:")."<br><br><i>$name</i>");
    }

    if (count($name) == 1){
      return (_("You have no permission to move the object:")."<br>".msgPool::buildList($name));
    }

    return (_("You have no permission to move these objects:")."<br>".msgPool::buildList($name));
  }


  public static function dbconnect($name, $error= "", $dbinfo= "")
  {
    if ($error != ""){
      $error= "<br><br><i>"._("Error").":</i> ".$error;
    }
    if ($dbinfo != ""){
      $error.= "<br><br><i>"._("Connection information").":</i> ".$dbinfo;
    }
    return (sprintf(_("Cannot connect to %s database!"), $name).$error);
  }


  public static function dbselect($name, $error= "", $dbinfo= "")
  {
    if ($error != ""){
      $error= "<br><br><i>"._("Error").":</i> ".$error;
    }
    if ($dbinfo != ""){
      $error.= "<br><br><i>"._("Connection information").":</i> ".$dbinfo;
    }
    return (sprintf(_("Cannot select %s database!"), $name).$error);
  }


  public static function noserver($name)
  {
    return (sprintf(_("No %s server defined!"), $name));
  }


  public static function dbquery($name, $error= "", $dbinfo= "")
  {
    if ($error != ""){
      $error= "<br><br><i>"._("Error").":</i> ".$error;
    }
    if ($dbinfo != ""){
      $error.= "<br><br><i>"._("Connection information").":</i> ".$dbinfo;
    }
    return (sprintf(_("Cannot query %s database!"), $name).$error);
  }


  public static function reserved($name)
  {
    return (sprintf(_("The field '%s' contains a reserved keyword!"), $name));
  }


  public static function cmdnotfound($type, $plugin)
  {
    return (sprintf(_("Command specified as %s hook for plugin '%s' does not exist!"), $type, $plugin));
  }


  public static function cmdinvalid($type, $command = "",$plugin="")
  {
    if(empty($command)){
      return (sprintf(_("'%s' command is invalid!"), $type));
    }elseif($command != "" && $plugin != ""){
      return (sprintf(_("'%s' command (%s) for plugin %s is invalid!"), $type,$command,$plugin));
    }elseif($plugin != "" && $command =""){
      return (sprintf(_("'%s' command for plugin %s is invalid!"), $type,$plugin));
    }else{
      return (sprintf(_("'%s' command (%s) is invalid!"), $type,$command));
    }
  }


  public static function cmdexecfailed($type, $command = "",$plugin="")
  {
    if(empty($command)){
      return (sprintf(_("Cannot execute '%s' command!"), $type));
    }elseif($command != "" && $plugin != ""){
      return (sprintf(_("Cannot execute '%s' command (%s) for plugin %s!"), $type,$command,$plugin));
    }elseif($plugin != "" && $command =""){
      return (sprintf(_("Cannot execute '%s' command for plugin %s!"), $type,$plugin));
    }else{
      return (sprintf(_("Cannot execute '%s' command (%s)!"), $type,$command));
    }
  }


  public static function toobig($name, $min= "")
  {
    if ($min == ""){
      return (sprintf(_("Value for '%s' is too large!"), $name));
    } else {
      return (sprintf(_("'%s' must be smaller than %s!"), $name, $min));
    }
  }


  public static function toosmall($name, $min= "")
  {
    if ($min == ""){
      return (sprintf(_("Value for '%s' is too small!"), $name));
    } else {
      return (sprintf(_("'%s' must be %d or above!"), $name, $min));
    }
  }


  public static function depends($name1, $name2)
  {
    return (sprintf(_("'%s' depends on '%s' - please provide both values!"), $name1, $name2));
  }


  public static function duplicated($name)
  {
    return (sprintf(_("There is already an entry with this '%s' attribute in the system!"), $name));
  }


  public static function required($name)
  {
    return (sprintf(_("The required field '%s' is empty!"), $name));
  }


  public static function invalid($name, $data= "", $regex= "", $example= "")
  {
    /* Stylize example */
    if ($example != ""){
      $example= "<br><br><i>"._("Example").":</i> ".$example;
    }

    /* If validChars are posted, take data and paint all invalid
       characters... */
    if ($regex) {
      $result= "";
      $mismatch= "";

      mb_internal_encoding('UTF-8');
      for($i=0; $i<=mb_strlen($data);$i++){
        $currentChar= mb_substr($data, $i,1);
        if (preg_match("$regex", $currentChar)){
          $result.= $currentChar;
        } else {
          $result.= "<font style='color:red;text-decoration:underline;'>".($currentChar)."</font>";
          $mismatch.= $currentChar;
        }
      }

      return sprintf(_("The Field '%s' contains invalid characters"), $name).". ".
        (strlen($mismatch)==1?sprintf(_("'%s' is not allowed:"), $mismatch):sprintf(_("'%s' are not allowed!"), $mismatch)).
        "<br><br> \"$result\"$example";
    } else {
      return sprintf(_("The Field '%s' contains invalid characters!"), $name)."!$example";
    }
  }


  public static function missingext($name)
  {
    return sprintf(_("Missing %s PHP extension!"), $name);
  }


  public static function cancelButton()
  {
    return sprintf(_("Cancel"));
  }


  public static function okButton()
  {
    return sprintf(_("Ok"));
  }


  public static function applyButton()
  {
    return sprintf(_("Apply"));
  }


  public static function saveButton()
  {
    return sprintf(_("Save"));
  }


  public static function addButton($what= "")
  {
    return $what == "" ? sprintf(_("Add")): sprintf(_("Add %s"), $what);
  }


  public static function delButton($what= "")
  {
    return $what == "" ? sprintf(_("Delete")): sprintf(_("Delete %s"), $what);
  }


  public static function setButton($what= "")
  {
    return $what == "" ? sprintf(_("Set")): sprintf(_("Set %s"), $what);
  }


  public static function editButton($what= "")
  {
    return $what == "" ? sprintf(_("Edit...")): sprintf(_("Edit %s..."), $what);
  }


  public static function backButton($what= "")
  {
    return _("Back");
  }


  public static function buildList($data)
  {
    $objects= "<ul>";
    foreach ($data as $key => $value){
      if (is_numeric($key)){
        $objects.= "<li>\n<i>$value</i></li>";
      } else {
        $objects.= "<li>\n$value (<i>$key</i>)</li>";
      }
    }
    $objects.= "</ul>";
    return($objects);
  }

  public static function noValidExtension($name)
  {
    return sprintf(_("This account has no valid %s extensions!"), $name);
  }

  public static function featuresEnabled($name, $depends= "")
  {
    if ($depends == ""){
      return sprintf(_("This account has %s settings enabled. You can disable them by clicking below."), $name);
    } else {
      if (count($depends) == 1){
        return sprintf(_("This account has %s settings enabled. To disable them, you'll need to remove the %s settings first!"), $name, $depends);
      } else {
        $deps= "";
        foreach ($depends as $dep){
          $deps.= "$dep / ";
        }
        $deps= preg_replace("/ \/ $/", "", $deps);
        return sprintf(_("This account has %s settings enabled. To disable them, you'll need to remove the %s settings first!"), $name, $deps);
      }
    }
  }


  public static function featuresDisabled($name, $depends= "")
  {
    if ($depends == ""){
      return sprintf(_("This account has %s settings disabled. You can enable them by clicking below."), $name);
    } else {
      if (count($depends) == 1){
        return sprintf(_("This account has %s settings disabled. To enable them, you'll need to add the %s settings first!"), $name, $depends);
      } else {
        $deps= "";
        foreach ($depends as $dep){
          $deps.= "$dep / ";
        }
        $deps= preg_replace("/ \/ $/", "", $deps);
        return sprintf(_("This account has %s settings disabled. To enable them, you'll need to add the %s settings first!"), $name, $deps);
      }
    }
  }


  public static function addFeaturesButton($name)
  {
    return sprintf(_("Add %s settings"), $name);
  }


  public static function removeFeaturesButton($name)
  {
    return sprintf(_("Remove %s settings"), $name);
  }


  public static function clickEditToChange()
  {
    return _("Click the 'Edit' button below to change informations in this dialog");
  }


  public static function months()
  {
    return array(_("January"), _("February"), _("March"), _("April"),
        _("May"), _("June"), _("July"), _("August"), _("September"),
        _("October"), _("November"), _("December"));
  }


  public static function weekdays()
  {
    return array( _("Sunday"), _("Monday"), _("Tuesday"), _("Wednesday"), _("Thursday"), _("Friday"), _("Saturday"));
  }


  public static function mysqlerror($error,  $plugin= "")
  {
    /* Assign headline depending on type */
    $headline= _("MySQL operation failed!");
    return $headline."<br><br><i>"._("Error").":</i> ".$error;
  }


  public static function ldaperror($error, $dn= "", $type= 0, $plugin= "")
  {
    /* Assign headline depending on type */
    $typemap= array(1 => _("read operation"), _("add operation"), _("modify operation"),
        _("delete operation"), _("search operation"), _("authentication"));

    if (isset($typemap[$type])){
      $headline= sprintf(_("LDAP %s failed!"), $typemap[$type]);
    } else {
      $headline= _("LDAP operation failed!");
    }

    /* Fill DN information */
    if ($dn != ""){
      $dn_info= "<br><br><i>"._("Object").":</i> ".LDAP::fix($dn);
    }

    return $headline.$dn_info."<br><br><i>"._("Error").":</i> ".$error;
  }


  public static function incorrectUpload($reason= "")
  {
    if ($reason == ""){
      return _("Upload failed!");
    }

    return sprintf(_("Upload failed: %s"), "<br><br><i>$reason</i>");
  }


  public static function siError($error= "")
  {
    if ($error == ""){
      return _("Communication failure with the infrastructure service!");
    }
    return sprintf(_("Communication failure with the infrastructure service: %s"), "<br><br>"._("Error").": ".$error);
  }


  public static function stillInUse($type, $objects= array())
  {
    if (!is_array($objects)){
      return sprintf(_("This '%s' is still in use by this object: %s"), $type, "<br><br>".$objects);
    }
    if (count($objects) == 1){
      return sprintf(_("This '%s' is still in use by this object: %s"), $type, "<br>".msgPool::buildList($objects));
    }
    if (count($objects) == 0){
      return sprintf(_("This '%s' is still in use."), $type);
    }
    return sprintf(_("This '%s' is still in use by these objects: %s"), $type, "<br>".msgPool::buildList($objects));
  }


  public static function fileDoesNotExist($file)
  {
    return sprintf(_("File '%s' does not exist!"), $file);
  }


  public static function cannotReadFile($file)
  {
    return sprintf(_("Cannot open file '%s' for reading!"), $file);
  }


  public static function cannotWriteFile($file)
  {
    return sprintf(_("Cannot open file '%s' for writing!"), $file);
  }


  public static function invalidConfigurationAttribute($attr)
  {
    return sprintf(_("The value for '%s' is currently unconfigured or invalid, please check your configuration file!"), $attr);
  }


  public static function cannotDeleteFile($file)
  {
    return sprintf(_("Cannot delete file '%s'!"), $file);
  }


  public static function cannotCreateFolder($path)
  {
    return sprintf(_("Cannot create folder '%s'!"), $path);
  }


  public static function cannotDeleteFolder($path)
  {
    return sprintf(_("Cannot delete folder '%s'!"), $path);
  }


  public static function checkingFor($what)
  {
    return sprintf(_("Checking for %s support"), $what);
  }


  public static function installPhpModule($what)
  {
    return sprintf(_("Install and activate the %s PHP module."), $what);
  }


  public static function class_not_found($plugin)
  {
    return (sprintf(_("Cannot initialize class '%s'! Maybe there is a plugin missing in your FusionDirectory setup?"), $plugin));
  }


  public static function check_base()
  {
    return _("The supplied base is not valid and has been reset to the previous value!");
  }

}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
