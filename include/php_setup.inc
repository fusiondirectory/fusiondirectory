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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

require_once ("variables.inc");

function gosaRaiseError($errno, $errstr, $errfile, $errline)
{
  global $error_collector,$config, $error_collector_mailto;

  // To avoid recursion - restore original error handler.
  restore_error_handler();

  /* Return if error reporting is set to zero */
  if (error_reporting() == 0){
    set_error_handler('gosaRaiseError', E_WARNING |  E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT) ;
    return;
  }

  /* Workaround for buggy imap_open error outputs */
  if (preg_match('/imap_open/', $errstr)){
    set_error_handler('gosaRaiseError', E_WARNING |  E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT) ;
    return;
  }

  /* Hide ldap size limit messages */
  if (preg_match('/ldap_error/', $errstr)){
    if (preg_match('/sizelimit/', $errstr)){
      set_error_handler('gosaRaiseError', E_WARNING |  E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT) ;
      return;
    }
  }

  /* Error messages are hidden in FusionDirectory, so we only send them to the logging class and abort here */
  if(isset($config->data) && $config->get_cfg_value("displayerrors") != "true"){

    /* Write to syslog */
    if(class_exists("log") && !preg_match("/No such object/",$errstr)){
      new log("view","error","",array(),"PHP error: $errstr ($errfile, line $errline)");
    }

    set_error_handler('gosaRaiseError', E_WARNING |  E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT) ;
    return;
  }
 
  /* Send all errors to logging class, except "Ldap : No such object" messages*/
  if(class_exists("log") && !preg_match("/No such object/",$errstr)){
    new log("debug","all",$errfile,array(),"Type:".$errno.", Message:".$errstr.", File:".$errfile.", Line: ".$errline);
  }

  /* Create header as needed */
  if ($error_collector == ""){

    /* Mailto body header */
    if(function_exists("prepare4mailbody")){
      $version= "unknown";
      if(function_exists("get_gosa_version")){
	      $version= get_gosa_version();
      }
      $error_collector_mailto .=prepare4mailbody(
                                "Oups. Seems like you've catched some kind of bug inside FusionDirectory/PHP. You may want to help ".
                                "us to improve the software stability. If so, please provide some more information below.".
                                "\n\n".
                                "*** FusionDirectory bug report ***".
                                "\nFusionDirectory Version: $version".
                                "\n\n".
                                "Please describe what you did to produce this error as detailed as possible. Can you ".
                                "reproduce this bug using the demo on http://demo.FusionDirectory.org ?".
                                "\n\n".
                                "*** PHP error information ***\n\n");
      }
    
    if (class_exists('session') && session::is_set('js') && session::get('js')==FALSE){
      $error_collector= "<div>";
    } else {

      $warning_path = (is_callable('get_template_path')) ? get_template_path('images/warning.png') : 'images/warning.png' ;

      $error_collector= "
        <table summary=\"\" width=\"100%\" style='background-color:#E0E0E0;border-bottom:1px solid black;z-index:150;'>
          <tr>
            <td>
              <img alt=\"\" align=\"middle\" src='{$warning_path}'>&nbsp;
              <font style='font-size:14px;font-weight:bold'>".
                _("Generating this page caused the PHP interpreter to raise some errors!")."
              </font>
            </td>
            <td align=right>
              <a href=\"mailto:bugs@fusiondirectory.org?subject=FusionDirectory%20bugreport&amp;body=%BUGBODY%\">
                <img border='0' src='images/mailto.png' title='"._("Send bug report to the FusionDirectory Team")."' class='center' alt=''>&nbsp;"._("Send bugreport")."
              </a>
            </td>
            <td align=right>
              <button onClick=\"$('errorbox').toggle();\">".
                _("Toggle information")."
              </button>
            </td>
          </tr>
        </table>
        <div id='errorbox' style='position:absolute; z-index:150; display: none;'>";
    }
  }
 
  /* Create error header */
  $error_collector.= "<table summary=\"\" width=\"100%\" cellspacing=0 style='background-color:#402005;color:white;border:2px solid red'><tr><td colspan=3><h1 style='color:white'>"._("PHP error")." \"$errstr\"</h1></td></tr>";

  $error_collector_mailto .= prepare4mailbody("=== Error === \n");
  $error_collector_mailto .= prepare4mailbody("PHP error: $errstr ($errfile, line $errline)\n");
  $error_collector_mailto .= prepare4mailbody("=== /Error === \n\n");
  
  /* Extract traceback data - if available */
  if (function_exists('debug_backtrace')){
    $trace= debug_backtrace();

    $error_collector_mailto .= prepare4mailbody("=== Trace ===");

    /* Generate trace history */
    for ($index= 0, $c= count($trace); $index<$c; $index++){


      $ct= $trace[$index];
      $loc= "";
      if (isset($ct['class'])){
        $loc.= _("class")." ".$ct['class'];
        if (isset($ct['function'])){
          $loc.= " / ";
        }
      }
      if (isset($ct['function'])){
        $loc.= _("function")." ".$ct['function'];
      }
      if (isset($ct['type'])){
        switch ($ct['type']){
          case "::":
            $type= _("static");
          break;

          case "->":
            $type= _("method");
          break;
        }
      } else {
        $type= "-";
      }
      $args= "";
      if (isset($ct['args'])){
        foreach ($ct['args'] as $arg){
       
          /* Avoid convertig object to string errors */ 
          if(is_object($arg)){
            $arg = "CLASS:&nbsp;".get_class($arg);
          }
  
          $args.= htmlentities("\"$arg\", ");
        }
      }
      $args= preg_replace("/, $/", "", $args);
      if ($args == ""){
        $args= "-";
      }
      if(isset($ct['file'])) {
        $file= $ct['file'];
      }else{
        $file="";
      }
      if(isset($ct['line'])) {
        $line= $ct['line'];
      }else{
        $line="";
      }
      $color= ($index&1)?'#404040':'606060';
      $error_collector.= "<tr style='background-color:$color'><td style='padding-left:20px' width=\"30%\">"._("Trace")."[$index]: $loc</td>";
      $error_collector.= "<td>"._("File").": $file ("._('Line')." $line)</td><td width=\"10%\">"._("Type").": $type</td></tr>";
      $error_collector.= "<tr style='background-color:$color'><td colspan=3 style='padding-left:20px;'>"._("Arguments").": $args</td></tr>";

      /* Add trace part to mailto body */
      $error_collector_mailto .= prepare4mailbody(
                                   "\nTrace[".$index."]:".$loc.
                                   "\nFile : ".$file.
                                   "\nLine : ".$line.
                                   "\nType : ".$type.
                                   "\n  ".$args.
                                   "\n");

    }
  }

  $error_collector_mailto .= prepare4mailbody("=== /Trace === \n");

  /* Close error table */
  $error_collector.= "</table>";

  /* Flush in case of fatal errors */
  set_error_handler('gosaRaiseError', E_WARNING |  E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT) ;
  if (preg_match('/^fatal/i', $errstr)){
    echo $error_collector."</div>";
    flush();
    exit;
  }
}


