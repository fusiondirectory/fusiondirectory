<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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
 * \file class_config.inc
 *  Source code for the class config
 */

/*!
 * \brief This class is responsible for parsing and querying the
 * fusiondirectory configuration file.
 */

class config  {

  /* XML parser */
  var $parser;
  var $config_found     = FALSE;
  var $tags             = array();
  var $level            = 0;
  var $gpc              = 0;
  var $section          = "";
  var $currentLocation  = "";

  /*!
   * \brief Store configuration for current location
   */
  var $current = array();

  /* Link to LDAP-server */
  var $ldap       = NULL;
  var $referrals  = array();

  /*
   * \brief Configuration data
   *
   * - $data['SERVERS'] contains server informations.
   */
  var $data = array(
    'TABS'      => array(),
    'LOCATIONS' => array(),
    'SERVERS'   => array(),
    'MAIN'      => array(),
    'MENU'      => array()
  );
  var $basedir        = "";
  var $config_version = "NOT SET";

  /* Keep a copy of the current department list */
  var $departments      = array();
  var $idepartments     = array();
  var $department_info  = array();
  var $filename         = "";
  var $last_modified    = 0;


  /*!
   * \brief Class constructor of the config class
   *
   * \param string $filename path to the configuration file
   *
   * \param string $basedir base directory
   */
  function __construct($filename, $basedir = "")
  {
    $this->parser   = xml_parser_create();
    $this->basedir  = $basedir;

    xml_set_object($this->parser, $this);
    xml_set_element_handler($this->parser, "tag_open", "tag_close");

    /* Parse config file directly? */
    if ($filename != "") {
      $this->parse($filename);
    }
  }


  /*!
   * \brief Check and reload the configuration
   *
   * This function checks if the configuration has changed, since it was
   * read the last time and reloads it. It uses the file mtime to check
   * weither the file changed or not.
   */
  function check_and_reload($force = FALSE)
  {
    /* Check if class_location.inc has changed, this is the case
        if we have installed or removed plugins. */
    $tmp = stat(CACHE_DIR."/".CLASS_CACHE);
    if (session::global_is_set("class_location.inc:timestamp")) {
      if ($tmp['mtime'] != session::global_get("class_location.inc:timestamp")) {
        session::global_un_set("plist");
      }
    }
    session::global_set("class_location.inc:timestamp", $tmp['mtime']);

    if (($this->filename != "") && ((filemtime($this->filename) != $this->last_modified) || $force)) {

      $this->config_found     = FALSE;
      $this->tags             = array();
      $this->level            = 0;
      $this->gpc              = 0;
      $this->section          = "";
      $this->currentLocation  = "";

      $this->parser = xml_parser_create();
      xml_set_object($this->parser, $this);
      xml_set_element_handler($this->parser, "tag_open", "tag_close");
      $this->parse($this->filename);
      $this->set_current($this->current['NAME']);
    }
  }


  /*!
   * \brief Parse the given configuration file
   *
   * Parses the configuration file and displays errors if there
   * is something wrong with it.
   *
   * \param string $filename The filename of the configuration file.
   */
  function parse($filename)
  {
    $this->data = array(
      "TABS"      => array(),
      "LOCATIONS" => array(),
      "MAIN"      => array(),
      "MENU"      => array()
    );

    $this->last_modified  = filemtime($filename);
    $this->filename       = $filename;
    $fh       = fopen($filename, "r");
    $xmldata  = fread($fh, 100000);
    fclose($fh);
    if (!xml_parse($this->parser, chop($xmldata))) {
      $msg = sprintf(_("XML error in fusiondirectory.conf: %s at line %d"),
            xml_error_string(xml_get_error_code($this->parser)),
            xml_get_current_line_number($this->parser));
      msg_dialog::display(_("Configuration error"), $msg, FATAL_ERROR_DIALOG);
      exit;
    }
  }


  /*!
   * \brief Open xml tag when parsing the xml config
   *
   * \param string $parser
   *
   * \param string $tag
   *
   * \param string $attrs
   */
  function tag_open($parser, $tag, $attrs)
  {
    /* Save last and current tag for reference */
    $this->tags[$this->level] = $tag;
    $this->level++;

    /* Trigger on CONF section */
    if ($tag == 'CONF') {
      $this->config_found = TRUE;
      if (isset($attrs['CONFIGVERSION'])) {
        $this->config_version = $attrs['CONFIGVERSION'];
      }
    }

    /* Return if we're not in config section */
    if (!$this->config_found) {
      return;
    }

    /* yes/no to true/false and upper case TRUE to true and so on*/
    foreach ($attrs as $name => $value) {
      if (preg_match("/^(true|yes)$/i", $value)) {
        $attrs[$name] = "TRUE";
      } elseif (preg_match("/^(false|no)$/i", $value)) {
        $attrs[$name] = "FALSE";
      }
    }

    /* Look through attributes */
    switch ($this->tags[$this->level - 1]) {

      /* Handle tab section */
      case 'TAB':
        $name = $this->tags[$this->level - 2];

        /* Create new array? */
        if (!isset($this->data['TABS'][$name])) {
          $this->data['TABS'][$name] = array();
        }

        /* Add elements */
        $this->data['TABS'][$name][] = $attrs;
        break;

      /* Handle location */
      case 'LOCATION':
        if ($this->tags[$this->level - 2] == 'MAIN') {
          $name           = $attrs['NAME'];
          $name           = preg_replace("/[<>\"']/", "", $name);
          $attrs['NAME']  = $name;

          $this->currentLocation = $name;

          /* Add location elements */
          $this->data['LOCATIONS'][$name] = $attrs;
        }
        break;

      /* Handle referral tags */
      case 'REFERRAL':
        if ($this->tags[$this->level - 2] == 'LOCATION') {
          $url    = $attrs['URI'];
          $server = preg_replace('!^([^:]+://[^/]+)/.*$!', '\\1', $url);

          /* Add location elements */
          if (!isset($this->data['LOCATIONS'][$this->currentLocation]['REFERRAL'])) {
            $this->data['LOCATIONS'][$this->currentLocation]['REFERRAL'] = array();
          }

          $this->data['LOCATIONS'][$this->currentLocation]['REFERRAL'][$server] = $attrs;
        }
        break;

      /* Load main parameters */
      case 'MAIN':
        $this->data['MAIN'] = array_merge ($this->data['MAIN'], $attrs);
        break;
    }
  }


