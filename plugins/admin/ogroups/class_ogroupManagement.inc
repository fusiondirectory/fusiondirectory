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
    $types  = preg_replace("/[^a-z]/i", "", $gosaGroupObjects[0]);
    $result = "";
    for ($i = 0; $i < strlen($types); $i++) {
      if ($types[$i] == 'I') {
        continue;
      }
      $info   = objects::infos(ogroup::$objectTypes[$types[$i]]);
      $result .= '<img class="center" src="'.$info['icon'].'" '.
        'alt="'.$info['name'].'" title="'.$info['name'].'" style="padding:1px"/>';
    }
    if ($result == "") {
      return '&nbsp;';
    }
    return $result;
  }
}
?>
