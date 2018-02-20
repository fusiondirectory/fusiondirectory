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
 * \brief Column base class
 */
class StringColumn
{
  protected $attribute;
  protected $label;
  protected $type = 'string';

  /* management class instance */
  protected $parent = NULL;

  function __construct($attribute = NULL, $label = NULL)
  {
    $this->attribute  = $attribute;
    $this->label      = $label;
  }

  function setParent(managementListing $parent)
  {
    $this->parent = $parent;
  }

  function isSortable()
  {
    return TRUE;
  }

  function getRowClasses()
  {
    return array();
  }

  function getHtmlProps()
  {
    return '';
  }

  function getHtmlCellProps()
  {
    return '';
  }

  function getLabel()
  {
    if (isset($this->label)) {
      return _($this->label);
    } else {
      return '&nbsp;';
    }
  }

  function fillNeededAttributes(array &$attrs)
  {
    if (isset($this->attribute)) {
      $attrs[$this->attribute] = '1';
    }
  }

  function renderCell($pid, $row, $dn, $type, array $entry)
  {
    if (isset($this->attribute) && isset($entry[$this->attribute])) {
      return htmlentities($entry[$this->attribute], ENT_COMPAT, 'UTF-8');
    } else {
      return '&nbsp;';
    }
  }

  function compare($ao, $bo)
  {
    if ($this->attribute == 'sort-attribute') {
      // Override sort attribute from data if needed
      $attribute_a = $ao['sort-attribute'];
      $attribute_b = $bo['sort-attribute'];
    } else {
      $attribute_a = $this->attribute;
      $attribute_b = $this->attribute;
    }

    // Extract values from ao and bo
    $a = $b = '';
    if (isset($ao[$attribute_a])) {
      $a = $ao[$attribute_a];
      if (is_array($a)) {
        $a = $a[0];
      }
    }
    if (isset($bo[$attribute_b])) {
      $b = $bo[$attribute_b];
      if (is_array($b)) {
        $b = $b[0];
      }
    }

    // Take a look at the several types
    switch ($this->type) {
      // Sort for string by default
      default:
      case 'string':
        return strcoll($a, $b);

      case 'department':
        return strnatcmp($a, $b);

      case 'integer':
        return $b - $a;

      case 'date':
        if ($a == '') {
          $a = '31.12.0000';
        }
        if ($b == '') {
          $b = '31.12.0000';
        }
        list($d, $m, $y) = explode('.', $a);
        $a = (int)sprintf('%04d%02d%02d', $y, $m, $d);
        list($d, $m, $y) = explode('.', $b);
        $b = (int)sprintf('%04d%02d%02d', $y, $m, $d);
        return $b - $a;

      case 'ip':
        $parts_a = explode('.', $a, 4);
        $parts_b = explode('.', $b, 4);
        for ($i = 0; $i < 4; $i++) {
          if ((int)($parts_a[$i]) != (int)($parts_b[$i])) {
            return (int)($parts_a[$i]) - (int)($parts_b[$i]);
          }
        }
        return 0;
    }
  }
}

class LinkColumn extends StringColumn
{
  function renderCell($pid, $row, $dn, $type, array $entry)
  {
    if (isset($entry[$this->attribute])) {
      return '<a href="?plug='.$_GET['plug'].'&amp;PID='.$pid.'&amp;act=listing_edit_'.$row.'" title="'.$dn.'">'.htmlentities($entry[$this->attribute], ENT_COMPAT, 'UTF-8').'</a>';
    } else {
      return '&nbsp;';
    }
  }
}

class ObjectTypeColumn extends StringColumn
{
  function isSortable()
  {
    return FALSE;
  }

  function renderCell($pid, $row, $dn, $type, array $entry)
  {
    if ($type) {
      $infos = objects::infos($type);
      return '<img class="center" title="'.$dn.'" src="'.htmlentities($infos['icon'], ENT_COMPAT, 'UTF-8').'" alt="'.$infos['name'].'"/>';
    } else {
      return '&nbsp;';
    }
  }
}

class ActionsColumn extends StringColumn
{
  function isSortable()
  {
    return FALSE;
  }

  function renderCell($pid, $row, $dn, $type, array $entry)
  {
    return $this->parent->parent->renderActionColumn($pid, $row, $dn, $type, $entry);
  }
}

class PropertiesColumn extends StringColumn
{
  function isSortable()
  {
    return FALSE;
  }

  function fillNeededAttributes(array &$attrs)
  {
    $attrs['objectClass'] = '*';
  }

  function renderCell($pid, $row, $dn, $type, array $entry)
  {
    global $config;

    $infos = objects::infos($type);

    static $tabs = array();

    if (empty($tabs[$type])) {
      $tabs[$type] = array();
      foreach ($config->data['TABS'][$infos['tabGroup']] as $plug) {
        if ($plug['CLASS'] == $infos['mainTab']) {
          continue;
        }
        if (class_available($plug['CLASS'])) {
          $name = $plug['CLASS'];

          $tabs[$type][$name] = new $name('new');
        }
      }
    }

    /* Main tab is always there */
    $pInfos = pluglist::pluginInfos($infos['mainTab']);
    $result = '<input class="center" type="image" src="'.htmlentities($pInfos['plSmallIcon'], ENT_COMPAT, 'UTF-8').'" '.
               'alt="'.$pInfos['plShortName'].'" title="'.$pInfos['plShortName'].'" '.
               'name="listing_edit_tab_'.$infos['mainTab'].'_'.$row.'"/>';
    if (!empty($entry)) {
      //~ if (in_array_ics ('fdTemplate', $attrs['objectClass'])) {
        //~ $attrs = templateHandling::fieldsFromLDAP($attrs);
      //~ }
      foreach ($tabs[$type] as $class => $tab) {
        if ($tab->is_this_account($entry)) {
          $pInfos = pluglist::pluginInfos($class);
          if (isset($pInfos['plSmallIcon'])) {
            $result .= '<input class="center" type="image" src="'.htmlentities($pInfos['plSmallIcon'], ENT_COMPAT, 'UTF-8').'" '.
                       'alt="'.$pInfos['plShortName'].'" title="'.$pInfos['plShortName'].'" '.
                       'name="listing_edit_tab_'.$class.'_'.$row.'"/>';
          } else {
            @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $pInfos['plShortName']." ($class)", 'No icon for');
          }
        } else {
          $result .= '<img src="images/empty.png" alt="" class="center optional '.$class.'"/>';
        }
      }
    }

    return $result;
  }
}
