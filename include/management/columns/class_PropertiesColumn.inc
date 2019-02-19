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

  function __construct (managementListing $parent, array $attributes = NULL, $label = NULL)
  {
    global $config;

    parent::__construct($parent, NULL, $label);

    $this->tabs = array();

    foreach ($this->parent->parent->objectTypes as $type) {
      $infos = objects::infos($type);
      $this->tabs[$type] = array();
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

  function renderCell (ListingEntry $entry)
  {
    $tabs   = $this->computeIcons($entry);
    $result = '';
    foreach ($tabs as $tab) {
      if (empty($tab['icon'])) {
        $result .= '<img src="images/empty.png" alt="" class="optional '.$tab['tab'].'"/>';
      } else {
        $result .= '<input type="image" src="'.htmlentities($tab['icon'], ENT_COMPAT, 'UTF-8').'" '.
                    'alt="'.htmlentities($tab['title'], ENT_COMPAT, 'UTF-8').'" title="'.htmlentities($tab['title'], ENT_COMPAT, 'UTF-8').'" '.
                    'name="listing_edit_tab_'.$tab['tab'].'_'.$entry->row.'"/>';
      }
    }

    return $result;
  }

  function compare (ListingEntry $ao, ListingEntry $bo)
  {
    if ($ao->getTemplatedType() != $bo->getTemplatedType()) {
      return strcmp($ao->getTemplatedType(), $bo->getTemplatedType());
    }

    // Extract values from ao and bo
    $a = $this->computeSortString($ao);
    $b = $this->computeSortString($bo);

    return strcmp($a, $b);
  }

  protected function computeSortString (ListingEntry $entry)
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
    ));;

    return $entry->cache[__CLASS__]['sort'];
  }

  protected function computeIcons (ListingEntry $entry)
  {
    if (isset($entry->cache[__CLASS__]['icons'])) {
      return $entry->cache[__CLASS__]['icons'];
    }

    $infos = objects::infos($entry->getTemplatedType());
    $icons = array();

    /* Main tab is always there */
    $pInfos = pluglist::pluginInfos($infos['mainTab']);
    $icons[]  = array(
      'icon'  => (isset($pInfos['plSmallIcon']) ? $pInfos['plSmallIcon'] : $infos['icon']),
      'title' => $pInfos['plShortName'],
      'tab'   => $infos['mainTab'],
    );
    if (!empty($entry)) {
      if ($entry->isTemplate()) {
        $attrs = $entry->getTemplatedFields();
      } else {
        $attrs = $entry;
      }
      foreach ($this->tabs[$entry->getTemplatedType()] as $class) {
        $status = $class::isAccount($attrs);
        if ($status !== FALSE) {
          $pInfos = pluglist::pluginInfos($class);
          if (isset($pInfos['plSmallIcon'])) {
            $icons[] = array(
              'icon'  => $pInfos['plSmallIcon'],
              'title' => $pInfos['plShortName'],
              'tab'   => $class,
            );
          } else {
            @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $pInfos['plShortName']." ($class)", 'No icon for');
          }
        } else {
          $icons[] = array(
            'tab' => $class,
          );
        }
      }
    }

    $entry->cache[__CLASS__]['icons'] = $icons;
    return $entry->cache[__CLASS__]['icons'];
  }
}
