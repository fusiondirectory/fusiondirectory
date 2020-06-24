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
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Dashboard'),
      'plTitle'       => _('Dashboard'),
      'plDescription' => _('Statistics and various information'),
      'plIcon'        => 'geticon.php?context=applications&icon=utilities-system-monitor&size=48',
      'plObjectType'  => [
        'dashboard' => [
          'name'      => _('Dashboard'),
          'tabClass'  => 'dashboardTabs',
          'mainAttr'  => FALSE
        ]
      ],
      'plSection'     => ['reporting' => ['name' => _('Reporting'), 'priority' => 30]],
      'plPriority'    => 0,

      'plProvidedAcls'  => [
        'dashboard' => _('See dashboard')
      ]
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'stats' => [
        'name'  => _('Statistics'),
        'attrs' => [new FakeAttribute('stats')],
        'template' => get_template_path('main_stats.tpl', TRUE, dirname(__FILE__)),
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->stats = $this->overview_stats();
  }

  /* Compute some statistics */
  function overview_stats ()
  {
    global $config;
    $stats = [];
    foreach ($config->data['OBJECTS'] as $type => $infos) {
      try {
        $nb = count(objects::ls($type, NULL, NULL, '', TRUE));
      } catch (EmptyFilterException $e) {
        $nb = 0;
      } catch (FusionDirectoryException $e) {
        $nb = 0;
        $error = new FusionDirectoryError(
          htmlescape(sprintf(
            _('Statistics for type "%s" could not be computed because of the following error: %s'),
            $type,
            $e->getMessage()
          )),
          0,
          $e
        );
        $error->display();
      }
      if ($nb == 0) {
        continue;
      }
      $node = [
        'name'  => $infos['name'],
        'nb'    => $nb
      ];
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

  static function mainInc ($classname = NULL, $entry_dn = NULL, $tabs = TRUE, $edit_mode = FALSE, $objectType = FALSE)
  {
    global $config;

    if ($classname === NULL) {
      $classname = get_called_class();
    }

    if ($entry_dn === NULL) {
      $entry_dn = $config->current['BASE'];
    }

    parent::mainInc($classname, $entry_dn, $tabs, $edit_mode, $objectType);
  }
}
