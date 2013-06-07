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

  function __construct($config,$ui)
  {
    $this->config = $config;
    $this->ui = $ui;

    $this->storagePoints = array(get_ou("userRDN"));

#    // Build filter
#    if (session::global_is_set(get_class($this)."_filter")){
#      $filter= session::global_get(get_class($this)."_filter");
#    } else {
      $filter = new filter(get_template_path("user-filter.xml", true, dirname(__FILE__)));
      $filter->setObjectStorage($this->storagePoints);
#    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("user-list.xml", true, dirname(__FILE__)));
    $headpage->setFilter($filter);
    parent::__construct($config, $ui, "user", $headpage);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
