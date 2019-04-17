<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2016-2019  FusionDirectory

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
 * \brief Phone number selection
 */
class phoneSelect extends selectManagement
{
  /* Default columns */
  public static $columns = [
    ['ObjectTypeColumn', []],
    ['LinkColumn',       ['attributes' => 'nameAttr',         'label' => 'Name']],
    ['LinkColumn',       ['attributes' => 'telephoneNumber',  'label' => 'Number']],
  ];

  protected function setUpFilter ()
  {
    parent::setUpFilter();
    $this->filter->addElement(new FixedFilterElement($this->filter, '(telephoneNumber=*)'));
  }
}

/*!
 * \brief Phone number selection dialog
 */
class phoneSelectDialog extends GenericSingleSelectManagementDialog
{
  protected $dialogClass = 'phoneSelect';
}

/*! \brief This class allow to handle easily an LDAP attribute that contains a phone number
 */
class PhoneNumberAttribute extends StringAttribute
{
  protected $trim       = TRUE;
  protected $inputType  = 'tel';

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = '', $acl = '', $regexp = '/^[\/0-9 ()+*-]+$/', $example = NULL)
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl, $regexp, $example);
  }
}

/*! \brief Displays a text field and a button to select the phone from object list
 */
class PhoneNumberButtonAttribute extends PhoneNumberAttribute
{
  protected $type;
  protected $dialogClass = 'phoneSelectDialog';

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = '', $type = 'phone', $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);

    if (class_available('phoneGeneric')) {
      $this->type = $type;
    } else {
      $this->type = FALSE;
    }
  }

  public function getSelectManagementParameters (): array
  {
    switch ($this->type) {
      case 'phone':
        $objectTypes = ['phone'];
        break;
      case 'mobile':
        $objectTypes = ['mobilePhone'];
        break;
      case 'any':
        $objectTypes = ['phone','mobilePhone'];
        break;
      default:
        throw new FusionDirectoryException('Invalid type "'.$this->type.'" for PhoneNumberButtonAttribute');
    }

    return [
      $objectTypes,
      FALSE,
      [
        'objectClass'     => '*',
        'dn'              => 'raw',
        'cn'              => '*',
        'ipHostNumber'    => '*',
        'macAddress'      => '*',
        'telephoneNumber' => '*',
      ],
    ];
  }

  function renderFormInput (): string
  {
    $id = $this->getHtmlId();
    $display = parent::renderFormInput();
    if ($this->type !== FALSE) {
      $display .= $this->renderAcl($this->renderInputField(
        'image', $id.'_edit',
        [
          'class' => 'center dialog',
          'src'   => 'geticon.php?context=actions&amp;icon=document-edit&amp;size=16',
          'title' => _('Edit'),
          'alt'   => _('Edit')
        ]
      ));
    }
    return $display;
  }

  public function htmlIds (): array
  {
    $ids = parent::htmlIds();
    if ($this->type !== FALSE) {
      $ids[] = $ids[0].'_edit';
    }
    return $ids;
  }

  function loadPostValue ()
  {
    parent::loadPostValue();
    if ($this->type !== FALSE) {
      $id = $this->getHtmlId();
      if (!$this->disabled && $this->isVisible()) {
        foreach (array_keys($_POST) as $name) {
          if (preg_match('/^'.$id.'_edit_/', $name)) {
            $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
            break;
          }
        }
      }
    }
  }

  function handleDialogResult ($dn, $entry)
  {
    if (isset($entry['telephoneNumber'][0])) {
      $this->setValue($entry['telephoneNumber'][0]);
    } elseif (isset($entry['telephoneNumber'])) {
      $this->setValue($entry['telephoneNumber']);
    } else {
      $this->setValue('');
    }
  }
}
