<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2021 Antoine Gallavardin

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

class pluginsManager extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Plugins'),
      'plDescription' => _('Plugins List'),
      'plObjectType'  => ['dashboard'],

      'plProvidedAcls'  => []
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'plugins' => [
        'name'  => _('Plugins'),
        'attrs' => [new FakeAttribute('pluginsList')],
        'template' => get_template_path('plugins_list.tpl', TRUE, dirname(__FILE__)),
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    // pluginsList refers to the FakeAttribute.
    // The return of drawList in plugisns_list requires an attribute definition.
    $this->pluginsList = $this->plugins_list();
  }


  function plugins_list ()
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=fdPlugin)', ['cn','description','fdPluginInfoVersion','fdPluginInfoAuthors','fdPluginInfoStatus','fdPluginInfoOrigin','fdPluginSupportProvider','fdPluginSupportHomeUrl','fdPluginReqPlugin']);

    $id   = 'pluginsStats';
    $div  = new divSelectBox('rows'.$id);
    $smarty = get_smarty();
    $div->setHeight(300);
    $div->setHeaders([_('Nom'), _('Description'), _('Version'),_('Authors'),_('Status'),_('Origin'),_('Provider'),_('Dependencies'),_('Link')]);

    while ($plugin = $ldap->fetch()) {
      $fields = [
        ['string' => $plugin['cn'][0]],
        ['string' => $plugin['description'][0]],
        ['string' => $plugin['fdPluginInfoVersion'][0]],
        ['string' => $plugin['fdPluginInfoAuthors'][0]],
        ['string' => $plugin['fdPluginInfoStatus'][0]],
        ['string' => $plugin['fdPluginInfoOrigin'][0]],
        ['string' => $plugin['fdPluginSupportProvider'][0]],
        ['string' => $plugin['fdPluginReqPlugin'][0]],
        ['string' => $plugin['fdPluginSupportHomeUrl'][0]],

      ];

      $div->addEntry($fields);
    }

    return $div->drawList();
  }
}
