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

class userSelect extends management
{
  protected $skipFooter = TRUE;
  protected $skipHeader = TRUE;

  function __construct($config, $ui)
  {
    $this->storagePoints = array(get_ou("userRDN"));

    // Build filter
    $filter = new filter(get_template_path("user-filter.xml", TRUE, dirname(__FILE__)));
    $filter->setObjectStorage($this->storagePoints);
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("user-list.xml", TRUE, dirname(__FILE__)));
    $headpage->registerElementFilter("selectLink", get_class($this)."::filterSelectLink");
    $headpage->setFilter($filter);
    parent::__construct($config, $ui, "user", $headpage);
  }

  static function filterSelectLink()
  {
    // TODO : replace this with javascript that select the line
    $pid    = func_get_arg(0);
    $row    = func_get_arg(1);
    $dn     = func_get_arg(2);
    $trans  = func_get_arg(3)[0];
    if ($trans != "") {
      return "<a href='?plug=".$_GET['plug']."&amp;PID=$pid&amp;act=listing_select_$row&amp;add_users_finish=1' title='$dn'>$trans</a>";
    }

    return '';
  }
}

class singleUserSelect extends userSelect
{
  function __construct($config, $ui)
  {
    parent::__construct($config, $ui);
    $this->headpage->multiSelect = FALSE;
  }

  static function filterSelectLink()
  {
    $pid    = func_get_arg(0);
    $row    = func_get_arg(1);
    $dn     = func_get_arg(2);
    $trans  = func_get_arg(3)[0];
    if ($trans != "") {
      return "<a href='?plug=".$_GET['plug']."&amp;PID=$pid&amp;act=listing_select_$row&amp;add_users_finish=1' title='$dn'>$trans</a>";
    }

    return '';
  }
}
?>
