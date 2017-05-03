<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016  FusionDirectory

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

class simpleSelectManagement extends simpleManagement
{
  protected $skipFooter = TRUE;
  protected $skipHeader = TRUE;

  protected $skipCpHandler    = TRUE;
  protected $skipSnapHandler  = TRUE;
  protected $autoActions      = FALSE;

  protected $departmentRootVisible  = FALSE;

  function __construct()
  {
    if ($this->listXMLPath === NULL) {
      $this->listXMLPath = get_template_path('simple-select-list.xml', TRUE, dirname(__FILE__));
    }

    parent::__construct();
  }

  /* Build the action menu, fill the object definition, etc, based on objectTypes info */
  function configureHeadpage ()
  {
    parent::configureHeadpage();
    $this->headpage->registerElementFilter("selectLink", "simpleSelectManagement::filterSelectLink");
  }

  function configureFilter ()
  {
    parent::configureFilter();
    if ($this->autoFilter) {
      $this->filter->query['0']['backend'] = 'LDAPBlacklist';
    }
  }

  function configureHeadline ()
  {
  }

  function configureFilterCategory ()
  {
    $infos = objects::infos($this->objectTypes[0]);
    $this->filter->category = $infos['aclCategory'];
  }

  static function filterSelectLink()
  {
    if (func_num_args() < 4) {
      return '&nbsp';
    }
    $pid    = func_get_arg(0);
    $row    = func_get_arg(1);
    $dn     = func_get_arg(2);
    $trans  = func_get_arg(3);
    if (is_array($trans)) {
      $trans  = $trans[0];
    }
    if ($trans != '') {
      return '<a href="?plug='.$_GET['plug'].'&amp;PID='.$pid.'&amp;act=listing_select_'.$row.'&amp;add_finish=1" title="'.htmlentities($dn, ENT_COMPAT, 'UTF-8').'">'.htmlentities($trans, ENT_COMPAT, 'UTF-8').'</a>';
    }

    return '';
  }
}
?>
