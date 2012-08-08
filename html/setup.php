<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/* Get standard functions */
require_once ("../include/php_setup.inc");
require_once ("functions.inc");
require_once ("variables.inc");

require_once("../setup/class_setup.inc");
require_once("../setup/class_setupStep.inc");
require_once("../setup/class_setupStep_Welcome.inc");
require_once("../setup/class_setupStep_Language.inc");
require_once("../setup/class_setupStep_Checks.inc");
require_once("../setup/class_setupStep_Ldap.inc");
require_once("../setup/class_setupStep_Config1.inc");
require_once("../setup/class_setupStep_Config2.inc");
require_once("../setup/class_setupStep_Config3.inc");
require_once("../setup/class_setupStep_Schema.inc");
require_once("../setup/class_setupStep_Migrate.inc");
require_once("../setup/class_setupStep_Finish.inc");


/* Set header */
header("Content-type: text/html; charset=UTF-8");

/* Set cookie lifetime to one day (The parameter is in seconds ) */
session_set_cookie_params(24*60*60);
session_cache_expire(60*24);  // default is 180
ini_set("session.gc_maxlifetime",24*60*60);

/* Start session */
session::start();
session::global_set('DEBUGLEVEL',1);
session::set('errorsAlreadyPosted',array());

/* Attribute initialization, reset errors */
reset_errors();

/* Set template compile directory */
$smarty->compile_dir= SPOOL_DIR;

/* Check for compile directory */
if (!(is_dir($smarty->compile_dir) && is_writable($smarty->compile_dir))){
  msg_dialog::display(_("Smarty"),sprintf(_("Directory '%s' specified as compile directory is not accessible!"),
    $smarty->compile_dir),FATAL_ERROR_DIALOG);
  exit();
}

/* Get posted language */
if(!session::global_is_set('lang')){
  session::global_set('lang',get_browser_language());
}
if(isset($_POST['lang_selected'])){
  if($_POST['lang_selected'] != ""){
    session::global_set('lang',$_POST['lang_selected']);
  }else{
    session::global_set('lang',get_browser_language());
  }
}

/* Check for js */
if (!isset($_GET['js']) && !session::global_is_set('js')){
  echo '<script language="JavaScript" type="text/javascript">';
  echo '  location = "setup.php?js=true";';
  echo '</script>';

  session::global_set('js',FALSE);
} elseif(isset($_GET['js'])) {
  session::global_set('js',TRUE);
}


$lang = session::global_get('lang');
/* Append .UTF-8 to language string if necessary */
if(!preg_match("/utf(-)8$/i",$lang)){
  $lang .= ".UTF-8";
}
putenv("LANGUAGE=");
putenv("LANG=$lang");
setlocale(LC_ALL, $lang);
$GLOBALS['t_language']= $lang;
$GLOBALS['t_gettext_message_dir'] = $BASE_DIR.'/locale/';

/* Set the text domain as 'messages' */
$domain = 'messages';
bindtextdomain($domain, LOCALE_DIR);
textdomain($domain);


/* Call setup */
$display = "";
require_once("../setup/main.inc");

$smarty->assign("date", date("l, dS F Y H:i:s O"));
$header = $smarty->fetch(get_template_path('setup_headers.tpl'));



/* Set focus to the error button if we've an error message */
$focus= "";
if (session::is_set('errors') && session::get('errors') != ""){
  $focus= '<script language="JavaScript" type="text/javascript">';
  $focus.= 'document.forms[0].error_accept.focus();';
  $focus.= '</script>';
}

$focus= '<script language="JavaScript" type="text/javascript">';
$focus.= 'next_msg_dialog();';
$focus.= '</script>';

/* show web frontend */
$setup = session::global_get('setup');
$smarty->assign("contents"  , $display);
$smarty->assign("navigation", $setup->get_navigation_html());
$smarty->assign("header", $setup->get_header_html());
$smarty->assign("bottom", $focus.$setup->get_bottom_html());
$smarty->assign("msg_dialogs", msg_dialog::get_dialogs());

if ($error_collector != ""){
  $smarty->assign("php_errors", preg_replace("/%BUGBODY%/",$error_collector_mailto,$error_collector)."</div>");
} else {
  $smarty->assign("php_errors", "");
}

$smarty->assign("version",FD_VERSION);


echo $header.$smarty->fetch("../setup/setup_frame.tpl");
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
