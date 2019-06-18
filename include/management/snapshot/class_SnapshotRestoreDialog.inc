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
 * \brief Snapshot restoration dialog
 */
class SnapshotRestoreDialog extends ManagementDialog
{
  public $global;
  public $aclCategory;

  protected $snapDn;
  protected $snapAction;

  static function plInfo (): array
  {
    return [
      'plShortName' => 'SnapshotRestoreDialog',
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'main' => [
        'name'  => _('Restoring snapshots'),
        'class' => ['fullwidth'],
        'attrs' => [
          new DisplayAttribute(
            _('Object'), _('DN of the object you are creating a snapshot of'),
            'object_dn', FALSE
          ),
          new SnapshotsAttribute(
            _('Snapshots'), _('Existing snapshots for this object'),
            'snapshots', FALSE,
            [],
            'SnapshotHandler'
          ),
        ]
      ],
    ];
  }

  function __construct (string $dn, management $parent, bool $global, array $aclCategories)
  {
    $this->global       = $global;
    parent::__construct(NULL, NULL, $parent);
    $this->object_dn    = $dn;
    $this->acl_base     = $dn;
    $this->aclCategory  = $aclCategories;
    if ($this->global) {
      $this->attributesAccess['object_dn']->setVisible(FALSE);
    }
    $this->updateList();
  }

  function updateList ()
  {
    if ($this->global) {
      $this->snapshots = $this->parent->getAllDeletedSnapshots();
    } else {
      $this->snapshots = $this->parent->getAvailableSnapsShots($this->object_dn);
    }
  }

  function execute (): string
  {
    global $ui;
    if ($this->dialog == 'delete') {
      $objects = [
        [
          'name'  => 'snapshot',
          'dn'    => $this->snapDn,
          'icon'  => 'geticon.php?context=actions&amp;icon=document-restore&amp;size=16',
          'type'  => 'Snapshot'
        ]
      ];

      // Display confirmation dialog.
      $smarty = get_smarty();
      $smarty->assign('objects', $objects);
      $str = $smarty->fetch(get_template_path('simple-remove.tpl'));
    } elseif ($this->dialog == 'restore') {
      $objects = [
        [
          'name'  => 'snapshot',
          'dn'    => $this->snapDn,
          'icon'  => 'geticon.php?context=actions&amp;icon=document-restore&amp;size=16',
          'type'  => 'Snapshot'
        ]
      ];

      // Display confirmation dialog.
      $smarty = get_smarty();
      $smarty->assign('objects', $objects);
      $str = $smarty->fetch(get_template_path('restore-confirm.tpl'));
    } else {
      $smarty = get_smarty();
      $permissions = $ui->get_snapshot_permissions($this->acl_base, $this->aclCategory);
      $acl = '';
      if (in_array('r', $permissions)) {
        $acl .= 'r';
      }
      if (in_array(($this->global ? 'restore_deleted' : 'restore_over'), $permissions)) {
        $acl .= 'w';
      }
      if (in_array('d', $permissions)) {
        $acl .= 'd';
      }
      $smarty->assign('SnapshotHandlerACL', $acl);
      $str = parent::execute();
      $str .= '<p class="plugbottom">'.
             '  <input type="submit" formnovalidate="formnovalidate" name="'.$this->post_cancel.'" value="'.msgPool::backButton().'"/>'.
             '</p>';
    }

    return $str;
  }

  function attrIsWriteable ($attr): bool
  {
    global $ui;

    if (!is_object($attr)) {
      $attr = $this->attributesAccess[$attr];
    }
    if ($attr->getLdapName() == 'snapshots') {
      return in_array(($this->global ? 'restore_deleted' : 'restore_over'), $ui->get_snapshot_permissions($this->acl_base, $this->aclCategory));
    } else {
      return parent::attrIsWriteable($attr);
    }
  }

  function triggerRestore ($dn)
  {
    $this->dialog = 'restore';
    $this->snapDn = $dn;
  }

  function triggerDelete ($dn)
  {
    $this->dialog = 'delete';
    $this->snapDn = $dn;
  }

  function save_object ()
  {
    if ($this->dialog == 'delete') {
      if (isset($_POST['delete_confirmed'])) {
        $this->parent->removeSnapshot($this->snapDn);
        $this->closeDialog();
      } elseif (isset($_POST['delete_cancel'])) {
        $this->closeDialog();
      }
    } elseif ($this->dialog == 'restore') {
      if (isset($_POST['restore_confirmed'])) {
        $this->parent->restoreSnapshot($this->snapDn);
        $this->closeDialog();
      } elseif (isset($_POST['restore_cancel'])) {
        $this->closeDialog();
      }
    } else {
      parent::save_object();
    }
  }

  function closeDialog ()
  {
    parent::closeDialog();
    $this->snapDn = NULL;
    $this->updateList();
  }

  function save (): array
  {
    return [];
  }
}
