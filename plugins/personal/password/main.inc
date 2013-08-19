<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

/* Remove locks created by this plugin  */
$lock_msg = "";
if ($remove_lock) {
  if (session::is_set('password')) {
    //Nothing to do here
  }
}

/* Remove this plugin from session */
if ($cleanup) {
  session::un_set('password');
  session::un_set('edit');
} else {
  /* Reset requested? */
  if (isset($_POST['edit_cancel'])) {
    session::un_set('edit');
    session::un_set('password');
  }

  /* Create password object on demand */
  if (!session::is_set('password') || (isset($_GET['reset']) && $_GET['reset'] == 1)) {
    session::set('password', new password ($config, $ui->dn));
  }
  $password = session::get('password');

  /* Execute formular */
  $display .= $password->execute();

  /* Page header*/
  $display = print_header(get_template_path('plugins/password/images/icon.png'),
                         _("Password settings"), "").$display;

}
?>
