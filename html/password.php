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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

function displayPWchanger()
{
  global $smarty;

  $smarty->display(get_template_path('password.tpl'));
  exit();
}

/* Load required includes */
require_once ("../include/php_setup.inc");
require_once ("functions.inc");

if(!class_exists("log")){
  require_once("class_log.inc");
}

header("Content-type: text/html; charset=UTF-8");

session::start();

/* Destroy old session if exists.
    Else you will get your old session back, if you not logged out correctly. */
if(is_array(session::get_all()) && count(session::get_all())){
  session::destroy();
  session::start();
}

/* Reset errors */
session::global_set('js',true);
session::set('errors',"");
session::set('errorsAlreadyPosted',array());
session::set('LastError',"");

/* Check if CONFIG_FILE is accessible */
if (!is_readable(CONFIG_DIR."/".CONFIG_FILE)){
  msg_dialog::display(_("Fatal error"), sprintf(_("GOsa configuration %s/%s is not readable. Aborted."),CONFIG_DIR,CONFIG_FILE), FATAL_ERROR_DIALOG);
  exit;
}

/* Parse configuration file */
$config= new config(CONFIG_DIR."/".CONFIG_FILE, $BASE_DIR);
session::global_set('DEBUGLEVEL', $config->get_cfg_value("debuglevel"));
if ($_SERVER["REQUEST_METHOD"] != "POST"){
  @DEBUG (DEBUG_CONFIG, __LINE__, __FUNCTION__, __FILE__, $config->data, "config");
}

/* Set template compile directory */
$smarty->compile_dir= $config->get_cfg_value("templateCompileDirectory", '/var/spool/gosa');

/* Check for compile directory */
if (!(is_dir($smarty->compile_dir) && is_writable($smarty->compile_dir))){
  msg_dialog::display(_("Configuration error"), sprintf(_("Directory '%s' specified as compile directory is not accessible!"),
        $smarty->compile_dir), FATAL_ERROR_DIALOG);
  exit;
}

/* Check for old files in compile directory */
clean_smarty_compile_dir($smarty->compile_dir);

/* Language setup */
if ($config->get_cfg_value("language") == ""){
  $lang= get_browser_language();
} else {
  $lang= $config->get_cfg_value("language");
}
$lang.=".UTF-8";
putenv("LANGUAGE=");
putenv("LANG=$lang");
setlocale(LC_ALL, $lang);
$GLOBALS['t_language']= $lang;
$GLOBALS['t_gettext_message_dir'] = $BASE_DIR.'/locale/';

/* Set the text domain as 'messages' */
$domain = 'messages';
bindtextdomain($domain, LOCALE_DIR);
textdomain($domain);

/* Generate server list */
$servers= array();
foreach ($config->data['LOCATIONS'] as $key => $ignored){
	$servers[$key]= $key;
}
if (isset($_POST['server'])){
	$directory= validate($_POST['server']);
} else {
	$directory= $config->data['MAIN']['DEFAULT'];

  if(!isset($servers[$directory])){
    $directory = key($servers);
  }
}
if (isset($_GET['directory']) && isset($servers[$_GET['directory']])){
	$smarty->assign ("show_directory_chooser", false);
	$directory= validate($_GET['directory']);
} else {
	$smarty->assign ("server_options", $servers);
	$smarty->assign ("server_id", $directory);
	$smarty->assign ("show_directory_chooser", true);
}

/* Set config to selected one */
$config->set_current($directory);
session::global_set('config',$config);

if ($_SERVER["REQUEST_METHOD"] != "POST"){
  @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $lang, "Setting language to");
}


/* Check for SSL connection */
$ssl= "";
if (!isset($_SERVER['HTTPS']) ||
    !stristr($_SERVER['HTTPS'], "on")) {

  if (empty($_SERVER['REQUEST_URI'])) {
    $ssl= "https://".$_SERVER['HTTP_HOST'].
      $_SERVER['PATH_INFO'];
  } else {
    $ssl= "https://".$_SERVER['HTTP_HOST'].
      $_SERVER['REQUEST_URI'];
  }
}

/* If SSL is forced, just forward to the SSL enabled site */
if ($config->get_cfg_value("forcessl") == 'true' && $ssl != ''){
  header ("Location: $ssl");
  exit;
}

/* Check for selected password method */
$method= $config->get_cfg_value("passwordDefaultHash", "crypt/md5");
if (isset($_GET['method'])){
	$method= validate($_GET['method']);
	$tmp = new passwordMethod($config);
	$available = $tmp->get_available_methods();
	if (!isset($available[$method])){
    msg_dialog::display(_("Password method"),_("Error: Password method not available!"),FATAL_ERROR_DIALOG);
		exit;
	}
}


