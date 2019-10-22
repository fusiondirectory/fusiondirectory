<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2017-2019  FusionDirectory

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

/*!
 * \brief Management class for selection dialogs
 */
class selectManagement extends management
{
  protected $skipCpHandler      = TRUE;
  public static $skipSnapshots  = TRUE;

  protected $multiSelect = TRUE;
  public $blackList;
  public $whiteList;

  /* Default columns */
  public static $columns = [
    ['ObjectTypeColumn', []],
    ['LinkColumn',       ['attributes' => 'nameAttr',    'label' => 'Name']],
    ['LinkColumn',       ['attributes' => 'description', 'label' => 'Description']],
  ];

  function __construct (
    $objectTypes        = FALSE,
    bool $multiSelect   = TRUE,
    array $neededAttrs  = [],
    array $blackList    = [],
    array $whiteList    = [],
    array $filterElementDefinitions = [
      ['TabFilterElement',  []],
    ])
  {
    $this->neededAttrs  = $neededAttrs;
    $this->multiSelect  = $multiSelect;
    $this->blackList    = $blackList;
    $this->whiteList    = $whiteList;
    parent::__construct($objectTypes, $filterElementDefinitions);
  }

  protected function setUpListing ()
  {
    $this->listing  = new managementListing($this, TRUE, $this->multiSelect);
  }

  protected function setUpHeadline ()
  {
    $this->headline = _('Please select the desired entries');
  }

  protected function configureActions ()
  {
    $this->registerAction(new HiddenAction('configure',       'configureDialog'));
  }

  function renderList (): string
  {
    $list = parent::renderList();

    $smarty = get_smarty();
    $smarty->assign('MULTISELECT',  $this->multiSelect);

    return $list.$smarty->fetch(get_template_path('management/select-footer.tpl'));
  }

  /*!
   * \brief Let header handling to parent
   */
  protected function getHeader (): string
  {
    return '';
  }
}
