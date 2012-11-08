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

class ogroupManagement extends management
{
  // Tab definition
  protected $tabClass = "ogrouptabs";
  protected $tabType = "OGROUPTABS";
  protected $aclCategory = "ogroups";
  protected $aclPlugin   = "ogroup";
  protected $objectName   = "object group";

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Object groups"),
      "plDescription" => _("Manage object groups"),
      "plIcon"        => "plugins/ogroups/images/plugin.png",
      "plSection"     => "admin",
      "plCategory"    => array("ogroups"),
      "plPriority"    => 4,

      "plProvidedAcls" => array()
    );
  }

  function __construct($config,$ui)
  {
    $this->config = $config;
    $this->ui = $ui;

    $this->storagePoints = array(get_ou("ogroupRDN"));

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")){
      $filter= session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path("ogroup-filter.xml", true));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("ogroup-list.xml", true));
    $headpage->registerElementFilter("filterProperties", "ogroupManagement::filterProperties");
    $headpage->setFilter($filter);

    // Add copy&paste and snapshot handler.
    if ($this->config->boolValueIsTrue("main", "copyPaste")){
      $this->cpHandler = new CopyPasteHandler($this->config);
    }
    if($this->config->get_cfg_value("enableSnapshots") == "TRUE"){
      $this->snapHandler = new SnapshotHandler($this->config);
    }
    parent::__construct($config, $ui, "ogroups", $headpage);

    $this->registerAction("edit_ogroup","editEntry");
    $this->registerAction("edit_phonequeue","editEntry");
    $this->registerAction("edit_workstartup","editEntry");
    $this->registerAction("edit_termgroup","editEntry");

    $this->registerAction("sendMessage", "sendMessage");
    $this->registerAction("saveEventDialog", "saveEventDialog");
    $this->registerAction("abortEventDialog", "closeDialogs");
  }


  // Inject user actions
  function detectPostActions()
  {
    $action = management::detectPostActions();

    if(isset($_POST['save_event_dialog'])) $action['action'] = "saveEventDialog";
    if(isset($_POST['abort_event_dialog'])) $action['action'] = "abortEventDialog";
    return($action);
  }


  function editEntry($action="",$target=array(),$all=array(), $altTabClass ="", $altTabType = "", $altAclCategory="")
  {
    $str = management::editEntry($action,$target);
    if(preg_match("/^edit_/",$action)){
      $tab = preg_replace("/^edit_/","",$action);
      if(isset($this->tabObject->by_object[$tab])){
        $this->tabObject->current = $tab;
      }else{
        trigger_error("Unknown tab: ".$tab);
      }
    }
    if(!empty($str)) return($str);
  }


  /*! \brief  Sends a message to a set of users using Argonauts events.
   */
  function saveEventDialog()
  {
    $this->dialogObject->save_object();
    $msgs = $this->dialogObject->check();
    if(count($msgs)){
      msg_dialog::displayChecks($msgs);
    }else{
      $o_queue = new gosaSupportDaemon();
      $o_queue->append($this->dialogObject);
      if($o_queue->is_error()){
        msg_dialog::display(_("Infrastructure error"), msgPool::siError($o_queue->get_error()),ERROR_DIALOG);
      }
      $this->closeDialogs();
    }
  }


  /*! \brief  Sends a message to a set of users using Argonauts events.
   */
  function sendMessage($action="",$target=array(),$all=array())
  {

    if(class_available("DaemonEvent_notify")){

      // Resolv targets
      $targets = array();
      $m_list  = array();
      $ldap = $this->config->get_ldap_link();

      // Collect selected ogroups
      foreach($target as $dn){
        $ldap->cat($dn, array('member'));
        while ($entry = $ldap->fetch()) {
          $m_list[] = $entry;
        }
      }

      // Collect object group member dns
      foreach($m_list as $entry){
        $members = $entry['member'];
        for($i=0;$i<$members['count'];$i++) {

          // Fetch member object
          $ldap->cat($members[$i], array('uid','cn','objectClass'));
          if ($ldap->count() > 0) {

            // Determine which type the object has
            $attrs = $ldap->fetch();
            if (array_search('gosaAccount', $attrs['objectClass'])) {
              $uid = $attrs['uid'][0];
              $targets['USERS'][] = $uid;
            }elseif (array_search('posixGroup', $attrs['objectClass'])) {
              $group = $attrs['cn'][0];
              $targets['GROUPS'][] = $group;
            }
          }
        }
      }

      // We've at least one recipient
      if(count($targets)){
        $events = DaemonEvent::get_event_types(USER_EVENT);
        if(isset($events['BY_CLASS']['DaemonEvent_notify'])){
          $type = $events['BY_CLASS']['DaemonEvent_notify'];
          $this->dialogObject = new $type['CLASS_NAME']($this->config);
          $this->dialogObject->add_targets($targets);
          $this->dialogObject->set_type(TRIGGERED_EVENT);
        }
      }

    }
  }

  static function filterProperties($row, $gosaGroupObjects)
  {

    $conv= array(
        "Y" => array("plugins/users/images/select_template.png",_("Templates") , "ogroup"),
        "U" => array("plugins/generic/images/head.png"        ,_("User")        , "ogroup"),
        "G" => array("plugins/groups/images/groups.png"       ,_("Group")       , "ogroup"),
        "A" => array("plugins/ogroups/images/application.png" ,_("Application") , "ogroup"),
        "D" => array("plugins/departments/department.png"     ,_("Department")  , "ogroup"),
        "S" => array("plugins/ogroups/images/server.png"      ,_("Server")      , "ogroup"),
        "F" => array("plugins/ogroups/images/asterisk.png"    ,_("Phone")       , "phonequeue"),
        "W" => array("plugins/ogroups/images/workstation.png" ,_("Workstation") , "workstartup"),
        "O" => array("plugins/ogroups/images/winstation.png"  ,_("Windows Install") , "ogroup"),
        "T" => array("plugins/ogroups/images/terminal.png"    ,_("Terminal")    , "termgroup"),
        "P" => array("plugins/ogroups/images/printer.png"     ,_("Printer")     , "ogroup"));

    $types = preg_replace("/[^a-z]/i","",$gosaGroupObjects[0]);
    $result ="";
    for($i = 0 ; $i < strlen($types); $i++){
      $type = $types[$i];
      $result.="<input class='center' type='image' src='".$conv[$type][0]."' ".
        "alt='".$conv[$type][1]."' title='".$conv[$type][1].
        "' name='listing_edit_".$conv[$type][2]."_$row' style='padding:1px'>";

    }
    return($result);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
