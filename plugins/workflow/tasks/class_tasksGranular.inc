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

class tasksGranular extends simplePlugin
{

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('Tasks Granular'),
      'plDescription'  => _('Granular tasks management allowing details reports'),
      'plObjectClass'  => ['fdTasksGranular'],
      'plFilter'       => '(objectClass=fdTasksGranular)',
      'plPriority'     => 41,
      'plObjectType'   => ['tasksGranular' => [
        'name' => _('TasksGranular'),
        'ou'   => get_ou('tasksRDN'),
        'icon' => 'geticon.php?context=applications&icon=tasks&size=16',
      ]],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  /*
   * Info : getAttributesInfo return is important for the management class.
   * It is the return of that method which render the columns and data properly.
   * Although not used by the interface directly, it is created by simpleTab during
   * the save of mailTasks.
   */
  static function getAttributesInfo (): array
  {
    return [
      // Attributes are grouped by section
      'tasksGranular' => [
        'name'  => _('Tasks Granular'),
        'attrs' => [
          new StringAttribute(
            _('Task Granular Name'), _('Name for this task'),
            'cn', TRUE
          ),
          new HiddenAttribute('fdTasksGranularStatus', TRUE, '1', '', 'Status', 'Status of the task'),
          new HiddenAttribute('fdTasksGranularMaster', TRUE, '', '', 'Master CN', 'Name of the Master task'),
          new HiddenAttribute('fdTasksGranularType', TRUE, '', '', 'Type', 'Type of the task'),
          new HiddenArrayAttribute('fdTasksGranularRef', FALSE, '', '', 'Type', 'Reference towards a required CN (mail template E.g'),
          new HiddenArrayAttribute('fdTasksGranularHelper', FALSE, '', '', 'Type', 'Reference towards important info from main tasks (user reminder two different call E.g'),
          new DateTimeAttribute(
            _('Schedule'), '',
            'fdTasksGranularSchedule', FALSE
          ),
          // Below attribute are for tasks of type lifeCycle and other tasks added as plugin
          new StringAttribute( // Attribute is mostly used to store important DN requiring processing by Orchestrator.
            _('Related DN'), _('DN list filled by main tasks'),
            'fdTasksGranularDN', FALSE
          ),
          // Below attributes are for tasks of type Mail
          new MailAttribute(
            _('Email'),
            _('Email address to which messages will be sent'), 'fdTasksGranularMail', FALSE),
          new MailAttribute(
            _('Email'),
            _('Email address from which emails will be sent'), 'fdTasksGranularMailFrom', FALSE),
          new MailAttribute(
            _('Email'),
            _('BCC Email address'), 'fdTasksGranularMailBCC', FALSE),
        ]
      ]
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
  }

}
