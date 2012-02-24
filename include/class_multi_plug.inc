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

/*
Data structure : 
================ 
  |->o_tab                      <-- dummy object, collects HTML posts, displays ui 
  |->a_handles                  <-- tab object for each given dn
     |->tab object for dn 1 
     |->tab object for dn 2
      ...
     |->tab object for dn n


Other functions implemented:
============================

CLASS tab
  - multiple_support_available()  Check if there is at least one plugin with 
                                  enabled multiple edit support
  - enable_multiple_support()     Enable multiple edit, for this tab.
  
CLASS plugin
  - enable_multiple_support()     Enable multiple support for this plugin.
  - init_multiple_support()       Init summy object, to preset some values.
  - multiple_execute()            Display dummy object ui.
  - multiple_save_object()        Get posted values in multiple edit mode.
  - multiple_check()              Check values specified in dummy object.
  - get_multi_edit_values()       Get values changed from dummy object.
  - set_multi_edit_values()       Set values collected with get_multi_edit_values.
                                  to all objects currently edited.


Process:
========

multi_plug::multi_plug()
  |->o_tab = new tab()                      #Initialize ui handle
  |
  |->handles
  | |->handles[] = new tab()                #Initialize objects we want to edit at once
  |
  |->o_tab->enable_multiple_support()       #Enable multiple support for ui handle
  |->detect_multiple_used_attributes()      #Update ui handle with some default values
    |->handles[]
      |->by_object->get_multi_init_values() #Get attributes from all handles
    |->o_tab
      |->by_object->init_multiple_support() #Assign values to ui handle
  |
  |->execute()                              #Display ui
  | |->o_tab->execute()
      |->by_object->multiple_execute()
  |
  |->check()                                #Check given values
    |->o_tab->check()
      |->by_object->multiple_check()
  |
  |->save_object()                          #Save posts 
    |->o_tab->save_object()
      |->by_object->multiple_save_object()
  |
  |->save()                                 #Save collected values 
    |->populate_values()                    #Populate values to all handles
      |->o_tab->get_multi_edit_values()     #Get values to populate
      |->handles->set_multi_edit_values()   #Set values 
    |->handles->save()                      #Save handles
  



Using this class: 
=================
  Simple Example:

    $dn   = array(dn1,dn2,dn3);
    $tmp  = new multi_plug($config,"usertabs",$config->data['TABS']['USERTABS'],$dn);
    echo $tmp->execute();

  $tmp can now be used like the normal tab class, execute, save_object ...
  
  To enable multipe edit for a specific plugin, 
  just set the plugin variable 'multiple_support' to true:

    var $multiple_support = TRUE;

  If plugin::multiple_support is true, the member function 
  multiple_execute() will be called and displayed, NOT execute().

  (I will put this in the wiki, later. This are just notes for me.)
  
*/

/*! \brief   This class edits multiple tab objects at once
 *
 * \author  Fabian Hickert
 * \version 1.01
 * \date    2007/12/07
 * 
 * This class edits multiple tab objects at once. 
 * 1. There is a dummy tab object initialized that collects the user input.
 * 2. All given objects specified by '$dn' will be initialized and the collected
 * data from the dummy object will be populated to them.
 */
class multi_plug
{
  /* Tab handler for each given dn entry */
  public $a_handles = array();

  /* Dummy handler which collects the data */
  private $o_tab     = NULL;  

  public $dn      = array();
  public $config  = NULL;
  private $s_class= "";
  public $current = "";
  public $by_object = array();
  public $by_name = array();

