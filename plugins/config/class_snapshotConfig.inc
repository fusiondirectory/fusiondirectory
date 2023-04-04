<?php
/*
This code is part of FusionDirectory (http://www.fusiondirectory.org/)
Copyright (C) 2012-2023 FusionDirectory

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

class snapshotsConfig extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Snapshots'),
      'plDescription'   => _('FusionDirectory Snapshot Configuration'),
      'plObjectClass'   => ['fusionDirectoryConf'],
      'plObjectType'    => ['configuration'],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    global $config;

    return [
      'snapshotsConf' => [
        'name'  => _('Snapshots Configuration'),
        'attrs' => [
          new BooleanAttribute(
            _('Enable snapshots'), _('This enables you to save certain states of entries and restore them later on.'),
            'fdEnableSnapshots', FALSE,
            TRUE
          ),
          new BooleanAttribute(
            _('Enable automatic snapshots'), _('This enables you to automatically create a snapshot upon saving if any modifications have been found.'),
            'fdEnableAutomaticSnapshots', FALSE,
            FALSE
          ),
          new StringAttribute(
            _('Snapshot base'), _('The base where snapshots should be stored inside the LDAP directory.'),
            'fdSnapshotBase', FALSE,
            'ou=snapshots,'.$config->current['BASE']
          ),
        ]
      ],
      'snapshotsAdvanceConf' => [
        'name'  => _('Snapshots Advance Configuration'),
        'attrs' => [
          new IntAttribute(
            _('Minimum number of snapshots to be kept'), _('Set the minimum number of snapshots to be kept'),
            'fdSnapshotMinRetention', FALSE, '', FALSE, ''
          ),
          new IntAttribute(
            _('Retention time in days'), _('Set the retention time in days for a snapshots to be kept'),
            'fdSnapshotRetentionDays', FALSE, '', FALSE, ''
          ),
        ]
      ],
      'OriginDataSource' => [
        'name' => _('List of available sources / origin of data'),
        'attrs' => [
          new SetAttribute(
            new StringAttribute(
              _('Origin / source of data'), _('Origin / Source of data'),
              'fdSnapshotSourceData', FALSE,
            )
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['fdEnableSnapshots']->setManagedAttributes(
      [
        'disable' => [
          FALSE => [
            'fdSnapshotBase',
            'fdEnableAutomaticSnapshots',
            'fdSnapshotMinRetention',
            'fdSnapshotRetentionDays',
          ]
        ]
      ]
    );
  }

}

