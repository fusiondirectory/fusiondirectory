<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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
 * \file class_msgPool.inc
 * Source code for class msgPool
 */

define ("LDAP_READ",   1);
define ("LDAP_ADD",    2);
define ("LDAP_MOD",    3);
define ("LDAP_DEL",    4);
define ("LDAP_SEARCH", 5);
define ("LDAP_AUTH",   6);

/*!
 * \brief This class contains all the messages for the various actions
 */
class msgPool {

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

  /*!
   * \brief Display the deleted informations
   *
   * \param string $name Name of the object which will be deleted
   *
   * \param string $type Type of the object which will be deleted
   */
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

  /*!
   * \brief Display that we have no permission to delete an object
   *
   * \param string $name Name of the object which will be deleted
   */
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

  /*!
   * \brief Display that we have no permission to create an object
   *
   * \param string $name Name of the object which will be created
   */
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

  /*!
   * \brief Display that we have no permission to modify an object
   *
   * \param string $name Name of the object which will be modified
   */
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

  /*!
   * \brief Display that we have no permission to view an object
   *
   * \param string $name Name of the object which will be viewed
   */
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

  /*!
   * \brief Display that we have no permission to move an object
   *
   * \param string $name Name of the object which will be moved
   */
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

  /*!
   * \brief Display informations about database connection
   *
   * \param string $name Name of the database
   *
   * \param string $error Error messsage to display
   *
   * \param string $dbinfo Database information
   */
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

   /*!
   * \brief Display informations about database select
   *
   * \param string $name Name of the database
   *
   * \param string $error Error messsage to display
   *
   * \param string $dbinfo Database information
   */
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

  /*!
   * \brief Display error about no server found
   *
   * \param string $name Name of the server
   */
  public static function noserver($name)
  {
    return (sprintf(_("No %s server defined!"), $name));
  }

   /*!
   * \brief Display informations about database query
   *
   * \param string $name Name of the database
   *
   * \param string $error Error messsage to display
   *
   * \param string $dbinfo Database information
   */
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

  /*!
   * \brief Display field contains reserved keyword
   *
   * \param string $name The field which contains reserved keyword
   */
  public static function reserved($name)
  {
    return (sprintf(_("The field '%s' contains a reserved keyword!"), $name));
  }

  /*!
   * \brief Display that a command doesn't exist in this plugin
   *
   * \param string $type Command type
   *
   * \param string $plugin Name of the plugin
   */
  public static function cmdnotfound($type, $plugin)
  {
    return (sprintf(_("Command specified as %s hook for plugin '%s' does not exist!"), $type, $plugin));
  }

  /*!
   * \brief Display that a command doesn't valid in this plugin
   *
   * \param string $type Command type
   *
   * \param string $command Command name
   *
   * \param string $plugin Name of the plugin
   */
  public static function cmdinvalid($type, $command = "",$plugin="")
  {
    if(empty($command)){
      return (sprintf(_("'%s' command is invalid!"), $type));
    }elseif($command != "" && $plugin != ""){
      return (sprintf(_("'%s' command (%s) for plugin %s is invalid!"), $type,$command,$plugin));
    }elseif($plugin != "" && $command ==""){
      return (sprintf(_("'%s' command for plugin %s is invalid!"), $type,$plugin));
    }else{
      return (sprintf(_("'%s' command (%s) is invalid!"), $type,$command));
    }
  }

  /*!
   * \brief Display that a command execution failed in this plugin
   *
   * \param string $type Command type
   *
   * \param string $command Command name
   *
   * \param string $plugin Name of the plugin
   */
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

  /*!
   * \brief Display error about too larged value
   *
   * \param string $name Name of the value
   *
   * \param string $min The largest value
   */
  public static function toobig($name, $min= "")
  {
    if ($min == ""){
      return (sprintf(_("Value for '%s' is too large!"), $name));
    } else {
      return (sprintf(_("'%s' must be smaller than %s!"), $name, $min));
    }
  }

  /*!
   * \brief Display error about too small value
   *
   * \param string $name Name of the value
   *
   * \param string $min The smallest value
   */
  public static function toosmall($name, $min= "")
  {
    if ($min == ""){
      return (sprintf(_("Value for '%s' is too small!"), $name));
    } else {
      return (sprintf(_("'%s' must be %d or above!"), $name, $min));
    }
  }

  /*!
   * \brief Display a dependence between two objects
   *
   * \param string $name1 First object
   *
   * \param string $name2 Second object
   */
  public static function depends($name1, $name2)
  {
    return (sprintf(_("'%s' depends on '%s' - please provide both values!"), $name1, $name2));
  }

  /*!
   * \brief Display error about existing entry in the system
   *
   * \param string $name The entry duplicated
   */
  public static function duplicated($name)
  {
    return (sprintf(_("There is already an entry with this '%s' attribute in the system!"), $name));
  }

