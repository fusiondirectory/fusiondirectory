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

class groupManagement extends management
{
  var $plHeadline     = "Groups";
  var $plDescription  = "Group administration";
  var $plIcon  = "plugins/groups/images/plugin.png";

  // Tab definition 
  protected $tabClass = "grouptabs";
  protected $tabType = "GROUPTABS";
  protected $aclCategory = "groups";
  protected $aclPlugin   = "group";
  protected $objectName   = "group";

  function __construct($config,$ui)
  {
    $this->config = $config;
    $this->ui = $ui;
   
    $this->storagePoints = array(get_ou("groupRDN"));
 
    // Build filter
    if (session::global_is_set(get_class($this)."_filter")){
      $filter= session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path("group-filter.xml", true));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("group-list.xml", true));
    $headpage->registerElementFilter("filterProperties", "groupManagement::filterProperties");
    $headpage->setFilter($filter);

    // Add copy&paste and snapshot handler.
    if ($this->config->boolValueIsTrue("main", "copyPaste")){
      $this->cpHandler = new CopyPasteHandler($this->config);
    }
    if($this->config->get_cfg_value("enableSnapshots") == "true"){
      $this->snapHandler = new SnapshotHandler($this->config);
    }
    parent::__construct($config, $ui, "groups", $headpage);

    $this->registerAction("edit_group","editEntry");
    $this->registerAction("edit_environment","editEntry");
    $this->registerAction("edit_appgroup","editEntry");
    $this->registerAction("edit_mailgroup","editEntry");

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


  /*! \brief  Sends a message to a set of users using goto-si events.
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


  /*! \brief  Sends a message to a set of users using goto-si events.
   */
  function sendMessage($action="",$target=array(),$all=array())
  {
    $uids = array();
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    foreach($target as $dn){
      $ldap->cat($dn,array('cn'));
      if($ldap->count()){
        $attrs = $ldap->fetch();
        $uids[] = $attrs['cn'][0];
      }
    }
    if(count($uids)){
      $events = DaemonEvent::get_event_types(USER_EVENT);
      $event = "DaemonEvent_notify";
      if(isset($events['BY_CLASS'][$event])){
        $type = $events['BY_CLASS'][$event];
        $this->dialogObject = new $type['CLASS_NAME']($this->config);
        $this->dialogObject->add_groups($uids);
        $this->dialogObject->set_type(SCHEDULED_EVENT);
      }
    }
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

  static function filterProperties($row, $classes)
  {
    $result= "";

    $map = array(
        "posixGroup" => 
        array(
          "image" => "plugins/groups/images/groups.png",
          "plugin" => "group",
          "alt" => _("Posix"),
          "title" => _("Edit posix properties")
          ),

        "gosaMailAccount" => 
        array(
          "image" => "plugins/groups/images/mail.png",
          "plugin" => "mailgroup",
          "alt" => _("Mail"),
          "title" => _("Edit mail properties")
          ),

        "sambaGroupMapping" => 
        array(
          "image" => "plugins/groups/images/samba.png",
          "plugin" => "group",
          "alt" => _("Samba"),
          "title" => _("Edit samba properties")
          ),

        "goFonPickupGroup" => 
          array(
              "image" => "plugins/groups/images/asterisk.png",
              "plugin" => "group",
              "alt" => _("Phone"),
              "title" => _("Edit phone properties")
              ),

        "gotoMenuGroup" => 
          array(
              "image" => "plugins/groups/images/menu.png",
              "plugin" => "appgroup",
              "alt" => _("Menu"),
              "title" => _("Edit start menu properties")
              ),

        "gotoEnvironment" => 
          array(
              "image" => "plugins/groups/images/environment.png",
              "plugin" => "environment",
              "alt" => _("Environment"),
              "title" => _("Edit environment properties")
              )
          );


    // Walk thru map
    foreach ($map as $oc => $properties) {
      if (in_array_ics($oc, $classes)) {
        $result.="<input class='center' type='image' src='".$properties['image']."' ".
          "alt='".$properties['alt']."' title='".$properties['title'].
                 "' name='listing_edit_".$properties['plugin']."_$row' style='padding:1px'>";
      } else {
        $result.="<img src='images/empty.png' alt=' ' class='center' style='padding:1px'>";
      }
    }
    return $result;
  }
} 
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
