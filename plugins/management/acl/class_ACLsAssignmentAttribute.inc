<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2019  FusionDirectory

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

class ACLsAssignmentAttribute extends DialogOrderedArrayAttribute
{
  protected $order        = TRUE;
  protected $dialogClass  = 'ACLsAssignmentDialog';
  protected $height       = 300;

  protected function getAttributeArrayValue ($key, $value)
  {
    /* Convert text value to displayable array value */
    sort($value['members']);
    static $nbShown = 4;
    $members = join(', ', array_slice($value['members'], 0, $nbShown));
    if (count($value['members']) > $nbShown) {
      $members .= sprintf(_(', and %d others'), (count($value['members']) - $nbShown));
    }
    $value['members'] = $members;
    return $value;
  }

  function readValue ($value)
  {
    $acl = explode(':', $value);
    return [$acl[0], [
      'scope'         => $acl[1],
      'role'          => base64_decode($acl[2]),
      'members'       => array_map('base64_decode', explode(',', $acl[3])),
      'userfilter'    => (isset($acl[4]) ? base64_decode($acl[4]) : ''),
      'targetfilter'  => (isset($acl[5]) ? base64_decode($acl[5]) : ''),
    ]];
  }

  function writeValue ($key, $value)
  {
    return $key.
            ':'.$value['scope'].
            ':'.base64_encode($value['role']).
            ':'.join(',', array_map('base64_encode', $value['members'])).
            ':'.base64_encode($value['userfilter']).
            ':'.base64_encode($value['targetfilter']);
  }

  function foreignKeyUpdate ($oldvalue, $newvalue, array $source)
  {
    foreach ($this->value as $key => &$value) {
      if ($source['FIELD'] == 'dn') {
        if ($newvalue === NULL) {
          if (preg_match('/'.preg_quote($oldvalue, '/').'$/', $value['role'])) {
            unset($this->value[$key]);
          }
          foreach ($value['members'] as $member_key => $member) {
            if (preg_match('/'.preg_quote($oldvalue, '/').'$/', $member)) {
              unset($value['members'][$member_key]);
            }
          }
          unset($member);
        } elseif ($source['MODE'] == 'move') {
          $value['role'] = preg_replace('/'.preg_quote($oldvalue, '/').'$/', $newvalue, $value['role']);
          foreach ($value['members'] as &$member) {
            $member = preg_replace('/'.preg_quote($oldvalue, '/').'$/', $newvalue, $member);
          }
          unset($member);
        } elseif ($source['MODE'] == 'copy') {
          /* Copied members are added */
          foreach ($value['members'] as $member) {
            if (preg_match('/'.preg_quote($oldvalue, '/').'$/', $member)) {
              $value['members'][] = preg_replace('/'.preg_quote($oldvalue, '/').'$/', $newvalue, $member);
            }
          }
        }
      } else {
        trigger_error('unknown source "'.$source['CLASS'].'" with field "'.$source['FIELD'].'"');
      }
    }
    unset($value);
  }

  function foreignKeyCheck ($oldvalue, array $source): bool
  {
    foreach ($this->value as $value) {
      if (
          (($source['CLASS'] == 'aclRole') && ($value['role'] == $oldvalue)) ||
          (in_array($source['CLASS'], ['user','posixGroup','roleGeneric']) && in_array($oldvalue, $value['members']))
        ) {
        return TRUE;
      } elseif (!in_array($source['CLASS'], ['aclRole','user','posixGroup','roleGeneric'])) {
        trigger_error('unknown source '.$source['CLASS']);
      }
    }
    return FALSE;
  }
}