  /*! \brief    Creates a multi_plug object
     @param   object  $config FusionDirectory Configuration object
     @param   string  $class  The class name of the tab we want to edit. e.g. usertabs
     @param   string  $tab    The config tab name e.g. USERTABS 
     @param   array   $dns    The object dns we want to edit.
     @return  object  multi_plug
   */
  public function __construct($config,$class,$tab,$dns,$acl_base,$acl_category)
  {
    if(!count($dns)){
      return;
    }

    $this->dn       = $dns;
    $this->config   = $config;
    $this->s_class  = $class;

    /* Initialize collector object 
     * Used to display the ui and to collect the user input.
     */
    $this->o_tab    = new $class($config,$tab,"new",$acl_category, TRUE, TRUE);
    $this->o_tab->set_acl_base($acl_base);
    $this->by_object = &$this->o_tab->by_object;
    $this->by_name   = &$this->o_tab->by_name;
    $this->current   = &$this->o_tab->current;

    /* Check if the specified tab object supports multiple edits 
     */
    if($this->o_tab->multiple_support_available()){

      /* Enable multiple actions for the collector object 
       */ 
      $this->o_tab->enable_multiple_support();

      /* Initialize the objects we want to edit at once 
       */
      foreach($dns as $dn){
        $obj = new $class($config,$tab,$dn,$acl_category, TRUE, TRUE);
        $obj->set_acl_base($acl_base);
        $this->a_handles[] = $obj;
      }
    }

    /* Detect attributes used by all plugins and set 
     *  those values as default in edit handle 
     */
    $this->detect_multiple_used_attributes();
  }


  /*! \brief    Combine two ldap result arrays. 
   * @param     array   $base   Base array 
   * @param     array   $add    Array to add
   * @returns   array   Combination of $base and $add
   */  
  private function array_combine($base,$add)
  {

    foreach($add as $key => $attr) {
      if(!is_numeric($key)){
  
        if(!is_array($add[$key])){
          $add[$key] = array('count' => 1,$add[$key]);
        }

        if(!isset($base[$key])){
          $base[$key] = $add[$key]; 
        }else{

          if(!isset($base[$key]['count'])){
            $base[$key]['count'] = count($base[$key]);
          }

          if(!isset($add[$key]['count'])){
            $add[$key]['count'] = count($add[$key]);
          }
          for($i=0;$i<$add[$key]['count'];$i++){
            if(!in_array($add[$key][$i],$base[$key])){
              $base[$key][] = $add[$key][$i];
              $base[$key]['count']++;
            }
          }
        }
      }
    }
    return($base);
  }


  /*! \brief    Intersect two ldap result arrays/Inner join of two ldap result arrays
   * @param     array   $base   Base array 
   * @param     array   $minus  Array number two
   * @returns   array   Result intersection
   */  
  private function array_intersect($base,$minus)
  {
    foreach($base as $key => $entry){
      if(is_numeric($key) || !isset($minus[$key])){
        unset($base[$key]);
      }elseif(gettype($base[$key]) != gettype($minus[$key])){
        unset($base[$key]);
      }elseif(is_string($base[$key]) && $base[$key]!=$minus[$key]){
        unset($base[$key]);
      }elseif(is_array($base[$key])){
        $tmp = array();
          if(!isset($base[$key]['count'])){
            $base[$key]['count'] = count($base[$key]);
          }
        for($i = 0 ; $i < $base[$key]['count'] ; $i ++){
          if(isset($base[$key][$i]) && in_array($base[$key][$i],$minus[$key])){
            $tmp[] = $base[$key][$i];
          }
        }
        if(count($tmp)){
          $tmp['count'] = count($tmp);
          $base[$key] = $tmp;
        }else{
          unset($base[$key]);
        }
      }
    }
    return($base);
  }

  
  /*! \brief    Detect values that are used in all edited objects.
   */  
  private function detect_multiple_used_attributes()
  {
    foreach($this->o_tab->by_object as $name => $plug){

      if(empty($name))    continue;

      $attrs = array();
      $all   = array();
      foreach($this->a_handles as $hid => $handle){
        $h_attrs = $this->a_handles[$hid]->by_object[$name]->get_multi_init_values();
        if(count($attrs) == 0){
          $attrs = $h_attrs;
        }else{
          $attrs = $this->array_intersect($attrs,$h_attrs);
        }
        $all = $this->array_combine($all,$h_attrs);
      }
      $this->o_tab->by_object[$name]->init_multiple_support($attrs,$all);
    }
  }


