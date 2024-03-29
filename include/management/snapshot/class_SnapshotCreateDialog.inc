<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2019  FusionDirectory

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

/*!
 * \brief Snapshot creation dialog
 */
class SnapshotCreateDialog extends ManagementDialog
{
  public $aclCategory;

  static function plInfo (): array
  {
    return [
      'plShortName' => 'SnapshotCreateDialog',
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Creating an object snapshot'),
        'attrs' => [
          new DisplayAttribute(
            _('Object'), _('DN of the object you are creating a snapshot of'),
            'object_dn', FALSE
          ),
          new DisplayAttribute(
            _('Timestamp'), _('Timestamp of this snapshot creation'),
            'timestamp', FALSE
          ),
          new TextAreaAttribute(
            _('Reason'), _('Reason for creating this snapshot'),
            'description', TRUE,
            '',
            'SnapshotHandler'
          ),
        ]
      ],
      'dataSource' => [
        'name'  => _('dataSource - only available via web-service.'),
        'attrs' => [
            new SelectAttribute(
              'Data source', _('Origin / Source of the data'),
              'snapshotSource', FALSE,
            ),
        ]
      ],
    ];
  }

  function __construct (string $dn, management $parent, string $aclCategory)
  {
    parent::__construct(NULL, NULL, $parent);
    // The attribut will be passed to parent for later saving, dataSource might require same logic.
    $this->attributesAccess['description']->setInLdap(FALSE);
    $this->attributesAccess['snapshotSource']->setInLdap(FALSE);
    $this->attributesAccess['snapshotSource']->setVisible(FALSE);

    $recordedDataSources = $this->getLdapRecordedDataSources();
    if (!empty($recordedDataSources)) {
      $this->attributesAccess['snapshotSource']->setChoices($recordedDataSources);
    }

    $this->object_dn    = $dn;
    $this->aclCategory  = $aclCategory;
  }

  /*
   * Retrieve the data sources from configuration.
   */
  public function getLdapRecordedDataSources () : array
  {
    global $config;

    $recordedDataSources = [];
    if (isset($config->current['SNAPSHOTSOURCEDATA']) && !empty($config->current['SNAPSHOTSOURCEDATA'])) {
      $recordedDataSources = $config->current['SNAPSHOTSOURCEDATA'];
    }

    return $recordedDataSources;
  }

  /*!
   * \brief Get LDAP base to use for ACL checks
   */
  function getAclBase (bool $callParent = TRUE): string
  {
    return $this->object_dn;
  }

  /*! \brief Check if logged in user have enough right to write this attribute value
   *
   * \param mixed $attr Attribute object or name (in this case it will be fetched from attributesAccess)
   */
  function attrIsWriteable ($attr): bool
  {
    global $ui;

    if (!is_object($attr)) {
      $attr = $this->attributesAccess[$attr];
    }
    if ($attr->getLdapName() == 'description') {
      return in_array('c', $ui->get_snapshot_permissions($this->object_dn, $this->aclCategory));
    } else {
      return parent::attrIsWriteable($attr);
    }
  }

  function renderAttributes (bool $readOnly = FALSE)
  {
    global $ui;
    $smarty = get_smarty();

    $permissions = $ui->get_snapshot_permissions($this->object_dn, $this->aclCategory);
    $acl = '';
    if (in_array('c', $permissions)) {
      $acl .= 'crw';
    }
    $smarty->assign('SnapshotHandlerACL', $acl);

    return parent::renderAttributes($readOnly);
  }

  public function update (): bool
  {
    $this->timestamp  = date(_('Y-m-d, H:i:s'));
    return parent::update();
  }

  function save (): array
  {
    // snapshotSource is always set but can be empty and must be defaulted.
    if (empty($this->snapshotSource)) {
      $this->snapshotSource = 'FD';
    }
    $this->parent->createSnapshot($this->object_dn, $this->description, $this->snapshotSource);
    return [];
  }

  function fillHookAttrs (array &$addAttrs)
  {
    parent::fillHookAttrs($addAttrs);
    foreach (array_keys($this->attributesAccess) as $attr) {
      if (!isset($addAttrs[$attr])) {
        $addAttrs[$attr] = $this->$attr;
      }
    }
  }
}