  /*!
   * \brief Close xml tag when parsing the xml config
   *
   * \param string $parser
   *
   * \param string $tag
   */
  function tag_close($parser, $tag)
  {
    /* Close config section */
    if ($tag == 'CONF') {
      $this->config_found = FALSE;
    }
    $this->level--;
  }


  /*!
   * \brief Get the password when needed from the config file
   *
   * This function can be used to get the password associated to
   * a keyword in the config file
   *
   * \param string $creds the keyword associated to the password needed
   *
   * \return string the password corresponding to the keyword
   */
  function get_credentials($creds)
  {
    if (isset($_SERVER['HTTP_FDKEY'])) {
      if (!session::global_is_set('HTTP_FDKEY_CACHE')) {
        session::global_set('HTTP_FDKEY_CACHE', array());
      }
      $cache = session::global_get('HTTP_FDKEY_CACHE');
      if (!isset($cache[$creds])) {
        try {
          $cache[$creds] = cred_decrypt($creds, $_SERVER['HTTP_FDKEY']);
          session::global_set('HTTP_FDKEY_CACHE', $cache);
        } catch (Exception $e) {
          $msg = sprintf(
            _('It seems you are trying to decode something which is not encoded : %s<br/>\n'.
              'Please check you are not using a fusiondirectory.secrets file while your passwords are not encrypted.'),
            $e->getMessage()
          );
          msg_dialog::display(_("Configuration error"), $msg, FATAL_ERROR_DIALOG);
        }
      }
      return $cache[$creds];
    }
    return $creds;
  }


  /*!
   * \brief Get a LDAP link object
   *
   * This function can be used to get an ldap object, which in turn can
   * be used to query the LDAP. See the LDAP class for more information
   * on how to use it.
   *
   * Example usage:
   * \code
   * $ldap = $this->config->get_ldap_link();
   * \endcode
   *
   * \param boolean $sizelimit Weither to impose a sizelimit on the LDAP object or not.
   * Defaults to false. If set to true, the size limit in the configuration
   * file will be used to set the option LDAP_OPT_SIZELIMIT.
   *
   * \return ldapMultiplexer object
   */
  function get_ldap_link($sizelimit = FALSE)
  {
    if ($this->ldap === NULL || !is_resource($this->ldap->cid)) {

      /* Build new connection */
      $this->ldap = ldap_init ($this->current['SERVER'], $this->current['BASE'],
          $this->current['ADMINDN'], $this->get_credentials($this->current['ADMINPASSWORD']));

      /* Check for connection */
      if (is_null($this->ldap) || (is_int($this->ldap) && $this->ldap == 0)) {
        msg_dialog::display(_("LDAP error"), _("Cannot bind to LDAP. Please contact the system administrator."), FATAL_ERROR_DIALOG);
        exit();
      }

      /* Move referrals */
      if (!isset($this->current['REFERRAL'])) {
        $this->ldap->referrals = array();
      } else {
        $this->ldap->referrals = $this->current['REFERRAL'];
      }

      if (!session::global_is_set('size_limit')) {
        session::global_set('size_limit',   $this->current['LDAPSIZELIMIT']);
        session::global_set('size_ignore',  preg_match('/true/i', $this->current['LDAPSIZEIGNORE']));
      }
    }

    $obj  = new ldapMultiplexer($this->ldap);
    if ($sizelimit) {
      $obj->set_size_limit(session::global_get('size_limit'));
    } else {
      $obj->set_size_limit(0);
    }
    return $obj;
  }


  /*!
   * \brief Set the current location
   *
   * \param string $name the name of the location
   */
  function set_current($name)
  {
    $this->current = $this->data['LOCATIONS'][$name];

    if (isset($this->current['INITIAL_BASE'])) {
      session::global_set('CurrentMainBase', $this->current['INITIAL_BASE']);
    }

    /* Sort referrals, if present */
    if (isset($this->current['REFERRAL'])) {
      $bases    = array();
      $servers  = array();
      foreach ($this->current['REFERRAL'] as $ref) {
        $server = preg_replace('%^(.*://[^/]+)/.*$%', '\\1', $ref['URI']);
        $base   = preg_replace('%^.*://[^/]+/(.*)$%', '\\1', $ref['URI']);

        $bases[$base]   = strlen($base);
        $servers[$base] = $server;
      }
      asort($bases);
      reset($bases);
    }

    /* SERVER not defined? Load the one with the shortest base */
    if (!isset($this->current['SERVER'])) {
      $this->current['SERVER'] = $servers[key($bases)];
    }

    /* BASE not defined? Load the one with the shortest base */
    if (!isset($this->current['BASE'])) {
      $this->current['BASE'] = key($bases);
    }

    /* Convert BASE to have escaped special characters */
    $this->current['BASE'] = @LDAP::convert($this->current['BASE']);

    /* Parse LDAP referral informations */
    if (!isset($this->current['ADMINDN']) || !isset($this->current['ADMINPASSWORD'])) {
      $url      = $this->current['SERVER'];
      $referral = $this->current['REFERRAL'][$url];

      $this->current['ADMINDN']       = $referral['ADMINDN'];
      $this->current['ADMINPASSWORD'] = $referral['ADMINPASSWORD'];
    }

    /* We need LDAPSIZELIMIT and LDAPSIZEIGNORE set before we connect to the ldap */
    if (!isset($this->current['LDAPSIZELIMIT'])) {
      $this->current['LDAPSIZELIMIT'] = 200;
    }
    if (!isset($this->current['LDAPSIZEIGNORE'])) {
      $this->current['LDAPSIZEIGNORE'] = "TRUE";
    }

    /* Load in-ldap configuration */
    $this->load_inldap_config();

    /* Remove possibly added ',' from end of group and people ou */
    $this->current['GROUPRDN']  = preg_replace("/,*$/", "", $this->current['GROUPRDN']);
    $this->current['USERRDN']   = preg_replace("/,*$/", "", $this->current['USERRDN']);

    /* We update LDAPSIZELIMIT as it may have been changed by ldap config */
    session::global_set('size_limit', $this->current['LDAPSIZELIMIT']);

    if (class_available('systemManagement')) {
      /* Load server informations */
      $this->load_servers();
    }

    $debugLevel = $this->get_cfg_value('DEBUGLEVEL');
    if ($debugLevel & DEBUG_CONFIG) { // value from LDAP can't activate DEBUG_CONFIG
      $debugLevel -= DEBUG_CONFIG;
    }
    if (isset($this->data['MAIN']['DEBUGLEVEL'])) {
      $debugLevel |= $this->data['MAIN']['DEBUGLEVEL'];
    }
    session::global_set('DEBUGLEVEL', $debugLevel);

    IconTheme::loadThemes('themes');

    timezone::setDefaultTimezoneFromConfig();
  }


