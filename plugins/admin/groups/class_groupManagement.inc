<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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
  protected $objectTypes  = array('ogroup', 'role', 'group');

  protected $autoFilter = FALSE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Groups and roles'),
      'plTitle'       => _('Manage groups and roles'),
      'plDescription' => _('Allows you to manage object groups, POSIX groups and roles'),
      'plIcon'        => 'geticon.php?context=types&icon=resource-group&size=48',
      'plSection'     => 'accounts',
      'plManages'     => array('ogroup', 'group', 'role'),
      'plPriority'    => 20,

      'plProvidedAcls' => array()
    );
  }

  function __construct($unused, $ui)
  {
    $this->filterXMLPath  = get_template_path('group-filter.xml', TRUE, dirname(__FILE__));
    $this->listXMLPath    = get_template_path('group-list.xml', TRUE, dirname(__FILE__));

    parent::__construct($unused, $ui);
    $this->headpage->registerElementFilter('filterProperties', 'groupManagement::filterProperties');
  }

  static function filterProperties($row, $dn, $gosaGroupObjects = NULL)
  {
    global $config;
    static $grouptabs = array();

    // Load information if needed
    $ldap = $config->get_ldap_link();
    $ldap->cat($dn);
    $result = '&nbsp;';
    if ($attrs = $ldap->fetch()) {
      if (objects::isOfType($attrs, 'ogroup')) {
        if (empty($gosaGroupObjects)) {
          return $result;
        } else {
          return self::filterGroupObjects($row, $gosaGroupObjects);
        }
      }
      if (objects::isOfType($attrs, 'role')) {
        return '<input class="center" type="image" src="geticon.php?context=types&amp;icon=role&amp;size=16" '.
                 'alt="'._('Role').'" title="'._('Edit role properties').'" '.
                 'name="listing_edit_'.$row.'" style="padding:1px"/>';
      }
      if (empty($grouptabs)) {
        foreach ($config->data['TABS']['GROUPTABS'] as $plug) {
          if ($plug['CLASS'] == 'group') {
            continue;
          }
          if (class_available($plug['CLASS'])) {
            $name = $plug['CLASS'];

            $grouptabs[$name] = new $name(NULL, $dn);
          }
        }
      }
      $result = '<input class="center" type="image" src="geticon.php?context=types&amp;icon=user-group&amp;size=16" '.
                       'alt="'._('Posix').'" title="'._('Edit posix properties').'" '.
                       'name="listing_edit_tab_group_'.$row.'" style="padding:1px"/>';
      foreach ($grouptabs as $class => $grouptab) {
        if ($grouptab->is_this_account($attrs)) {
          $infos = pluglist::pluginInfos($class);
          if (isset($infos['plSmallIcon'])) {
            $result .= '<input class="center" type="image" src="'.htmlentities($infos['plSmallIcon'], ENT_COMPAT, 'UTF-8').'" '.
                       'alt="'.$infos['plTitle'].'" title="'.$infos['plTitle'].'" '.
                       'name="listing_edit_tab_'.$class.'_'.$row.'" style="padding:1px"/>';
          } else {
            @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $infos['plShortName']." ($class)", 'No icon for');
          }
        } else {
          $result .= '<img src="images/empty.png" alt=" " class="center optional '.$class.'" style="padding:1px"/>';
        }
      }
    }

    return $result;
  }

  static function filterGroupObjects($row, $gosaGroupObjects)
  {
    $types  = preg_replace('/[^a-z]/i', '', $gosaGroupObjects[0]);
    $result = '';
    for ($i = 0; $i < strlen($types); $i++) {
      if ($types[$i] == 'I') {
        continue;
      }
      $info   = objects::infos(ogroup::$objectTypes[$types[$i]]);
      $result .= '<img class="center" src="'.htmlentities($info['icon'], ENT_COMPAT, 'UTF-8').'" '.
        'alt="'.$info['name'].'" title="'.$info['name'].'" style="padding:1px"/>';
    }
    if ($result == '') {
      return '&nbsp;';
    }
    return $result;
  }

  function renderList()
  {
    $smarty = get_smarty();
    $filters = array(
      array('id'  => 'USER',        'label'  => _('Show user groups')),
      array('id'  => 'PRIMARY',     'label'  => _('Show primary groups')),
      array('id'  => 'ROLE',        'label'  => _('Show organizational roles')),
      array('id'  => 'APPLICATION', 'label'  => _('Show application groups')),
      array('id'  => 'UNKNOWN',     'label'  => _('Show unidentified groups')),
    );
    if (class_available('mailAccount')) {
      $filters[]= array('id'  => 'MAIL',        'label'  => _('Show mail groups'));
    }
    if (class_available('sambaAccount')) {
      $filters[]= array('id'  => 'SAMBA',       'label'  => _('Show samba groups'));
    }
    if (class_available('systemManagement')) {
      $filters = array_merge(
        $filters,
        array(
          array('id'  => 'SERVER',      'label'  => _('Show server groups')),
          array('id'  => 'WORKSTATION', 'label'  => _('Show workstation groups')),
          array('id'  => 'WINDOWS',     'label'  => _('Show windows groups')),
          array('id'  => 'TERMINAL',    'label'  => _('Show terminal groups')),
          array('id'  => 'PRINTER',     'label'  => _('Show printer groups')),
          array('id'  => 'PHONE',       'label'  => _('Show phone groups')),
        )
      );
    }
    $smarty->assign('objectFilters', $filters);
    return management::renderList();
  }
}
?>
