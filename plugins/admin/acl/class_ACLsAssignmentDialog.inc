<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2019  FusionDirectory

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

class ACLsAssignmentDialog extends GenericDialog
{
  protected $initialAclValue;
  protected $post_cancel = 'add_acl_cancel';
  protected $post_finish = 'add_acl_finish';
  protected $dialogClass = 'aclAssignmentDialogWindow';

  function __construct ($simplePlugin, $attribute, $acl = NULL)
  {
    $isContainer = FALSE;
    if (isset($simplePlugin->attrs['objectClass'])) {
      if (count(array_intersect(
                  $simplePlugin->attrs['objectClass'],
                  ['organizationalUnit', 'organization', 'domain', 'country', 'locality'])
                )) {
        $isContainer = TRUE;
      }
    } else {
      $isContainer = TRUE;
    }
    $this->attribute        = $attribute;
    $this->dialog           = new $this->dialogClass($acl, $isContainer);
    $this->dialog->dn       = $simplePlugin->dn;
    $this->initialAclValue  = $acl;
  }

  function dialog_execute ()
  {
    $this->dialog->save_object();
    return $this->dialog->execute();
  }

  function handle_finish ()
  {
    $this->dialog->save_object();
    $messages = $this->dialog->check();
    if (!empty($messages)) {
      msg_dialog::displayChecks($messages);
      return $this->dialog->execute();
    }
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
