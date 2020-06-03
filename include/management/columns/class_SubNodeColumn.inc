<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2017-2020  FusionDirectory

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
 * \brief Column showing an attribute from subnodes
 */
class SubNodeColumn extends LinkColumn
{
  function fillNeededAttributes (array &$attrs)
  {
    $attrs['dn'] = 'raw';
  }

  function fillSearchedAttributes (array &$attrs)
  {
  }

  protected function getAttributeValues (ListingEntry $entry): array
  {
    global $config;

    if (isset($entry->cache[__CLASS__]['values'])) {
      return $entry->cache[__CLASS__]['values'];
    }

    $values = [];

    $attrs = $this->attributes;
    if (isset($this->templateAttributes) && $entry->isTemplate()) {
      $attrs = $this->templateAttributes;
    }
    if (isset($attrs)) {
      $ldap = $config->get_ldap_link();
      $ldap->cd($entry->dn);
      $ldap->search('(objectClass=*)', $attrs, 'subtree');
      while ($node = $ldap->fetch(TRUE)) {
        if ($node['dn'] === $entry->dn) {
          continue;
        }
        foreach ($attrs as $attr) {
          if (isset($node[$attr])) {
            for ($i = 0; $i < $node[$attr]['count']; ++$i) {
              $values[] = $node[$attr][$i];
            }
          }
        }
      }
    }

    $entry->cache[__CLASS__]['values'] = $values;

    return $values;
  }
}
