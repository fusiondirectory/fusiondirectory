<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2020  FusionDirectory

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

/*! \brief Generic dialog for selection using a selectManagement class
 */
class GenericSelectManagementDialog extends GenericDialog
{
  protected $dialogClass = 'selectManagement';

  public function __construct ($simplePlugin, $attribute)
  {
    $this->attribute  = $attribute;
    $this->dialog     = new $this->dialogClass(...$this->attribute->getSelectManagementParameters());
  }

  protected function handleFinish ()
  {
    $result = $this->dialog->detectPostActions();
    if (isset($result['targets'])) {
      if ($this->dialog instanceof selectManagement) {
        $headpage = $this->dialog->listing;
      } else {
        $headpage = $this->dialog->getHeadpage();
      }
      foreach ($result['targets'] as $dn) {
        $entry = $headpage->getEntry($dn);

        $this->attribute->addValue($dn, $entry);
      }
    }
    unset($this->dialog);
  }
}
