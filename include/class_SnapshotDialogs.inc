<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class SnapshotCreateDialog extends simplePlugin
{
  public $aclCategory;

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Creating an object snapshot'),
        'attrs' => array(
          new DisplayAttribute (
            _('Object'), _('DN of the object you are creating a snapshot of'),
            'object_dn', FALSE
          ),
          new DisplayAttribute (
            _('Timestamp'), _('Timestamp of this snapshot creation'),
            'timestamp', FALSE
          ),
          new TextAreaAttribute (
            _('Reason'), _('Reason for creating this snapshot'),
            'description', TRUE
          ),
        )
      ),
    );
  }

  function __construct ($dn, $parent, $aclCategory)
  {
    parent::__construct();
    $this->attributesAccess['description']->setInLdap(FALSE);
    $this->object_dn    = $dn;
    $this->parent       = $parent;
    $this->aclCategory  = $aclCategory;
  }

  function execute ()
  {
    $this->timestamp  = date(_('Y-m-d, H:i:s'));
    $str = parent::execute();

    $str .= '<p class="plugbottom">'.
           '  <input type="submit" name="edit_finish" value="'.msgPool::saveButton().'"/>&nbsp;'.
           '  <input type="submit" name="edit_cancel" value="'.msgPool::cancelButton().'"/>'.
           '</p>';

    return $str;
  }

  function save ()
  {
    $this->parent->createSnapshot($this->object_dn, $this->description);
  }
}

class SnapshotsAttribute extends OrderedArrayAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = array(), $acl = '')
  {
    Attribute::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->edit_enabled = FALSE;
    $this->order        = FALSE;
    $this->attribute    = FALSE;
    $this->setInLdap(FALSE);
  }

  function loadPostValue ()
  {
    if ($this->isVisible()) {
      parent::loadPostValue();
      parent::applyPostValue();
    }
  }

  protected function handleEdit($key)
  {
  }

  function applyPostValue ()
  {
  }

  function addValue($value)
  {
    $this->value[] = $value;
    $this->reIndexValues();
  }

  function renderButtons ()
  {
    return '';
  }

  protected function getAttributeArrayValue($key, $value)
  {
    $values = array(
      date(_('Y-m-d, H:i:s'), preg_replace('/\-.*$/', '', $value['gosaSnapshotTimestamp'][0])),
      $value['description'][0]
    );
    if ($this->plugin->global) {
      array_unshift($values, $value['gosaSnapshotDN'][0]);
    }
    return $values;
  }

  protected function genRowIcons($key, $value)
  {
    $id = $this->getHtmlId();
    list ($img, $nbicons) = parent::genRowIcons($key, $value);
    $img = $this->renderInputField(
      'image', $id.'_restore_'.$key,
      array(
        'src'   => 'geticon.php?context=actions&amp;icon=document-restore&amp;size=16',
        'title' => _('Restore'),
        'alt'   => _('Restore'),
        'class' => 'center'
      )
    ).$img;
    $nbicons++;

    return array ($img, $nbicons);
  }

  protected function handlePostValueActions($id, $postValue)
  {
    if (parent::handlePostValueActions($id, $postValue)) {
      return TRUE;
    }
    if (preg_match('/^'.$id.'_restore_/', $postValue)) {
      $key = preg_replace('/^'.$id.'_restore_/', '', $postValue);
      $key = preg_replace('/_[xy]$/', '', $key);

      $this->plugin->triggerRestore($this->value[$key]['dn']);
      return TRUE;
    }
    return FALSE;
  }

  function delPostValue ($key)
  {
    $this->plugin->triggerDelete($this->value[$key]['dn']);
  }
}

class SnapshotRestoreDialog extends simplePlugin
{
  public $global;
  public $aclCategory;

  protected $snapDn;
  protected $snapAction;

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Restoring snapshots'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new DisplayAttribute (
            _('Object'), _('DN of the object you are creating a snapshot of'),
            'object_dn', FALSE
          ),
          new SnapshotsAttribute (
            _('Snapshots'), _('Existing snapshots for this object'),
            'snapshots', FALSE
          ),
        )
      ),
    );
  }

  function __construct ($dn, $parent, $global, $aclCategory)
  {
    parent::__construct();
    $this->object_dn    = $dn;
    $this->parent       = $parent;
    $this->global       = $global;
    $this->aclCategory  = $aclCategory;
    if ($this->global) {
      $this->attributesAccess['object_dn']->setVisible(FALSE);
    }
    $this->updateList();
  }

  function updateList ()
  {
    if ($this->global) {
      $this->snapshots = $this->parent->getAllDeletedSnapshots(TRUE);
    } else {
      $this->snapshots = $this->parent->getAvailableSnapsShots($this->object_dn, TRUE);
    }
  }

  function execute ()
  {
    if ($this->dialog == 'delete') {
      $objects = array(
        array(
          'name'  => 'snapshot',
          'dn'    => LDAP::fix($this->snapDn),
          'icon'  => 'geticon.php?context=actions&amp;icon=document-restore&amp;size=16',
          'type'  => 'Snapshot'
        )
      );

      // Display confirmation dialog.
      $smarty = get_smarty();
      $smarty->assign('objects', $objects);
      $str = $smarty->fetch(get_template_path('simple-remove.tpl'));
    } elseif ($this->dialog == 'restore') {
      $objects = array(
        array(
          'name'  => 'snapshot',
          'dn'    => LDAP::fix($this->snapDn),
          'icon'  => 'geticon.php?context=actions&amp;icon=document-restore&amp;size=16',
          'type'  => 'Snapshot'
        )
      );

      // Display confirmation dialog.
      $smarty = get_smarty();
      $smarty->assign('objects', $objects);
      $str = $smarty->fetch(get_template_path('restore-confirm.tpl'));
    } else {
      $str = parent::execute();
      $str .= '<p class="plugbottom">'.
             '  <input type="submit" name="edit_cancel" value="'.msgPool::backButton().'"/>'.
             '</p>';
    }

    return $str;
  }

  function triggerRestore($dn)
  {
    $this->dialog = 'restore';
    $this->snapDn = $dn;
  }

  function triggerDelete($dn)
  {
    $this->dialog = 'delete';
    $this->snapDn = $dn;
  }

  function save_object()
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

  function save ()
  {
  }
}
?>
