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

class Step_Checks extends setup_step
{
  var $basic_checks = array();
  var $config_checks= array();
  var $is_writeable = array();
  var $header_image = "images/setup/system.png";

  function Step_Checks()
  {
    $this->update_strings();
  }
 

  function update_strings()
  {
    $this->s_title      = _("Installation check");
    $this->s_title_long = _("Installation check");
    $this->s_info       = _("Basic checks for PHP compatibility and extensions");
  }
  
  
  /* Execute and display template */ 
  function execute()
  {
    $this->run_checks();
    $smarty = get_smarty();
    $smarty->assign("basic"       ,$this->basic_checks);
    $smarty->assign("config"      ,$this->config_checks);
    $smarty->assign("is_writeable",$this->is_writeable);
    return($smarty->fetch(get_template_path("setup_checks.tpl",TRUE,dirname(__FILE__))));
  }


  /* Execute all checks */ 
  function run_checks()
  {
    $this->basic_checks = array();
    $this->config_checks = array();
    $this->is_writeable = array();

    /* PHP version check */
    $N = _("Checking PHP version"); 
    $D = sprintf(_("PHP must be of version %s or above."), "5.2.0");
    $S = _("Please upgrade to a supported version.");
    $R = $this->check_php_version();
    $M = TRUE;
    $this->basic_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    /* Checking imap extension */
    $N = msgPool::checkingFor("LDAP");
    $D = _("FusionDirectory requires this module to talk with your LDAP server.");
    $S = msgPool::installPhpModule("LDAP");
    $R = is_callable("ldap_bind");
    $M = TRUE;
    $this->basic_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    /* Check for gettext support */
    $N = msgPool::checkingFor("gettext");
    $D = _("FusionDirectory requires this module for an internationalized interface.");
    $S = msgPool::installPhpModule("gettext");
    $R = is_callable("bindtextdomain");
    $M = TRUE;
    $this->basic_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    /* Check for php5-curl support */
    $N = msgPool::checkingFor("curl");
    $D = _("FusionDirectory requires this module to communicate with different types of servers and protocols.");
    $S = msgPool::installPhpModule("php5-curl");
    $R = is_callable("curl_init");
    $M = TRUE;
    $this->basic_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    /* Checing for iconv */
    $N = msgPool::checkingFor("inconv");
    $D = _("FusionDirectory requires this module for the samba integration.");
    $S = msgPool::installPhpModule("iconv");
    $R = is_callable("iconv");
    $M = TRUE;
    $this->basic_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    /* Check for installed mhash module */
    $N = msgPool::checkingFor("hash method");
    $D = _("FusionDirectory requires either 'mhash' or the 'sha1' module to make use of SSHA encryption.");
    $S = msgPool::installPhpModule("mhash/sha1");
    $R = is_callable('mhash') || is_callable('sha1');
    $M = FALSE;
    $this->basic_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    /* Is imap module available */
    $N = msgPool::checkingFor("IMAP");
    $D = _("FusionDirectory requires this module to talk to an IMAP server.");
    $S = msgPool::installPhpModule("IMAP");
    $R = is_callable("imap_open");
    $M = TRUE;
    $this->basic_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    /* Is imap module available */
    $N = msgPool::checkingFor(_("mbstring"));
    $D = _("FusionDirectory requires this module to handle unicode strings.");
    $S = msgPool::installPhpModule("mbstring");
    $R = is_callable("mb_strlen");
    $M = TRUE;
    $this->basic_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    /* Mysql module loaded ? */
    $N = msgPool::checkingFor(_("MySQL"));
    $D = _("FusionDirectory requires this module to communicate with several supported databases.");
    $S = msgPool::installPhpModule("MySQL");
    $R = is_callable("mysql_query");
    $M = TRUE;
    $this->basic_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    /* Checking generate LM/NT password hashes */
    #TODO: either this, or goto-si will do the hash generation
    $query= "mkntpwd 2>&1";
    $output= shell_exec ($query);
    $have_mkntpwd= preg_match("/^Usage: mkntpwd /", $output);
    if (!$have_mkntpwd){
      $query= 'LC_ALL=C LANG=C perl -MCrypt::SmbHash -e "print join(q[:], ntlmgen $ARGV[0]), $/;" >/dev/null';
      system ($query, $ret);
      $have_mkntpwd |= ($ret == 0);
    }

    $N = msgPool::checkingFor(_("samba hash generator"));
    $D = _("FusionDirectory requires this command to synchronize POSIX and samba passwords.");
    $S = _("Deploy a goto-si installation or install the perl Crypt::SmbHash modules.");
    $R = $have_mkntpwd;
    $M = TRUE;
    $this->basic_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    /* Read data written by convert */
    $output= "";
    $sh= popen("convert", 'r');
    while (!feof($sh)){
      $output.= fread($sh, 4096);
    }
    pclose($sh);

    $N = msgPool::checkingFor(_("imagick"));
    $D = _("FusionDirectory requires this extension to handle images.");
    $S = msgPool::installPhpModule("php5-imagick");

    $IMGVER = phpversion('imagick');

    if ($IMGVER > 1.0) {
        $R = method_exists('imagick','getImageBlob');
    }
    else {
        $R = is_callable("imagick_blob2image") || !empty($output);
    }

    $M = TRUE;
    $this->basic_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    $N = msgPool::checkingFor(_("compression module"));
    $D = _("FusionDirectory requires this extension to handle snapshots.");
    $S = msgPool::installPhpModule("compile with --with-zlib");
    $R = is_callable("gzcompress");
    $M = FALSE;
    $this->basic_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );


