<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2019-2020  FusionDirectory

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

class ACLPermissions
{
  static protected $letters = [
    'r' => 'read',
    'w' => 'write',
    'c' => 'create',
    'd' => 'delete',
    'm' => 'move',
  ];

  protected $read;
  protected $write;
  protected $create;
  protected $delete;
  protected $move;

  /* Rights on self only */
  protected $self;

  public function __construct (string $rights = '')
  {
    foreach (static::$letters as $letter => $var) {
      $this->$var = (strpos($rights, $letter) !== FALSE);
    }
    $this->self = (strpos($rights, 's') !== FALSE);
  }

  public function toString (bool $readOnly = FALSE): string
  {
    $string = ($this->self ? 's' : '');
    if ($readOnly) {
      return $string.($this->read ? 'r' : '');
    } else {
      foreach (static::$letters as $letter => $var) {
        if ($this->$var) {
          $string .= $letter;
        }
      }
      return $string;
    }
  }

  public function __toString ()
  {
    return $this->toString(FALSE);
  }

  public function merge (ACLPermissions $merge)
  {
    foreach (static::$letters as $var) {
      $this->$var = ($this->$var || $merge->$var);
    }
  }

  public function isSelf (): bool
  {
    return $this->self;
  }

  public function isFull (): bool
  {
    return ($this->read && $this->write && $this->create && $this->delete && $this->move);
  }
}
