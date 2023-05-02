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
  protected $tabs;

  function __construct (managementListing $parent, array $attributes = NULL, string $label = NULL)
  {
    global $config;

    parent::__construct($parent, NULL, $label);

    $this->tabs = [];

    foreach ($this->parent->parent->objectTypes as $type) {
      $infos = objects::infos($type);
      $this->tabs[$type] = [];
      foreach ($config->data['TABS'][$infos['tabGroup']] as $plug) {
        if ($plug['CLASS'] == $infos['mainTab']) {
          continue;
        }
        if (class_available($plug['CLASS'])) {
          $this->tabs[$type][] = $plug['CLASS'];
        }
      }
    }
  }

  function fillNeededAttributes (array &$attrs)
  {
    foreach ($this->tabs as $classes) {
      foreach ($classes as $class) {
        $class::fillAccountAttrsNeeded($attrs);
      }
    }
  }

  function renderCell (ListingEntry $entry): string
  {
    $tabInfos = $this->computeIcons($entry);
    $result   = '';
    foreach ($tabInfos as $tabInfo) {
      if (empty($tabInfo['icon'])) {
        $result .= '<img src="images/empty.png" alt="" class="optional '.$tabInfo['tab'].'"/>';
      } else {
        $result .= '<input type="image" src="'.htmlescape($tabInfo['icon']).'" '.
                    'alt="'.htmlescape($tabInfo['title']).'" title="'.htmlescape($tabInfo['title']).'" '.
                    'name="listing_edit_tab_'.$tabInfo['tab'].'_'.$entry->row.'"/>';
      }
    }

    return $result;
  }

  function compare (ListingEntry $ao, ListingEntry $bo): int
  {
    if ($ao->getTemplatedType() != $bo->getTemplatedType()) {
      return strcmp($ao->getTemplatedType(), $bo->getTemplatedType());
    }

    // Extract values from ao and bo
    $a = $this->computeSortString($ao);
    $b = $this->computeSortString($bo);

    return strcmp($a, $b);
  }

  protected function computeSortString (ListingEntry $entry): string
  {
    if (isset($entry->cache[__CLASS__]['sort'])) {
      return $entry->cache[__CLASS__]['sort'];
    }

    $icons = $this->computeIcons($entry);
    $entry->cache[__CLASS__]['sort'] = implode('', array_map(
      function($tab)
      {
        return (empty($tab['icon']) ? 0 : 1);
      },
      $icons
    ));

    return $entry->cache[__CLASS__]['sort'];
  }

  protected function computeIcons (ListingEntry $entry): array
  {
    global $ui;

    if (isset($entry->cache[__CLASS__]['icons'])) {
      return $entry->cache[__CLASS__]['icons'];
    }

    $infos = objects::infos($entry->getTemplatedType());
    $icons = [];

    /* Main tab is always there */
    $pInfos = pluglist::pluginInfos($infos['mainTab']);
    $icons[]  = [
      'icon'  => (isset($pInfos['plSmallIcon']) ? $pInfos['plSmallIcon'] : $infos['icon']),
      'title' => $pInfos['plShortName'],
      'tab'   => $infos['mainTab'],
    ];
    if (!empty($entry)) {
      if ($entry->isTemplate()) {
        $attrs = $entry->getTemplatedFields();
      } else {
        $attrs = $entry;
      }
      foreach ($this->tabs[$entry->getTemplatedType()] as $class) {
        if (empty($ui->get_permissions($entry->dn, $infos['aclCategory'].'/'.$class))) {
          /* Skip tabs we have no ACL rights on */
          continue;
        }

        $status = $class::isAccount($attrs);
        if ($status !== FALSE) {
          $pInfos = pluglist::pluginInfos($class);
          if (isset($pInfos['plSmallIcon'])) {
            $icons[] = [
              'icon'  => $pInfos['plSmallIcon'],
              'title' => $pInfos['plShortName'],
              'tab'   => $class,
            ];
          } else {
            logging::debug(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $pInfos['plShortName']." ($class)", 'No icon for');
          }
        } else {
          $icons[] = [
            'tab' => $class,
          ];
        }
      }
    }

    $entry->cache[__CLASS__]['icons'] = $icons;
    return $entry->cache[__CLASS__]['icons'];
  }
}
