<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012  FusionDirectory

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

/*! \brief This class is a demo of simplePlugin usage.
 * This plugin have 4 values, 2 of them are related to the person hairs: color (multiple values), length
 * 2 of them are related its bicycle: brand, bell (boolean)
 */
class demoPlugin extends simplePlugin
{
  // We set displayHeader to FALSE, because we don't want a header allowing to activate/deactivate this plugin,
  // we want it activated on all objects
  var $displayHeader = FALSE;

  // Here we indicate which LDAP classes our plugin is using.
  var $objectclasses = array("demoPlugin");

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    return array(
      // Attributes are grouped by section
      'section1' => array(
        'name'  => _('Hair Information'),
        'attrs' => array(
          new SetAttribute(                 // This attribute is multi-valuated
            new SelectAttribute (
              _('Color'),                     // Label of the attribute
              _('Color of the hair'),         // Description
              'hairColor',                    // LDAP name
              TRUE,                           // Mandatory
              array('blond','black','brown'), // [SelectAttribute] Choices
              "", // We don't set any default value, it will be the first one
              array('Blond','Black','Brown')  // [SelectAttribute] Output choices
            )
          ),
          new FloatAttribute  (
            _('Length'),                    // Label
            _('Length of the hair in cm'),  // Description
            'hairLength',                   // LDAP name
            FALSE,                          // Not mandatory
            0,                              // [FloatAttribute] Minimum value
            FALSE,                          // [FloatAttribute] No maximum value
            10                              // [FloatAttribute] Default value
          ),
        )
      ),
      'section2' => array(
        'name'  => _('Bicycle'),
        'attrs' => array(
          new StringAttribute (
            _('Brand'),                     // Label
            _('Brand of the bicycle'),      // Description
            'bicycleBrand',                 // LDAP name
            TRUE,                           // Mandatory
            'GreatBicycleBrand'             // Default value
          ),
          new BooleanAttribute (
            _('Has a bell'),                    // Label
            _('Does the bicycle have a bell'),  // Description
            'bicycleBell',                      // LDAP name
            FALSE,                              // Not mandatory
            FALSE                               // Default value
          ),
        )
      ),
      'ftp' => array(
        'name'  => _('FTP informations'),
        'attrs' => array(
          new CompositeAttribute (
            _('Informations for ftp login'),
            'ftpLoginInfo',
            array(
              new StringAttribute (_('Login'),    _('Login for FTP'),     'ftpLogin'),
              new StringAttribute (_('Password'), _('Password for FTP'),  'ftpPassword'),
              new StringAttribute (_('Host'),     _('Host for FTP'),      'ftpHost'),
              new IntAttribute    (_('Port'),     _('Port for FTP'),      'ftpPort', FALSE, 0, FALSE, 21),
            ),
            'ftp://%[^@:]:%[^@:]@%[^@:]:%d',
            'ftp://%s:%s@%s:%d'
          )
        )
      ),
    );
  }

  // We also need this function that return some information about the plugin
  static function plInfo ()
  {
    return array(
      "plShortName"       => _("Demo Plugin"),
      "plDescription"     => _("Edit some useless personal information"),
      "plSelfModify"      => TRUE,                  // Does this plugin have an owner that might be able to edit its entry
      "plSection"         => array("personal"),     // Should be either addon, administration or personal
      "plCategory"        => array("users"),

      // But simplePlugin can generate the ACL list for us
      "plProvidedAcls"    => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }
}
?>