  /*!
   * \brief Display error about required field empty
   *
   * \param string $name Name of the field
   */
  public static function required($name)
  {
    return (sprintf(_("The required field '%s' is empty!"), $name));
  }

  /*!
   * \brief Display error about invalid characters
   *
   * \param string $name The field name
   *
   * \param string $data The submited data
   *
   * \param string $regex
   *
   * \param string $example Example of a right submited data
   */
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
          $result.= "<div style='color:red;text-decoration:underline;'>".($currentChar)."</div>";
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

  /*!
   * \brief Display about missing PHP extension
   *
   * \param string $name The name of the extension
   */
  public static function missingext($name)
  {
    return sprintf(_("Missing %s PHP extension!"), $name);
  }

  /*!
   * \brief Text for a cancel button
   */
  public static function cancelButton()
  {
    return sprintf(_("Cancel"));
  }

  /*!
   * \brief Text for a ok button
   */
  public static function okButton()
  {
    return sprintf(_("Ok"));
  }

  /*!
   * \brief Text for an apply button
   */
  public static function applyButton()
  {
    return sprintf(_("Apply"));
  }

  /*!
   * \brief Text for a save button
   */
  public static function saveButton()
  {
    return sprintf(_("Save"));
  }

  /*!
   * \brief Text for an add button
   *
   * \param string $what Text for the button
   */
  public static function addButton($what= "")
  {
    return $what == "" ? sprintf(_("Add")): sprintf(_("Add %s"), $what);
  }

  /*!
   * \brief Text for an delete button
   *
   * \param string $what Text for the button
   */
  public static function delButton($what= "")
  {
    return $what == "" ? sprintf(_("Delete")): sprintf(_("Delete %s"), $what);
  }

  /*!
   * \brief Text for an set button
   *
   * \param string $what Text for the button
   */
  public static function setButton($what= "")
  {
    return $what == "" ? sprintf(_("Set")): sprintf(_("Set %s"), $what);
  }

  /*!
   * \brief Text for an edit button
   *
   * \param string $what Text for the button
   */
  public static function editButton($what= "")
  {
    return $what == "" ? sprintf(_("Edit...")): sprintf(_("Edit %s..."), $what);
  }

  /*!
   * \brief Text for a back button
   */
  public static function backButton($what= "")
  {
    return _("Back");
  }

  /*!
   * \brief a list from an array
   *
   * \param array $data Array with the elements of the list
   */
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

  /*!
   * \brief Display error about invalid extension from account
   *
   * \param string $name Name of the extension
   */
  public static function noValidExtension($name)
  {
    return sprintf(_("This account has no valid %s extensions!"), $name);
  }

  /*!
   * \brief List the features settings enabled
   *
   * \param string $name Name of the setting
   *
   * \param array $depends Contains all the settings enabled
   */
  public static function featuresEnabled($name, $depends= "")
  {
    if (($depends == "") || (is_array($depends) && count($depends) == 0)) {
      return sprintf(_("This account has %s settings enabled. You can disable them by clicking below."), $name);
    } else {
      if (count($depends) == 1) {
        if (is_array($depends)) {
          $depends = $depends[0];
        }
        return sprintf(_("This account has %s settings enabled. To disable them, you'll need to remove the %s settings first!"), $name, $depends);
      } else {
        $deps = "";
        foreach ($depends as $dep) {
          $deps .= "$dep / ";
        }
        $deps = preg_replace("/ \/ $/", "", $deps);
        return sprintf(_("This account has %s settings enabled. To disable them, you'll need to remove the %s settings first!"), $name, $deps);
      }
    }
  }

  /*!
   * \brief List the features settings disabled
   *
   * \param string $name Name of the setting
   *
   * \param array $depends Contains all the settings disabled
   */
  public static function featuresDisabled($name, $depends= "")
  {
    if (($depends == "") || (is_array($depends) && count($depends) == 0)) {
      return sprintf(_("This account has %s settings disabled. You can enable them by clicking below."), $name);
    } else {
      if (count($depends) == 1) {
        if (is_array($depends)) {
          $depends = $depends[0];
        }
        return sprintf(_("This account has %s settings disabled. To enable them, you'll need to add the %s settings first!"), $name, $depends);
      } else {
        $deps = "";
        foreach ($depends as $dep) {
          $deps .= "$dep / ";
        }
        $deps = preg_replace("/ \/ $/", "", $deps);
        return sprintf(_("This account has %s settings disabled. To enable them, you'll need to add the %s settings first!"), $name, $deps);
      }
    }
  }

  /*!
   * \brief Display Add features button
   *
   * \param string $name Name of the feature
   */
  public static function addFeaturesButton($name)
  {
    return sprintf(_("Add %s settings"), $name);
  }

  /*!
   * \brief Display Remove features button
   *
   * \param string $name Name of the feature
   */

  public static function removeFeaturesButton($name)
  {
    return sprintf(_("Remove %s settings"), $name);
  }

