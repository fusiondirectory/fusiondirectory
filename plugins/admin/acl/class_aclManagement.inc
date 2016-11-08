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

class aclAssignmentCreationDialog extends simplePlugin
{
  protected $post_finish = 'add_finish';
  protected $post_cancel = 'add_cancel';
  protected $simpleManagement;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('ACL assignment creation'),
      'plShortName'   => _('ACL assignment creation'),
      'plDescription' => _('Create an ACL assignment on an arbitrary dn'),
      'plCategory'    => array('acl'),

      'plProvidedAcls' => array()
    );
  }

  static function getAttributesInfo ()
  {
    global $config;
    return array(
      'main' => array(
        'name'  => _('ACL assignment creation'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new StringAttribute (
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

    $this->simpleManagement = $parent;

    parent::__construct($config->current['BASE']);

    $this->attributesAccess['baseDn']->setInLdap(FALSE);
  }

  function save_object()
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

  function save()
  {
  }

  function execute()
  {
    if ($this->save_object()) {
      $smarty = get_smarty();
      $smarty->assign($this->attributesAccess['baseDn']->getAcl().'ACL', 'rwcdm');
      return parent::execute()."\n".
      '<p class="plugbottom">'."\n".
      '  <input type="submit" name="'.$this->post_finish.'" value="'.msgPool::addButton().'"/>&nbsp;'."\n".
      '  <input type="submit" name="'.$this->post_cancel.'" value="'.msgPool::cancelButton().'"/>'."\n".
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
      $this->simpleManagement->newEntryConfirmed($this->baseDn);
    } catch (NonExistingLdapNodeException $e) {
      msg_dialog::displayChecks(array(_('The dn you entered could not be found in the LDAP')));
      return TRUE;
    }
    return FALSE;
  }

  function handle_cancel ()
  {
    $this->simpleManagement->closeDialogs();
    return FALSE;
  }
}

class aclManagement extends simpleManagement
{
  protected $objectTypes  = array('aclAssignment');

  protected $autoFilterAttributes = array('dn', 'cn', 'description', 'ou', 'dc');

  protected $departmentBrowser      = FALSE;
  protected $departmentRootVisible  = FALSE;
  protected $baseMode               = FALSE;
  protected $skipCpHandler          = TRUE;
  protected $skipSnapHandler        = TRUE;
  protected $autoActions            = FALSE;

  static function plInfo()
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

  function configureFilter()
  {
    parent::configureFilter();
    $this->filter->scope = 'sub';
  }

  function parseXML ($file)
  {
    $data = parent::parseXML($file);
    $data['list']['table']['column'][1]['value']          = '%{filter:filterLabel(row,dn,pid,base,ou,cn,dc,o)}';
    $data['list']['table']['column'][1]['sortAttribute']  = 'dn';
    $data['list']['actionmenu']['action'][0]['type'] = 'entry';
    $data['list']['actionmenu']['action'][0]['name'] = 'new';
    return $data;
  }

  function configureHeadpage ()
  {
    /* Remove department types so that we don't mistake assignments with them. */
    $this->headpage->objectTypes = array();
    $this->headpage->registerElementFilter('filterLabel', 'aclManagement::filterLabel');
    parent::configureHeadpage();
  }

  /*!
   * \brief  This method intiates the object creation.
   */
  function newEntry($action = '', $target = array(), $all = array(), $altTabClass = '', $altTabType = '', $altAclCategory = '')
  {
    $this->dialogObject = new aclAssignmentCreationDialog($this);
  }

  function newEntryConfirmed($dn)
  {
    $type = 'aclAssignment';

    /* This might throw an exception which is catched in aclAssignmentCreationDialog */
    $tabObject = objects::open($dn, $type);

    /* This will happen only if the exception is not thrown */
    $this->closeDialogs();

    $this->displayApplyBtn  = FALSE;
    $this->dn               = $dn;
    $this->is_single_edit   = FALSE;

    set_object_info($this->dn);

    $this->openTabObject($tabObject, $this->headpage->getBase());
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dn, 'Creating new ACL assignment');
  }

  static function filterLabel($row, $dn, $pid = 0, $base = "", $ou = array())
  {
    $ou = $ou[0];
    if ($dn == $base) {
      $ou = " . ";
    }
    $ou .= "  &nbsp; ["._("ACL Assignment")."]";

    $dn = LDAP::fix($dn);
    return "<a href='?plug=".$_GET['plug']."&amp;PID=$pid&amp;act=listing_edit_$row' title='$dn'>$ou</a>";
  }

  protected function removeEntryRequested($action = "", $target = array(), $all = array())
  {
    $result = parent::removeEntryRequested($action, $target, $all);
    if ($result != "") {
      $smarty = get_smarty();
      return $smarty->fetch(get_template_path('remove.tpl', TRUE, dirname(__FILE__)));
    }
  }
}
?>
