<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2018-2022 FusionDirectory

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/

class mailTemplateInvitations extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Mail Template Invitations'),
      'plDescription' => _('Mail Template Invitations'),
      'plIcon'        => 'geticon.php?context=applications&icon=supann&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=supann-status&size=16',
      'plPriority'    => 33,
      'plObjectClass' => ['fdMailTemplateInvitations'],
      'plFilter'      => '(objectClass=fdMailTemplateInvitations)',
      'plObjectType'  => ['mailTemplate'],
      'plConflicts'   => ['mailTemplateUserReminder', 'mailTemplateUserRecovery'],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return  [

    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);
  }

  protected function shouldSave (): bool
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=fdMailTemplateInvitations))', ['cn','fdMailTemplateInvitations']);
    while ($attrs = $ldap->fetch()) {
      if (isset($this->attrs['fdMailTemplateInvitations'])) {
        $error = new FusionDirectoryError(htmlescape(sprintf(_('Invitations Mail Template Is Already Existing!'))));
        $error->display();

        return FALSE;
      }
    }

    return TRUE;
  }

}
