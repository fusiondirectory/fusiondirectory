<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)

  Copyright (C) 2024 FusionDirectory

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

class tasksLifeCycle extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'    => _('Tasks Life Cycle'),
      'plDescription'  => _('Tasks Life Cycle Object'),
      'plIcon'         => 'geticon.php?context=applications&icon=tasks&size=16',
      'plPriority'     => 42,
      'plObjectClass'  => ['fdTasksLifeCycle'],
      'plFilter'       => '(objectClass=fdTasksLifeCycle)',
      'plObjectType'   => ['tasks'],
      'plConflicts'    => ['tasksMail'],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo()),
      'plForeignKeys'  => [],
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'section1' => [
        'class' => ['fullwidth', 'information'],
        'name'  => _('We recommend the task redundancy to be set to daily, and the case “new members only” to be unpicked'),
        'attrs' => [
          // This hiddenAttribute is only present to show the information label on the section.
          new HiddenAttribute(
            _('Information'), _('Information'),
            'fdShowInformation', FALSE
          ),
        ]
      ],
      'section3' => [
        'name'  => _('Post resources details'),
        'attrs' => [
          new SelectAttribute(
            _('Resource'), _('Supann resources'),
            'fdTasksLifeCyclePostResource', TRUE
          ),
          new SelectAttribute(
            _('State'), _('Resource state'),
            'fdTasksLifeCyclePosState', TRUE
          ),
          new SelectAttribute(
            _('Sub state'), _('Resource sub state'),
            'fdTasksLifeCyclePostSubState', FALSE
          ),
          new IntAttribute(
            _('Extra days to add'), _('Extra days to be added after today'),
            'fdTasksLifeCyclePostEndDate', FALSE, '0', FALSE, '0'
          ),
        ]
      ],
      // Attributes are grouped by section
      'section2' => [
        'name'  => _('Pre / existing resources details'),
        'attrs' => [
          new SelectAttribute(
            _('Resource'), _('Supann resources'),
            'fdTasksLifeCyclePreResource', TRUE
          ),
          new SelectAttribute(
            _('State'), _('Resource state'),
            'fdTasksLifeCyclePreState', TRUE
          ),
          new SelectAttribute(
            _('Sub state'), _('Resource sub state'),
            'fdTasksLifeCyclePreSubState', FALSE
          ),
        ]
      ],
    ];
  }

  /**
   * @throws NonExistingLdapNodeException
   */
  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);
    $this->setSupannStates();
  }

  /*
 * Return table of existing Population Codes containing main's and local's
 */
  protected function setSupannStates () : void
  {
    global $config;

    $resources = ['COMPTE' => _('Account'),'MAIL' => _('Mail')];
    foreach ($config->get_cfg_value('SupannRessourceLabels', []) as $line) {
      list($resource,$label) = explode(':', $line, 2);
      $resources[$resource] = $label;
    }

    $this->attributesAccess['fdTasksLifeCyclePreResource']->setChoices(array_keys($resources),array_keys($resources));


//    $this->attributesAccess['fdTasksLifeCyclePostResource']->setValue(array_keys($resources));


//    $this->attributesAccess['fdTasksLifeCyclePreState']->setChoice(array_values($resources));
//    $this->attributesAccess['fdTasksLifeCyclePostState']->setChoice(array_values($resources));

  }

  public function update (): bool
  {
    parent::update();
    return TRUE;
  }

  function save (): array
  {
    // Verify if this tasks has to be executed upon saving.
//    $execTasks = $this->parent->getBaseObject()->fdSubTasksActivation ?? NULL;

//    if ($execTasks) {
//      $this->generateSlaveTasks();
//    }

    return parent::save();
  }
}
