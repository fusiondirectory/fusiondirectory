<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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
      'plIcon'        => 'geticon.php?context=types&icon=resource-group&size=48',
      'plSection'     => 'accounts',
      'plManages'     => array('ogroup'),
      'plPriority'    => 21,

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
    $conv= array(
      "Y" => array("plugins/users/images/select_template.png"                   ,_("Templates")       ,"ogroup"),
      "U" => array('geticon.php?context=types&icon=user&size=16'                ,_("User")            ,"ogroup"),
      "G" => array('geticon.php?context=types&icon=user-group&size=16'          ,_("Group")           ,"ogroup"),
      "A" => array('geticon.php?context=types&icon=application&size=16'         ,_("Application")     ,"ogroup"),
      "D" => array('geticon.php?context=places&icon=folder&size=16'             ,_("Department")      ,"ogroup"),
      "S" => array('geticon.php?context=devices&icon=server&size=16'            ,_("Server")          ,"ogroup"),
      "F" => array('geticon.php?context=devices&icon=telephone&size=16'         ,_("Phone")           ,"phonequeue"),
      "W" => array('geticon.php?context=devices&icon=computer&size=16'          ,_("Workstation")     ,"workstartup"),
      "O" => array('geticon.php?context=devices&icon=computer-windows&size=16'  ,_("Windows Install") ,"ogroup"),
      "T" => array('geticon.php?context=devices&icon=terminal&size=16'          ,_("Terminal")        ,"termgroup"),
      "P" => array('geticon.php?context=devices&icon=printer&size=16'           ,_("Printer")         ,"ogroup")
    );

    $types = preg_replace("/[^a-z]/i","",$gosaGroupObjects[0]);
    $result ="";
    for($i = 0 ; $i < strlen($types); $i++){
      $type = $types[$i];
      $result.="<input class='center' type='image' src='".$conv[$type][0]."' ".
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
