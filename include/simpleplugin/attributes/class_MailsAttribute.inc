<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2018-2019  FusionDirectory

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

class mailAddressSelect extends selectManagement
{
  /* Default columns */
  public static $columns = [
    ['ObjectTypeColumn', []],
    ['LinkColumn',       ['attributes' => 'mail',     'label' => 'Email']],
    ['LinkColumn',       ['attributes' => 'nameAttr', 'label' => 'Name']],
  ];
}

/*! \brief Mail address selection dialog
 */
class MailSelectDialog extends GenericSelectManagementDialog
{
  protected $dialogClass = 'mailAddressSelect';
}

/*! \brief This class allows to handle an attribute for selecting an mail address
 *
 * It looks like a SetAttribute, but clicking "Add (from list)" will open a dialog that allow to select one or more users.
 *
 */
class MailsAttribute extends DialogAttribute
{
  protected $dialogClass = 'MailSelectDialog';

  /*! \brief The constructor of MailsAttribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param array $defaultValue The default value for this attribute
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = [], $acl = '')
  {
    SetAttribute::__construct(new MailAttribute($label, $description, $ldapName, $required, '', $acl), $defaultValue);
  }

  public function getSelectManagementParameters (): array
  {
    return [
      ['user','group'],
      TRUE,
      [
        'objectClass' => '*',
        'dn'          => 'raw',
        'cn'          => '*',
        'mail'        => '*',
      ],
      $this->getFilterBlackList(),
      [],
      [
        ['TabFilterElement',    []],
        ['FixedFilterElement',  ['(mail=*)']],
      ]
    ];
  }

  function getFilterBlackList ()
  {
    return ['mail' => $this->getValue()];
  }

  function addValue (string $dn, $attrs = NULL)
  {
    $value = (($attrs != NULL) ? $attrs['mail'][0] : $dn);
    if (!in_array($value, $this->value)) {
      $this->value[]    = $value;
    }
  }

  function renderButtons ()
  {
    $id = $this->getHtmlId();
    $buttons  = $this->renderInputField(
      'submit', 'add'.$id,
      [
        'class' => 'subattribute',
        'value' => msgPool::addButton(FALSE),
      ]
    );
    $buttons  .= $this->renderInputField(
      'submit', 'add'.$id.'_dialog',
      [
        'class' => 'dialog subattribute',
        'value' => sprintf(_('%s (from list)'), msgPool::addButton(FALSE)),
      ]
    );
    $buttons  .= $this->renderInputField(
      'submit', 'del'.$id,
      [
        'class' => 'subattribute',
        'value' => msgPool::delButton(FALSE),
      ]
    );
    return $buttons;
  }

  public function htmlIds (): array
  {
    $id = $this->getHtmlId();
    return ['add'.$id,'add'.$id.'_dialog','del'.$id,'row'.$id];
  }
}
