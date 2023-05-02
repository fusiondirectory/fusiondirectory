<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2019-2020  FusionDirectory

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
 * \file interface_FusionDirectoryDialog.inc
 * Source code for the interface FusionDirectoryDialog
 */

/*! \brief This interface should be implemented by all dialog classes in FusionDirectory
 */
interface FusionDirectoryDialog
{
  /*! \brief Interpret POST content
   */
  public function readPost ();

  /*! \brief Update state and return FALSE if the dialog was closed
   */
  public function update (): bool;

  /*! \brief Render the dialog and returns the HTML code
   */
  public function render (): string;
}
