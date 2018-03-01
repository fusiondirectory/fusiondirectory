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

class ListingEntry implements ArrayAccess
{
  public $dn;
  public $row;
  public $type;
  private $attrs;
  protected $listing;

  public function __construct(managementListing $listing, $type, $dn, array $attrs, $row = NULL)
  {
    $this->listing  = $listing;
    $this->type     = $type;
    $this->dn       = $dn;
    $this->attrs    = $attrs;
    $this->row      = $row;
  }

  public function offsetSet($offset, $value)
  {
    $this->attrs[$offset] = $value;
  }

  public function offsetExists($offset)
  {
    return isset($this->attrs[$offset]);
  }

  public function offsetUnset($offset)
  {
    unset($this->attrs[$offset]);
  }

  public function offsetGet($offset)
  {
    return (isset($this->attrs[$offset]) ? $this->attrs[$offset] : NULL);
  }

  public function getPid()
  {
    return $this->listing->pid;
  }

  public function isTemplate()
  {
    return preg_match('/^template_/', $this->type);
  }

  public function getTemplatedType()
  {
    return preg_replace('/^template_/', '', $this->type);
  }

  public function getTemplatedFields()
  {
    return templateHandling::fieldsFromLDAP($this->attrs);
  }

  public function checkAcl($acls)
  {
    global $ui;

    $infos  = objects::infos($this->getTemplatedType());
    $rights = $ui->get_permissions($this->dn, $infos['aclCategory'].'/'.($this->isTemplate() ? 'template' : $infos['mainTab']));
    foreach (str_split($acls) as $acl) {
      if (strpos($rights, $acl) === FALSE) {
        return FALSE;
      }
    }

    return TRUE;
  }

  public function snapshotCreationAllowed()
  {
    global $ui;

    $infos  = objects::infos($this->getTemplatedType());
    return $ui->allow_snapshot_create($this->dn, $infos['aclCategory']);
  }

  public function snapshotRestoreAllowed()
  {
    global $ui;

    $infos  = objects::infos($this->getTemplatedType());
    return $ui->allow_snapshot_restore($this->dn, $infos['aclCategory'], FALSE);
  }
}