  /*!
   * \brief Load server information from config/LDAP
   *
   * This function searches the LDAP for servers (e.g. goImapServer, goMailServer etc.)
   * and stores information about them $this->data['SERVERS']. In the case of mailservers
   * the main section of the configuration file is searched, too.
   */
  function load_servers ()
  {
    /* Only perform actions if current is set */
    if ($this->current === NULL) {
      return;
    }

    /* Fill imap servers */
    $ldap = $this->get_ldap_link();
    $ldap->cd ($this->current['BASE']);

    /* Search mailMethod configuration in main section too */
    $this->current['MAILMETHOD']    = $this->get_cfg_value("mailMethod", "");
    $mailMethodClass                = 'mailMethod'.$this->current['MAILMETHOD'];
    if (class_available($mailMethodClass)) {
      $this->data['SERVERS']['IMAP']  = $mailMethodClass::get_server_list($this);
    } elseif (class_available('mailMethod')) {
      msg_dialog::display(
        _("Error"),
        sprintf(_("The selected mail method (class %s) is not available"), $mailMethodClass),
        ERROR_DIALOG
      );
    }

    /* Get kerberos servers */
    $ldap->cd($this->current['BASE']);
    $ldap->search("(objectClass=goKrbServer)(goKrbRealm=*)(goKrbAdmin=*))");
    $this->data['SERVERS']['KERBEROS'] = array();
    while ($attrs = $ldap->fetch()) {
      $this->data['SERVERS']['KERBEROS'][] = array(
        'SERVER'  => $attrs['cn'][0],
        'REALM'   => $attrs['goKrbRealm'][0],
        'ADMIN'   => $attrs['goKrbAdmin'][0]
      );
    }

    /* Get cups servers */
    $ldap->cd($this->current['BASE']);
    $ldap->search("(objectClass=goCupsServer)");
    $this->data['SERVERS']['CUPS'] = array();
    while ($attrs = $ldap->fetch()) {
      $this->data['SERVERS']['CUPS'][] = $attrs['cn'][0];
    }

    /* Get fax servers */
    $ldap->cd($this->current['BASE']);
    $ldap->search("(objectClass=goFaxServer)");
    $this->data['SERVERS']['FAX'] = array();
    while ($attrs = $ldap->fetch()) {
      $this->data['SERVERS']['FAX'][] = array(
        'SERVER'    => $attrs['cn'][0],
        'LOGIN'     => $attrs['goFaxAdmin'][0],
        'PASSWORD'  => $attrs['goFaxPassword'][0]
      );
    }

    /* Get asterisk servers */
    $ldap->cd($this->current['BASE']);
    $ldap->search("(objectClass=goFonServer)");
    $this->data['SERVERS']['FON'] = array();
    while ($attrs = $ldap->fetch()) {

      $entry = array(
        'DN'                  => $attrs['dn'],
        'SERVER'              => $attrs['cn'][0],
        'LOGIN'               => $attrs['goFonAdmin'][0],
        'PASSWORD'            => $attrs['goFonPassword'][0],
        'DB'                  => "gophone",
        'SIP_TABLE'           => "sip_users",
        'EXT_TABLE'           => "extensions",
        'VOICE_TABLE'         => "voicemail_users",
        'QUEUE_TABLE'         => "queues",
        'QUEUE_MEMBER_TABLE'  => "queue_members"
      );
      if (isset($attrs['fdAsteriskDriver'][0])) {
        $entry['DRIVER'] = $attrs['fdAsteriskDriver'][0];
      }

      /* Add 0 entry for development */
      if (count($this->data['SERVERS']['FON']) == 0) {
        $this->data['SERVERS']['FON'][0] = $entry;
      }

      /* Add entry with 'dn' as index */
      $this->data['SERVERS']['FON'][$attrs['dn']] = $entry;
    }

    /* Get logdb server */
    $ldap->cd($this->current['BASE']);
    $ldap->search("(objectClass=goLogDBServer)");
    if ($ldap->count()) {
      $attrs = $ldap->fetch();
      if (!isset($attrs['gosaLogDB'][0])) {
        $attrs['gosaLogDB'][0] = "gomon";
      }
      $this->data['SERVERS']['LOG'] = array(
        'SERVER'    => $attrs['cn'][0],
        'LOGIN'     => $attrs['goLogAdmin'][0],
        'DB'        => $attrs['gosaLogDB'][0],
        'PASSWORD'  => $attrs['goLogPassword'][0]
      );
    }

    /* Get NFS server lists */
    $tmp  = array("default");
    $tmp2 = array("default");
    $ldap->cd($this->current['BASE']);
    $ldap->search("(&(objectClass=goShareServer)(goExportEntry=*))");
    while ($attrs = $ldap->fetch()) {
      for ($i = 0; $i < $attrs["goExportEntry"]["count"]; $i++) {
        if (preg_match('/^[^|]+\|[^|]+\|NFS\|.*$/', $attrs["goExportEntry"][$i])) {
          $path   = preg_replace ("/^[^|]+\|[^|]+\|[^|]+\|[^|]+\|([^|]+).*$/", '\1', $attrs["goExportEntry"][$i]);
          $tmp[]  = $attrs["cn"][0].":$path";
        }
        if (preg_match('/^[^|]+\|[^|]+\|NBD\|.*$/', $attrs["goExportEntry"][$i])) {
          $path   = preg_replace ("/^[^|]+\|[^|]+\|[^|]+\|[^|]+\|([^|]+).*$/", '\1', $attrs["goExportEntry"][$i]);
          $tmp2[] = $attrs["cn"][0].":$path";
        }
      }
    }
    $this->data['SERVERS']['NFS'] = $tmp;
    $this->data['SERVERS']['NBD'] = $tmp2;

    /* Load Terminalservers */
    $ldap->cd ($this->current['BASE']);
    $ldap->search ("(objectClass=goTerminalServer)", array("cn","gotoSessionType"));
    $this->data['SERVERS']['TERMINAL']                = array();
    $this->data['SERVERS']['TERMINAL'][]              = "default";
    $this->data['SERVERS']['TERMINAL_SESSION_TYPES']  = array();

    while ($attrs = $ldap->fetch()) {
      $this->data['SERVERS']['TERMINAL'][] = $attrs["cn"][0];
      if (isset( $attrs["gotoSessionType"]['count'])) {
        for ($i = 0; $i < $attrs["gotoSessionType"]['count']; $i++) {
          $this->data['SERVERS']['TERMINAL_SESSION_TYPES'][$attrs["cn"][0]][] = $attrs["gotoSessionType"][$i];
        }
      }
    }

    /* Ldap Server
     */
    $this->data['SERVERS']['LDAP'] = array();
    $ldap->cd($this->current['BASE']);
    $ldap->search("(&(objectClass=goLdapServer)(goLdapBase=*))");
    while ($attrs = $ldap->fetch()) {
      $this->data['SERVERS']['LDAP'][$attrs['dn']] = $attrs;
    }

    /* Get misc server lists */
    $this->data['SERVERS']['SYSLOG']  = array("default");
    $this->data['SERVERS']['NTP']     = array("default");
    $ldap->cd($this->current['BASE']);
    $ldap->search("(objectClass=goNtpServer)");
    while ($attrs = $ldap->fetch()) {
      $this->data['SERVERS']['NTP'][] = $attrs["cn"][0];
    }
    $ldap->cd($this->current['BASE']);
    $ldap->search("(objectClass=goSyslogServer)");
    while ($attrs = $ldap->fetch()) {
      $this->data['SERVERS']['SYSLOG'][] = $attrs["cn"][0];
    }

    /* Get samba servers from LDAP */
    $this->data['SERVERS']['SAMBA'] = array();
    if (class_available('sambaAccount')) {
      $ldap->cd($this->current['BASE']);
      $ldap->search("(objectClass=sambaDomain)");
      while ($attrs = $ldap->fetch()) {
        $this->data['SERVERS']['SAMBA'][$attrs['sambaDomainName'][0]] = array( "SID" => "","RIDBASE" => "");
        if (isset($attrs["sambaSID"][0])) {
          $this->data['SERVERS']['SAMBA'][$attrs['sambaDomainName'][0]]["SID"] = $attrs["sambaSID"][0];
        }
        if (isset($attrs["sambaAlgorithmicRidBase"][0])) {
          $this->data['SERVERS']['SAMBA'][$attrs['sambaDomainName'][0]]["RIDBASE"] = $attrs["sambaAlgorithmicRidBase"][0];
        }
      }

      /* If no samba servers are found, look for configured sid/ridbase */
      if (count($this->data['SERVERS']['SAMBA']) == 0) {
        if (!isset($this->current["SAMBASID"]) || !isset($this->current["SAMBARIDBASE"])) {
          msg_dialog::display(_("Configuration error"), _("sambaSID and/or sambaRidBase missing in the configuration!"), ERROR_DIALOG);
        } else {
          $this->data['SERVERS']['SAMBA']['DEFAULT'] = array(
              "SID" => $this->current["SAMBASID"],
              "RIDBASE" => $this->current["SAMBARIDBASE"]);
        }
      }
    }

  }

