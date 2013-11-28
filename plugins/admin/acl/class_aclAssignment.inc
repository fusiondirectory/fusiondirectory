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

class aclAssignmentDialogWindow extends simplePlugin
{
  protected $mainTab = TRUE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Acl Assignment Dialog'),
      'plDescription' => _('Access control roles assignment dialog'),
      'plCategory'    => array('acl'),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo()
  {
    return array(
      'properties' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new SelectAttribute(
            _('Mode'), _('Is this applying on complete subtree or only the base?'),
            'aclMode', TRUE,
            array('subtree',    'base'), 'sub',
            array(_('Subtree'), _('Base only'))
          ),
          new SelectAttribute(
            _('Role'), _('Role to apply'),
            'aclRole', TRUE,
            array()
          ),
          new BooleanAttribute(
            _('For all users'), _('Apply this ACL for all LDAP users'),
            'allUsers', FALSE,
            FALSE
          ),
          new UsersGroupsAttribute(
            _('Members'), _('Users or groups to assign this role to.'),
            'aclMembers', TRUE
          )
        )
      ),
    );
  }

  function __construct (&$config, $value)
  {
    parent::__construct($config);
    $roles = objects::ls('aclRole', 'cn');
    $this->attributesAccess['aclRole']->setChoices(array_keys($roles), array_values($roles));
    $this->attributesAccess['allUsers']->setInLdap(FALSE);
    $this->attributesAccess['allUsers']->setManagedAttributes(
      array(
        'disable' => array(
          TRUE => array('aclMembers')
        )
      )
    );

    if ($value !== NULL) {
      $this->aclMode    = $value['scope'];
      $this->aclRole    = $value['role'];
      $this->aclMembers = $value['members'];
    }
  }

  function execute()
  {
    $smarty = get_smarty();
    $display = parent::execute();
    if (!is_object($this->dialog)) {
    $display .= $smarty->fetch('string:'.
      '<p class="plugbottom">'.
      '  <input type="submit" name="add_acl_finish" value="{msgPool type=addButton}"/>'.
      '  &nbsp;'.
      '  <input type="submit" name="add_acl_cancel" value="{msgPool type=cancelButton}"/>'.
      '</p>');
    }
    return $display;
  }

  function getAclEntry()
  {
    $entry = array(
      'scope'   => $this->aclMode,
      'role'    => $this->aclRole,
      'members' => $this->aclMembers,
    );
    if ($this->allUsers) {
      $entry['members'] = '*';
    }
  }
}

class ACLsAssignmentDialog extends GenericDialog
{
  protected $initialAclValue;
  protected $post_cancel = 'add_acl_cancel';
  protected $post_finish = 'add_acl_finish';
  protected $dialogClass = 'aclAssignmentDialogWindow';

  function __construct($simplePlugin, &$attribute, $acl = NULL)
  {
    $this->attribute        = $attribute;
    $this->dialog           = new $this->dialogClass($simplePlugin->config, $acl);
    $this->dialog->set_acl_base($simplePlugin->acl_base);
    $this->initialAclValue  = $acl;
  }

  function dialog_execute ()
  {
    $this->dialog->save_object();
    return $this->dialog->execute();
  }

  function handle_finish ()
  {
    $this->attribute->addValue($this->dialog->getAclEntry());
    return FALSE;
  }

  function handle_cancel ()
  {
    if ($this->initialAclValue !== NULL) {
      $this->attribute->addValue($this->initialAclValue);
    }
    return FALSE;
  }
}

class ACLsAssignmentAttribute extends DialogOrderedArrayAttribute
{
  protected $order = TRUE;
  protected $dialogClass = 'ACLsAssignmentDialog';

  protected function getAttributeArrayValue($value)
  {
    /* Convert text value to displayable array value */
    //~ return explode(':', $value);
    $value['members'] = join(', ', $value['members']);
    return $value;
  }

  function readValue($value)
  {
    $acl = explode(':', $value);
    return array($acl[0], array(
      'scope'   => $acl[1],
      'role'    => base64_decode($acl[2]),
      'members' => array_map('base64_decode', explode(',', $acl[3])),
    ));
  }

  function writeValue($key, $value)
  {
    return $key.':'.$value['scope'].':'.base64_encode($value['role']).':'.join(',', array_map('base64_encode', $value['members']));
  }
}

class aclAssignment extends simplePlugin
{
  var $objectclasses  = array('gosaAcl');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('ACL Assignment'),
      'plDescription' => _('Access control roles assignment'),
      'plObjectType'  => array(
        'aclAssignment' => array(
          'aclCategory' => 'acl',
          'name'        => _('ACL assignment'),
          'filter'      => 'objectClass=gosaAcl',
          'ou'          => ''
        ),
        'special' => array(
          'name' => 'special'
        ),
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo()
  {
    return array(
      'main' => array(
        'name'  => _('Assignments'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new ACLsAssignmentAttribute(
            '', _('ACL role assignments for this base'),
            'gosaAclEntry', FALSE
          )
        )
      )
    );
  }

  function compute_dn()
  {
    return $this->dn;
  }

  function save_object()
  {
    $this->ignore_account = FALSE;
    $this->is_account = TRUE;
    parent::save_object();
    $this->is_account = (count($this->gosaAclEntry) != 0);
  }

  function post_remove()
  {
    parent::post_remove();

    /* Refresh users ACLs */
    $ui = get_userinfo();
    $ui->loadACL();
    session::global_set('ui', $ui);
  }

  function post_save()
  {
    parent::post_save();

    /* Refresh users ACLs */
    $ui = get_userinfo();
    $ui->loadACL();
    session::global_set('ui', $ui);
  }
}
