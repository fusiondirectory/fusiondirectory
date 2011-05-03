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

/* Include classes and configs */
@require_once ("../include/php_setup.inc");
@require_once ("functions.inc");
@require_once ("functions_helpviewer.inc");

error_reporting(E_ALL | E_STRICT);
restore_error_handler();
header("Content-type: text/html; charset=UTF-8");

session::start();

/* If no config object is found in the session, abort help */
if (!session::global_is_set('config')){
  new log("security","all/all","",array(),"Help viewer called without session") ;
  header ("Location: index.php");
  exit;
}

$ui= session::global_get('ui');
$config= session::global_get('config');

/* Language setup */
$lang= $config->get_cfg_value("language");
if ($lang == ""){
  $lang= get_browser_language();
}

$lang.=".UTF-8";

putenv("LANGUAGE=");
putenv("LANG=$lang");
setlocale(LC_ALL, $lang);
$GLOBALS['t_language']= $lang;
$GLOBALS['t_gettext_message_dir'] = $BASE_DIR.'/locale/';

$domain = 'messages';
bindtextdomain($domain, LOCALE_DIR);
textdomain($domain);
@DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $lang, "Setting language to");

$smarty->compile_dir= $config->get_cfg_value("templateCompileDirectory", "/var/spool/fusiondirectory/");
$smarty->assign("title", "FusionDirectory - "._("Help browser"));

/* HELP management starts here ...
 */

/* Generate helpobject */
if(session::global_is_set('helpobject')){
  $helpobject = session::global_get('helpobject');
}else{

  $plist = session::global_get('plist');

  $helpobject['lang']         = $lang; 
  $helpobject['helpconf']     = array();  
  $helpobject['currentplug']  = "";
  $helpobject['file']         = "index.html";
  $helpobject['helpconf']     = $plist->gen_headlines();
}

$lang = $lang[0].$lang[1];

$helpobject['lang']           = $lang;  
$defaultpage                  = "index.html";                       // alternative file, shown on error, or on first call
$prefix                       = "node";                             // Prefix of the generated help files 
$suffix                       = ".html";                            // Suffix of the generated helpfiles
$maxresults                   = 10;                                 // max number of results shown in result list
$minwordlength                = 3;                                  // Word less than 3 chars will be dropped in search
$allowed_chars_in_searchword  = "'[^a-z0-9 %_-]'i";                 // Remove all chars that would disturb our search like < or > ...

/* Default pages */
$backward =$defaultpage;
$index    =$defaultpage;
$forward  ="node1.html";

$helpdir ="";
/* Every class which is called within a tab, stores its name in the Session.
 * If session::global_is_set('current_class_for_help') is true, 
 *  get the helpfile specified in the xml file and display it.
 * Unset this Session entry, to avoid displaying it again.
 */
if(session::global_is_set('current_class_for_help')){

  /* Create new XML parser with the path to the Xml file */
  $xml = new parseXml("../doc/guide.xml");
  
  /* Generate help array */
  $str = $xml->parse();

  /* __LANG__ is used as placeholder for the used language*/
  $helpdir= @preg_replace("/__LANG__/i",$lang,$str[(session::global_get('current_class_for_help'))]['PATH']);

  /* If there is no entry in the xml file for this class, display an error message */
  if($helpdir == ""){
    
    $smarty->assign("help_contents","<br><h2>"._("There is no helpfile specified for this class"))."</h2>";
    $smarty->assign("iePngWorkaround", $config->get_cfg_value("iePngWorkaround","false" ) == "true");
    $header= "<!-- headers.tpl-->".$smarty->fetch(get_template_path('headers.tpl'));
    $display= (  $header.$smarty->fetch(get_template_path('help.tpl')));
    echo $display;
    session::global_un_set('current_class_for_help');
    exit();  
  }
 
  /* Save filename */
  $helpobject['file']= $str[(session::global_get('current_class_for_help'))]['FILE'];
  
  /* Save path to the file */
  $helpobject['currentplug']  = $helpdir;
  
  /* Avoid displaying the same help every time */
  if(isset($_GET['pg'])){
    session::global_un_set('current_class_for_help');
  }

}elseif(isset($_GET['plug'])){
  /* This displays helpfiles depending on the current $_GET[plug] */
  $ui= get_userinfo();
  $tmp                          = new pluglist($config, $ui);
  $path                         = $tmp->get_path($_GET['plug']);
  $helpobject['currentplug']    = $path;
  $helpobject['file']           = "index.html";
  $helpdir = "../doc/core/".$helpobject['lang']."/html/".preg_replace("/^.*\//i","",$helpobject['currentplug']);
  if(empty($helpobject['currentplug'])){
    $helpdir= "";
  }
}
/* this Post var is set if another page is requested */
if(isset($_GET['pg'])){
 
  if(preg_match("/\//",$_GET['pg'])){
    $arr = explode("/", $_GET['pg']);
    $helpobject['currentplug'] = "../doc/core/".$helpobject['lang']."/html/".$arr[0];
    $helpdir = $helpobject['currentplug'];
    $helpobject['file']= $arr[1];
  }else{
    /* PG should contain a filename */
    $helpobject['file'] = $_GET['pg'];


    /* If empty, force displaying the index */
    if(empty($_GET['pg'])){
      $helpobject['currentplug']  = "";
      $helpobject['file'] = "index.html";
    }
    /* Create new helpdir (The path where the requested page is located)*/
    $helpdir = "../doc/core/".$helpobject['lang']."/html/".preg_replace("/^.*\//i","",$helpobject['currentplug']);

    /* If helpdir is empty, force index */
    if(empty($helpobject['currentplug'])){
      $helpdir= "";
    }
  }
}
$helpdir.="/";

