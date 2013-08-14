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

class ogroupManagement extends simpleManagement
{
  protected $objectName   = 'ogroup';
  protected $objectTypes  = array('ogroup');

  protected $departmentBrowser      = TRUE;
  protected $departmentRootVisible  = FALSE;
  protected $baseMode               = TRUE;

  protected $autoFilter = FALSE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Object groups'),
      'plDescription' => _('Manage object groups'),
      'plIcon'        => 'plugins/ogroups/images/icon.png',
      'plSection'     => 'admin',
      'plCategory'    => array('ogroup'),
      'plManages'     => array('ogroup'),
      'plPriority'    => 3,

      'plProvidedAcls' => array()
    );
  }

  function __construct($config, $ui)
  {
    $this->filterXMLPath  = get_template_path('ogroup-filter.xml', TRUE, dirname(__FILE__));
    $this->listXMLPath    = get_template_path('ogroup-list.xml', TRUE, dirname(__FILE__));

    parent::__construct($config, $ui);
    $this->headpage->registerElementFilter("filterProperties", "ogroupManagement::filterProperties");
  }

  static function filterProperties($row, $gosaGroupObjects)
  {
    $conv = array(
      "Y" => array("plugins/users/images/select_template.png",  _("Templates"),       "ogroup"),
      "U" => array("plugins/users/images/iconMini.png",         _("User"),            "ogroup"),
      "G" => array("plugins/groups/images/iconMini.png",        _("Group"),           "ogroup"),
      "A" => array("plugins/ogroups/images/application.png",    _("Application"),     "ogroup"),
      "D" => array("plugins/departments/department.png",        _("Department"),      "ogroup"),
      "S" => array("plugins/ogroups/images/server.png",         _("Server"),          "ogroup"),
      "F" => array("plugins/ogroups/images/asterisk.png",       _("Phone"),           "phonequeue"),
      "W" => array("plugins/ogroups/images/workstation.png",    _("Workstation"),     "workstartup"),
      "O" => array("plugins/ogroups/images/winstation.png",     _("Windows Install"), "ogroup"),
      "T" => array("plugins/ogroups/images/terminal.png",       _("Terminal"),        "termgroup"),
      "P" => array("plugins/ogroups/images/printer.png",        _("Printer"),         "ogroup")
    );

    $types  = preg_replace("/[^a-z]/i", "", $gosaGroupObjects[0]);
    $result = "";
    for ($i = 0; $i < strlen($types); $i++) {
      $type   = $types[$i];
      $result .= "<input class='center' type='image' src='".$conv[$type][0]."' ".
        "alt='".$conv[$type][1]."' title='".$conv[$type][1].
        "' name='listing_edit_tab_".$conv[$type][2]."_$row' style='padding:1px'>";
    }
    if ($result == "") {
      return '&nbsp;';
    }
    return $result;
  }
}
?>
