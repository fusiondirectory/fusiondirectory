<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2022  FusionDirectory

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

class mailTemplateManagement extends management
{

  public static $columns = [
    ['ObjectTypeColumn', []],
    ['LinkColumn',       ['attributes' => 'cn', 'label' => 'Template Name']],
    ['ActionsColumn',    ['label' => 'Actions']],
  ];

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Mail Template'),
      'plTitle'       => _('Mail Template'),
      'plDescription' => _('Manages mails templates'),
      'plIcon'        => 'geticon.php?context=types&icon=user&size=48',
      'plManages'     => ['mailTemplate'],
      'plSection'     => 'conf',
      'plPriority'    => 30,
      'plProvidedAcls' => [],
    ];
  }

  protected function configureActions ()
  {
    global $config;

    $createMenu = [];

    foreach ($this->objectTypes as $type) {
      $infos  = objects::infos($type);
      $img    = 'geticon.php?context=actions&icon=document-new&size=16';
      if (isset($infos['icon'])) {
        $img = $infos['icon'];
      }
      $createMenu[] = new Action(
        'new_'.$type, $infos['name'], $img,
        '0', 'newEntry',
        [$infos['aclCategory'].'/'.$infos['mainTab'].'/c']
      );
    }

    $this->registerAction(
     new SubMenuAction(
       'new', _('Create'), 'geticon.php?context=actions&icon=document-new&size=16',
       $createMenu
     )
    );

    // Add export actions
    $exportMenu = [];
    foreach ($this->exporters as $action => $exporter) {
      $exportMenu[] = new Action(
        $action, $exporter['label'], $exporter['image'],
        '0', 'export'
      );
    }
    $this->registerAction(
     new SubMenuAction(
       'export', _('Export list'), 'geticon.php?context=actions&icon=document-export&size=16',
       $exportMenu
     )
    );

    $this->registerAction(
     new Action(
       'edit', _('Edit'), 'geticon.php?context=actions&icon=document-edit&size=16',
       '+', 'editEntry'
     )
    );
    $this->actions['edit']->setSeparator(TRUE);

    if (!$this->skipCpHandler) {
      $this->registerAction(
        new Action(
          'cut', _('Cut'), 'geticon.php?context=actions&icon=edit-cut&size=16',
          '+', 'copyPasteHandler',
          ['dr']
        )
      );
      $this->registerAction(
        new Action(
          'copy', _('Copy'), 'geticon.php?context=actions&icon=edit-copy&size=16',
          '+', 'copyPasteHandler',
          ['r']
        )
      );
      $this->registerAction(
        new Action(
          'paste', _('Paste'), 'geticon.php?context=actions&icon=edit-paste&size=16',
          '0', 'copyPasteHandler',
          ['w']
        )
      );
      $this->actions['paste']->setEnableFunction([$this, 'enablePaste']);
    }

    $this->registerAction(
     new Action(
       'remove', _('Remove'), 'geticon.php?context=actions&icon=edit-delete&size=16',
       '+', 'removeRequested',
       ['d']
     )
    );

    if (!static::$skipSnapshots && ($config->get_cfg_value('enableSnapshots') == 'TRUE')) {
      $this->registerAction(
        new Action(
          'snapshot', _('Create snapshot'), 'geticon.php?context=actions&icon=snapshot&size=16',
          '1', 'createSnapshotDialog',
          ['/SnapshotHandler/c']
        )
      );
      $this->registerAction(
        new Action(
          'restore', _('Restore snapshot'), 'geticon.php?context=actions&icon=document-restore&size=16',
          '*', 'restoreSnapshotDialog',
          ['w', '/SnapshotHandler/r']
        )
      );
      $this->actions['snapshot']->setSeparator(TRUE);
      $this->actions['restore']->setEnableFunction([$this, 'enableSnapshotRestore']);
    }


    /* Actions from footer are not in any menus and do not need a label */
    $this->registerAction(new HiddenAction('apply',           'applyChanges'));
    $this->registerAction(new HiddenAction('save',            'saveChanges'));
    $this->registerAction(new HiddenAction('cancel',          'cancelEdit'));
    $this->registerAction(new HiddenAction('cancelDelete',    'cancelEdit'));
    $this->registerAction(new HiddenAction('removeConfirmed', 'removeConfirmed'));
    if (!$this->skipConfiguration) {
      $this->registerAction(new HiddenAction('configure', 'configureDialog'));
    }
  }


}