  /* Check that configuration is in LDAP, check that no plugin got installed since last configuration update */
  function checkLdapConfig ()
  {
    global $ui;
    $ldap   = $this->get_ldap_link();
    $dn     = CONFIGRDN.$this->current['BASE'];
    $reload = TRUE;
    $ldap->cat($dn, array('fusionConfigMd5'));
    if ($attrs = $ldap->fetch()) {
      if (isset($attrs['fusionConfigMd5'][0]) && ($attrs['fusionConfigMd5'][0] == md5_file(CACHE_DIR."/".CLASS_CACHE))) {
        $reload = FALSE;
      }
    }
    if ($reload) {
      add_lock($dn, $ui->dn);
      $config_plugin = new tabs_configInLdap($this, $this->data['TABS']['CONFIGURATIONTABS'], $dn, 'configuration');
      $config_plugin->save_object();
      $config_plugin->save();
      del_lock($dn);
    }
  }

  function load_inldap_config()
  {
    $ldap = $this->get_ldap_link();
    $ldap->cat(CONFIGRDN.$this->current['BASE']);
    if ($attrs = $ldap->fetch()) {
      for ($i = 0; $i < $attrs['count']; $i++) {
        $key = $attrs[$i];
        if (preg_match('/^fdTabHook$/i', $key)) {
          for ($j = 0; $j < $attrs[$key]['count']; ++$j) {
            $parts  = explode('|', $attrs[$key][$j], 3);
            $class  = strtoupper($parts[0]);
            $mode   = strtoupper($parts[1]);
            $cmd    = $parts[2];
            if (!isset($this->data['HOOKS'][$class])) {
              $this->data['HOOKS'][$class] = array('CLASS' => $parts[0]);
            }
            $this->data['HOOKS'][$class][$mode] = $cmd;
          }
        } elseif (preg_match('/^fd/', $key)) {
          if (isset($attrs[$key]['count']) && ($attrs[$key]['count'] > 1)) {
            $value = $attrs[$key];
            unset($value['count']);
          } else {
            $value = $attrs[$key][0];
          }
          $key = strtoupper(preg_replace('/^fd/', '', $key));
          $this->current[$key] = $value;
        }
      }
    }
  }


