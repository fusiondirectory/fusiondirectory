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

class objectSelect extends management
{
  // Tab definition 
  protected $skipFooter = TRUE;
  protected $skipHeader = TRUE;

  function __construct($config,$ui)
  {
    $this->config = $config;
    $this->ui = $ui;
    $this->storagePoints = array(
        get_ou("userRDN"), 
        get_ou("groupRDN"),
        get_ou("terminalRDN"),
        get_ou("workstationRDN"),
        get_ou("serverRDN"),
        get_ou("printerRDN"),
        get_ou("phoneRDN"));

#    // Build filter
 #   if (session::global_is_set(get_class($this)."_filter")){
  #    $filter= session::global_get(get_class($this)."_filter");
   # } else {
      $filter = new filter(get_template_path("selectObject-filter.xml", true, dirname(__FILE__)));
      $filter->setObjectStorage($this->storagePoints);
    #}
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("selectObject-list.xml", true, dirname(__FILE__)));
    $headpage->registerElementFilter("depLabel", "objectSelect::filterDepLabel");
    $this->registerAction("open","openEntry");
    $headpage->setFilter($filter);
    parent::__construct($config, $ui, "object", $headpage);
  }


  // An action handler which enables to switch into deparmtment by clicking the names.
  function openEntry($action,$entry)
  {
    $headpage = $this->getHeadpage();
    $headpage->setBase(array_pop($entry));
  }


  // A filter which allows to open a department by clicking on the departments name.
  static function filterDepLabel($row,$dn,$pid,$base,$objectClass, $cn)
  {
    $cn = $cn[0];
    if(!in_array('gosaDepartment', $objectClass)){
       return($cn);
    }
    if($dn == $base){
      $cn =".";
    }
    $dn= LDAP::fix(func_get_arg(1));
    return("<a href='?plug=".$_GET['plug']."&amp;PID=$pid&amp;act=listing_open_$row' title='$dn'>$cn</a>");
  }

  function save()
  {
    $act = $this->detectPostActions();
    $headpage = $this->getHeadpage();
    if(!isset($act['targets'])) return(array());
    $ret = array();
    foreach($act['targets'] as $dn){
      $ret[] = $headpage->getEntry($dn);
    }
    return($ret);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
