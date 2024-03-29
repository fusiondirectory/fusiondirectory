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
    parent::__construct($description, $ldapName, $attributes, FALSE, FALSE, $acl, $label);
  }
}

/*!
 * \brief Management configuration dialog
 */
class ManagementConfigurationDialog extends ManagementDialog
{
  static function plInfo (): array
  {
    return [
      'plShortName' => 'ManagementConfigurationDialog',
    ];
  }

  static function getAttributesInfo (): array
  {
    global $class_mapping;

    // Load column types
    $types = [];
    foreach (array_keys($class_mapping) as $class) {
      if (preg_match('/Column$/', $class) && is_a($class, 'Column', TRUE)) {
        $types[] = $class;
      }
    }
    sort($types);

    return [
      'main' => [
        'class' => ['fullwidth'],
        'name'  => _('Management configuration'),
        'attrs' => [
          new OrderedArrayAttribute(
            new ManagementColumnAttribute(
              _('Columns'),
              _('Columns displayed for this management list'),
              'managementColumns',
              [
                new SelectAttribute(
                  _('Type'), _('Type of column'),
                  'columnType', TRUE,
                  $types, 'LinkColumn'
                ),
                new StringAttribute(
                  _('Attribute'), _('LDAP attributes to display, comma separated. Special values "nameAttr" and "mainAttr" also work.'),
                  'columnAttribute', FALSE
                ),
                new StringAttribute(
                  _('Label'), _('Column title'),
                  'columnLabel', FALSE
                ),
              ],
              'ManagementConfiguration'
            ),
            TRUE, // ordered
            [],
            TRUE // edition
          ),
          new BooleanAttribute(
            _('Persistent'), _('Should this configuration be saved in the LDAP as your default configuration for this management page'),
            'saveInLdapUser', FALSE,
            FALSE,
            'ManagementConfiguration'
          ),
          new ButtonAttribute(
            _('Forget my persistent configuration'), _('Delete the persistent configuration for this management page so that the default one is used'),
            'resetInLdapUser', _('Forget'),
            NULL, '',
            'ManagementConfiguration'
          ),
          new BooleanAttribute(
            _('Global default'), _('Should this configuration be saved in the LDAP as the default configuration for this management page for all users'),
            'saveInLdap', FALSE,
            FALSE,
            'fdManagementConfig'
          ),
          new ButtonAttribute(
            _('Forget global default'), _('Delete the global default configuration for this management page so that the default one is used'),
            'resetInLdap', _('Forget'),
            NULL, '',
            'fdManagementConfig'
          ),
        ]
      ],
    ];
  }

  function __construct (management $parent)
  {
    global $config;

    parent::__construct(NULL, NULL, $parent);

    $this->attributesAccess['saveInLdap']->setInLdap(FALSE);
    $this->attributesAccess['saveInLdapUser']->setInLdap(FALSE);
    if (!$this->attrIsWriteable('saveInLdap')) {
      $this->attributesAccess['saveInLdap']->setVisible(FALSE);
    }
    $this->attributesAccess['resetInLdap']->setInLdap(FALSE);
    $this->attributesAccess['resetInLdapUser']->setInLdap(FALSE);
    if (!$this->attrIsWriteable('resetInLdap')) {
      $this->attributesAccess['resetInLdap']->setVisible(FALSE);
    }

    if (!$config->hasManagementConfig(get_class($this->parent), TRUE)) {
      $this->attributesAccess['resetInLdapUser']->setVisible(FALSE);
    }
    if (!$config->hasManagementConfig(get_class($this->parent), FALSE)) {
      $this->attributesAccess['resetInLdap']->setVisible(FALSE);
    }

    $this->attributesAccess['managementColumns']->setInLdap(FALSE);
    $this->attributesAccess['managementColumns']->setLinearRendering(FALSE);
    $columnInfos  = $this->parent->getColumnConfiguration();
    $value        = [];
    foreach ($columnInfos as $column) {
      if (!isset($column[1]['attributes'])) {
        $column[1]['attributes'] = '';
      } elseif (is_array($column[1]['attributes'])) {
        $column[1]['attributes'] = json_encode($column[1]['attributes']);
      }
      if (!isset($column[1]['label'])) {
        $column[1]['label'] = '';
      }
      $value[] = [$column[0], $column[1]['attributes'], $column[1]['label']];
    }
    $this->attributesAccess['managementColumns']->setValue($value);
  }

  function attrIsReadable ($attr): bool
  {
    return $this->attrIsWriteable($attr);
  }

  function attrIsWriteable ($attr): bool
  {
    global $config, $ui;

    $noAclAttrs   = ['managementColumns', 'saveInLdapUser', 'resetInLdapUser'];
    $configAttrs  = ['saveInLdap', 'resetInLdap'];

    if ((is_object($attr) && in_array($attr->getLdapName(), $noAclAttrs)) || in_array($attr, $noAclAttrs)) {
      return TRUE;
    } elseif ((is_object($attr) && in_array($attr->getLdapName(), $configAttrs)) || in_array($attr, $configAttrs)) {
      $acl = $ui->get_permissions(CONFIGRDN.$config->current['BASE'], 'configuration/configInLdap', 'fdManagementConfig', $this->readOnly());
      return (strpos($acl, 'w') !== FALSE);
    } else {
      return parent::attrIsWriteable($attr);
    }
  }

  function handle_resetInLdapUser ()
  {
    global $config;
    $errors = $config->updateManagementConfig(get_class($this->parent), NULL, TRUE);
    msg_dialog::displayChecks($errors);
    if (empty($errors)) {
      $this->attributesAccess['resetInLdapUser']->setVisible(FALSE);
    }
  }

  function handle_resetInLdap ()
  {
    global $config;
    $errors = $config->updateManagementConfig(get_class($this->parent), NULL, FALSE);
    msg_dialog::displayChecks($errors);
    if (empty($errors)) {
      $this->attributesAccess['resetInLdap']->setVisible(FALSE);
    }
  }

  public function render (): string
  {
    global $config, $ui;

    $smarty = get_smarty();
    $smarty->assign('ManagementConfigurationACL', 'rw');
    $smarty->assign('fdManagementConfigACL', $ui->get_permissions(CONFIGRDN.$config->current['BASE'], 'configuration/configInLdap', 'fdManagementConfig', $this->readOnly()));
    return parent::render();
  }

  public function save (): array
  {
    global $config;
    $columnInfos  = [];
    $values       = $this->managementColumns;
    foreach ($values as $value) {
      $column = [$value[0], []];
      if (!empty($value[1])) {
        $jsonDecoded = json_decode($value[1], TRUE);
        if ($jsonDecoded !== NULL) {
          $column[1]['attributes'] = $jsonDecoded;
        } else {
          $column[1]['attributes'] = $value[1];
        }
      }
      if (!empty($value[2])) {
        $column[1]['label'] = $value[2];
      }
      $columnInfos[] = $column;
    }
    $this->parent->setColumnConfiguration($columnInfos);

    if ($this->saveInLdapUser) {
      return $config->updateManagementConfig(get_class($this->parent), $columnInfos, TRUE);
    }

    if ($this->saveInLdap) {
      return $config->updateManagementConfig(get_class($this->parent), $columnInfos);
    }

    return [];
  }
}
