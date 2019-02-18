<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2018  FusionDirectory

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

class aclAssignmentCreationDialog extends simplePlugin
{
  protected $post_finish = 'add_finish';
  protected $post_cancel = 'add_cancel';
  protected $management;

  static function plInfo (): array
  {
    return array(
      'plShortName'   => _('ACL assignment creation'),
      'plShortName'   => _('ACL assignment creation'),
      'plDescription' => _('Create an ACL assignment on an arbitrary dn'),
      'plCategory'    => array('acl'),

      'plProvidedAcls' => array()
    );
  }

  static function getAttributesInfo (): array
  {
    global $config;
    return array(
      'main' => array(
        'name'  => _('ACL assignment creation'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new StringAttribute(
            _('Dn'), _('DN you wish to add assignments for'),
            'baseDn', TRUE,
            $config->current['BASE'], 'aclMode',
            '/^([^=,]+=[^=,]+,)+'.preg_quote($config->current['BASE'], '/').'$/',
            'ou=people,'.$config->current['BASE']
          ),
        )
      ),
    );
  }

  function __construct ($parent)
  {
    global $config;

    $this->management = $parent;

    parent::__construct($config->current['BASE']);

    $this->attributesAccess['baseDn']->setInLdap(FALSE);
  }

  function save_object ()
  {
    parent::save_object();
    if (isset($_POST[$this->post_cancel])) {
      return $this->handle_cancel();
    }
    if (isset($_POST[$this->post_finish])) {
      return $this->handle_finish();
    }
    return TRUE;
  }

  function save (): array
  {
  }

  function execute (): string
  {
    if ($this->save_object()) {
      $smarty = get_smarty();
      $smarty->assign($this->attributesAccess['baseDn']->getAcl().'ACL', 'rwcdm');
      return parent::execute()."\n".
      '<p class="plugbottom">'."\n".
      '  <input type="submit" name="'.$this->post_finish.'" value="'.msgPool::addButton().'"/>&nbsp;'."\n".
      '  <input type="submit" formnovalidate="formnovalidate" name="'.$this->post_cancel.'" value="'.msgPool::cancelButton().'"/>'."\n".
      '</p>';
    }
  }

  function handle_finish ()
  {
    $msgs = $this->check();
    if (count($msgs)) {
      msg_dialog::displayChecks($msgs);
      return TRUE;
    }
    try {
      $this->management->newEntryConfirmed($this->baseDn);
    } catch (NonExistingLdapNodeException $e) {
      msg_dialog::displayChecks(array(_('The dn you entered could not be found in the LDAP')));
      return TRUE;
    }
    return FALSE;
  }

  function handle_cancel ()
  {
    $this->management->closeDialogs();
    return FALSE;
  }
}

class aclManagement extends management
{
  protected $skipCpHandler     = TRUE;

  public static $skipSnapshots = TRUE;

  public static $skipTemplates = TRUE;

  /* We cannot show anything more than DN as there are no ACLs for other fields on aclAssignment object */
  public static $columns = array(
    array('ObjectTypeColumn', array()),
    array('LinkColumn',       array('attributes' => 'dn', 'label' => 'DN')),
    array('ActionsColumn',    array('label' => 'Actions')),
  );

  static function plInfo (): array
  {
    return array(
      'plShortName'   => _('ACL assignments'),
      'plTitle'       => _('ACL assignments management'),
      'plDescription' => _('Manage ACL roles assignments to users'),
      'plIcon'        => 'geticon.php?context=categories&icon=acl&size=48',
      'plSection'     => 'accounts',
      'plPriority'    => 28,
      'plManages'     => array('aclAssignment'),

      'plProvidedAcls' => array()
    );
  }

  protected function setUpListing()
  {
    /* Set baseMode to FALSE */
    $this->listing  = new managementListing($this, FALSE);
  }

  protected function setUpFilter()
  {
    /* Set scope to sub */
    parent::setUpFilter();
    $this->filter->setScope('sub');
  }

  /*!
   * \brief Opens aclAssignmentCreationDialog to enter an arbitrary DN
   */
  function newEntry(array $action)
  {
    $this->dialogObject = new aclAssignmentCreationDialog($this);
  }


  /*!
   * \brief Called by aclAssignmentCreationDialog
   */
  function newEntryConfirmed($dn)
  {
    $type = 'aclAssignment';

    /* This might throw an exception which is catched in aclAssignmentCreationDialog */
    $tabObject = objects::open($dn, $type);

    /* This will happen only if the exception is not thrown */
    $this->closeDialogs();

    $this->currentDn        = $dn;

    set_object_info($this->currentDn);

    $this->openTabObject($tabObject, $this->listing->getBase());
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->currentDn, 'Creating new ACL assignment');
  }

  /*! \brief Display confirmation dialog
   */
  protected function removeConfirmationDialog(array $objects)
  {
    $smarty = get_smarty();
    $smarty->assign('objects', $objects);
    $smarty->assign('multiple', TRUE);
    return $smarty->fetch(get_template_path('remove.tpl', TRUE, dirname(__FILE__)));
  }

  protected function renderFilter()
  {
    /* Hide filter box */
    return '';
  }
}
?>
