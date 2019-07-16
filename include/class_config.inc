<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2017  FusionDirectory

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
class config
{
  /* XML parser */
  var $parser;
  var $config_found     = FALSE;
  var $tags             = [];
  var $level            = 0;
  var $currentLocation  = '';

  /*!
   * \brief Store configuration for current location
   */
  var $current = [];

  /* Link to LDAP-server */
  protected $ldapLink = NULL;
  var $referrals      = [];

  /*
   * \brief Configuration data
   *
   * - $data['SERVERS'] contains server informations.
   */
  var $data = [
    'LOCATIONS' => [],
    'SERVERS'   => [],
    'MAIN'      => [],
  ];
  var $basedir        = '';

  /* Keep a copy of the current department list */
  protected $departmentList;
  protected $departmentTree;
  protected $departmentInfo;

  var $filename         = '';
  var $last_modified    = 0;

  /*!
   * \brief Class constructor of the config class
   *
   * \param string $filename path to the configuration file
   *
   * \param string $basedir base directory
   */
  function __construct ($filename, $basedir = '')
  {
    $this->basedir  = $basedir;

    /* Parse config file directly? */
    if ($filename != '') {
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
  function check_and_reload ($force = FALSE)
  {
    /* Check if class_location.inc has changed, this is the case
        if we have installed or removed plugins. */
    $tmp = stat(CACHE_DIR.'/'.CLASS_CACHE);
    if (session::global_is_set('class_location.inc:timestamp')
      && ($tmp['mtime'] != session::global_get('class_location.inc:timestamp'))) {
      session::global_un_set('plist');
    }
    session::global_set('class_location.inc:timestamp', $tmp['mtime']);

    if (($this->filename != '') && ((filemtime($this->filename) != $this->last_modified) || $force)) {
      $this->config_found     = FALSE;
      $this->tags             = [];
      $this->level            = 0;
      $this->currentLocation  = '';

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
  function parse ($filename)
  {
    $this->last_modified  = filemtime($filename);
    $this->filename       = $filename;
    $fh       = fopen($filename, 'r');
    $xmldata  = fread($fh, 100000);
    fclose($fh);
    $this->parse_data($xmldata);
  }

  function parse_data ($xmldata)
  {
    $this->data = [
      'LOCATIONS' => [],
      'SERVERS'   => [],
      'MAIN'      => [],
    ];

    $this->parser = xml_parser_create();
    xml_set_object($this->parser, $this);
    xml_set_element_handler($this->parser, "tag_open", "tag_close");

    if (!xml_parse($this->parser, chop($xmldata))) {
      $msg = sprintf(_("XML error in fusiondirectory.conf: %s at line %d"),
            xml_error_string(xml_get_error_code($this->parser)),
            xml_get_current_line_number($this->parser));
      msg_dialog::display(_("Configuration error"), $msg, FATAL_ERROR_DIALOG);
      exit;
    }
    xml_parser_free($this->parser);
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
  function tag_open ($parser, $tag, $attrs)
  {
    /* Save last and current tag for reference */
    $this->tags[$this->level] = $tag;
    $this->level++;

    /* Trigger on CONF section */
    if ($tag == 'CONF') {
      $this->config_found = TRUE;
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
      /* Handle location */
      case 'LOCATION':
        if ($this->tags[$this->level - 2] == 'MAIN') {
          $attrs['NAME'] = preg_replace('/[<>"\']/', '', $attrs['NAME']);

          $this->currentLocation = $attrs['NAME'];

          /* Add location elements */
          $this->data['LOCATIONS'][$attrs['NAME']] = $attrs;
        }
        break;

      /* Handle referral tags */
      case 'REFERRAL':
        if ($this->tags[$this->level - 2] == 'LOCATION') {
          if (isset($attrs['BASE'])) {
            $server = $attrs['URI'];
          } elseif (isset($this->data['LOCATIONS'][$this->currentLocation]['BASE'])) {
            /* Fallback on location base */
            $server         = $attrs['URI'];
            $attrs['BASE']  = $this->data['LOCATIONS'][$this->currentLocation]['BASE'];
          } else {
            /* Format from FD<1.3 */
            $server         = preg_replace('!^([^:]+://[^/]+)/.*$!', '\\1', $attrs['URI']);
            $attrs['BASE']  = preg_replace('!^[^:]+://[^/]+/(.*)$!', '\\1', $attrs['URI']);
            $attrs['URI']   = $server;
          }

          /* Add location elements */
          if (!isset($this->data['LOCATIONS'][$this->currentLocation]['REFERRAL'])) {
            $this->data['LOCATIONS'][$this->currentLocation]['REFERRAL'] = [];
          }

          $this->data['LOCATIONS'][$this->currentLocation]['REFERRAL'][$server] = $attrs;
        }
        break;

      /* Load main parameters */
      case 'MAIN':
        $this->data['MAIN'] = array_merge($this->data['MAIN'], $attrs);
        break;

      /* Ignore other tags */
      default:
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
  function tag_close ($parser, $tag)
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
  function get_credentials ($creds)
  {
    if (isset($_SERVER['HTTP_FDKEY'])) {
      if (!session::global_is_set('HTTP_FDKEY_CACHE')) {
        session::global_set('HTTP_FDKEY_CACHE', []);
      }
      $cache = session::global_get('HTTP_FDKEY_CACHE');
      if (!isset($cache[$creds])) {
        try {
          $cache[$creds] = cred_decrypt($creds, $_SERVER['HTTP_FDKEY']);
          session::global_set('HTTP_FDKEY_CACHE', $cache);
        } catch (FusionDirectoryException $e) {
          $msg = sprintf(
            _('It seems you are trying to decode something which is not encoded : %s<br/>'."\n".
              'Please check you are not using a fusiondirectory.secrets file while your passwords are not encrypted.'),
            $e->getMessage()
          );
          msg_dialog::display(_('Configuration error'), $msg, FATAL_ERROR_DIALOG);
          exit;
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
   * $ldap = $config->get_ldap_link();
   * \endcode
   *
   * \param boolean $sizelimit Weither to impose a sizelimit on the LDAP object or not.
   * Defaults to false. If set to true, the size limit in the configuration
   * file will be used to set the option LDAP_OPT_SIZELIMIT.
   *
   * \return ldapMultiplexer object
   */
  function get_ldap_link ($sizelimit = FALSE)
  {
    global $ui;

    if ($this->ldapLink === NULL || !is_resource($this->ldapLink->cid)) {
      /* Build new connection */
      $this->ldapLink = ldap_init($this->current['SERVER'], $this->current['BASE'],
          $this->current['ADMINDN'], $this->get_credentials($this->current['ADMINPASSWORD']));

      /* Check for connection */
      if (is_null($this->ldapLink) || (is_int($this->ldapLink) && $this->ldapLink == 0)) {
        msg_dialog::display(_("LDAP error"), _("Cannot bind to LDAP. Please contact the system administrator."), FATAL_ERROR_DIALOG);
        exit();
      }

      /* Move referrals */
      if (!isset($this->current['REFERRAL'])) {
        $this->ldapLink->referrals = [];
      } else {
        $this->ldapLink->referrals = $this->current['REFERRAL'];
      }
    }

    $obj  = new ldapMultiplexer($this->ldapLink);
    if ($sizelimit) {
      $obj->set_size_limit($ui->getSizeLimitHandler()->getSizeLimit());
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
  function set_current ($name)
  {
    global $ui;

    if (!isset($this->data['LOCATIONS'][$name])) {
      msg_dialog::display(_('Error'), sprintf(_('Location "%s" could not be found in the configuration file'), $name), FATAL_ERROR_DIALOG);
      exit;
    }
    $this->current = $this->data['LOCATIONS'][$name];

    if (isset($this->current['INITIAL_BASE']) && isset($ui)) {
      $ui->setCurrentBase($this->current['INITIAL_BASE']);
    }

    /* Sort referrals, if present */
    if (isset($this->current['REFERRAL'])) {
      $servers  = [];
      foreach ($this->current['REFERRAL'] as $server => $ref) {
        $servers[$server] = strlen($ref['BASE']);
      }
      asort($servers);
      reset($servers);
    }

    /* SERVER not defined? Load the one with the shortest base */
    if (!isset($this->current['SERVER'])) {
      $this->current['SERVER'] = key($servers);
    }

    /* Parse LDAP referral informations */
    if (!isset($this->current['ADMINDN']) || !isset($this->current['ADMINPASSWORD'])) {
      $this->current['BASE']          = $this->current['REFERRAL'][$this->current['SERVER']]['BASE'];
      $this->current['ADMINDN']       = $this->current['REFERRAL'][$this->current['SERVER']]['ADMINDN'];
      $this->current['ADMINPASSWORD'] = $this->current['REFERRAL'][$this->current['SERVER']]['ADMINPASSWORD'];
    }

    /* Load in-ldap configuration */
    $this->load_inldap_config();

    /* Parse management config */
    $this->loadManagementConfig();

    if (class_available('systemManagement')) {
      /* Load server informations */
      $this->load_servers();
    }

    $debugLevel = $this->get_cfg_value('DEBUGLEVEL');
    if ($debugLevel & DEBUG_CONFIG) {
      /* Value from LDAP can't activate DEBUG_CONFIG */
      $debugLevel -= DEBUG_CONFIG;
    }
    if (isset($this->data['MAIN']['DEBUGLEVEL'])) {
      $debugLevel |= $this->data['MAIN']['DEBUGLEVEL'];
    }
    session::global_set('DEBUGLEVEL', $debugLevel);

    IconTheme::loadThemes('themes');

    timezone::setDefaultTimezoneFromConfig();

    Language::init();
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

    $ldap = $this->get_ldap_link();

    /* Get samba servers from LDAP */
    $this->data['SERVERS']['SAMBA'] = [];
    if (class_available('sambaAccount')) {
      $ldap->cd($this->current['BASE']);
      $ldap->search('(objectClass=sambaDomain)');
      while ($attrs = $ldap->fetch()) {
        $this->data['SERVERS']['SAMBA'][$attrs['sambaDomainName'][0]] = [ 'SID' => '','RIDBASE' => ''];
        if (isset($attrs['sambaSID'][0])) {
          $this->data['SERVERS']['SAMBA'][$attrs['sambaDomainName'][0]]['SID'] = $attrs['sambaSID'][0];
        }
        if (isset($attrs['sambaAlgorithmicRidBase'][0])) {
          $this->data['SERVERS']['SAMBA'][$attrs['sambaDomainName'][0]]['RIDBASE'] = $attrs['sambaAlgorithmicRidBase'][0];
        }
      }

      /* If no samba servers are found, look for configured sid/ridbase */
      if ((count($this->data['SERVERS']['SAMBA']) == 0)
        && isset($this->current['SAMBASID']) && isset($this->current['SAMBARIDBASE'])) {
        $this->data['SERVERS']['SAMBA']['DEFAULT'] = [
          'SID'     => $this->get_cfg_value('SAMBASID'),
          'RIDBASE' => $this->get_cfg_value('SAMBARIDBASE')
        ];
      }
    }
  }

  /* Check that configuration is in LDAP, check that no plugin got installed since last configuration update */
  function checkLdapConfig ($forceReload = FALSE)
  {
    global $ui;
    $dn = CONFIGRDN.$this->current['BASE'];

    if (!$forceReload) {
      $ldap = $this->get_ldap_link();
      $ldap->cat($dn, ['fusionConfigMd5']);
      if (($attrs = $ldap->fetch()) && isset($attrs['fusionConfigMd5'][0])
        && ($attrs['fusionConfigMd5'][0] == md5_file(CACHE_DIR.'/'.CLASS_CACHE))) {
        return;
      }
    }

    add_lock($dn, $ui->dn);
    $config_plugin = objects::open($dn, 'configuration');
    $config_plugin->save_object();
    $config_plugin->save();
    del_lock($dn);
  }

  function load_inldap_config ()
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
              $this->data['HOOKS'][$class] = ['CLASS' => $parts[0]];
            }
            if (!isset($this->data['HOOKS'][$class][$mode])) {
              $this->data['HOOKS'][$class][$mode] = [];
            }
            $this->data['HOOKS'][$class][$mode][] = $cmd;
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
   * \brief Loads the management classes config to index them by class
   */
  private function loadManagementConfig ()
  {
    if (isset($this->current['MANAGEMENTCONFIG'])) {
      if (!is_array($this->current['MANAGEMENTCONFIG'])) {
        $this->current['MANAGEMENTCONFIG'] = [$this->current['MANAGEMENTCONFIG']];
      }
      $value = [];
      foreach ($this->current['MANAGEMENTCONFIG'] as $config) {
        list($class, $json) = explode(':', $config, 2);
        $value[$class] = $json;
      }
      $this->current['MANAGEMENTCONFIG'] = $value;
    }
    if (isset($this->current['MANAGEMENTUSERCONFIG'])) {
      if (!is_array($this->current['MANAGEMENTUSERCONFIG'])) {
        $this->current['MANAGEMENTUSERCONFIG'] = [$this->current['MANAGEMENTUSERCONFIG']];
      }
      $value = [];
      foreach ($this->current['MANAGEMENTUSERCONFIG'] as $config) {
        list($user, $class, $json) = explode(':', $config, 3);
        $value[$user][$class] = $json;
      }
      $this->current['MANAGEMENTUSERCONFIG'] = $value;
    }
  }

  /*!
   * \brief Update the management config in the LDAP and the cache
   */
  public function updateManagementConfig (string $managementClass, $managementConfig, bool $userConfig = FALSE): array
  {
    global $ui;

    $changes = [];
    if ($userConfig) {
      if (!isset($this->current['MANAGEMENTUSERCONFIG'][$ui->dn])) {
        $this->current['MANAGEMENTUSERCONFIG'][$ui->dn] = [];
      }
      $currentConfig  =& $this->current['MANAGEMENTUSERCONFIG'][$ui->dn];
      $attrib         = 'fdManagementUserConfig';
      $prefix         = $ui->dn.':'.$managementClass;
    } else {
      if (!isset($this->current['MANAGEMENTCONFIG'])) {
        $this->current['MANAGEMENTCONFIG'] = [];
      }
      $currentConfig  =& $this->current['MANAGEMENTCONFIG'];
      $attrib         = 'fdManagementConfig';
      $prefix         = $managementClass;
    }

    if ($managementConfig !== NULL) {
      $managementConfig = json_encode($managementConfig);
    }

    if (isset($currentConfig[$managementClass])) {
      /* If there already was a config for this class, remove it */
      if ($currentConfig[$managementClass] === $managementConfig) {
        /* Unless it's the same one and we've got nothing to do */
        return [];
      }
      $changes[] = [
        'attrib'  => $attrib,
        'modtype' => LDAP_MODIFY_BATCH_REMOVE,
        'values'  => [$prefix.':'.$currentConfig[$managementClass]],
      ];
    }

    if ($managementConfig !== NULL) {
      /* Add the new one, if any */
      $changes[] = [
        'attrib'  => $attrib,
        'modtype' => LDAP_MODIFY_BATCH_ADD,
        'values'  => [$prefix.':'.$managementConfig],
      ];
    }
    $ldap = $this->get_ldap_link();
    $ldap->cd(CONFIGRDN.$this->current['BASE']);
    if (!$ldap->modify_batch($changes)) {
      return [$ldap->get_error()];
    }

    if ($managementConfig !== NULL) {
      $currentConfig[$managementClass] = $managementConfig;
    } else {
      unset($currentConfig[$managementClass]);
    }

    return [];
  }

  /*!
   * \brief Test if there is a stored management config
   */
  public function hasManagementConfig (string $managementClass, bool $userConfig = FALSE): bool
  {
    global $ui;

    if ($userConfig) {
      return isset($this->current['MANAGEMENTUSERCONFIG'][$ui->dn][$managementClass]);
    } else {
      return isset($this->current['MANAGEMENTCONFIG'][$managementClass]);
    }
  }

  /*!
   * \brief Returns the config for a management class, or NULL
   */
  public function getManagementConfig ($managementClass)
  {
    global $ui;

    if (isset($this->current['MANAGEMENTUSERCONFIG'][$ui->dn][$managementClass])) {
      return json_decode($this->current['MANAGEMENTUSERCONFIG'][$ui->dn][$managementClass], TRUE);
    } elseif (isset($this->current['MANAGEMENTCONFIG'][$managementClass])) {
      return json_decode($this->current['MANAGEMENTCONFIG'][$managementClass], TRUE);
    } else {
      return NULL;
    }
  }

  function getDepartmentList (): array
  {
    if (!isset($this->departmentList)) {
      $this->storeDepartmentList();
    }
    return $this->departmentList;
  }

  function getDepartmentTree (): array
  {
    if (!isset($this->departmentTree)) {
      $this->storeDepartmentTree();
    }
    return $this->departmentTree;
  }

  function getDepartmentInfo (): array
  {
    if (!isset($this->departmentInfo)) {
      $this->storeDepartmentList();
    }
    return $this->departmentInfo;
  }

  function resetDepartmentCache ()
  {
    unset($this->departmentList);
    unset($this->departmentTree);
    unset($this->departmentInfo);
  }

  /*!
   * \brief Store the departments from ldap in $this->departmentList
   */
  protected function storeDepartmentList ()
  {
    /* Initialize result hash */
    $result = ['/' => $this->current['BASE']];

    $this->departmentInfo = [];

    /* Get all department types from department Management, to be able detect the department type.
        -It is possible that different department types have the same name,
         in this case we have to mark the department name to be able to differentiate.
          (e.g l=Name  or   o=Name)
     */
    $types = departmentManagement::getDepartmentTypes();

    /* Create a list of attributes to fetch */
    $filter       = '';
    $ldap_values  = ['objectClass', 'description'];
    foreach ($types as $type) {
      $i = objects::infos($type);
      $filter         .= $i['filter'];
      /* Add type main attr to fetched attributes list */
      $ldap_values[]  = $i['mainAttr'];
    }
    $filter = '(|'.$filter.')';

    /* Get list of department objects */
    $ldap = $this->get_ldap_link();
    $ldap->cd($this->current['BASE']);
    $ldap->search($filter, $ldap_values);
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

      $dn     = $attrs['dn'];
      $infos  = objects::infos($oc);
      $this->departmentInfo[$dn] = [
        'img'         => $infos['icon'],
        'description' => (isset($attrs['description'][0]) ? $attrs['description'][0] : ''),
        'name'        => $attrs[$infos['mainAttr']][0]
      ];

      /* Only assign non-root departments */
      if ($dn != $result['/']) {
        $c_dn = convert_department_dn($dn).' ('.$infos['mainAttr'].')';
        $result[$c_dn] = $dn;
      }
    }

    $this->departmentList = $result;
  }

  /*!
   * \brief Store the tree render for departments in $this->departmentTree
   */
  protected function storeDepartmentTree ()
  {
    if (!isset($this->departmentList)) {
      $this->storeDepartmentList();
    }

    $base   = $this->current['BASE'];
    $qbase  = preg_quote($base, '/');

    $arr  = [];

    /* Create multidimensional array, with all departments. */
    foreach ($this->departmentList as $val) {

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
    $ret = [
      '/' => [
        'ENTRY' => $base,
        'SUB'   => $arr,
      ]
    ];
    $this->departmentTree = $this->generateDepartmentArray($ret, -1, 28);
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
  protected function generateDepartmentArray ($arr, $depth, $max_size)
  {
    $ret = [];
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
      $name = preg_replace('/\\\\,/', ',', $name);

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
   * \brief Search for hooks
   *
   *  Example usage:
   *  \code
   *  $postcmd = $config->search(get_class($this), 'POSTMODIFY');
   *  \endcode
   *
   *  \param string $class The class name
   *
   *  \param string $value Key to search in the hooks
   *
   *  \return array of hook commands or empty array
   */
  function searchHooks ($class, $value)
  {
    $class = strtoupper($class);
    $value = strtoupper($value);
    return (isset($this->data['HOOKS'][$class][$value]) ? $this->data['HOOKS'][$class][$value] : []);
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
  function get_cfg_value ($name, $default = '')
  {
    $name = strtoupper($name);
    $res  = $default;

    /* Check if we have a current value for $name */
    if (isset($this->current[$name])) {
      $res = $this->current[$name];
    } elseif (isset($this->data['MAIN'][$name])) {
      /* Check if we have a global value for $name */
      $res = $this->data['MAIN'][$name];
    }

    if (is_array($default) && !is_array($res)) {
      $res = [$res];
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
  function check_session_lifetime ()
  {
    $cfg_lifetime = $this->get_cfg_value('SESSIONLIFETIME', 0);
    if ($cfg_lifetime > 0) {
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
  function snapshotEnabled ()
  {
    if ($this->get_cfg_value('enableSnapshots') != 'TRUE') {
      return FALSE;
    }

    /* Check if the snapshot_base is defined */
    if ($this->get_cfg_value('snapshotBase') == '') {
      /* Send message if not done already */
      if (!session::is_set('snapshotFailMessageSend')) {
        session::set('snapshotFailMessageSend', TRUE);
        msg_dialog::display(_('Configuration error'),
            sprintf(_('The snapshot functionality is enabled, but the required variable "%s" is not set.'),
                    'snapshotBase'), ERROR_DIALOG);
      }
      return FALSE;
    }

    /* Check if gzcompress is available */
    if (!is_callable('gzcompress')) {
      /* Send message if not done already */
      if (!session::is_set('snapshotFailMessageSend')) {
        session::set('snapshotFailMessageSend', TRUE);
        msg_dialog::display(_('Configuration error'),
            sprintf(_('The snapshot functionality is enabled, but the required compression module is missing. Please install "%s".'), 'php5-zip / php5-gzip'), ERROR_DIALOG);
      }
      return FALSE;
    }
    return TRUE;
  }

  function loadPlist (pluglist $plist)
  {
    $this->data['OBJECTS']    = [];
    $this->data['SECTIONS']   = [];
    $this->data['CATEGORIES'] = [];
    $this->data['MENU']       = [];
    $this->data['TABS']       = [];
    foreach ($plist->info as $class => &$plInfo) {
      if (isset($plInfo['plObjectType'])) {
        $entry = ['CLASS' => $class,'NAME' => $plInfo['plShortName']];
        if (isset($plInfo['plSubTabs'])) {
          $entry['SUBTABS'] = strtoupper($plInfo['plSubTabs']);
        }
        foreach ($plInfo['plObjectType'] as $key => $value) {
          if (is_numeric($key)) {
            /* This is not the main tab */
            $tabclass = strtoupper($value).'TABS';
            if (($tabclass == 'GROUPTABS') && class_available('mixedGroup')) {
              $tabclass = 'OGROUP-USERTABS';
            }
            @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $tabclass, "Adding $class to tab list");
            if (!isset($this->data['TABS'][$tabclass])) {
              $this->data['TABS'][$tabclass] = [];
            }
            $this->data['TABS'][$tabclass][] = $entry;
          } else {
            /* This is the main tab */
            if (isset($this->data['OBJECTS'][strtoupper($key)])) {
              die("duplicated object type ".strtoupper($key)." in ".$this->data['OBJECTS'][strtoupper($key)]['mainTab']." and $class");
            }
            $tabclass = strtoupper($key)."TABS";
            $value['tabGroup']        = $tabclass;
            $value['mainTab']         = $class;
            $value['templateActive']  = FALSE;
            $value['snapshotActive']  = FALSE;
            foreach (['ou', 'tabClass'] as $i) {
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
            } elseif (isset($plInfo['plFilter'])) {
              $value['filter'] = $plInfo['plFilter'];
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
            @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $tabclass, "Adding $class as main tab of");
            if (!isset($this->data['TABS'][$tabclass])) {
              $this->data['TABS'][$tabclass] = [];
            }
            array_unshift($this->data['TABS'][$tabclass], $entry);
          }
        }
      } elseif (class_available($class) && is_subclass_of($class, 'simpleService')) {
        @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $class, "Adding service");
        if (!isset($this->data['TABS']['SERVERSERVICE'])) {
          $this->data['TABS']['SERVERSERVICE'] = [];
        }
        $this->data['TABS']['SERVERSERVICE'][] = [
          'CLASS' => $class,
          'NAME' => $plInfo['plShortName']
        ];
      }
      /* Feed categories */
      if (isset($plInfo['plCategory'])) {
        /* Walk through supplied list and feed only translated categories */
        $cats = [];
        foreach ($plInfo['plCategory'] as $idx => $infos) {
          $cat    = (is_numeric($idx) ? $infos : $idx);
          $cats[] = $cat;
          if (!isset($this->data['CATEGORIES'][$cat])) {
            $this->data['CATEGORIES'][$cat] = [ 'classes' => ['0'] ];
          }
          if (!empty($plInfo['plProvidedAcls'])) {
            $this->data['CATEGORIES'][$cat]['classes'][] = $class;
          }
          if (!is_numeric($idx)) {
            /* Non numeric index means -> base object containing more informations */
            $this->data['CATEGORIES'][$cat]['description'] = $infos['description'];
            if (!is_array($infos['objectClass'])) {
              $infos['objectClass'] = [$infos['objectClass']];
            }
            $this->data['CATEGORIES'][$cat]['objectClass'] = $infos['objectClass'];
          }
        }
        $plInfo['plCategory'] = $cats;
      }
    }
    unset($plInfo);
    $this->data['CATEGORIES']['all'] = [
      'classes'     => ['all'],
      'description' => '*&nbsp;'._('All categories'),
      'objectClass' => [],
    ];
    /* Extract categories definitions from object types */
    foreach ($this->data['OBJECTS'] as $key => $infos) {
      @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $infos['aclCategory'], "ObjectType $key category");
      if (strtoupper($infos['aclCategory']) == $key) {
        $cat = $infos['aclCategory'];
        if (!isset($this->data['CATEGORIES'][$cat])) {
          $this->data['CATEGORIES'][$cat] = ['classes' => ['0']];
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
      $acl = [];
      if (isset($plInfo['plCategory'])) {
        $acl = $plInfo['plCategory'];
        if (!is_array($acl)) {
          $acl = [$acl];
        }
      }
      if (isset($plInfo['plObjectType'])) {
        foreach ($plInfo['plObjectType'] as $key => $value) {
          if (is_numeric($key)) {
            /* This is not the main tab */
            $obj = strtoupper($value);
          } else {
            /* This is the main tab */
            $obj = strtoupper($key);
          }
          if (strpos($obj, 'OGROUP-') === 0) {
            $obj = 'OGROUP';
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
            if (isset($class::$skipTemplates) && !$class::$skipTemplates) {
              $this->data['OBJECTS'][$obj]['templateActive']  = TRUE;
              $this->data['CATEGORIES'][$cat]['classes'][]    = 'template';
            }
            if (isset($class::$skipSnapshots) && !$class::$skipSnapshots) {
              $this->data['OBJECTS'][$obj]['snapshotActive']  = TRUE;
              $this->data['CATEGORIES'][$cat]['classes'][]    = 'SnapshotHandler';
            }
          }
        }
      }
      @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, join(',', array_unique($acl)), "Class $class categories");
      /* Feed menu */
      if (isset($plInfo['plSection'])) {
        $section = $plInfo['plSection'];
        if (!is_array($acl)) {
          $acl = [$acl];
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
          $this->data['MENU'][$section] = [];
        }
        $attrs = ['CLASS' => $class];
        if (!empty($acl)) {
          $attrs['ACL'] = $acl;
        }
        $this->data['MENU'][$section][] = $attrs;
      }
      if (isset($plInfo['plMenuProvider']) && $plInfo['plMenuProvider']) {
        list($sections, $entries) = $class::getMenuEntries();
        foreach ($sections as $section => $infos) {
          if (!isset($this->data['SECTIONS'][$section])) {
            $this->data['SECTIONS'][$section] = array_change_key_case($infos, CASE_UPPER);
          }
          if (!isset($this->data['MENU'][$section])) {
            $this->data['MENU'][$section] = [];
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
    ksort($this->data['CATEGORIES']);
    foreach ($this->data['CATEGORIES'] as $name => &$infos) {
      $infos['classes'] = array_unique($infos['classes']);
      if (!isset($infos['description'])) {
        $infos['description'] = $name;
        $infos['objectClass'] = [];
      }
    }
    unset($infos);
    $this->data['SECTIONS']['personal'] = ['NAME' => _('My account'), 'PRIORITY' => 40];
    $personal = [];
    foreach ($this->data['TABS']['USERTABS'] as $tab) {
      if ($plist->info[$tab['CLASS']]['plSelfModify']) {
        $personal[] = ['CLASS' => $tab['CLASS'], 'ACL' => 'user/'.$tab['CLASS'].':self'];
      }
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
}
