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

class dashboardSystems extends simplePlugin
{
  /* default values*/
  var $default_start_computer_id = 0;

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->stats = array(
      'systems' => $this->systems_stats(),
      'argonaut' => $this->argonaut_stats(),
    );
    $this->pc_ids = $this->computer_ids_rules();
  }

  function systems_stats ()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);

    /* Statistics */
    $stats = array(
      array('name' => _('Workstations'),
            'filter' => 'objectClass=gotoWorkstation',
            'img' => 'geticon.php?context=devices&icon=computer&size=16'),
      array('name' => _('Servers'),
            'filter' => 'objectClass=goServer',
            'img' => 'geticon.php?context=devices&icon=server&size=16'),
      array('name' => _('Windows Workstations'),
            'filter' => '(&(objectClass=sambaSamAccount)(uid=*$))',
            'img' => 'geticon.php?context=devices&icon=computer-windows&size=16'),
      array('name' => _('Terminals'),
            'filter' => 'objectClass=gotoTerminal',
            'img' => 'geticon.php?context=devices&icon=terminal&size=16'),
      array('name' => _('Printers'),
            'filter' => 'objectClass=gotoPrinter',
            'img' => 'geticon.php?context=devices&icon=printer&size=16'),
      array('name' => _('Phones'),
            'filter' => 'objectClass=goFonHardware',
            'img' => 'geticon.php?context=devices&icon=telephone&size=16'),
      array('name' => _('Components'),
            'filter' => 'objectClass=ieee802Device',
            'img' => 'geticon.php?context=devices&icon=network-device&size=16'),
      array('name' => _('Mobile phones'),
            'filter' => 'objectClass=fdMobilePhone',
            'img' => 'geticon.php?context=devices&icon=phone&size=16'),
    );

    $ldap->cd($this->config->current['BASE']);
    foreach ($stats as &$stat) {
      $ldap->search($stat['filter'], array("cn"));
      $stat['nb'] = $ldap->count();
    }
    unset($stat);

    return $stats;
  }

  function argonaut_stats ()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=argonautServer)", array('cn','ipHostNumber','argonautProtocol','argonautPort'));
    $nb_argonaut_server = $ldap->count();
    $argonaut_server = array();
    if ($nb_argonaut_server == 1) {
      $attrs = $ldap->fetch();
      foreach (array('cn','ipHostNumber','argonautProtocol','argonautPort') as $key) {
        $argonaut_server[$key] = $attrs[$key][0];
      }
      $argonaut_server['link'] = objects::link($attrs['dn'], 'server', 'service_serviceArgonaut', $attrs);
    }
    $ldap->search("(objectClass=argonautClient)", array('cn'));
    $nb_argonaut_clients = $ldap->count();

    return array(
      'nb_servers'  => $nb_argonaut_server,
      'server'      => $argonaut_server,
      'nb_clients'  => $nb_argonaut_clients,
    );
  }

  function computer_ids_rules ()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    /* Begin of code for selecting next computer IDs
    * Global variable is board_next_computer_ids_rule
    * syntax is :
    * 7 : nb total of digit
    * LYP3 : prefix to use
    * each prefix is followed by the first item flagged by an equal sign
    * ex : "7;LYP0=3;LYP1=6;LYP2;LYP3"
    * if to item is doned it's 0 by default
    *
    */

    $output_next_computer_ids = "";
    $computer_ids_rules = $this->config->get_cfg_value('dashboardPrefix', array('PC'));
    if (!is_array($computer_ids_rules)) {
      $computer_ids_rules = array($computer_ids_rules);
    }

    /* $get_cfg_value return the OU like ou=computer .. */
    $winstation_ou  = mb_substr($this->config->get_cfg_value('sambaMachineAccountRDN'), 3); //FIXME use get_ou
    $nb_digits      = $this->config->get_cfg_value('dashboardNumberOfDigit', 3);

    /* running all the table */
    foreach ($computer_ids_rules as $rule) {
      /* aray initialization*/
      $array_complete_list  = array();
      $array_real_list      = array();
      $unused_computer_ids  = array();
      /* get computer ids configuration */
      $config_ids = explode("=", $rule);
      /* fist is is the prefix */
      $prefix = $config_ids[0];
      /* second (if specified) is the first item */
      if (!isset($config_ids[1])) {
        $start_id = $this->default_start_computer_id;
      } else {
        $start_id = $config_ids[1];
      }

      $output_next_computer_ids = $output_next_computer_ids.'<tr><th style="padding:4px;border:1px solid #BBB;">'.$prefix."</th>";
      $nb_digits_suffix         = $nb_digits - strlen($prefix);

      /* generation of list of suffixe */
      for ($d = $start_id;$d < pow(10, $nb_digits_suffix);$d++) {
        /* padding : 34 on 4 digit become : 0034*/
        $array_complete_list[] = str_pad($d, $nb_digits_suffix, "0", STR_PAD_LEFT);
      }

      /* request of all computer beginning by the prefix */
      $request = "(&(|(objectClass=sambaSamAccount)(objectClass=GOhard))(cn=$prefix*)(|(ou:dn:=$winstation_ou)(ou:dn:=systems)))";
      $ldap->search($request, array("cn"));
      while ($attrs = $ldap->fetch()) {
        /* if a computer is a windows host, we have to delete the $ at the end */
        $computer_id        = str_replace("$", "", $attrs["cn"][0]);
        $array_real_list[]  = substr($computer_id, -$nb_digits_suffix);
      }
      /* make dfference between real and complete list */
      $unused_computer_ids = array_diff($array_complete_list, $array_real_list);
      asort($unused_computer_ids);
      $unused_computer_ids = array_values($unused_computer_ids);
      /* we take the 5 first */

      for ($r = 0;$r <= 5;$r++) {
        $output_next_computer_ids .= '<td style="padding:4px;border-bottom:1px solid #BBB;">'.$prefix.$unused_computer_ids[$r]."</td>";
      }
      $output_next_computer_ids .= "</tr>";
    }

    return $output_next_computer_ids;
  }

  static function getAttributesInfo()
  {
    return array(
      'stats' => array(
        'name'  => _('Statistics'),
        'attrs' => array(new FakeAttribute('stats')),
        'template' => get_template_path('systems_stats.tpl', TRUE, dirname(__FILE__)),
      ),
      'pc_ids' => array(
        'name'  => _('Computer name to use by unit'),
        'attrs' => array(new FakeAttribute('pc_ids')),
        'template' => get_template_path('systems_pcids.tpl', TRUE, dirname(__FILE__)),
      ),
    );
  }

  /* Return plugin informations for acl handling  */
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Systems'),
      'plDescription' => _('Statistics and informations about systems'),
      'plIcon'        => 'plugins/dashboard/images/icon.png',
      'plObjectType'  => array('dashboard'),

      'plProvidedAcls'  => array()
    );
  }
}
?>
