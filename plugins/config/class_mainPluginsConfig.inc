<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2016 FusionDirectory

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

class mainPluginsConfig extends simplePlugin
{
  var $objectclasses  = array("fusionDirectoryPluginsConf");

  static function plInfo()
  {
    return array(
      "plShortName"     => _("Plugins configuration"),
      "plDescription"   => _("FusionDirectory plugins configuration"),
      "plSelfModify"    => FALSE,
      "plCategory"      => array("configuration"),
      "plObjectType"    => array("smallConfig"),

      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'ogroups' => array(
        'name'  => _('Object groups'),
        'attrs' => array(
          new StringAttribute (
            _('OGroup RDN'), _('Branch in which object groups will be stored'),
            'fdOGroupRDN', TRUE,
            'ou=groups'
          ),
        )
      ),
      'sasl' => array(
        'name'  => _('SASL'),
        'attrs' => array(
          new BooleanAttribute (
            _('Force to ask for password'), _('Useful if you add a hook using password value when SASL user passwords are edited'),
            'fdForceSaslPasswordAsk', TRUE,
            FALSE
          ),
        )
      ),
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);
  }
}
?>
