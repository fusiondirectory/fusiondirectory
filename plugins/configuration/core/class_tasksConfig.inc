<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2022 FusionDirectory

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

class tasksConfig extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Tasks'),
      'plDescription'   => _('FusionDirectory Tasks Configuration'),
      'plObjectClass'   => ['fdTasksConf'],
      'plObjectType'    => ['configuration'],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'tasksConf' => [
        'name'  => _('Tasks Configuration'),
        'attrs' => [
          new StringAttribute(
            _('Tasks RDN'), _('Branch in which Tasks will be stored'),
            'fdTasksRDN', TRUE,
            'ou=tasks'
          ),
           new StringAttribute(
            _('Subtasks RDN'), _('Branch in which Subtasks will be stored'),
            'fdMainTasksRDN', TRUE,
            'ou=maintasks,ou=tasks'
          ),
           new StringAttribute(
            _('Subtasks RDN'), _('Branch in which Subtasks will be stored'),
            'fdSubTasksRDN', TRUE,
            'ou=subtasks,ou=tasks'
          ),
          new StringAttribute(
            _('FusionDirectory Orchestrator Tokens RDN'), _('The branch where FusionDirectory Orchestrator tokens are stored'),
            'fdOrchestratorTokenRDN', TRUE,
            'ou=orchestrator,ou=tokens'
          ),
        ]
      ],
      'Mail' => [
        'name'  => _('Mail Anti-Spam Configuration'),
        'attrs' => [
          new GeneralizedTimeDateAttribute(
           _('Last Sent Mail'), '_(Last GeneralizedTimeDate when last email was sent succesfully)',
           'fdTasksConfLastExecTime', FALSE, ''
           ),
          new IntAttribute(
            _('Maximum number of e-mails processed per time'), _('Select max emails to be proccessed'),
            'fdTasksConfMaxEmails', TRUE, '50', FALSE, ''
          ),
          new IntAttribute(
            _('Time interval between e-mails processing (Minutes) '), _('Select min interval time between emails processing'),
            'fdTasksConfIntervalEmails', TRUE, '2', FALSE, ''
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    // Check if token branch is here
    $ldap = $config->get_ldap_link();
    $tokenBranch = get_ou('OrchestratorTokenRDN').$config->current['BASE'];
    $ldap->cat($tokenBranch, ['dn']);
    if (!$ldap->count()) {
      $ldap->cd($config->current['BASE']);
      try {
        $ldap->create_missing_trees($tokenBranch);
      } catch (FusionDirectoryError $error) {
        $error->display();
      }
      fusiondirectory_log('Created token branch '.$tokenBranch);
    }

    // This attribute will be updated and verified by Orchestrator
    $this->attributesAccess['fdTasksConfLastExecTime']->setVisible(FALSE);

  }

}

