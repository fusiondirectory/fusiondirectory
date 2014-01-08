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

class groupManagement extends simpleManagement
{
  protected $objectTypes  = array('group');

  protected $departmentBrowser      = TRUE;
  protected $departmentRootVisible  = FALSE;
  protected $baseMode               = TRUE;

  protected $autoFilter = FALSE;

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Groups'),
      'plDescription' => _('Manage groups'),
      'plIcon'        => '/fusiondirectory/geticon.php?context=applications&icon=system-users&size=48',
      'plSection'     => 'admin',
      'plManages'     => array('group'),
      'plPriority'    => 2,

      'plProvidedAcls' => array()
    );
  }

  function __construct($config, $ui)
  {
    $this->filterXMLPath  = get_template_path('group-filter.xml', TRUE, dirname(__FILE__));
    $this->listXMLPath    = get_template_path('group-list.xml', TRUE, dirname(__FILE__));

    parent::__construct($config, $ui);
    $this->headpage->registerElementFilter("filterProperties", "groupManagement::filterProperties");
  }

  static function filterProperties($row, $dn)
  {
    global $config;
    static $grouptabs = array();
    if (empty($grouptabs)) {
      foreach ($config->data['TABS']['GROUPTABS'] as $plug) {
        if ($plug['CLASS'] == 'group') {
          continue;
        }
        if (class_available($plug['CLASS'])) {
          $name = $plug['CLASS'];

          $grouptabs[$name] = new $name($config, $dn);
        }
      }
    }

    // Load information if needed
    $ldap = $config->get_ldap_link();
    $ldap->cat($dn);
    $result = "<input class='center' type='image' src='plugins/groups/images/iconMini.png' ".
                       "alt='"._('Posix')."' title='"._("Edit posix properties")."' ".
                       "name='listing_edit_tab_group_$row' style='padding:1px'/>";
    if ($attrs = $ldap->fetch()) {
      foreach ($grouptabs as $class => $grouptab) {
        if ($grouptab->is_this_account($attrs)) {
          $infos = pluglist::pluginInfos($class);
          if (isset($infos['plSmallIcon'])) {
            $result .= "<input class='center' type='image' src='".$infos['plSmallIcon']."' ".
                       "alt='".$infos['plShortName']."' title='".$infos['plShortName']."' ".
                       "name='listing_edit_tab_".$class."_$row' style='padding:1px'/>";
          } else {
            @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $infos['plShortName']." ($class)", "No icon for");
          }
        } else {
          $result .= "<img src='images/empty.png' alt=' ' class='center optional $class' style='padding:1px'>";
        }
      }
    }

    return $result;
  }
}
?>