  /*! \brief    Returns the edit ui for multiple edit.
     @return    string  HTML User interface for given tab object.
   */
  public function execute()
  {
    $str = $this->o_tab->execute();
    return($str);
  }


  /*! \brief    Checks if one of the objects we want to edit is locked. 
     @return    boolean   Returns TRUE if at least one entry is locked, else false.
   */
  public function entries_locked()
  {
    $ui = get_userinfo();
    foreach($this->dn as $dn){
      if(get_lock($dn) != ""){
        return(TRUE);
      }
    }
    return(FALSE);
  }


  /*! \brief    Generates a lock message that can be displayed if an entry is locked.
     @return    string  Returns a list of locked entries 
   */
  public function display_lock_message()
  {
    $ui = get_userinfo();
    $lock_msg = "";
    $lock_msg.=  gen_locked_message ($ui->dn, $this->dn);
    return($lock_msg);
  }


  /*! \brief    Locks all currently managed objects (array $this->dn) 
     @return    boolean   Returns TRUE
   */
  public function lock_entries($uid)
  {
    foreach($this->dn as $dn)
    add_lock($dn,$uid);
    return(TRUE);
  }


  /*! \brief    Checks if the given tab object supports multiple edit.
     @return    boolean   Returns TRUE if the given tab objects supports multiple edit else FALSE.
   */
  public function multiple_available()
  { 
    if(isset($this->o_tab) && is_object($this->o_tab)){
      return($this->o_tab->multiple_support_available());
    }else{
      return(FALSE);
    }
  }


  /*! \brief    Sets the currently active tab. The tab that will be displayed by $this->execute(). 
   */
  public function set_active_tab($str)
  {
    $this->current = $str;
  }


  /*! \brief    Returns the object info string, that can be displayed in the tab header.
      @return   string  Returns an information string, containing the list of currently edited dns.
   */
  public function get_object_info()
  {
    return(_("You are currently editing mutliple entries."));
  }


  /*! \brief    Handles all HTML posts from the dummy tab object. 
   */
  public function save_object()
  {
    $this->o_tab->save_object(); 
  }


  /*! \brief    Checks if the values fetched by $this->save_object() are valid.
      @param    array Returns an array containig all error messages.
   */
  public function check()
  {
    $messages = $this->o_tab->check();
    return($messages);
  }


  /*! \brief    Currently not implemented, later be used to trigger password changes. 
      @param    boolean Returns TRUE if a password change is needed.
   */
  public function password_change_needed()
  {
    foreach($this->a_handles as $i_id => $o_handle){
      if($o_handle->password_change_needed() && isset($o_handle->by_object['user'])){
        new msg_dialog(_("Password reset"),_("The user password was resetted, please set a new password value!"),WARNING_DIALOG);
        change_password ($o_handle->dn, "",0, $o_handle->by_object['user']->pw_storage);
      }
    }
    return(FALSE);
  }


  /*! \brief    Populate all collected values to the target tab objects ($this->o_handles)
   */
  public function populate_values()
  {
    if($this->multiple_available() && is_array($this->a_handles)){
      foreach($this->o_tab->by_object as $name => $obj){

        $values = $this->o_tab->by_object[$name]->get_multi_edit_values();
        foreach($this->a_handles as $i_id => $o_handle){
          $this->a_handles[$i_id]->by_object[$name]->set_multi_edit_values($values);
        }
      }
    }
  }


  /*! \brief    Save all edited tab objects ($this->o_handles). 
   */
  public function save()
  {
    if($this->multiple_available() && is_array($this->a_handles)){
      $this->populate_values();
      foreach($this->a_handles as $i_id => $o_handle){
        $o_handle->save();
      }
    }
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