  /*!
   * \brief Store the departments from ldap in $this->departments
   *
   * \param string $ignore_dn dn of the department
   */
  function get_departments($ignore_dn = "")
  {
    /* Initialize result hash */
    $result = array();

    $result['/'] = $this->current['BASE'];

    /* Get all department types from department Management, to be able detect the department type.
        -It is possible that different department types have the same name,
         in this case we have to mark the department name to be able to differentiate.
          (e.g l=Name  or   o=Name)
     */
    $types = departmentManagement::getDepartmentTypes();

    /* Create a list of attributes to fetch */
    $filter       = "";
    $ldap_values  = array("objectClass", "description");
    foreach ($types as $type) {
      $i = objects::infos($type);
      $filter         .= $i['filter'];
      $ldap_values[]  = $i['mainAttr']; // Specific key for departement objectTypes
    }
    $filter = "(|".$filter.")";

    /* Get list of department objects */
    $ldap = $this->get_ldap_link();
    $ldap->cd ($this->current['BASE']);
    $ldap->search ($filter, $ldap_values);
    while ($attrs = $ldap->fetch()) {

      /* Detect department type */
      $oc = NULL;
      foreach ($types as $type) {
        if (objects::isOfType($attrs, $type)) {
          $oc = $type;
          break;
        }
      }

      /* Unknown department type -> skip */
      if ($oc == NULL) {
        continue;
      }

      $dn   = $ldap->getDN();
      $data = objects::infos($oc);
      $this->department_info[$dn] = array("img"         => $data['icon'],
                                         "description"  => isset($attrs['description'][0])?$attrs['description'][0]:"",
                                         "name"         => $attrs[$data['mainAttr']][0]);

      if ($dn == $ignore_dn) {
        continue;
      }
      $c_dn = convert_department_dn($dn)." (".$data['mainAttr'].")";

      /* Only assign non-root departments */
      if ($dn != $result['/']) {
        $result[$c_dn] = $dn;
      }
    }

    $this->departments = $result;
  }

  function make_idepartments($max_size = 28)
  {
    $base   = $this->current['BASE'];
    $qbase  = preg_quote($base, '/');

    $arr  = array();

    $this->idepartments = array();

    /* Create multidimensional array, with all departments. */
    foreach ($this->departments as $key => $val) {

      /* Split dn into single department pieces */
      $elements = array_reverse(explode(',', preg_replace("/$qbase$/", '', $val)));

      /* Add last ou element of current dn to our array */
      $last = &$arr;
      foreach ($elements as $key => $ele) {

        /* skip empty */
        if (empty($ele)) {
          continue;
        }

        /* Extract department name */
        $elestr = trim(preg_replace('/^[^=]*+=/', '', $ele), ',');
        $nameA  = trim(preg_replace('/=.*$/', '', $ele), ',');
        if ($nameA != 'ou') {
          $nameA = " ($nameA)";
        } else {
          $nameA = '';
        }

        /* Add to array */
        if ($key == (count($elements) - 1)) {
          $last[$elestr.$nameA]['ENTRY'] = $val;
        }

        /* Set next array appending position */
        $last = &$last[$elestr.$nameA]['SUB'];
      }
    }

    /* Add base entry */
    $ret['/']['ENTRY']  = $base;
    $ret['/']['SUB']    = $arr;
    $this->idepartments = $this->generateDepartmentArray($ret, -1, $max_size);
  }


  /*
   * \brief Creates display friendly output from make_idepartments
   *
   * \param $arr arr
   *
   * \param int $depth initialized at -1
   *
   * \param int $max_size initialized at 256
   */
  function generateDepartmentArray($arr, $depth = -1, $max_size = 256)
  {
    $ret = array();
    $depth++;

    /* Walk through array */
    ksort($arr);
    foreach ($arr as $name => $entries) {

      /* If this department is the last in the current tree position
       * remove it, to avoid generating output for it */
      if (count($entries['SUB']) == 0) {
        unset($entries['SUB']);
      }

      /* Fix name, if it contains a replace tag */
      $name = preg_replace('/\\\\,/', ',', LDAP::fix($name));

      /* Check if current name is too long, then cut it */
      if (mb_strlen($name, 'UTF-8') > $max_size) {
        $name = mb_substr($name, 0, ($max_size - 3), 'UTF-8')." ...";
      }

      /* Append the name to the list */
      if (isset($entries['ENTRY'])) {
        $a = "";
        for ($i = 0; $i < $depth; $i++) {
          $a .= ".";
        }
        $ret[$entries['ENTRY']] = $a."&nbsp;".$name;
      }

      /* recursive add of subdepartments */
      if (isset($entries['SUB'])) {
        $ret = array_merge($ret, $this->generateDepartmentArray($entries['SUB'], $depth, $max_size));
      }
    }

    return $ret;
  }


