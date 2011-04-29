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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

if (!$cleanup){
	session::set('plugin_dir',dirname(__FILE__));
	$smarty->assign("iconmenu", $plist->show_iconmenu());
	$smarty->assign("header", print_header(get_template_path('images/welcome.png'), sprintf(_("Welcome %s!"), $ui->cn)));
	$smarty->assign("year", date("Y"));

	if(function_exists("get_gosa_version")){
		$smarty->assign("revision", get_gosa_version());
	}

	$display= $smarty->fetch(get_template_path('welcome.tpl', TRUE));
}

?>
