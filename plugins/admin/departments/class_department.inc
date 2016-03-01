<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class department extends simplePlugin
{
  static $namingAttr = 'ou';

  /* Do not append the structural object classes here, they are added dynamically in the constructor */
  var $objectclasses  = array("top", "gosaDepartment");
  var $structuralOC   = array("organizationalUnit");
  var $mainTab        = TRUE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Department'),
      'plDescription' => _('Departments'),
      'plCategory'    => array('department' => array('objectClass' => 'gosaDepartment', 'description' => _('Departments'))),
      'plObjectType'  =>
        array(
          'department' => array(
            'name'      => _('Department'),
            'filter'    => '(&(objectClass=organizationalUnit)(objectClass=gosaDepartment))',
            'ou'        => '',
            'icon'      => 'geticon.php?context=places&icon=folder&size=16',
            'tabClass'  => 'deptabs',
            'mainAttr'  => self::$namingAttr,
          )
        ),
      'plForeignKeys'  => array(
        'manager' => 'user'
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo()
  {
    return self::getDepartmentAttributesInfo(_('department'), self::$namingAttr);
  }
  static function getDepartmentAttributesInfo($name, $namingAttr)
  {
    $attributesInfo = array(
      'properties' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new BaseSelectorAttribute(''),
          new StringAttribute(
            sprintf(_('Name of %s'), $name), sprintf(_('A name for this %s'), $name),
            $namingAttr, TRUE,
            '', '', '/^[a-z0-9\._-]+$/i'
          ),
          new TextAreaAttribute(
            _('Description'), sprintf(_('Short description of this %s'), $name),
            'description', TRUE
          ),
          new SelectAttribute(
            _('Category'), sprintf(_('Category of this %s'), $name),
            'businessCategory', FALSE
          ),
          new StringAttribute(
            _('Web Site'), sprintf(_('Web site of this %s'), $name),
            'labeledURI', FALSE
          ),
          new UserAttribute(
            _('Manager'), sprintf(_('Manager of this %s'), $name),
            'manager', FALSE
          )
        )
      ),
      'location' => array(
        'name'  => _('Location'),
        'attrs' => array(
          new StringAttribute(
            _('State'), _('State'), 'st', FALSE
          ),
          new StringAttribute(
            _('Location'), _('Location'), 'l', FALSE
          ),
          new TextAreaAttribute(
            _('Address'), sprintf(_('A postal address for this %s'), $name),
            'postalAddress', FALSE
          ),
          new PhoneNumberAttribute(
            _('Phone'), _('Telephone number'),
            'telephoneNumber', FALSE
          ),
          new PhoneNumberAttribute(
            _('Fax'), _('Facsimile telephone number'),
            'facsimileTelephoneNumber', FALSE
          ),
        )
      ),
    );
    if ($namingAttr != 'ou') {
      $attributesInfo['properties']['attrs'][] = new HiddenAttribute('ou');
    }
    return $attributesInfo;
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    /* Add the default structural object class if this is a new entry */
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    if ($dn == "" || $dn == "new" || !$ldap->dn_exists($dn)) {
      $this->objectclasses = array_merge($this->structuralOC, $this->objectclasses);
    } else {
      $ldap->cat($dn, array("structuralObjectClass"));
      $attrs = $ldap->fetch();
      if (isset($attrs['structuralObjectClass']['count'])) {
        for ($i = 0; $i < $attrs['structuralObjectClass']['count']; $i++) {
          $this->objectclasses[] = $attrs['structuralObjectClass'][$i];
        }
      } else {
        /* Could not detect structural object class for this object, fall back to the default
         */
        $this->objectclasses = array_merge($this->structuralOC, $this->objectclasses);
      }
    }
    $this->objectclasses = array_unique($this->objectclasses);

    parent::__construct($config, $dn, $object);

    $categoriesList = $this->config->get_cfg_value('DepartmentCategories', array());

    /* Insert current value to possibilities */
    $businessCategory = $this->attributesAccess['businessCategory']->getValue();
    if (($businessCategory != "") && !in_array($businessCategory, $categoriesList)) {
      $categoriesList[] = $businessCategory;
    }
    $this->attributesAccess['businessCategory']->setChoices($categoriesList);

    $this->attributesAccess[$this->getNamingAttr()]->setUnique(TRUE);
  }

  function getNamingAttr()
  {
    $class = get_class($this);
    return $class::$namingAttr;
  }

  /* Check values */
  function check()
  {
    /* Call common method to give check the hook */
    $message = parent::check();

    $namingAttr = $this->getNamingAttr();

    if (tests::is_department_name_reserved($this->$namingAttr, $this->base)) {
      $message[] = msgPool::reserved(_("Name"));
    }

    return $message;
  }

  function execute()
  {
    /* Reload departments */
    $this->config->get_departments($this->dn);
    $this->config->make_idepartments();

    /* Call parent execute */
    return parent::execute();
  }

  function get_allowed_bases()
  {
    /* Hide all departments, that are subtrees of this department */
    $bases = parent::get_allowed_bases();
    if (($this->dn != "new") && ($this->dn != "")) {
      foreach (array_keys($bases) as $dn) {
        if (preg_match("/".preg_quote($this->dn)."/", $dn)) {
          unset($bases[$dn]);
        }
      }
    }
    return $bases;
  }

  function prepare_save()
  {
    if ($this->getNamingAttr() != 'ou') {
      $this->attributesAccess['ou']->setValue($this->attributesAccess[$this->getNamingAttr()]->getValue());
    }
    return parent::prepare_save();
  }
}
    /* Hide base selector, if this object represents the base itself

    $smarty->assign("is_root_dse", FALSE);
    if ($this->dn == $this->config->current['BASE']) {
      $smarty->assign("is_root_dse", TRUE);
      $nA = $this->namingAttr."ACL";
      $smarty->assign($nA, $this->getacl($this->namingAttr, TRUE));
    }*/


?>
