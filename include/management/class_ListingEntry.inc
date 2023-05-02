<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2017-2019 FusionDirectory

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
  /*!
   * \brief LDAP dn if any, unique id otherwise
   */
  public $dn;

  /*!
   * \brief DN to use for ACL checks, usually the same as $dn
   */
  public $aclBase;

  /*!
   * \brief Row number
   */
  public $row;

  /*!
   * \brief Object type
   */
  public $type;

  private $attrs;
  protected $listing;

  /* Cache where columns may store stuff */
  public $cache = [];

  public function __construct (managementListing $listing, string $type, string $dn, array $attrs, int $row = NULL)
  {
    $this->listing  = $listing;
    $this->type     = $type;
    $this->dn       = $dn;
    $this->aclBase  = $dn;
    $this->attrs    = $attrs;
    $this->row      = $row;
  }

  public function offsetSet ($offset, $value)
  {
    $this->attrs[$offset] = $value;
  }

  public function offsetExists ($offset)
  {
    return isset($this->attrs[$offset]);
  }

  public function offsetUnset ($offset)
  {
    unset($this->attrs[$offset]);
  }

  public function offsetGet ($offset)
  {
    return (isset($this->attrs[$offset]) ? $this->attrs[$offset] : NULL);
  }

  public function getPid (): string
  {
    return $this->listing->pid;
  }

  public function isTemplate (): bool
  {
    return preg_match('/^template_/', $this->type);
  }

  public function getTemplatedType (): string
  {
    return preg_replace('/^template_/', '', $this->type);
  }

  public function getTemplatedFields (): array
  {
    return templateHandling::fieldsFromLDAP($this->attrs);
  }

  public function checkAcl (string $acls): bool
  {
    global $ui;

    $infos  = objects::infos($this->getTemplatedType());
    $rights = $ui->get_permissions($this->aclBase, $infos['aclCategory'].'/'.($this->isTemplate() ? 'template' : $infos['mainTab']));
    foreach (str_split($acls) as $acl) {
      if (strpos($rights, $acl) === FALSE) {
        return FALSE;
      }
    }

    return TRUE;
  }

  public function snapshotCreationAllowed (): bool
  {
    global $ui;

    $infos  = objects::infos($this->getTemplatedType());
    return $ui->allow_snapshot_create($this->aclBase, $infos['aclCategory']);
  }

  public function snapshotRestoreAllowed (): bool
  {
    global $ui;

    $infos  = objects::infos($this->getTemplatedType());
    return $ui->allow_snapshot_restore($this->aclBase, $infos['aclCategory'], FALSE);
  }
}
