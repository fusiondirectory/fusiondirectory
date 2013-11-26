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

class aclManagement extends management
{
  // Tab definition
  protected $tabClass     = "simpleTabs_noSpecial";
  protected $tabType      = "ACLASSIGNMENTTABS";
  protected $aclCategory  = "acl";
  protected $aclPlugin    = "aclAssignment";
  protected $objectName   = "ACL";

  /* Return plugin informations for acl handling */
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

  function __construct(&$config, $ui)
  {
    $this->config = $config;
    $this->ui     = $ui;

    $this->storagePoints = array(''); // ACLs are attached to department containers

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")) {
      $filter = session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path("acl-filter.xml", TRUE));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("acl-list.xml", TRUE));
    $headpage->registerElementFilter("filterLabel", "aclManagement::filterLabel");
    $headpage->setFilter($filter);

    parent::__construct($this->config, $ui, "acl", $headpage);
  }

  function removeEntryConfirmed ($action = "", $target = array(), $all = array(), $altTabClass = "", $altTabType = "", $altAclCategory = "")
  {
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $target, "Entry removal confirmed!");
    $headpage = $this->getHeadpage();

    foreach ($this->dns as $key => $dn) {
      // Check permissions, are we allowed to remove this object?
      $acl = $this->ui->get_permissions($dn, $this->aclCategory."/".$this->aclPlugin);
      if (preg_match("/d/", $acl)) {
        // Delete the object
        $this->dn = $dn;
        $this->tabObject = new $this->tabClass('aclAssignment', $this->dn);
        $this->tabObject->set_acl_base($this->dn);
        $this->tabObject->delete();
        $this->tabObject->parent = &$this;

        // Remove the lock for the current object.
        del_lock($this->dn);
      } else {
        msg_dialog::display(_("Permission error"), msgPool::permDelete(), ERROR_DIALOG);
        new log("security", "acl/".get_class($this), $dn, array(), "Tried to trick deletion.");
      }
    }

    // Cleanup
    $this->remove_lock();
    $this->closeDialogs();
  }

  // A filter which allows to open a department by clicking on the departments name.
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
}
?>