  /*!
   * \brief Get all available shares defined in the current LDAP
   *
   * This function returns all available Shares defined in this ldap
   *
   * \param boolean $listboxEntry If set to TRUE, only name and path are
   *  attached to the array. If FALSE, the whole entry will be parsed an atached to the result.
   *
   * \return array list of share defined in the current LDAP
   */
  function getShareList($listboxEntry = FALSE)
  {
    $tmp    = get_sub_list("(&(objectClass=goShareServer)(goExportEntry=*))", "server", get_ou("serverRDN"),
                $this->current['BASE'], array("goExportEntry","cn"), GL_NONE);
    $return = array();
    foreach ($tmp as $entry) {

      if (isset($entry['goExportEntry']['count'])) {
        unset($entry['goExportEntry']['count']);
      }
      if (isset($entry['goExportEntry'])) {
        foreach ($entry['goExportEntry'] as $export) {
          $shareAttrs = explode("|", $export);
          if ($listboxEntry) {
            $return[$shareAttrs[0]."|".$entry['cn'][0]] = $shareAttrs[0]." - ".$entry['cn'][0];
          } else {
            $return[$shareAttrs[0]."|".$entry['cn'][0]]['server']       = $entry['cn'][0];
            $return[$shareAttrs[0]."|".$entry['cn'][0]]['name']         = $shareAttrs[0];
            $return[$shareAttrs[0]."|".$entry['cn'][0]]['description']  = $shareAttrs[1];
            $return[$shareAttrs[0]."|".$entry['cn'][0]]['type']         = $shareAttrs[2];
            $return[$shareAttrs[0]."|".$entry['cn'][0]]['charset']      = $shareAttrs[3];
            $return[$shareAttrs[0]."|".$entry['cn'][0]]['path']         = $shareAttrs[4];
            $return[$shareAttrs[0]."|".$entry['cn'][0]]['option']       = $shareAttrs[5];
          }
        }
      }
    }
    return $return;
  }


  /*!
   * \brief Return all available share servers in LDAP
   *
   * This function returns all available ShareServers.
   *
   * \return array list of share defined in the current LDAP
   */
  function getShareServerList()
  {
    $return = array();
    $ui     = get_userinfo();
    $base   = $this->current['BASE'];
    $res    = get_sub_list("(&(objectClass=goShareServer)(goExportEntry=*))", "server",
              get_ou("serverRDN"), $base, array("goExportEntry","cn"), GL_NONE | GL_NO_ACL_CHECK);

    foreach ($res as $entry) {
      $acl = $ui->get_permissions($entry['dn'], "server", "");
      if (isset($entry['goExportEntry']['count'])) {
        unset($entry['goExportEntry']['count']);
      }
      foreach ($entry['goExportEntry'] as $share) {
        $a_share    = explode("|", $share);
        $sharename  = $a_share[0];
        $data       = array();

        $data['NAME']     = $sharename;
        $data['ACL']      = $acl;
        $data['SERVER']   = $entry['cn']['0'];
        $data['SHARE']    = $sharename;
        $data['DISPLAY']  = $entry['cn'][0]." [".$sharename."]";

        $return[$entry['cn'][0]."|".$sharename] = $data;
      }
    }

    return $return;
  }


  /*!
   * \brief Check if there's the specified bool value set in the configuration
   *
   *  The function checks, weither the specified bool value is set to a true
   *  value in the configuration file. Considered true are either true or yes,
   *  case-insensitive.
   *
   *  Example usage:
   *  \code
   *  if ($this->config->boolValueIsTrue("main", "copyPaste")) {
   *    echo "Copy Paste Handling is enabled";
   *  }
   *  \endcode
   *
   *  \param string $section Section in the configuration file.
   *
   *  \param string $value Key in the given section, which is subject to check
   *
   *  \return bool TRUE if option set in the config file
   */
  function boolValueIsTrue($section, $value)
  {
    $section  = strtoupper($section);
    $value    = strtoupper($value);
    if (isset($this->data[$section][$value])) {

      $data = $this->data[$section][$value];
      if (preg_match("/^true$/i", $data) || preg_match("/yes/i", $data)) {
        return TRUE;
      }

    }

    return FALSE;
  }


  function __search(&$arr, $name, $return)
  {
    $return = strtoupper($return);
    if (is_array($arr)) {
      foreach ($arr as &$a) {
        if (isset($a['CLASS']) && strcasecmp($name, $a['CLASS']) == 0) {
          return (isset($a[$return])?$a[$return]:"");
        } else {
          $res = $this->__search ($a, $name, $return);
          if ($res != "") {
            return $res;
          }
        }
      }
      unset($a);
    }
    return "";
  }


  /*!
   * \brief Search for a configuration setting in different categories
   *
   *  Searches for the value of a given key in the configuration data.
   *  Optionally the list of categories to search (tabs, main, locations) can
   *  be specified. The first value that matches is returned.
   *
   *  Example usage:
   *  \code
   *  $postcmd = $this->config->search(get_class($this), "POSTCOMMAND", array("menu", "tabs"));
   *  \endcode
   *
   *  \param string $class The class name
   *
   *  \param string $value Key to search in the configuration data
   *
   *  \param array $categories Array where the search should run
   *
   *  \return string the value you asked for or empty
   */
  function search($class, $value, $categories = "")
  {
    if (is_array($categories)) {
      foreach ($categories as $category) {
        $res = $this->__search($this->data[strtoupper($category)], $class, $value);
        if ($res != "") {
          return $res;
        }
      }
    } else {
      if ($categories == "") {
        return $this->__search($this->data, $class, $value);
      } else {
        return $this->__search($this->data[strtoupper($categories)], $class, $value);
      }
    }

    return "";
  }


  /*!
   * \brief Get a configuration value from the config
   *
   *  This returns a configuration value from the config. It either
   *  uses the data of the current location ($this->current),
   *  if it contains the value (e.g. current['BASE']) or otherwise
   *  uses the data from the main configuration section.
   *
   *  If no value is found and an optional default has been specified,
   *  then the default is returned.
   *
   * \param string $name The configuration key (case-insensitive)
   *
   * \param string $default A default that is returned, if no value is found
   *
   * \return string the configuration value if found or the default value
   */
  function get_cfg_value($name, $default = "")
  {
    $name = strtoupper($name);
    $res  = $default;

    /* Check if we have a current value for $name */
    if (isset($this->current[$name])) {
      $res = $this->current[$name];
    } elseif (isset($this->data["MAIN"][$name])) {
      /* Check if we have a global value for $name */
      $res = $this->data["MAIN"][$name];
    }

    if (is_array($default) && !is_array($res)) {
      $res = array($res);
    }

    return $res;
  }


