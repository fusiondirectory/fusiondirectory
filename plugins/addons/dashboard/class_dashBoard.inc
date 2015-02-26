<?php
/*
 * This code is part of FusionDirectory (http://www.fusiondirectory.org)
 * Copyright (C) 2010 Antoine Gallavardin
 * Copyright (C) 2011-2013 FusionDirectory project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

class dashboard extends simplePlugin
{
  /* Generic */
  var $view_logged    = FALSE;

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->stats = $this->overview_stats();
  }

    /* Compute some statistics */
  function overview_stats()
  {
    $ldap = $this->config->get_ldap_link();

    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(!(objectClass=gosaUserTemplate))(objectClass=gosaAccount))", array("cn"));
    $nb_accounts = $ldap->count();

    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=posixGroup)", array("cn"));
    $nb_groups = $ldap->count();
    $ldap->search("(objectClass=gosaGroupOfNames)", array("cn"));
    $nb_ogroups = $ldap->count();

    $stats = array(
      array('name' => 'Accounts',      'nb' => $nb_accounts, 'img' => 'geticon.php?context=types&icon=user&size=16'),
      array('name' => 'Groups',        'nb' => $nb_groups,   'img' => 'geticon.php?context=types&icon=user-group&size=16'),
      array('name' => 'Object groups', 'nb' => $nb_ogroups,  'img' => 'geticon.php?context=types&icon=resource-group&size=16'),
    );

    if (class_available("systemManagement")) {
      $ldap->cd($this->config->current['BASE']);
      $workstations = '(objectClass=gotoWorkstation)';
      $servers      = '(objectClass=goServer)';
      $winstations  = '(&(objectClass=sambaSamAccount)(uid=*$))';
      $terminals    = '(objectClass=gotoTerminal)';
      $printers     = '(objectClass=gotoPrinter)';
      $phones       = '(objectClass=goFonHardware)';
      $devices      = '(objectClass=ieee802Device)';
      $ldap->search(
        "(|$workstations$servers$winstations$terminals$printers$phones$devices)",
        array("cn")
      );
      $nb_systems = $ldap->count();
      array_unshift(
        $stats,
        array('name' => 'Systems', 'nb' => $nb_systems, 'img' => 'geticon.php?context=devices&icon=server&size=16')
      );
    }

    return $stats;
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

  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Dashboard'),
      'plDescription' => _('Statistics and various informations'),
      'plIcon'        => 'geticon.php?context=applications&icon=utilities-system-monitor&size=48',
      'plObjectType'  => array('dashboard' => array('name' => _('Dashboard'))),
      'plSection'     => 'addons',
      'plPriority'    => 2,

      'plProvidedAcls'  => array()
    );
  }
}
?>
