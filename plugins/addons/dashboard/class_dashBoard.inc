<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C)  2010 Antoine Gallavardin
  Copyright (C)  2011-2016 FusionDirectory project

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

class dashboard extends simplePlugin
{
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Dashboard'),
      'plTitle'       => _('Dashboard'),
      'plDescription' => _('Statistics and various informations'),
      'plIcon'        => 'geticon.php?context=applications&icon=utilities-system-monitor&size=48',
      'plObjectType'  => array(
        'dashboard' => array(
          'name'      => _('Dashboard'),
          'tabClass'  => 'tabs_dashboard',
          'mainAttr'  => FALSE
        )
      ),
      'plSection'     => array('reporting' => array('name' => _('Reporting'), 'priority' => 30)),
      'plPriority'    => 0,

      'plProvidedAcls'  => array()
    );
  }

  static function getAttributesInfo()
  {
    return array(
      'stats' => array(
        'name'  => _('Statistics'),
        'attrs' => array(new FakeAttribute('stats')),
        'template' => get_template_path('main_stats.tpl', TRUE, dirname(__FILE__)),
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->stats = $this->overview_stats();
  }

  /* Compute some statistics */
  function overview_stats()
  {
    global $config;
    $stats = array();
    foreach ($config->data['OBJECTS'] as $type => $infos) {
      try {
        $nb = objects::count($type);
      } catch (FusionDirectoryException $e) {
        $nb = 0;
        trigger_error("Problem with $type:".$e->getMessage());
      }
      if ($nb == 0) {
        continue;
      }
      $node = array(
        'name'  => $infos['name'],
        'nb'    => $nb
      );
      if (isset($infos['icon'])) {
        $node['img'] = $infos['icon'];
      } else {
        $node['img'] = 'images/empty.png';
      }
      if (isset($infos['management']) && ($infos['management'] != '')) {
        $pInfos = pluglist::pluginInfos($infos['management']);
        $node['href']     = 'main.php?plug='.$pInfos['INDEX'].'&amp;reset=1';
        $section = $pInfos['plSection'];
        if (is_array($section)) {
          $section = key($section);
        }
        $node['priority'] = $config->data['SECTIONS'][$section]['PRIORITY'] * 100 + (isset($pInfos['plPriority']) ? $pInfos['plPriority'] : 99);
      } else {
        $node['priority'] = 10000;
      }
      $stats[] = $node;
    }

    usort($stats,
      function ($a, $b)
      {
        return ($b['priority'] < $a['priority']);
      }
    );

    return $stats;
  }
}
?>
