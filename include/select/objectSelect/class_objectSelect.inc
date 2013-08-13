<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
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

class objectSelect extends management
{
  protected $skipFooter = TRUE;
  protected $skipHeader = TRUE;

  function __construct($config, $ui)
  {
    $this->config = $config;
    $this->ui     = $ui;
    $this->storagePoints = array(
      get_ou("userRDN"),
      get_ou("groupRDN"),
      get_ou("applicationRDN"),
      get_ou("terminalRDN"),
      get_ou("workstationRDN"),
      get_ou("sambaMachineAccountRDN"),
      get_ou("serverRDN"),
      get_ou("printerRDN"),
      get_ou("phoneRDN")
    );

    // Build filter
    $filter = new filter(get_template_path("selectObject-filter.xml", TRUE, dirname(__FILE__)));
    $filter->setObjectStorage($this->storagePoints);
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("selectObject-list.xml", TRUE, dirname(__FILE__)));
    $headpage->registerElementFilter("selectLink", "objectSelect::filterSelectLink");
    $headpage->setFilter($filter);
    parent::__construct($config, $ui, "object", $headpage);
  }

  static function filterSelectLink()
  {
    $pid    = func_get_arg(0);
    $row    = func_get_arg(1);
    $dn     = func_get_arg(2);
    $trans  = func_get_arg(3)[0];
    if ($trans != "") {
      return "<a href='?plug=".$_GET['plug']."&amp;PID=$pid&amp;act=listing_select_$row&amp;objectSelect_save=1' title='$dn'>$trans</a>";
    }

    return '';
  }

  function save()
  {
    trigger_error('deprecated');
    $act      = $this->detectPostActions();
    $headpage = $this->getHeadpage();
    if (!isset($act['targets'])) {
      return array();
    }
    $ret = array();
    foreach ($act['targets'] as $dn) {
      $ret[] = $headpage->getEntry($dn);
    }
    return $ret;
  }
}
?>
