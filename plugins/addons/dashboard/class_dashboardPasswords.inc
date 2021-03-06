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
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Passwords'),
      'plDescription' => _('Statistics about passwords'),
      'plObjectType'  => ['dashboard'],
      'plPriority'    => 12,

      'plProvidedAcls'  => []
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'pwds' => [
        'name'  => _('Passwords statistics'),
        'attrs' => [new FakeAttribute('pwds_stats')],
        'template' => get_template_path('pwd_stats.tpl', TRUE, dirname(__FILE__)),
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->pwds_stats  = $this->computeStats();
  }

  function computeStats ()
  {
    global $config;

    $temp = passwordMethod::get_available_methods();

    $allowedMethods = $config->get_cfg_value('passwordAllowedHashes', $temp['name']);
    $defaultMethod  = $config->get_cfg_value('passwordDefaultHash', 'ssha');
    $forceDefault   = ($config->get_cfg_value('forcePasswordDefaultHash', 'FALSE') == 'TRUE');
    try {
      $users = objects::ls('user', ['userPassword' => '1', 'dn' => 'raw'], NULL, '', TRUE);
    } catch (LDAPFailureException $e) {
      $error = new FusionDirectoryError(
        htmlescape(sprintf(
          _('Password statistics could not be computed because of the following LDAP error: %s'),
          $e->getMessage()
        )),
        0,
        $e
      );
      $error->display();
      $users = [];
    }
    $nb_accounts = count($users);
    $nb_locked_accounts = 0;
    $methods_stats = [];
    foreach ($users as $attrs) {
      $userPassword = '';
      if (isset($attrs['userPassword'])) {
        $userPassword = $attrs['userPassword'];
      }
      $method       = passwordMethod::get_method($userPassword);
      $methodName   = $method->get_hash();
      if ($method->is_locked('', $userPassword)) {
        $nb_locked_accounts++;
      }
      if (!isset($methods_stats[$methodName])) {
        $methods_stats[$methodName] = [
          'nb'    => 0,
          'name'  => $methodName,
        ];
        if ($methodName == $defaultMethod) {
          $methods_stats[$methodName]['style'] = 'default';
        } elseif ($methodName == 'clear') {
          $methods_stats[$methodName]['style'] = 'clear';
        } elseif ($forceDefault || !in_array($methodName, $allowedMethods)) {
          $methods_stats[$methodName]['style'] = 'forbidden';
        } else {
          $methods_stats[$methodName]['style'] = 'none';
        }
      }
      $methods_stats[$methodName]['nb']++;
    }

    return [
      'methods'         => $methods_stats,
      'nb'              => $nb_accounts,
      'img'             => 'geticon.php?context=types&icon=user&size=16',
      'locked_accounts' => [
        'nb'  => $nb_locked_accounts,
        'img' => 'geticon.php?context=status&icon=object-locked&size=16'
      ],
    ];
  }
}
