<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018  FusionDirectory

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

class ManagementColumnAttribute extends CompositeAttribute
{

  function __construct ($label, $description, $ldapName, $attributes, $acl = "")
  {
    parent::__construct ($description, $ldapName, $attributes, FALSE, FALSE, $acl, $label);
  }
}

/*!
 * \brief Management configuration dialog
 */
class ManagementConfigurationDialog extends simplePlugin
{
  static function plInfo ()
  {
    return array(
      'plShortName' => 'ManagementConfigurationDialog',
    );
  }

  static function getAttributesInfo ()
  {
    global $class_mapping;

    // Load column types
    $types = array();
    foreach (array_keys($class_mapping) as $class) {
      if (preg_match('/Column$/', $class) && is_a($class, 'Column', TRUE)) {
        $types[] = $class;
      }
    }
    sort($types);

    return array(
      'main' => array(
        'class' => array('fullwidth'),
        'name'  => _('Management configuration'),
        'attrs' => array(
          new OrderedArrayAttribute (
            new ManagementColumnAttribute (
              _('Columns'),
              _('Columns displayed for this management list'),
              'managementColumns',
              array(
                new SelectAttribute(
                  _('Type'), _('Type of column'),
                  'columnType', TRUE,
                  $types, 'LinkColumn'
                ),
                new StringAttribute(
                  _('Attribute'), _('LDAP attributes to display, comma separated. Special values "nameAttr" and "mainAttr" also works.'),
                  'columnAttribute', FALSE
                ),
                new StringAttribute(
                  _('Label'), _('Column title'),
                  'columnLabel', FALSE
                ),
              ),
              'ManagementConfiguration'
            ),
            TRUE, // ordered
            array(),
            TRUE // edition
          ),
          new BooleanAttribute(
            _('Persitent'), _('Should this configuration be saved in the LDAP as the default configuration for this management page'),
            'saveInLdap', FALSE,
            FALSE,
            'fdManagementConfig'
          ),
        )
      ),
    );
  }

  function __construct ($parent)
  {
    parent::__construct();
    $this->parent       = $parent;
    $this->attributesAccess['saveInLdap']->setInLdap(FALSE);
    if (!$this->attrIsWriteable('saveInLdap')) {
      $this->attributesAccess['saveInLdap']->setVisible(FALSE);
    }
    $this->attributesAccess['managementColumns']->setInLdap(FALSE);
    $this->attributesAccess['managementColumns']->setLinearRendering(FALSE);
    $columnInfos  = $this->parent->getColumnConfiguration();
    $value        = array();
    foreach ($columnInfos as $column) {
      if (!isset($column[1]['attributes'])) {
        $column[1]['attributes'] = '';
      }
      if (is_array($column[1]['attributes'])) {
        $column[1]['attributes'] = implode(',', $column[1]['attributes']);
      }
      if (!isset($column[1]['label'])) {
        $column[1]['label'] = '';
      }
      $value[] = array($column[0], $column[1]['attributes'], $column[1]['label']);
    }
    $this->attributesAccess['managementColumns']->setValue($value);
  }

  function attrIsWriteable($attr)
  {
    global $config, $ui;

    if (($attr === 'managementColumns') || (is_object($attr) && ($attr->getLdapName() == 'managementColumns'))) {
      return TRUE;
    } elseif (($attr === 'saveInLdap') || (is_object($attr) && ($attr->getLdapName() == 'saveInLdap'))) {
      $acl = $ui->get_permissions(CONFIGRDN.$config->current['BASE'], 'configuration/configInLdap', 'fdManagementConfig', $this->readOnly());
      return (strpos($acl, 'w') !== FALSE);
    } else {
      return parent::attrIsWriteable($attr);
    }
  }

  function execute ()
  {
    global $config, $ui;

    $smarty = get_smarty();
    $smarty->assign('ManagementConfigurationACL', 'rw');
    $smarty->assign('fdManagementConfigACL', $ui->get_permissions(CONFIGRDN.$config->current['BASE'], 'configuration/configInLdap', 'fdManagementConfig', $this->readOnly()));
    $str = parent::execute();

    $str .= '<p class="plugbottom">'.
           '  <input type="submit" name="edit_finish" value="'.msgPool::saveButton().'"/>&nbsp;'.
           '  <input type="submit" formnovalidate="formnovalidate" name="edit_cancel" value="'.msgPool::cancelButton().'"/>'.
           '</p>';

    return $str;
  }

  function save ()
  {
    global $config;
    $columnInfos  = array();
    $values       = $this->managementColumns;
    foreach ($values as $value) {
      $column = array($value[0], array());
      if (!empty($value[1])) {
        $column[1]['attributes'] = $value[1];
      }
      if (!empty($value[2])) {
        $column[1]['label'] = $value[2];
      }
      $columnInfos[] = $column;
    }
    $this->parent->setColumnConfiguration($columnInfos);

    if ($this->saveInLdap) {
      $errors = $config->updateManagementConfig(get_class($this->parent), $columnInfos);
      msg_dialog::displayChecks($errors);
    }
  }
}
