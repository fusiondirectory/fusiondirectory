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

class aclManagement extends simpleManagement
{
  protected $objectTypes  = array('aclAssignment');

  protected $autoFilterAttributes = array('dn', 'cn', 'description', 'ou', 'dc');

  protected $departmentBrowser      = FALSE;
  protected $departmentRootVisible  = FALSE;
  protected $baseMode               = FALSE;
  protected $skipCpHandler          = TRUE;
  protected $skipSnapHandler        = TRUE;
  protected $autoActions            = FALSE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('ACL assignments'),
      'plDescription' => _('Access control list management'),
      'plIcon'        => 'plugins/acl/images/icon.png',
      'plSection'     => 'admin',
      'plPriority'    => 42,
      'plManages'     => array('aclAssignment'),

      'plProvidedAcls' => array()
    );
  }

  function configureFilter()
  {
    parent::configureFilter();
    $this->filter->scope = 'sub';
  }

  function parseXML ($file)
  {
    $data = parent::parseXML($file);
    $data['list']['table']['column'][1]['value']          = '%{filter:filterLabel(row,dn,ou,cn,pid,base)}';
    $data['list']['table']['column'][1]['sortAttribute']  = 'dn';
    array_shift($data['list']['actionmenu']['action']);
    return $data;
  }

  function configureHeadpage ()
  {
    $this->headpage->objectTypes = array(); // Remove department types so that we don't mistake assignments with them.
    $this->headpage->registerElementFilter('filterLabel', 'aclManagement::filterLabel');
    parent::configureHeadpage();
  }

  static function filterLabel($row, $dn, $ou = array(), $pid = 0, $base = "")
  {
    $ou = $ou[0];
    if ($dn == $base) {
      $ou = " . ";
    }
    $ou .= "  &nbsp; ["._("ACL Assignment")."]";

    $dn = LDAP::fix($dn);
    return "<a href='?plug=".$_GET['plug']."&amp;PID=$pid&amp;act=listing_edit_$row' title='$dn'>$ou</a>";
  }

  protected function removeEntryRequested($action = "", $target = array(), $all = array())
  {
    $result = parent::removeEntryRequested($action, $target, $all);
    if ($result != "") {
      $smarty = get_smarty();
      return $smarty->fetch(get_template_path('remove.tpl', TRUE, dirname(__FILE__)));
    }
  }
}
?>
