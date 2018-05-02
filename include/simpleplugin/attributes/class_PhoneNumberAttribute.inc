<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2016-2017  FusionDirectory

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
class phoneSelect extends simpleSelectManagement
{
  protected $multiSelect          = FALSE;
  protected $autoFilterAttributes = array('dn','cn','ipHostNumber','macAddress','telephoneNumber');
  protected $objectTypes          = array('phone');

  function parseXML ($file)
  {
    $data = parent::parseXML($file);
    /* Add column for phone number */
    $data['list']['table']['layout'] .= '|';
    $columns = array (
      array (
        'label'         => _('Number'),
        'sortAttribute' => 'telephoneNumber',
        'sortType'      => 'string',
        'value'         => '%{filter:selectLink(pid,row,dn,telephoneNumber)}',
        'export'        => 'true',
      ),
    );
    array_splice($data['list']['table']['column'], 2, 0, $columns);
    return $data;
  }

  function configureFilter ()
  {
    parent::configureFilter();
    /* Only display entries with a phone number */
    $this->filter->query[0]['filter'] = '(&(telephoneNumber=*)'.$this->filter->query[0]['filter'].')';
  }
}

/*!
 * \brief Mobile phone number selection
 */
class mobilePhoneSelect extends phoneSelect
{
  protected $objectTypes = array('mobilePhone');
}

/*!
 * \brief Phone number selection allowing both Phone and Mobile phone objects
 */
class anyPhoneSelect extends phoneSelect
{
  protected $objectTypes = array('phone','mobilePhone');
}

/*!
 * \brief Phone number selection dialog
 */
class phoneSelectDialog extends GenericDialog
{
  function __construct($simplePlugin, $attribute, $type)
  {
    $this->attribute  = $attribute;
    switch ($type) {
      case 'phone':
        $this->dialog = new phoneSelect();
        break;
      case 'mobile':
        $this->dialog = new mobilePhoneSelect();
        break;
      case 'any':
        $this->dialog = new anyPhoneSelect();
        break;
      default:
        trigger_error('Unknown phone select type '.$type);
        break;
    }
  }

  function handle_finish ()
  {
    $result = $this->dialog->detectPostActions();
    if (isset($result['targets']) && count($result['targets'])) {
      $headpage = $this->dialog->getHeadpage();
      $dn       = $result['targets'][0];
      $attrs    = $headpage->getEntry($dn);
      $this->attribute->handleDialogResult($dn, $attrs);
    }
    return FALSE;
  }
}

/*! \brief This class allow to handle easily an LDAP attribute that contains a phone number
 */
class PhoneNumberAttribute extends TestValidateAttribute
{
  protected $trim     = TRUE;
  protected $testFunc = 'is_phone_nr';
}

/*! \brief Displays a text field and a button to select the phone from object list
 */
class PhoneNumberButtonAttribute extends PhoneNumberAttribute
{
  protected $type;

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = '', $type = 'phone', $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->dialogClass = 'phoneSelectDialog';
    if (class_available('phoneGeneric')) {
      $this->type = $type;
    } else {
      $this->type = FALSE;
    }
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display = parent::renderFormInput();
    if ($this->type !== FALSE) {
      $display .= $this->renderAcl($this->renderInputField(
        'image', $id.'_edit',
        array(
          'class' => 'center',
          'src'   => 'geticon.php?context=actions&amp;icon=document-edit&amp;size=16',
          'title' => _('Edit'),
          'alt'   => _('Edit')
        )
      ));
    }
    return $display;
  }

  public function htmlIds()
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
            $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this, $this->type));
            break;
          }
        }
      }
    }
  }

  function handleDialogResult ($dn, array $attrs)
  {
    if (isset($attrs['telephoneNumber'][0])) {
      $this->setValue($attrs['telephoneNumber'][0]);
    } else {
      $this->setValue('');
    }
  }
}