/* Check for selected user... */
if (isset($_GET['uid']) && $_GET['uid'] != ""){
	$uid= validate($_GET['uid']);
	$smarty->assign('display_username', false);
} elseif (isset($_POST['uid'])){
	$uid= validate($_POST['uid']);
	$smarty->assign('display_username', true);
} else {
	$uid= "";
	$smarty->assign('display_username', true);
}
$current_password= "";
$smarty->assign("changed", false);

/* Got a formular answer, validate and try to log in */
if ($_SERVER["REQUEST_METHOD"] == "POST" && isset($_POST['apply'])){

  /* Destroy old sessions, they cause a successfull login to relog again ...*/
  if(session::global_is_set('_LAST_PAGE_REQUEST')){
    session::global_set('_LAST_PAGE_REQUEST',time());
  }

  $message= array();
  $current_password= $_POST['current_password'];

  /* Do new and repeated password fields match? */
  $new_password= $_POST['new_password'];
  if ($_POST['new_password'] != $_POST['new_password_repeated']){
	  $message[]= _("The passwords you've entered as 'New password' and 'Repeated new password' do not match.");
  } else {
	  if ($_POST['new_password'] == ""){
      $message[]= msgPool::required(_("New password"));
	  }
  }

  /* Password policy fulfilled? */
  if ($config->get_cfg_value("passwordMinDiffer") != ""){
	  $l= $config->get_cfg_value("passwordMinDiffer");
	  if (substr($_POST['current_password'], 0, $l) == substr($_POST['new_password'], 0, $l)){
		  $message[]= _("The password used as new and current are too similar.");
	  }
  }
  if ($config->get_cfg_value("passwordMinLength") != ""){
	  if (strlen($_POST['new_password']) < $config->get_cfg_value("passwordMinLength")){
		  $message[]= _("The password used as new is to short.");
	  }
  }

  /* Validate */
  if (!tests::is_uid($uid)){
	  $message[]= msgPool::invalid(_("Login"));
  } elseif (mb_strlen($_POST["current_password"], 'UTF-8') == 0){
    $message[]= msgPool::required(_("Current password"));
  } else {

    /* Do we have the selected user somewhere? */
    $ui= ldap_login_user ($uid, $current_password);

    if ($ui === NULL){
      $message[]= _("Please check the username/password combination.");
    } else {
      $acls = $ui->get_permissions($ui->dn,"users/password");
      if(!preg_match("/w/i",$acls)){
        $message[]= _("You have no permissions to change your password.");
      }
    }
  }

  /* Do we need to show error messages? */
  if (count ($message) != 0){
	  /* Show error message and continue editing */
	  msg_dialog::displayChecks($message);
  } else {

	  /* Passed quality check, just try to change the password now */
	  $output= "";
	  if ($config->get_cfg_value("passwordHook") != ""){
		  exec($config->get_cfg_value("passwordHook")." ".escapeshellarg($ui->username)." ".
				  escapeshellarg($_POST['current_password'])." ".escapeshellarg($_POST['new_password']), $resarr);
		  if(count($resarr) > 0) {
			  $output= join('\n', $resarr);
		  }
	  }
	  if ($output != ""){
		  $message[]= _("External password changer reported a problem: ".$output);
		  msg_dialog::displayChecks($message);
	  } else {
		  if ($method != ""){
			  change_password ($ui->dn, $_POST['new_password'], 0, $method);
		  } else {
			  change_password ($ui->dn, $_POST['new_password']);
		  }
		  gosa_log ("User/password has been changed");
		  $smarty->assign("changed", true);
	  }
  }


}

/* Parameter fill up */
$params= "";
foreach (array('uid', 'method', 'directory') as $index){
	$params.= "&amp;$index=".urlencode($$index);
}
$params= preg_replace('/^&amp;/', '?', $params);
$smarty->assign('params', $params);

/* Fill template with required values */
$smarty->assign ('date', gmdate("D, d M Y H:i:s"));
$smarty->assign ('uid', $uid);
$smarty->assign ('password_img', get_template_path('images/password.png'));

/* Displasy SSL mode warning? */
if ($ssl != "" && $config->get_cfg_value("warnssl") == 'true'){
  $smarty->assign ("ssl", "<b>"._("Warning").":</b> "._("Session will not be encrypted.")." <a style=\"color:red;\" href=\"".htmlentities($ssl)."\"><b>"._("Enter SSL session")."</b></a>!");
} else {
  $smarty->assign ("ssl", "");
}

/* show login screen */
$smarty->assign("JS",session::global_get('js'));
$smarty->assign ("PHPSESSID", session_id());
if (session::is_set('errors')){
  $smarty->assign("errors", session::get('errors'));;
}
if ($error_collector != ""){
  $smarty->assign("php_errors", $error_collector."</div>");
} else {
  $smarty->assign("php_errors", "");
}

$smarty->assign("msg_dialogs", msg_dialog::get_dialogs());
displayPWchanger();

?>

</body>
</html>
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
