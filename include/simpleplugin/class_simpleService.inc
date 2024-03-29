<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2011-2019  FusionDirectory

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

/*! \brief This class is made for easy service creation for editing LDAP attributes
 *
 */
class simpleService extends simplePlugin
{
  protected static $showActions = TRUE;

  protected $status = '';

  public $conflicts   = [];
  public $DisplayName = '';

  /*! \brief constructor
   *
   *  \param string $dn The dn of this instance
   *  \param Object $parent The servicesManagement instance
   *  \param array $attributesInfo An attributesInfo array, if NULL, getAttributesInfo will be used.
   *
   */
  function __construct ($dn = NULL, $parent = NULL, $attributesInfo = NULL)
  {
    /* $parent is the instance of servicesManagement in this case, we set it as parent */
    parent::__construct($dn, $parent, $parent, FALSE, $attributesInfo);

    /* Services do not have the activation header, but can still be disabled */
    $this->ignore_account = FALSE;

    $plInfos = pluglist::pluginInfos(get_class($this));
    $this->DisplayName  = $plInfos['plShortName'];
  }

  /*! \brief This function display the service and return the html code
   */
  public function render (): string
  {
    $str = parent::render();

    if (!$this->dialog) {
      $str .= '<p class="plugbottom servicebottom">'.
             '  <input type="submit" name="SaveService" value="'.msgPool::saveButton().'"/>&nbsp;'.
             '  <input type="submit" formnovalidate="formnovalidate" name="CancelService" value="'.msgPool::cancelButton().'"/>'.
             '</div>';
    }

    return $str;
  }

  protected function acl_skip_write (): bool
  {
    return FALSE;
  }

  /*! \brief Get service information for servicesManagement plugin */
  function getListEntry ()
  {
    /* Assign status flag */
    $fields['Status']       = $this->status;

    /* Name displayed in service overview */
    $fields['Message']      = $this->DisplayName;

    if (static::$showActions && is_object($this->parent->parent) && isset($this->parent->parent->by_object['argonautClient']) && $this->parent->parent->by_object['argonautClient']->is_account) {
      /* Allow/disallow some functions */
      $fields['AllowStatus']  = ($this->status == '') && $this->acl_is_writeable('simpleServiceStatus');
      $fields['AllowStart']   = ($this->status == 'stopped') && $this->acl_is_writeable('simpleServiceStart');
      $fields['AllowStop']    = ($this->status == 'running') && $this->acl_is_writeable('simpleServiceStop');
      $fields['AllowRestart'] = ($this->status == 'running') && $this->acl_is_writeable('simpleServiceRestart');
    } else {
      /* Disable some functions */
      $fields['AllowStatus']  = FALSE;
      $fields['AllowStart']   = FALSE;
      $fields['AllowStop']    = FALSE;
      $fields['AllowRestart'] = FALSE;
    }

    $fields['AllowRemove']  = $this->acl_is_removeable();
    $fields['AllowEdit']    = $this->acl_is_readable('');

    return $fields;
  }

  /*! \brief This function save new status flag */
  function setStatus ($value)
  {
    /* Can't set status flag for new services (Object doesn't exists in ldap tree) */
    if (!$this->initially_was_account) {
      return;
    }

    $this->status = $value;
  }

  static function generatePlProvidedAcls (array $attributesInfo, bool $operationalAttributes = NULL): array
  {
    $acls = parent::generatePlProvidedAcls($attributesInfo);
    if (static::$showActions) {
      $acls ['simpleServiceStatus']   = _('Get service status');
      $acls ['simpleServiceStart']    = _('Start service');
      $acls ['simpleServiceStop']     = _('Stop service');
      $acls ['simpleServiceRestart']  = _('Restart service');
    }

    return $acls;
  }
}
