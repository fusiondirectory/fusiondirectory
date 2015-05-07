<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

/* Basic setup, remove eventually registered sessions */
require_once ("../include/php_setup.inc");
require_once ("functions.inc");
require_once ("variables.inc");

header("Content-type: text/html; charset=UTF-8");

/* try to start session, so we can remove userlocks,
  if the old session is still available */
@session::start();
session::set('errorsAlreadyPosted', array());
if (session::global_is_set('ui')) {

  /* Get config & ui informations */
  $ui = session::global_get("ui");

  /* config used for del_user_locks & some lines below to detect the language */
  $config = session::global_get("config");

  /* Remove all locks of this user */
  del_user_locks($ui->dn);

  /* Write something to log */
  new log("security", "logout", "", array(), "User \"".$ui->username."\" logged out");
}

initLanguage();

/* Set smarty template compile directory */
if (isset($config)) {
  $smarty->compile_dir = $config->get_cfg_value("templateCompileDirectory", SPOOL_DIR);
} else {
  $smarty->compile_dir = SPOOL_DIR;
}

$smarty->assign("date", date("l, dS F Y H:i:s O"));

/* If GET request is posted, the logout was forced by pressing the link */
if (isset($_GET['request'])) {

  /* destroy old session */
  session::destroy ();

  /* If we're not using htaccess authentication, just redirect... */
  if (isset($config) && $config->get_cfg_value("htaccessAuthentication") == "TRUE") {

    /* Else notice that the user has to close the browser... */
    $smarty->assign("usePrototype", "false");
    $smarty->display (get_template_path('headers.tpl'));
    $smarty->display (get_template_path('logout-close.tpl'));
    exit;
  }

  header ("Location: index.php");
  exit();

} else {  // The logout wasn't forced, so the session is invalid

  $smarty->assign("usePrototype", "false");
  $smarty->display (get_template_path('headers.tpl'));
  $smarty->display (get_template_path('logout.tpl'));
  exit;
}
?>
</html>