  /*!
   * \brief Display : Click the 'Edit' button below to change informations in this dialog
   */
  public static function clickEditToChange()
  {
    return _("Click the 'Edit' button below to change informations in this dialog");
  }

  /*!
   * \brief Build an array with the months
   */
  public static function months()
  {
    return array(_("January"), _("February"), _("March"), _("April"),
        _("May"), _("June"), _("July"), _("August"), _("September"),
        _("October"), _("November"), _("December"));
  }

  /*!
   * \brief Build an array with the days of a week.
   *    Start by Sunday
   */
  public static function weekdays()
  {
    return array( _("Sunday"), _("Monday"), _("Tuesday"), _("Wednesday"), _("Thursday"), _("Friday"), _("Saturday"));
  }

  /*!
   * \brief Display error about database
   *
   * \param string $error Error to display
   *
   * \param string $plugin
   */
  public static function dbError($error,  $plugin= "")
  {
    /* Assign headline depending on type */
    $headline= _("Database operation failed!");
    return $headline."<br><br><i>"._("Error").":</i> ".$error;
  }

  /*!
   * \brief Display LDAP error
   *
   * \param string $error Error to display
   *
   * \param string $dn the DN
   *
   * \param integer $type
   *
   * \param string $plugin
   */
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

  /*!
   * \brief Display error about an incorrect upload
   *
   * \param string $reason The reason of the upload failed
   */
  public static function incorrectUpload($reason= "")
  {
    if ($reason == ""){
      return _("Upload failed!");
    }

    return sprintf(_("Upload failed: %s"), "<br><br><i>$reason</i>");
  }

  /*!
   * \brief Display error about communication failure with the infrastructure service
   *
   * \param string $error The error of the communication failure
   */
  public static function siError($error= "")
  {
    if ($error == ""){
      return _("Communication failure with the infrastructure service!");
    }
    return sprintf(_("Communication failure with the infrastructure service: %s"), "<br><br>"._("Error").": ".$error);
  }

  /*!
   * \brief Display which it still in use
   *
   * \param $type Type
   *
   * \param array $objects
   */
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

  /*!
   * \brief Display file doesn't exist
   *
   * \param string $file Name of the file not found
   */
  public static function fileDoesNotExist($file)
  {
    return sprintf(_("File '%s' does not exist!"), $file);
  }

  /*!
   * \brief Display file isn't readable
   *
   * \param string $file Name of the file not readable
   */
  public static function cannotReadFile($file)
  {
    return sprintf(_("Cannot open file '%s' for reading!"), $file);
  }

  /*!
   * \brief Display file isn't writable
   *
   * \param string $file Name of the file not writable
   */
  public static function cannotWriteFile($file)
  {
    return sprintf(_("Cannot open file '%s' for writing!"), $file);
  }

  /*!
   * \brief Display error in the configuration
   *
   * \param string $attr Name of the invalide attribute
   */
  public static function invalidConfigurationAttribute($attr)
  {
    return sprintf(_("The value for '%s' is currently unconfigured or invalid, please check your configuration file!"), $attr);
  }

  /*!
   * \brief Display error when cannot delete a file
   *
   * \param string $file Filename which can't be deleted
   */
  public static function cannotDeleteFile($file)
  {
    return sprintf(_("Cannot delete file '%s'!"), $file);
  }

  /*!
   * \brief Display error when cannot create a folder
   *
   * \param string $path Name of folder which can't be created
   */
  public static function cannotCreateFolder($path)
  {
    return sprintf(_("Cannot create folder '%s'!"), $path);
  }

  /*!
   * \brief Display error when cannot delete a folder
   *
   * \param string $path Name of folder which can't be deleted
   */
  public static function cannotDeleteFolder($path)
  {
    return sprintf(_("Cannot delete folder '%s'!"), $path);
  }

  /*!
   * \brief Display checking for a support
   *
   * \param string $what Name of the support
   */
  public static function checkingFor($what)
  {
    return sprintf(_("Checking for %s support"), $what);
  }

  /*!
   * \brief Display install or activate a PHP module
   *
   * \param string $what Name of the module
   */
  public static function installPhpModule($what)
  {
    return sprintf(_("Install and activate the %s PHP module."), $what);
  }

  /*!
   * \brief Display install or activate a PEAR module
   *
   * \param string $what Name of the module
   */
  public static function installPearModule($what)
  {
    return sprintf(_("Install and activate the %s Pear module."), $what);
  }

  /*!
   * \brief Display error when a class is not found
   *
   * \param string $plugin Name of the class which can'b be initialized
   */
  public static function class_not_found($plugin)
  {
    return (sprintf(_("Cannot initialize class '%s'! Maybe there is a plugin missing in your FusionDirectory setup?"), $plugin));
  }

  /*!
   * \brief Display error when checking the base
   */
  public static function check_base()
  {
    return _("The supplied base is not valid and has been reset to the previous value!");
  }

}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
