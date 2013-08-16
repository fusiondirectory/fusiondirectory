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

class ogrouptabs extends simpleTabs_noSpecial
{
  var $base = "";
  var $acl_category;

  private $groupObjects = NULL;

  function __construct($config, $data, $dn, $category = "ogroups", $copied_object = NULL)
  {
    parent::__construct($config, $data, $dn, $category, $copied_object);
    $this->base         = $this->by_object['ogroup']->base;
    $this->acl_category = $category;

    $this->loadtabs($this->getBaseObject()->gosaGroupObjects);
  }

  function loadtabs($gosaGroupObjects)
  {
    if ($this->groupObjects == $gosaGroupObjects) {
      return;
    }
    $this->groupObjects = $gosaGroupObjects;
    $objects = preg_replace('/[\[\]]/', '', $gosaGroupObjects);

    $tabs = array();
    if (strlen($objects)) {
      $dtabs  = $this->config->data['TABS']['OGROUP-DYNAMICTABS'];
      $tabs   = FALSE;
      for ($i = 0; $i < strlen($objects); $i++) {
        $otype = ogroup::$objectTypes[$objects[$i]];
        $otabs = array_merge(
          array_uintersect(
            $dtabs,
            $this->config->data['TABS'][strtoupper($otype).'TABS'],
            function ($a, $b)
            {
              return strcmp($a['CLASS'], $b['CLASS']);
            }
          ),
          $this->config->data['TABS']['OGROUP-'.strtoupper($otype).'TABS']
        );
        if ($tabs === FALSE) {
          $tabs = $otabs;
        } else {
          $tabs = array_uintersect(
            $tabs,
            $otabs,
            function ($a, $b)
            {
              return strcmp($a['CLASS'], $b['CLASS']);
            }
          );
        }
      }
    }
    $tabs = array_merge($this->config->data['TABS']['OGROUPTABS'], (array)$tabs);
    $tabs = array_map(function ($a) { return $a['CLASS']; }, $tabs);

    foreach ($this->by_object as $tab => $object) {
      if (!in_array($tab, $tabs)) {
        $this->delTab($tab);
      }
    }
    foreach ($tabs as $class) {
      if (class_available($class) && !isset($this->by_object["$class"])) {
        $this->addTab($class);
      }
    }
    $this->addSpecialTabs();
  }

  function addTab ($class)
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $class, "Adding tab ");
    $plInfos = pluglist::pluginInfos($class);
    $this->by_name["$class"]            = $plInfos['plShortName'];
    $this->by_object["$class"]          = new $class($this->config, $this->dn, $this->getBaseObject());
    $this->by_object["$class"]->parent  = &$this;
    $this->by_object["$class"]->acl     = "#all#";
    $this->by_object["$class"]->set_acl_category($this->acl_category);
  }

  function delTab ($class)
  {
    $this->by_object["$class"]->remove_from_parent();
    unset($this->by_object["$class"]);
    unset($this->by_name["$class"]);
  }

  function gen_tabs($disabled = FALSE)
  {
    $this->loadtabs($this->getBaseObject()->getGroupObjectTypes());
    return parent::gen_tabs($disabled);
  }

  function check($ignore_account = FALSE)
  {
    return parent::check(FALSE);
  }

  function save_object($save_current = FALSE)
  {
    parent::save_object($save_current);

    /* Update reference, transfer variables */
    $baseobject = $this->getBaseObject();
    foreach ($this->by_object as $name => $obj) {

      /* Don't touch base object */
      if ($name != 'ogroup') {
        $obj->parent    = &$this;
        $obj->uid       = $baseobject->cn;
        $obj->cn        = $baseobject->cn;
        $obj->sn        = $baseobject->cn;
        $obj->givenName = $baseobject->cn;

        $this->by_object[$name] = $obj;
      }
    }

    /* Update parent in base object */
    $this->by_object['ogroup']->parent = &$this;
  }
}

class ogrouptabs_oldfuncs
{
  var $dynamic_tabs;
  function oldcode() {
    /* Add references/acls/snapshots */
    $this->dynamic_tabs = array(
      "/T/" => array(
        'termservice'     => _('Devices'),
        'termstartup'     => _('Startup'),
      ),
      "/[WT]/" => array(
        'termgroup'       => _('Systems'),
      ),
      "/W/" => array(
        'workservice'     => _('Devices'),
      ),
      "/[SW]/" => array(
        'workstartup'     => _('Workstartup'),
        'debconfStartup'  => _('Debconf'),
        'faiStartup'      => _('FAI'),
        'faiSummary'      => _('FAI summary'),
        'argonautClient'  => _('Argonaut client'),
      ),
    );
  }
  function reload($dd)
  {
    $objects = preg_replace('/[\[\]]/', '', $dd);
    $baseobject = $this->getBaseObject();

    /* Check if we have a group with a set different mixed objects.
     */
    $mixed_type = FALSE;
    for ($i = 0 ; $i < (strlen($objects) -1 );$i++) {
      $mixed_type |= $objects[$i] != $objects[($i+1)];
    }

    /* If there is a phonequeue,
     * but there is no user left with goPhoneAccount ... remove it.
     */
    $usePhoneTab = false;
    if (class_available("phonequeue")) {

      foreach($baseobject->memberList as $dn => $val){
        if(isset($baseobject->objcache[$dn])){
          $obj = $baseobject->objcache[$dn];
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

    /* Create goPhoneAccount if theres an user with goPhoneAccount
     * but only if there is currently no queue enabled.
     */
    if(class_available("phonequeue")){
      if(!isset($this->by_object['phonequeue'])){
        foreach($baseobject->memberList as $dn => $val){
          if(isset($baseobject->objcache[$dn])){
            $obj = $baseobject->objcache[$dn];
            if(isset($obj['objectClass'])){
              if(in_array("goFonAccount",$obj['objectClass'])){
                $this->by_name['phonequeue'] = _("Phone queue");
                $this->by_object['phonequeue'] = new phonequeue($this->config, $this->dn, $baseobject);
                $this->by_object['phonequeue']->parent = &$this;
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
          $this->by_object['mailogroup']= new mailogroup($this->config, $this->dn, $baseobject);
          $this->by_object['mailogroup']->parent= &$this;
        }
      }
    }

    /* Add Workstation tabs */
    if (!$mixed_type) {
      foreach ($this->dynamic_tabs as $pattern => $tabs) {
        if (preg_match($pattern,$objects)) {
          foreach ($tabs as $class => $name) {
            if (class_available($class) && !isset($this->by_object["$class"])) {
              $this->addTab($class, $name);
            }
          }
        } else {
          foreach ($tabs as $class => $name) {
            if (class_available($class) && isset($this->by_object["$class"])) {
              $this->by_object["$class"]->remove_from_parent();
              unset($this->by_object["$class"]);
              unset($this->by_name["$class"]);
            }
          }
        }
      }
      if (isset($this->by_object['workservice'])) {
        $this->by_object['workservice']->inheritTimeServer = false;
      }
      if (isset($this->by_object['termgroup'])) {
        $this->by_object['termgroup']->inheritTimeServer = false;
      }
    }

    foreach($this->by_object as $name => $obj){
      $this->by_object[$name]->set_acl_category($this->acl_category);
    }
  }
}

?>