/* Save current settings */
session::global_set('helpobject',$helpobject);

/* 
 * Display management 
 */

$files = array();
if (is_dir($helpdir)) {
	$f = opendir($helpdir);
		while($file = readdir($f)){
  	$files[$file]=$file;
	}
}

/* Some replacements */
$backwardlink  = "<a href=\"?pg=%s\"  class=\"maintitlebar\">
                  <img src='images/back.png' class=\"center\" align=\"middle\" alt=\""._("previous")."\" border=\"0\">
                 </a>";

$forwardlink   = "<a href=\"?pg=%s\"  class=\"maintitlebar\">
                  <img src='images/forward.png' class=\"center\" align=\"middle\" alt=\""._("next")."\" border=\"0\">
                 </a>";


$back = $for  ="";
if($helpobject['file'] == "index.html"){
  $back = "&nbsp;";
  $for  = sprintf($forwardlink,   $prefix."1".$suffix);
}else{
  $current = preg_replace("/^".$prefix."/","",$helpobject['file']);
  $current = preg_replace("/\.html$/","",$current);
 
  if(isset($files[$prefix.($current+1).$suffix])) {
    $for  = sprintf($forwardlink,    $prefix.($current+1).$suffix);
  }
  if(isset($files[$prefix.($current-1).$suffix])) {
    $back = sprintf($backwardlink,   $prefix.($current-1).$suffix);
  }
  if(($current-1) == 0){
    $back = sprintf($backwardlink,   "index.html");
  }
}


/* If there is no helpdir or file defined, display the index */
if(isset($_POST['search'])){
  $helpdir = "../doc/core/".$helpobject['lang']."/html/";
  /* read all available directories */
  $index = readfiles($helpdir,$prefix,$suffix,false,false);
  $smarty->assign("help_contents",((searchlist($index,search($index,$_POST['search_string']),10))));
  $header= "<!-- headers.tpl-->".$smarty->fetch(get_template_path('headers.tpl'));

  /* I don't know why, but we must use utf8_encode to avoid dispplay errors */
  $display= (  $header.$smarty->fetch(get_template_path('help.tpl')));
  echo $display;
}elseif(((empty($helpdir)))||($helpdir=="/")){
  /* Generate Index and display it */
  $smarty->assign("help_contents",genIndex());
  $header= "<!-- headers.tpl-->".$smarty->fetch(get_template_path('headers.tpl'));

  /* I don't know why, but we must use utf8_encode to avoid dispplay errors */
  $display= utf8_encode(  $header.$smarty->fetch(get_template_path('help.tpl')));
  echo $display;

}elseif((is_dir($helpdir))&&($fp = opendir($helpdir))){
  
  /* Readfile gets the content of the requested file, 
   * parse it, rework links images and so on */
  $index = readfiles($helpdir,$prefix,$suffix,false,$helpobject['file']);

  $lastresults = session::global_get('lastresults');

  /* if this page is result from a search, mark the search strings */
  if(isset($_GET['mark'])){
    $matches = $lastresults[preg_replace("/^.*\//i","",$helpobject['currentplug'])][$helpobject['file']];
    $index[$helpobject['file']]['content']   = markup_page($index[$helpobject['file']]['content'],$matches);
  }

  /* Display the help contents */
  $smarty->assign("help_contents",$index[$helpobject['file']]['content']);
  $header= "<!-- headers.tpl-->".$smarty->fetch(get_template_path('headers.tpl'));
  
  /* I don't know why, but we must use utf8_encode to avoid dispplay errors */
  $smarty->assign("backward",$back);
  $smarty->assign("forward" ,$for);
  $display= utf8_encode(  $header.$smarty->fetch(get_template_path('help.tpl')));
  echo $display;

}else{
  
  /* There was a file requested which actually doesn't exists */
  $smarty->assign("help_contents","<h2>".sprintf(_("Helpdir '%s' is not accessible, can't read any helpfiles."),$helpdir))."</h2><br>";
  $header= "<!-- headers.tpl-->".$smarty->fetch(get_template_path('headers.tpl'));
  $display= (  $header.$smarty->fetch(get_template_path('help.tpl')));
  echo $display;
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
