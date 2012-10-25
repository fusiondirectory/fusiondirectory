<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012 FusionDirectory

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


class configInLdap extends simplePlugin
{
  var $objectclasses  = array("fusionDirectoryConf");
  var $mainTab        = TRUE;

  static function plInfo()
  {
    return array(
      "plShortName"     => _("Configuration"),
      "plDescription"   => _("FusionDirectory configuration"),
      "plSelfModify"    => FALSE,
      "plPriority"      => 1,
      "plSection"       => array("administration"),
      "plCategory"      => array("configuration"),
      "plObjectType"    => array("configuration" => array("name"  => _("FusionDirectory configuration"),
                                                          "filter"  => "objectClass=fusionDirectoryConf")),

      "plProvidedAcls"  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'schema_setup' => array(
        'name'  => _("Schema setup"),
        'attrs' => array(
          new BooleanAttribute (
            _("rfc237bis"),
            _("Enables rfc2307bis style groups. You can use member attributes instead of memberUid in this case."),
            "fdRfc2307bis", FALSE,
            FALSE
          ),
          new BooleanAttribute (
            _("schemaCheck"),
            _("Enables schema checking during login."),
            "fdSchemaCheck", FALSE,
            FALSE
          ),
        )
      ),
      'password' => array(
        'name'  => _("Password settings"),
        'attrs' => array(
          new SelectAttribute (
            _("Password default hash"), _("Default hash to be used"),
            "fdPasswordDefaultHash", FALSE,
            array("", "crypt/md5")
          ),
          new IntAttribute (
            _("Password minimum length"), _("Minimum length of user passwords"),
            "fdPasswordMinLength", FALSE,
            0 /*min*/, FALSE /*no max*/
          ),
          new IntAttribute (
            _("Password minimum differs"), _("Minimum number of different characters from last password"),
            "fdPasswordMinDiffer", FALSE,
            0 /*min*/, FALSE /*no max*/
          ),
          new StringAttribute (
            _("Password hook"), _("External script to handle password settings"),
            "fdPasswordHook"
          ),
          new BooleanAttribute (
            _("Use account expiration"),
            _("Enables shadow attribute tests during the login to FusionDirectory and forces password renewal or account lock‐out"),
            "fdHandleExpiredAccounts"
          ),
          new StringAttribute (
            _("SASL Realm"), _("SASL Realm"),
            "fdSaslRealm"
          ),
          new StringAttribute (
            _("SASL Exop"), _("Attribute to be stored in the userPassword attribute"),
            "fdSaslExop"
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