  /*!
   * \brief Check if session lifetime matches session.gc_maxlifetime
   *
   *  On debian systems the session files are deleted with
   *  a cronjob, which detects all files older than specified
   *  in php.ini:'session.gc_maxlifetime' and removes them.
   *  This function checks if the fusiondirectory.conf value matches the range
   *  defined by session.gc_maxlifetime.
   *
   * \return boolean TRUE or FALSE depending on weither the settings match
   *  or not. If SESSIONLIFETIME is not configured in FusionDirectory it always returns
   *  TRUE.
   */
  function check_session_lifetime()
  {
    if (isset($this->data['MAIN']['SESSIONLIFETIME'])) {
      $cfg_lifetime = $this->data['MAIN']['SESSIONLIFETIME'];
      $ini_lifetime = ini_get('session.gc_maxlifetime');
      $deb_system   = file_exists('/etc/debian_version');
      return !($deb_system && ($ini_lifetime < $cfg_lifetime));
    } else {
      return TRUE;
    }
  }


  /*!
   * \brief Check if snapshot are enabled
   *
   * \return boolean TRUE if snapshot are enabled, FALSE otherwise
   */
  function snapshotEnabled()
  {
    if ($this->get_cfg_value("enableSnapshots") == "TRUE") {

      /* Check if the snapshot_base is defined */
      if ($this->get_cfg_value("snapshotBase") == "") {

        /* Send message if not done already */
        if (!session::is_set("snapshotFailMessageSend")) {
          session::set("snapshotFailMessageSend", TRUE);
          msg_dialog::display(_("Configuration error"),
              sprintf(_("The snapshot functionality is enabled, but the required variable '%s' is not set."),
                      "snapshotBase"), ERROR_DIALOG);
        }
        return FALSE;
      }

      /* Check if gzcompress is available */
      if (!is_callable("gzcompress")) {

        /* Send message if not done already */
        if (!session::is_set("snapshotFailMessageSend")) {
          session::set("snapshotFailMessageSend", TRUE);
          msg_dialog::display(_("Configuration error"),
              sprintf(_("The snapshot functionality is enabled, but the required compression module is missing. Please install '%s'."), "php5-zip / php5-gzip"), ERROR_DIALOG);
        }
        return FALSE;
      }
      return TRUE;
    }
    return FALSE;
  }

