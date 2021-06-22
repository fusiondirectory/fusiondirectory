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
 * \brief Attribute which list the snapshots
 */
class SnapshotsAttribute extends OrderedArrayAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $acl = '')
  {
    \FusionDirectory\Core\SimplePlugin\Attribute::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->edit_enabled = FALSE;
    $this->order        = FALSE;
    $this->attribute    = FALSE;
    $this->setInLdap(FALSE);
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      if ($this->plugin->global) {
        $this->setHeaders([
          _('DN'),
          _('Date'),
          _('Description'),
          ''
        ]);
      } else {
        $this->setHeaders([
          _('Date'),
          _('Description'),
          ''
        ]);
      }
    }
  }

  function loadPostValue ()
  {
    if ($this->isVisible()) {
      parent::loadPostValue();
      parent::applyPostValue();
    }
  }

  protected function handleEdit ($key)
  {
  }

  function applyPostValue ()
  {
  }

  function addValue (string $dn, $entry = NULL)
  {
    if ($entry !== NULL) {
      $this->value[] = $entry;
      $this->reIndexValues();
    }
  }

  function renderButtons ()
  {
    return '';
  }

  protected function getAttributeArrayValue ($key, $value)
  {
    $values = [
      date(_('Y-m-d, H:i:s'), preg_replace('/\-.*$/', '', $value['gosaSnapshotTimestamp'][0])),
      $value['description'][0]
    ];
    if ($this->plugin->global) {
      array_unshift($values, $value['gosaSnapshotDN'][0]);
    }
    return $values;
  }

  protected function genRowIcons ($key, $value)
  {
    $id = $this->getHtmlId();
    list ($img, $nbicons) = parent::genRowIcons($key, $value);
    $img = $this->renderInputField(
      'image', $id.'_restore_'.$key,
      [
        'src'   => 'geticon.php?context=actions&icon=document-restore&size=16',
        'title' => _('Restore'),
        'alt'   => _('Restore'),
        'class' => 'center'
      ],
      FALSE
    ).$img;
    $nbicons++;

    return  [$img, $nbicons];
  }

  protected function handlePostValueActions ($id, $postValue)
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
