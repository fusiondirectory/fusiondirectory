<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/* Remove locks created by this plugin */
if ($remove_lock){
  if(session::is_set('ogroupManagement')){
      $macl = session::get('ogroupManagement');
      $macl->remove_lock();
  }
}

/* Remove this plugin from session */
if ( $cleanup ){
  session::un_set('ogroupManagement');
}else{

  /* Create ogroupmanagement object on demand */
  if (!session::is_set('ogroupManagement')){
    $ogroupManagement= new ogroupManagement ($config, $ui);
    session::set('ogroupManagement',$ogroupManagement);
  }
  $ogroupManagement = session::get('ogroupManagement');
  $display= $ogroupManagement->execute();

  /* Reset requested? */
  if (isset($_GET['reset']) && $_GET['reset'] == 1){
    session::un_set ('ogroupManagement');
  }

  /* Show and save dialog */
  session::set('ogroupManagement',$ogroupManagement);
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
