<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2020  FusionDirectory

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
 * \file class_xml.inc
 * Source code for class xml
 */

/*!
 * \brief This class contains all the function needed to manage xml
 * files
 */
class xml
{
  /*!
   * \brief Transform a xml document to an array
   *
   * \param $contents Contents
   *
   * \param integer $get_attributes Initialized at 1
   *
   * \param string $priority Initialized at 'tag'
   */
  static function xml2array ($contents, $get_attributes = 1, $priority = 'tag')
  {
    if (!$contents) {
      return [];
    }

    if (!function_exists('xml_parser_create')) {
      trigger_error('xml_parser_create function does not exists');
      return [];
    }

    //Get the XML parser of PHP - PHP must have this module for the parser to work
    $parser = xml_parser_create('');
    xml_parser_set_option($parser, XML_OPTION_TARGET_ENCODING, "UTF-8"); // http://minutillo.com/steve/weblog/2004/6/17/php-xml-and-character-encodings-a-tale-of-sadness-rage-and-data-loss
    xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, 0);
    xml_parser_set_option($parser, XML_OPTION_SKIP_WHITE, 1);
    xml_parse_into_struct($parser, trim($contents), $xml_values);
    xml_parser_free($parser);

    if (!$xml_values) {
      return;
    }

    // Initializations
    $xml_array    = [];

    // Reference
    $current = &$xml_array;

    // Go through the tags.
    // Multiple tags with same name will be turned into an array
    $repeated_tag_index = [];
    foreach ($xml_values as $data) {
      $result = [];
      $attributes_data = [];

      if (isset($data['value'])) {
        if ($priority == 'tag') {
          $result = $data['value'];
        } else {
          // Put the value in a assoc array if we are in the 'Attribute' mode
          $result['value'] = $data['value'];
        }
      }

      //Set the attributes too.
      if (isset($data['attributes']) and $get_attributes) {
        foreach ($data['attributes'] as $attr => $val) {
          if ($priority == 'tag') {
            $attributes_data[$attr] = $val;
          } else {
            // Set all the attributes in a array called 'attr'
            $result['attr'][$attr] = $val;
          }
        }
      }

      // See tag status and do the needed.
      if ($data['type'] == 'open') {
        // The starting of the tag '<tag>'
        $parent[$data['level'] - 1] = &$current;
        if (!is_array($current) or (!in_array($data['tag'], array_keys($current)))) {
          // Insert New tag
          $current[$data['tag']] = $result;
          if ($attributes_data) {
            $current[$data['tag']. '_attr'] = $attributes_data;
          }
          $repeated_tag_index[$data['tag'].'_'.$data['level']] = 1;

          $current = &$current[$data['tag']];
        } else {
          // There was another element with the same tag name

          if (isset($current[$data['tag']][0])) {
            // If there is a 0th element it is already an array
            $current[$data['tag']][$repeated_tag_index[$data['tag'].'_'.$data['level']]] = $result;
            $repeated_tag_index[$data['tag'].'_'.$data['level']]++;
          } else {
            // This section will make the value an array if multiple tags with the same name appear together
            // This will combine the existing item and the new item together to make an array
            $current[$data['tag']] = [$current[$data['tag']],$result];
            $repeated_tag_index[$data['tag'].'_'.$data['level']] = 2;

            if (isset($current[$data['tag'].'_attr'])) {
              // The attribute of the last(0th) tag must be moved as well
              $current[$data['tag']]['0_attr'] = $current[$data['tag'].'_attr'];
              unset($current[$data['tag'].'_attr']);
            }
          }
          $last_item_index  = $repeated_tag_index[$data['tag'].'_'.$data['level']] - 1;
          $current          = &$current[$data['tag']][$last_item_index];
        }
      } elseif ($data['type'] == "complete") {
        // Tags that ends in 1 line '<tag />'
        // See if the key is already taken.
        if (!isset($current[$data['tag']])) {
          // New Key
          $current[$data['tag']]                                = $result;
          $repeated_tag_index[$data['tag'].'_'.$data['level']]  = 1;
          if ($priority == 'tag' and $attributes_data) {
            $current[$data['tag']. '_attr'] = $attributes_data;
          }
        } else {
          // If taken, put all things inside a list(array)
          if (isset($current[$data['tag']][0]) and is_array($current[$data['tag']])) {
            // If it is already an array...
            // ...push the new element into that array.
            $current[$data['tag']][$repeated_tag_index[$data['tag'].'_'.$data['level']]] = $result;

            if ($priority == 'tag' and $get_attributes and $attributes_data) {
              $current[$data['tag']][$repeated_tag_index[$data['tag'].'_'.$data['level']] . '_attr'] = $attributes_data;
            }
            $repeated_tag_index[$data['tag'].'_'.$data['level']]++;
          } else { //If it is not an array...
            //...Make it an array using using the existing value and the new value
            $current[$data['tag']] = [$current[$data['tag']],$result];
            $repeated_tag_index[$data['tag'].'_'.$data['level']] = 1;
            if ($priority == 'tag' and $get_attributes) {
              if (isset($current[$data['tag'].'_attr'])) {
                // The attribute of the last(0th) tag must be moved as well
                $current[$data['tag']]['0_attr'] = $current[$data['tag'].'_attr'];
                unset($current[$data['tag'].'_attr']);
              }

              if ($attributes_data) {
                $current[$data['tag']][$repeated_tag_index[$data['tag'].'_'.$data['level']] . '_attr'] = $attributes_data;
              }
            }
            // 0 and 1 index is already taken
            $repeated_tag_index[$data['tag'].'_'.$data['level']]++;
          }
        }
      } elseif ($data['type'] == 'close') {
        // End of tag '</tag>'
        $current = &$parent[$data['level'] - 1];
      }
    }

    return $xml_array;
  }
}