  function loadPlist ($plist)
  {
    $this->data['OBJECTS']    = array();
    $this->data['SECTIONS']   = array();
    $this->data['CATEGORIES'] = array();
    foreach ($plist->info as $class => &$plInfo) {
      if (isset($plInfo['plObjectType'])) {
        $entry = array('CLASS' => $class,'NAME' => $plInfo['plShortName']);
        foreach ($plInfo['plObjectType'] as $key => $value) {
          if (is_numeric($key)) { // This is not the main tab
            $tabclass = strtoupper($value)."TABS";
            @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $tabclass, "Adding $class to tab list");
            if (!isset($this->data['TABS'][$tabclass])) {
              $this->data['TABS'][$tabclass] = array();
            }
            $this->data['TABS'][$tabclass][] = $entry;
          } else {
            if (isset($this->data['OBJECTS'][strtoupper($key)])) {
              die("duplicated object type ".strtoupper($key)." in ".$this->data['OBJECTS'][strtoupper($key)]['mainTab']." and $class");
            }
            $tabclass = strtoupper($key)."TABS";
            $value['tabGroup']  = $tabclass;
            $value['mainTab']   = $class;
            foreach (array('ou', 'tabClass') as $i) {
              if (!isset($value[$i])) {
                $value[$i] = NULL;
              }
            }
            if (!isset($value['aclCategory'])) {
              $value['aclCategory'] = $key;
            }
            if (isset($value['filter'])) {
              if (!preg_match('/^\(.*\)$/', $value['filter'])) {
                $value['filter'] = '('.$value['filter'].')';
              }
            } else {
              $value['filter'] = NULL;
            }
            if (!isset($value['mainAttr'])) {
              $value['mainAttr'] = 'cn';
            }
            if (!isset($value['nameAttr'])) {
              $value['nameAttr'] = $value['mainAttr'];
            }
            if (!isset($value['tabClass'])) {
              $value['tabClass'] = 'simpleTabs';
            }
            $this->data['OBJECTS'][strtoupper($key)] = $value;
            @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $tabclass, "Adding $class as main tab of");
            if (!isset($this->data['TABS'][$tabclass])) {
              $this->data['TABS'][$tabclass] = array();
            }
            array_unshift($this->data['TABS'][$tabclass], $entry);
          }

        }
      } elseif (class_available($class) && is_subclass_of($class, 'simpleService')) {
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $class, "Adding service");
        if (!isset($this->data['TABS']['SERVERSERVICE'])) {
          $this->data['TABS']['SERVERSERVICE'] = array();
        }
        $this->data['TABS']['SERVERSERVICE'][] = array(
          'CLASS' => $class,
          'NAME' => $plInfo['plShortName']
        );
      }
      /* Feed categories */
      if (isset($plInfo['plCategory'])) {
        /* Walk through supplied list and feed only translated categories */
        $cats = array();
        foreach ($plInfo['plCategory'] as $idx => $data) {
          $cat    = (is_numeric($idx)?$data:$idx);
          $cats[] = $cat;
          if (!isset($this->data['CATEGORIES'][$cat])) {
            $this->data['CATEGORIES'][$cat] = array( 'classes' => array('0') );
          }
          if (!empty($plInfo['plProvidedAcls'])) {
            $this->data['CATEGORIES'][$cat]['classes'][] = $class;
          }
          if (!is_numeric($idx)) {
            /* Non numeric index means -> base object containing more informations */
            $this->data['CATEGORIES'][$cat]['description'] = $data['description'];
            if (!is_array($data['objectClass'])) {
              $data['objectClass'] = array($data['objectClass']);
            }
            $this->data['CATEGORIES'][$cat]['objectClass'] = $data['objectClass'];
          }
        }
        $plInfo['plCategory'] = $cats;
      }
    }
    unset($plInfo);
    $this->data['CATEGORIES']['all'] = array(
      'classes'     => array('all'),
      'description' => '*&nbsp;'._("All categories"),
      'objectClass' => array(),
    );
    /* Extract categories definitions from object types */
    foreach ($this->data['OBJECTS'] as $key => $infos) {
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $infos['aclCategory'], "ObjectType $key category");
      if (strtoupper($infos['aclCategory']) == $key) {
        $cat = $infos['aclCategory'];
        if (!isset($this->data['CATEGORIES'][$cat])) {
          $this->data['CATEGORIES'][$cat] = array('classes' => array('0'));
        }
        if (!isset($this->data['CATEGORIES'][$cat]['description'])) {
          $this->data['CATEGORIES'][$cat]['description'] = $infos['name'];
          preg_match_all('/objectClass=([^= \)\(]+)/', $infos['filter'], $m);
          $this->data['CATEGORIES'][$cat]['objectClass'] = $m[1];
        }
      }
    }
    /* Now that OBJECTS are filled, place tabs in categories */
    foreach ($plist->info as $class => &$plInfo) {
      $acl = array();
      if (isset($plInfo['plCategory'])) {
        $acl = $plInfo['plCategory'];
        if (!is_array($acl)) {
          $acl = array($acl);
        }
      }
      if (isset($plInfo['plObjectType'])) {
        foreach ($plInfo['plObjectType'] as $key => $value) {
          if (is_numeric($key)) { // This is not the main tab
            $obj = strtoupper($value);
          } else {
            $obj = strtoupper($key);
          }
          /* if this is an existing objectType, not just a tab group */
          if (isset($this->data['OBJECTS'][$obj])) {
            $cat    = $this->data['OBJECTS'][$obj]['aclCategory'];
            $acl[]  = $cat;

            if (!empty($plInfo['plProvidedAcls'])) {
              $this->data['CATEGORIES'][$cat]['classes'][] = $class;
            }
            if (!in_array($cat, $plInfo['plCategory'])) {
              $plInfo['plCategory'][] = $cat;
            }
          }
        }
      }
      /* Read management info */
      if (isset($plInfo['plManages'])) {
        foreach ($plInfo['plManages'] as $type) {
          $obj = strtoupper($type);
          if (!isset($this->data['OBJECTS'][$obj])) {
            continue;
          }
          $cat    = $this->data['OBJECTS'][$obj]['aclCategory'];
          $acl[]  = $cat;

          if (!empty($plInfo['plProvidedAcls'])) {
            $this->data['CATEGORIES'][$cat]['classes'][] = $class;
          }
          if (!in_array($cat, $plInfo['plCategory'])) {
            $plInfo['plCategory'][] = $cat;
          }

          if (isset($this->data['OBJECTS'][$obj])) {
            $this->data['OBJECTS'][$obj]['management'] = $class;
          }
        }
      }
      @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, join(',', array_unique($acl)), "Class $class categories");
      /* Feed menu */
      if (isset($plInfo['plSection'])) {
        $section = $plInfo['plSection'];
        if (!is_array($acl)) {
          $acl = array($acl);
        }
        if (!is_numeric(key($acl))) {
          $acl = array_keys($acl);
        }
        if (isset($plInfo['plSelfModify']) && $plInfo['plSelfModify']) {
          $acl[] = $acl[0].'/'.$class.':self';
        }
        $acl = join(',', array_unique($acl));

        if (is_array($section)) {
          $section  = key($section);
          if (is_numeric($section)) {
            trigger_error("$class have wrong setting in plInfo/plSection");
            continue;
          }
          $this->data['SECTIONS'][$section] = array_change_key_case($plInfo['plSection'][$section], CASE_UPPER);
        }
        if (!isset($this->data['MENU'][$section])) {
          $this->data['MENU'][$section] = array();
        }
        $attrs = array('CLASS' => $class);
        if (!empty($acl)) {
          $attrs['ACL'] = $acl;
        }
        $this->data['MENU'][$section][] = $attrs;
      }
      if (isset($plInfo['plMenuProvider']) && $plInfo['plMenuProvider']) {
        list($sections, $entries) = $class::getMenuEntries();
        foreach ($sections as $section => $infos) {
          $this->data['SECTIONS'][$section] = array_change_key_case($infos, CASE_UPPER);
          if (!isset($this->data['MENU'][$section])) {
            $this->data['MENU'][$section] = array();
          }
        }
        foreach ($entries as $section => $section_entries) {
          foreach ($section_entries as $entry) {
            $this->data['MENU'][$section][] = $entry;
          }
        }
      }
    }
    unset($plInfo);
    asort($this->data['CATEGORIES']);
    foreach ($this->data['CATEGORIES'] as $name => &$infos) {
      $infos['classes'] = array_unique($infos['classes']);
      if (!isset($infos['description'])) {
        trigger_error("ACL category '$name' for classes ".join(',', $infos['classes'])." has no definition");
        $infos['description'] = $name;
        $infos['objectClass'] = array();
      }
    }
    unset($infos);
    $this->data['SECTIONS']['personal'] = array('NAME' => _('My account'), 'PRIORITY' => 40);
    $personal = array();
    foreach ($this->data['TABS']['USERTABS'] as $tab) {
      $personal[] = array('CLASS' => $tab['CLASS'], 'ACL' => 'user/'.$tab['CLASS'].':self');
    }
    if (!isset($this->data['MENU']['personal'])) {
      $this->data['MENU']['personal'] = $personal;
    } else {
      $this->data['MENU']['personal'] = array_merge($personal, $this->data['MENU']['personal']);
    }
    uasort($this->data['SECTIONS'],
      function ($a, $b)
      {
        if ($a['PRIORITY'] == $b['PRIORITY']) {
          return 0;
        }
        return (($a['PRIORITY'] < $b['PRIORITY']) ? -1 : 1);
      }
    );
  }

  /*!
   * \brief Get list of object of objectType $type in $ou
   */
  function getObjectList ($type, $attrs = array(), $ou = NULL, $filter = '')
  {
    trigger_error('Deprecated');

    return objects::ls($type, $attrs, $ou, $filter);
  }
}

?>
