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

/*!
 * \brief Column showing the activated tabs of the object
 */
class PropertiesColumn extends Column
{
  function isSortable()
  {
    return FALSE;
  }

  function fillNeededAttributes(array &$attrs)
  {
    $attrs['objectClass'] = '*';
  }

  function renderCell(ListingEntry $entry)
  {
    global $config;

    $infos = objects::infos($entry->getTemplatedType());

    static $tabs = array();

    if (empty($tabs[$entry->type])) {
      $tabs[$entry->type] = array();
      foreach ($config->data['TABS'][$infos['tabGroup']] as $plug) {
        if ($plug['CLASS'] == $infos['mainTab']) {
          continue;
        }
        if (class_available($plug['CLASS'])) {
          $name = $plug['CLASS'];

          $tabs[$entry->type][$name] = new $name('new');
        }
      }
    }

    /* Main tab is always there */
    $pInfos = pluglist::pluginInfos($infos['mainTab']);
    $result = '<input type="image" src="'.htmlentities((isset($pInfos['plSmallIcon']) ? $pInfos['plSmallIcon'] : $infos['icon']), ENT_COMPAT, 'UTF-8').'" '.
               'alt="'.$pInfos['plShortName'].'" title="'.$pInfos['plShortName'].'" '.
               'name="listing_edit_tab_'.$infos['mainTab'].'_'.$entry->row.'"/>';
    if (!empty($entry)) {
      if ($entry->isTemplate()) {
        $attrs = $entry->getTemplatedFields();
      } else {
        $attrs = $entry;
      }
      foreach ($tabs[$entry->type] as $class => $tab) {
        if ($tab->is_this_account($attrs)) {
          $pInfos = pluglist::pluginInfos($class);
          if (isset($pInfos['plSmallIcon'])) {
            $result .= '<input type="image" src="'.htmlentities($pInfos['plSmallIcon'], ENT_COMPAT, 'UTF-8').'" '.
                       'alt="'.$pInfos['plShortName'].'" title="'.$pInfos['plShortName'].'" '.
                       'name="listing_edit_tab_'.$class.'_'.$entry->row.'"/>';
          } else {
            @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $pInfos['plShortName']." ($class)", 'No icon for');
          }
        } else {
          $result .= '<img src="images/empty.png" alt="" class="optional '.$class.'"/>';
        }
      }
    }

    return $result;
  }
}