function prepare4mailbody($string)
{
  $string = html_entity_decode($string);

  $from = array(
                "/%/",
                "/ /",
                "/\n/",
                "/\r/",
                "/!/",
                "/#/",
                "/\*/",
                "/\//",
                "/</",
                "/>/",
                "/\?/",
                "/\"/");

  $to = array(
                "%25",
                "%20",
                "%0A",
                "%0D",
                "%21",
                "%23",
                "%2A",
                "%2F",
                "%3C",
                "%3E",
                "%3F",
                "%22");

  $string = preg_replace($from,$to,$string);

  return($string);
}


function dummy_error_handler()
{
}

/* Bail out for incompatible/old PHP versions */
if (!version_compare(phpversion(),"5.2.0",">=")){
  echo "PHP version needs to be 5.2.0 or above to run FusionDirectory. Aborted.";
  exit();
}

/* Set timezone */
date_default_timezone_set("GMT");

/* Get base dir for reference */
$BASE_DIR= dirname(dirname(__FILE__));
$ROOT_DIR= $BASE_DIR."/html";
error_reporting (E_ALL | E_STRICT);

/* Register error handler */
$error_collector= "";
$error_collector_mailto= "";

set_error_handler('gosaRaiseError', E_WARNING |  E_NOTICE | E_USER_ERROR | E_USER_WARNING | E_USER_NOTICE | E_STRICT) ;

$variables_order= "ES";
ini_set("register_globals",0);
ini_set("track_vars",1);
ini_set("display_errors",1);
ini_set("report_memleaks",1);
ini_set("include_path",".:$BASE_DIR/include:$BASE_DIR/include/utils/excel:/usr/share/php");

/* Do smarty setup */

if (file_exists(SMARTY)) {
 require(SMARTY);
} else {
 require(SMARTY3);
}

/*require(SMARTY_PATH);*/
$smarty = new Smarty;
$smarty->template_dir = $BASE_DIR.'/ihtml/';
$smarty->caching= false;

// To be able to switch between smarty version 2/3
if(defined('SMARTY_PHP_REMOVE')){
    $smarty->php_handling= SMARTY_PHP_REMOVE;
}else{
    $smarty->php_handling= Smarty::PHP_REMOVE;
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
