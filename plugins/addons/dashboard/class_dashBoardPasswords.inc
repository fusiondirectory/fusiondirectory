<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2016-2017 FusionDirectory project

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

class dashboardPassword extends simplePlugin
{
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Passwords'),
      'plDescription' => _('Statistics about passwords'),
      'plObjectType'  => array('dashboard'),

      'plProvidedAcls'    => array()
    );
  }

  static function getAttributesInfo()
  {
    return array(
      'pwds' => array(
        'name'  => _('Passwords statistics'),
        'attrs' => array(new FakeAttribute('pwds_stats')),
        'template' => get_template_path('pwd_stats.tpl', TRUE, dirname(__FILE__)),
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->pwds_stats  = $this->computeStats();
  }

  function computeStats ()
  {
    global $config;

    $defaultMethod = $config->get_cfg_value('passwordDefaultHash', 'ssha');
    $forceDefault = ($config->get_cfg_value('forcePasswordDefaultHash', 'FALSE') == 'TRUE');
    $users = objects::ls('user', 'userPassword');
    $nb_accounts = count($users);
    $nb_locked_accounts = 0;
    $methods_stats = array();
    foreach ($users as $userPassword) {
      if (!empty($userPassword)) {
        if (preg_match("/^\{[^\}]+\}!/", $userPassword)) {
          $nb_locked_accounts++;
        }
        $method = passwordMethod::get_method($userPassword);
        $methodClass = get_class($method);
        if (!isset($methods_stats[$methodClass])) {
          $methods_stats[$methodClass] = array(
            'nb' => 0,
            'name' => $method->get_hash()
          );
          if ($method->get_hash() == $defaultMethod) {
            $methods_stats[$methodClass]['style'] = 'default';
          } elseif ($method->get_hash() == 'clear') {
            $methods_stats[$methodClass]['style'] = 'clear';
          } elseif ($forceDefault) {
            $methods_stats[$methodClass]['style'] = 'forbidden';
          } else {
            $methods_stats[$methodClass]['style'] = 'none';
          }
        }
        $methods_stats[$methodClass]['nb']++;
      }
    }

    return array(
      'methods'         => $methods_stats,
      'nb'              => $nb_accounts,
      'img'             => 'geticon.php?context=types&icon=user&size=16',
      'locked_accounts' => array(
        'nb'  => $nb_locked_accounts,
        'img' => 'geticon.php?context=status&icon=object-locked&size=16'
      ),
    );
  }
}
?>
