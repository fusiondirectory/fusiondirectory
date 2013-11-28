<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013  FusionDirectory

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

class reference extends simplePlugin
{
  static function plInfo()
  {
    return array(
      "plShortName"   => _("References"),
      "plDescription" => _("References"),
      "plCategory"    => array('acl'),
      "plObjectType"  => array('special'),
      'plPriority'    => 10,

      "plProvidedAcls"  => array()
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'      => _('References'),
        'class'     => array('fullwidth'),
        'attrs'     => array(
          new FakeAttribute('refs')
        ),
        'template'  => get_template_path('contents.tpl', TRUE, dirname(__FILE__))
      ),
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->refs = NULL;
  }

  function execute()
  {
    if ($this->refs === NULL) {
      $this->fillRefs();
    }
    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    return parent::execute();
  }

  function fillRefs()
  {
    if (!isset($this->parent)) {
      $this->refs = NULL;
      return;
    }
    $foreignRefs  = array();
    foreach (array_keys($this->parent->by_object) as $tabclass) {
      $infos = pluglist::pluginInfos($tabclass);
      foreach ($infos['plForeignRefs'] as $field => $refs) {
        foreach ($refs as $ref) {
          $class  = $ref[0];
          $ofield = $ref[1];
          $cinfos = pluglist::pluginInfos($class);
          foreach ($cinfos['plObjectType'] as $key => $objectType) {
            if (!is_numeric($key)) {
              $objectType = $key;
            }
            $foreignRefs[$objectType][$class][$ofield] = array($tabclass,$field);
          }
        }
      }
    }
    $refs         = array();
    // For each concerned objectType
    foreach ($foreignRefs as $objectType => $tabRefs) {
      // Search objects
      $objects = objects::ls($objectType, array('dn' => 'raw'));
      // For each object of this type
      foreach (array_keys($objects) as $dn) {
        // Build the object
        $tabobject = objects::open($dn, $objectType);
        // For each tab concerned
        foreach ($tabRefs as $tab => $fieldRefs) {
          // If the tab is activated on this object
          if (isset($tabobject->by_object[$tab])) {
            // For each field
            foreach ($fieldRefs as $ofield => $field_infos) {
              list ($tabclass,$field) = $field_infos;
              if ($tabobject->by_object[$tab]->foreignKeyCheck(
                    $ofield,
                    $this->parent->by_object[$tabclass]->$field,
                    $field
                  )) {
                if (!isset($refs[$dn])) {
                  $refs[$dn] = array(
                    'link'  => '',
                    'tabs'  => array(),
                  );
                  try {
                    $refs[$dn]['link'] = objects::link($dn, $objectType);
                  } catch (Exception $e) {
                    $refs[$dn]['link'] = $dn;
                  }
                }
                if (!isset($refs[$dn]['tabs'][$tab])) {
                  $refs[$dn]['tabs'][$tab] = array(
                    'link'    => '',
                    'fields'  => array(),
                  );
                  try {
                    $refs[$dn]['tabs'][$tab]['link'] = objects::link($dn, $objectType, "tab_$tab", sprintf(_('Tab "%s"'), $tabobject->by_name[$tab]));
                  } catch (Exception $e) {
                    $refs[$dn]['tabs'][$tab]['link'] = $tab;
                  }
                }
                $refs[$dn]['tabs'][$tab]['fields'][$ofield] = array(
                  'tab'   => $this->parent->by_name[$tabclass],
                  'field' => $field
                );
              }
            }
          }
        }
      }
    }
    $this->refs = $refs;
  }
}

?>