    /* PHP Configuration checks */

    /* Register_globals off */
    $N = "register_globals = <b>off</b>";
    $D = _("register_globals is a PHP mechanism to register all global variables to be accessible from scripts without changing the scope. This may be a security risk.");
    $S = _("Search for 'register_globals' in your php.ini and switch it to 'Off'.");
    $R = ini_get("register_globals") == 0;
    $M = FALSE;
    $this->config_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );


    /* session lifetime set to >=86400 seconds ? */
    $N = "session.gc_maxlifetime &gt;= <b>86400</b>";
    $D = _("PHP uses this value for the garbage collector to delete old sessions.")." ".
         _("Setting this value to one day will prevent loosing session and cookies before they really timeout.");
    $S = _("Search for 'session.gc_maxlifetime' in your php.ini and set it to 86400 or higher.");
    $R = ini_get("session.gc_maxlifetime") >= 86400;
    $M = FALSE;
    $this->config_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    /* Session auto start must be turned off */
    $session_auto_start = ini_get('session.auto_start');
    $N = "session.auto_start = <b>"._("Off")."</b>";
    $D = _("In Order to use FusionDirectory without any trouble, the session.auto_register option in your php.ini should be set to 'Off'.");
    $S = _("Search for 'session.auto_start' in your php.ini and set it to 'Off'.");
    $R = !$session_auto_start['local_value'];
    $M = TRUE;
    $this->config_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    /* Check if memory limit is set to 32 or > */
    $N = "memory_limit &gt;= <b>32</b>";
    $D = _("FusionDirectory needs at least 32MB of memory. Setting it below this limit may cause errors that are not reproducable! Increase it for larger setups.");
    $S = _("Search for 'memory_limit' in your php.ini and set it to '32M' or higher.");
    $R = ini_get('memory_limit') >= 32 ;
    $M = TRUE;
    $this->config_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    /* Implicit Flush disabled can increase performance */
    $N = "implicit_flush = <b>"._("Off")."</b>";
    $D = _("This option influences the PHP output handling. Turn this Option off, to increase performance.");
    $S = _("Search for 'implicit_flush' in your php.ini and set it to 'Off'."); 
    $R = !ini_get('implicit_flush');
    $M = FALSE;
    $this->config_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    /* Check if execution time is set to 30 */
    $N = "max_execution_time &gt;= <b>30</b>";
    $D = _("The Execution time should be at least 30 seconds.");
    $S = _("Search for 'max_execution_time' in your php.ini and set it to '30' or higher.");
    $R = ini_get("max_execution_time") >= 30 ;
    $M = TRUE;
    $this->config_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    /* Expose php should be set to off */
    $N = "expose_php = <b>"._("Off")."</b>";
    $D = _("Increase the server security by setting expose_php to 'off'. PHP won't send any information about the server you are running in this case.");
    $S = _("Search for 'expose_php' in your php.ini and set if to 'Off'.");
    $R = !ini_get("expose_php");
    $M = FALSE;
    $this->config_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

    /* Automatic quoting must be turned on */
		/* deprecated as of php5.3 will be off like it should */
    /*$N = "magic_quotes_gpc = <b>"._("On")."</b>";
    $D = _("Increase your server security by setting magic_quotes_gpc to 'on'. PHP will escape all quotes in strings in this case."); 
    $S = _("Search for 'magic_quotes_gpc' in your php.ini and set it to 'On'."); 
    $R = ini_get('magic_quotes_gpc'); 
    $M = TRUE;
    $this->config_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M ); */

    /* Emulating old stuff? */
    $N = "zend.ze1_compatibility_mode = <b>"._("Off")."</b>";
    $D = _("Increase your server performance by setting magic_quotes_gpc to 'off'."); 
    $S = _("Search for 'zend.ze1_compatibility_mode' in your php.ini and set it to 'Off'."); 
    $R = !ini_get('zend.ze1_compatibility_mode'); 
    $M = FALSE;
    $this->config_checks[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );


    /* Configuration file  
     */

    /* Check if we can create a config file.*/
    $N = _("Configuration writeable");
    $D = _("The configuration file can't be written");
    $S = sprintf(_("FusionDirectory reads its configuration from a file located in (%s/%s). The setup can write the configuration directly if it is writeable."),CONFIG_DIR,CONFIG_FILE); 
    $R =    ( file_exists(CONFIG_DIR."/".CONFIG_FILE) && is_writeable(CONFIG_DIR."/".CONFIG_FILE)) // is there a config file ? 
        ||  (!file_exists(CONFIG_DIR."/".CONFIG_FILE) && is_writeable(CONFIG_DIR));  // There is non, but can we create a file there ?
    $M = FALSE;
    $this->is_writeable[] = array("NAME" => $N , "DESC" => $D , "RESULT" => $R , "SOLUTION" => $S , "MUST" => $M );

  }
 

  function save_object()
  {
     /* If everything is fine, set this step to completed
     *  and allow switching to next setup step
     */
    $failed = false;
    foreach(array("basic_checks","config_checks","is_writeable") as $type){
      foreach($this->$type as $obj){

        if($obj['MUST'] && !$obj['RESULT']){
          $failed = true;
          break;
        }
      }
    }
    $this->is_completed = !$failed;
  }
 

  /* Check if current PHP version is compatible 
      with the current version of FusionDirectory */
  function check_php_version()
  {
    return(version_compare(phpversion(),"5.2.0",">=")); 
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
