<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2003 Alejandro Escanero Blanco <aescanero@chaosdimension.org>
  Copyright (C) 1998  Eric Kilfoil <eric@ipass.net>
  Copyright (C) 2011-2016  FusionDirectory

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
 * \file class_ldap.inc
 * Source code for Class LDAP
 */

define("SPECIALS_OVERRIDE", FALSE);

/*! \class LdapGeneralizedTimeBadFormatException
    \brief Exception class which can be thrown by LDAP if the LDIF format is broken
*/
class LDIFImportException extends Exception {}

/*!
 * \brief This class contains all ldap function needed to make
 * ldap operations easy
 */

class LDAP
{
  var $hascon         = FALSE;
  var $reconnect      = FALSE;
  var $tls            = FALSE;
  var $cid;               //connection identifier
  var $hasres         = array();
  var $sr             = array();
  var $re             = array();
  var $basedn         = "";
  var $start          = array(); // 0 if we are fetching the first entry, otherwise 1
  var $error          = ""; // Any error messages to be returned can be put here
  var $srp            = 0;
  var $objectClasses    = array(); // Information read from slapd.oc.conf
  var $binddn         = ""; // the dn for the bind
  var $bindpw         = ""; //the dn's password for the bind
  var $hostname       = "";
  var $follow_referral    = FALSE;
  var $referrals      = array();
  var $max_ldap_query_time  = 0;   // 0, empty or negative values will disable this check

  /*!
   * \brief Create a LDAP connection
   *
   * \param string $binddn Bind of the DN
   *
   * \param string $bindpw Bind
   *
   * \param string $hostname The hostname
   *
   * \param boolean $follow_referral FALSE
   *
   * \param boolean $tls FALSE
   */
  function __construct($binddn, $bindpw, $hostname, $follow_referral = FALSE, $tls = FALSE)
  {
    global $config;
    $this->follow_referral  = $follow_referral;
    $this->tls              = $tls;
    $this->binddn           = LDAP::convert($binddn);
    $this->bindpw           = $bindpw;
    $this->hostname         = $hostname;

    /* Check if MAX_LDAP_QUERY_TIME is defined */
    if (is_object($config) && ($config->get_cfg_value("ldapMaxQueryTime") != "")) {
      $str = $config->get_cfg_value("ldapMaxQueryTime");
      $this->max_ldap_query_time = (float)($str);
    }

    $this->connect();
  }

  /*!
   * \brief Get the search ressource
   *
   * \return increase srp
   */
  function getSearchResource()
  {
    $this->sr[$this->srp]     = NULL;
    $this->start[$this->srp]  = 0;
    $this->hasres[$this->srp] = FALSE;
    return $this->srp++;
  }

  /*! \brief Function to replace all problematic characters inside a DN by \001XX
   *
   * Function to replace all problematic characters inside a DN by \001XX, where
   * \001 is decoded to chr(1) [ctrl+a]. It is not impossible, but very unlikely
   * that this character is inside a DN.
   *
   * Currently used codes:
   * \code
   * ,   => CO
   * \2C => CO
   * (   => OB
   * )   => CB
   * /   => SL
   * "   => DQ
   * \22 => DQ
   * \endcode
   *
   * \param string $dn The DN
   *
   * \return String, the corrected DN
   */
  static function convert($dn)
  {
    if (SPECIALS_OVERRIDE == TRUE) {
      $tmp = preg_replace(
          array("/\\\\,/", "/\\\\2C/", "/\(/", "/\)/", "/\//", "/\\\\22/", '/\\\\"/'),
          array("\001CO", "\001CO", "\001OB", "\001CB", "\001SL", "\001DQ", "\001DQ"),
          $dn
      );
      return preg_replace('/,\s+/', ',', $tmp);
    } else {
      return $dn;
    }
  }

  /*!
   * \brief Function to fix all problematic characters inside a DN DN by replacing \001XX codes
   * to their original values
   *
   * Function to fix all problematic characters inside a DN by replacing \001XX codes
   * to their original values. See "convert" for more information.
   * ',' characters are always expanded to \, (not \2C), since all tested LDAP servers
   * seem to take it the correct way.
   *
   * \param string $dn The DN
   *
   * \return String, the fixed DN
   */
  static function fix($dn)
  {
    if (SPECIALS_OVERRIDE == TRUE) {
      return preg_replace(
        array("/\001CO/", "/\001OB/", "/\001CB/", "/\001SL/", "/\001DQ/"),
        array("\,", "(", ")", "/", '\"'),
        $dn
      );
    } else {
      return $dn;
    }
  }

  /*!
   * \brief Function to fix problematic characters in DN's that are used for search requests. I.e. member=....
   *
   * \param string $dn The DN
   */
  static function prepare4filter($dn)
  {
    trigger_error('deprecated, use ldap_escape_f instead');
    return ldap_escape_f($dn);
  }

  /*!
   *  \brief Create a connection to LDAP server
   *
   *  The string $error containts result of the connection
   */
  function connect()
  {
    $this->hascon     = FALSE;
    $this->reconnect  = FALSE;
    if ($this->cid = @ldap_connect($this->hostname)) {
      @ldap_set_option($this->cid, LDAP_OPT_PROTOCOL_VERSION, 3);
      if (function_exists("ldap_set_rebind_proc") && $this->follow_referral) {
        @ldap_set_option($this->cid, LDAP_OPT_REFERRALS, 1);
        @ldap_set_rebind_proc($this->cid, array(&$this, "rebind"));
      }
      if (function_exists("ldap_start_tls") && $this->tls) {
        @ldap_start_tls($this->cid);
      }

      $this->error = "No Error";
      if (@ldap_bind($this->cid, LDAP::fix($this->binddn), $this->bindpw)) {
        $this->error  = "Success";
        $this->hascon = TRUE;
      } else {
        if ($this->reconnect) {
          if ($this->error != "Success") {
            $this->error = "Could not rebind to " . $this->binddn;
          }
        } else {
          $this->error = "Could not bind to " . $this->binddn;
        }
      }
    } else {
      $this->error = "Could not connect to LDAP server";
    }
  }

  /*!
   *  \brief Rebind
   */
  function rebind($ldap, $referral)
  {
    $credentials = $this->get_credentials($referral);
    if (@ldap_bind($ldap, LDAP::fix($credentials['ADMINDN']), $credentials['ADMINPASSWORD'])) {
      $this->error      = "Success";
      $this->hascon     = TRUE;
      $this->reconnect  = TRUE;
      return 0;
    } else {
      $this->error = "Could not bind to " . $credentials['ADMINDN'];
      return NULL;
    }
  }

  /*!
   *  \brief Reconnect to LDAP server
   */
  function reconnect()
  {
    if ($this->reconnect) {
      $this->unbind();
    }
  }

  /*!
   *  \brief Unbind to LDAP server
   */
  function unbind()
  {
    @ldap_unbind($this->cid);
    $this->cid = NULL;
  }

  /*!
   *  \brief Disconnect to LDAP server
   */
  function disconnect()
  {
    if ($this->hascon) {
      @ldap_close($this->cid);
      $this->hascon = FALSE;
    }
  }

  /*!
   * \brief Change directory
   *
   * \param string $dir The new directory
   */
  function cd($dir)
  {
    if ($dir == "..") {
      $this->basedn = $this->getParentDir();
    } else {
      $this->basedn = LDAP::convert($dir);
    }
  }

  /*!
   * \brief Accessor of the parent directory of the basedn
   *
   * \param string $basedn The basedn which we want the parent directory
   *
   * \return String, the parent directory
   */
  function getParentDir($basedn = "")
  {
    if ($basedn == "") {
      $basedn = $this->basedn;
    } else {
      $basedn = LDAP::convert($basedn);
    }
    return preg_replace("/[^,]*[,]*[ ]*(.*)/", "$1", $basedn);
  }

  /*!
   * \brief Search about filter
   *
   * \param integer $srp srp
   *
   * \param string $filter The filter
   *
   * \param array $attrs
   */
  function search($srp, $filter, $attrs = array())
  {
    if ($this->hascon) {
      if ($this->reconnect) {
        $this->connect();
      }

      $start = microtime(TRUE);
      $this->clearResult($srp);
      $this->sr[$srp] = @ldap_search($this->cid, LDAP::fix($this->basedn), $filter, $attrs);
      $this->error = @ldap_error($this->cid);
      $this->resetResult($srp);
      $this->hasres[$srp] = TRUE;

      /* Check if query took longer as specified in max_ldap_query_time */
      if ($this->max_ldap_query_time) {
        $diff = microtime(TRUE) - $start;
        if ($diff > $this->max_ldap_query_time) {
          msg_dialog::display(_("Performance warning"), sprintf(_("LDAP performance is poor: last query took about %.2fs!"), $diff), WARNING_DIALOG);
        }
      }

      $this->log("LDAP operation: time=".(microtime(TRUE) - $start)." operation=search('".LDAP::fix($this->basedn)."', '$filter')");
      return $this->sr[$srp];
    } else {
      $this->error = "Could not connect to LDAP server";
      return "";
    }
  }

  /*
   * \brief List
   *
   * \param integer $srp
   *
   * \param string $filter Initialized at "(objectclass=*)"
   *
   * \param string $basedn Empty string
   *
   * \param array $attrs
   */
  function ls($srp, $filter = "(objectclass=*)", $basedn = "", $attrs = array("*"))
  {
    if ($this->hascon) {
      if ($this->reconnect) {
        $this->connect();
      }

      $this->clearResult($srp);
      if ($basedn == "") {
        $basedn = $this->basedn;
      } else {
        $basedn = LDAP::convert($basedn);
      }

      $start          = microtime(TRUE);
      $this->sr[$srp] = @ldap_list($this->cid, LDAP::fix($basedn), $filter, $attrs);
      $this->error    = @ldap_error($this->cid);
      $this->resetResult($srp);
      $this->hasres[$srp] = TRUE;

       /* Check if query took longer as specified in max_ldap_query_time */
      if ($this->max_ldap_query_time) {
        $diff = microtime(TRUE) - $start;
        if ($diff > $this->max_ldap_query_time) {
          msg_dialog::display(_("Performance warning"), sprintf(_("LDAP performance is poor: last query took about %.2fs!"), $diff), WARNING_DIALOG);
        }
      }

      $this->log("LDAP operation: time=".(microtime(TRUE) - $start)." operation=ls('".LDAP::fix($basedn)."', '$filter')");

      return $this->sr[$srp];
    } else {
      $this->error = "Could not connect to LDAP server";
      return "";
    }
  }

  /*
   * \brief Concatenate
   *
   * \param integer $srp
   *
   * \param string $dn The DN
   *
   * \param array $attrs
   *
   * \param string $filter Initialized at "(objectclass=*)"
   */
  function cat($srp, $dn, $attrs = array("*"), $filter = "(objectclass=*)")
  {
    if ($this->hascon) {
      if ($this->reconnect) {
        $this->connect();
      }

      $this->clearResult($srp);
      $this->sr[$srp] = @ldap_read($this->cid, LDAP::fix($dn), $filter, $attrs);
      $this->error    = @ldap_error($this->cid);
      $this->resetResult($srp);
      $this->hasres[$srp] = TRUE;
      return $this->sr[$srp];
    } else {
      $this->error = "Could not connect to LDAP server";
      return "";
    }
  }

  /*!
   * \brief Search object from a filter
   *
   * \param string $dn The DN
   *
   * \param string $filter The filter of the research
   */
  function object_match_filter($dn, $filter)
  {
    if ($this->hascon) {
      if ($this->reconnect) {
        $this->connect();
      }
      $res  = @ldap_read($this->cid, LDAP::fix($dn), $filter, array("objectClass"));
      $rv   = @ldap_count_entries($this->cid, $res);
      return $rv;
    } else {
      $this->error = "Could not connect to LDAP server";
      return FALSE;
    }
  }

  /*!
   * \brief Set a size limit
   *
   * \param $size The limit
   */
  function set_size_limit($size)
  {
    /* Ignore zero settings */
    if ($size == 0) {
      @ldap_set_option($this->cid, LDAP_OPT_SIZELIMIT, 10000000);
    }
    if ($this->hascon) {
      @ldap_set_option($this->cid, LDAP_OPT_SIZELIMIT, $size);
    } else {
      $this->error = "Could not connect to LDAP server";
    }
  }

  /*!
   * \brief Fetch
   *
   * \param integer $srp
   */
  function fetch($srp)
  {
    $att = array();
    if ($this->hascon) {
      if ($this->hasres[$srp]) {
        if ($this->start[$srp] == 0) {
          if ($this->sr[$srp]) {
            $this->start[$srp]  = 1;
            $this->re[$srp]     = @ldap_first_entry($this->cid, $this->sr[$srp]);
          } else {
            return array();
          }
        } else {
          $this->re[$srp] = @ldap_next_entry($this->cid, $this->re[$srp]);
        }
        if ($this->re[$srp]) {
          $att        = @ldap_get_attributes($this->cid, $this->re[$srp]);
          $att['dn']  = trim(LDAP::convert(@ldap_get_dn($this->cid, $this->re[$srp])));
        }
        $this->error = @ldap_error($this->cid);
        if (!isset($att)) {
          $att = array();
        }
        return $att;
      } else {
        $this->error = "Perform a fetch with no search";
        return "";
      }
    } else {
      $this->error = "Could not connect to LDAP server";
      return "";
    }
  }

  /*!
   * \brief Reset the result
   *
   * \param integer $srp Value to be reset
   */
  function resetResult($srp)
  {
    $this->start[$srp] = 0;
  }

  /*!
   * \brief Clear a result
   *
   * \param integer $srp The result to clear
   */
  function clearResult($srp)
  {
    if ($this->hasres[$srp]) {
      $this->hasres[$srp] = FALSE;
      @ldap_free_result($this->sr[$srp]);
    }
  }

  /*!
   * \brief Accessor of the DN
   *
   * \param $srp srp
   */
  function getDN($srp)
  {
    if ($this->hascon) {
      if ($this->hasres[$srp]) {
        if (!$this->re[$srp]) {
          $this->error = "Perform a Fetch with no valid Result";
        } else {
          $rv = @ldap_get_dn($this->cid, $this->re[$srp]);

          $this->error = @ldap_error($this->cid);
          return trim(LDAP::convert($rv));
        }
      } else {
        $this->error = "Perform a Fetch with no Search";
        return "";
      }
    } else {
      $this->error = "Could not connect to LDAP server";
      return "";
    }
  }

  /*!
   * \brief Return the numbers of entries
   *
   * \param $srp srp
   */
  function count($srp)
  {
    if ($this->hascon) {
      if ($this->hasres[$srp]) {
        $rv = @ldap_count_entries($this->cid, $this->sr[$srp]);
        $this->error = @ldap_error($this->cid);
        return $rv;
      } else {
        $this->error = "Perform a Fetch with no Search";
        return "";
      }
    } else {
      $this->error = "Could not connect to LDAP server";
      return "";
    }
  }


  /*!
   * \brief Remove
   *
   * \param string $attrs Empty string
   *
   * \param string $dn Empty string
   */
  function rm($attrs = "", $dn = "")
  {
    if ($this->hascon) {
      if ($this->reconnect) $this->connect();
      if ($dn == "")
        $dn = $this->basedn;

      $r = ldap_mod_del($this->cid, LDAP::fix($dn), $attrs);
      $this->error = @ldap_error($this->cid);
      return $r;
    } else {
      $this->error = "Could not connect to LDAP server";
      return "";
    }
  }

  function mod_add($attrs = "", $dn = "")
  {
    if ($this->hascon) {
      if ($this->reconnect) $this->connect();
      if ($dn == "")
        $dn = $this->basedn;

      $r = @ldap_mod_add($this->cid, LDAP::fix($dn), $attrs);
      $this->error = @ldap_error($this->cid);
      return $r;
    } else {
      $this->error = "Could not connect to LDAP server";
      return "";
    }
  }

  /*!
   * \brief Remove directory
   *
   * \param string $deletedn The DN to be deleted
  */
  function rmdir($deletedn)
  {
    if ($this->hascon) {
      if ($this->reconnect) $this->connect();
      $r = @ldap_delete($this->cid, LDAP::fix($deletedn));
      $this->error = @ldap_error($this->cid);
      return ($r ? $r : 0);
    } else {
      $this->error = "Could not connect to LDAP server";
      return "";
    }
  }


  /*!
   * \brief Move the given Ldap entry from $source to $dest
   *
   * \param  String  $source The source dn.
   *
   * \param  String  $dest   The destination dn.
   *
   * \return Boolean TRUE on success else FALSE.
   */
  function rename_dn($source, $dest)
  {
    /* Check if source and destination are the same entry */
    if (strtolower($source) == strtolower($dest)) {
      trigger_error("Source and destination can't be the same entry.");
      $this->error = "Source and destination can't be the same entry.";
      return FALSE;
    }

    /* Check if destination entry exists */
    if ($this->dn_exists($dest)) {
      trigger_error("Destination '$dest' already exists.");
      $this->error = "Destination '$dest' already exists.";
      return FALSE;
    }

    /* Extract the name and the parent part out ouf source dn.
        e.g.  cn=herbert,ou=department,dc=...
         parent   =>  ou=department,dc=...
         dest_rdn =>  cn=herbert
     */
    $parent   = preg_replace("/^[^,]+,/", "", $dest);
    $dest_rdn = preg_replace("/,.*$/", "", $dest);

    if ($this->hascon) {
      if ($this->reconnect) $this->connect();
      $r = ldap_rename($this->cid, @LDAP::fix($source), @LDAP::fix($dest_rdn), @LDAP::fix($parent), FALSE);
      $this->error = ldap_error($this->cid);

      /* Check if destination dn exists, if not the
          server may not support this operation */
      $r &= is_resource($this->dn_exists($dest));
      return $r;
    } else {
      $this->error = "Could not connect to LDAP server";
      return FALSE;
    }
  }


  /*!
   * \brief Function rmdir_recursive
   *
   * Based in recursive_remove, adding two thing: full subtree remove, and delete own node.
   *
   * \param $srp srp
   *
   * \param string $deletedn The dn to delete
   *
   * \return TRUE on sucessfull , 0 in error, and "" when we don't get a ldap conection
   */
  function rmdir_recursive($srp, $deletedn)
  {
    if ($this->hascon) {
      if ($this->reconnect) $this->connect();
      $delarray = array();

      /* Get sorted list of dn's to delete */
      $this->cd($deletedn);
      $this->search($srp, "(objectClass=*)", array('dn'));
      while ($attrs = $this->fetch($srp)) {
        $delarray[$attrs['dn']] = strlen($attrs['dn']);
      }
      arsort($delarray);
      reset($delarray);

      /* Really Delete ALL dn's in subtree */
      foreach (array_keys($delarray) as $key) {
        $r = @ldap_delete($this->cid, LDAP::fix($key));
        if ($r === FALSE) {
          break;
        }
      }
      $this->error = @ldap_error($this->cid);
      return ($r ? $r : 0);
    } else {
      $this->error = "Could not connect to LDAP server";
      return "";
    }
  }

  function makeReadableErrors($error, $attrs)
  {
    if ($this->success()) {
      return "";
    }

    $str = "";
    if (preg_match("/^objectClass: value #([0-9]*) invalid per syntax$/", $this->get_additional_error())) {
      $oc = preg_replace("/^objectClass: value #([0-9]*) invalid per syntax$/", "\\1", $this->get_additional_error());
      if (isset($attrs['objectClass'][$oc])) {
        $str .= " - <b>objectClass: ".$attrs['objectClass'][$oc]."</b>";
      }
    }
    if ($error == "Undefined attribute type") {
      $str = " - <b>attribute: ".preg_replace("/:.*$/", "", $this->get_additional_error())."</b>";
    }

    @DEBUG(DEBUG_LDAP, __LINE__, __FUNCTION__, __FILE__, $attrs, "Erroneous data");

    return $str;
  }

  /*!
   * \brief Modify a entry of the directory LDAP
   *
   * \param string $attrs The new entry
   */
  function modify($attrs)
  {
    if (count($attrs) == 0) {
      return 0;
    }
    if ($this->hascon) {
      if ($this->reconnect) $this->connect();
      $r = @ldap_modify($this->cid, LDAP::fix($this->basedn), $attrs);
      $this->error = @ldap_error($this->cid);
      if (!$this->success()) {
        $this->error .= $this->makeReadableErrors($this->error, $attrs);
      }
      return ($r ? $r : 0);
    } else {
      $this->error = "Could not connect to LDAP server";
      return "";
    }
  }

  /*!
   * \brief Add entry in the LDAP directory
   *
   * \param string $attrs The entry to add
   */
  function add($attrs)
  {
    if ($this->hascon) {
      if ($this->reconnect) $this->connect();
      $r = @ldap_add($this->cid, LDAP::fix($this->basedn), $attrs);
      $this->error = @ldap_error($this->cid);
      if (!$this->success()) {
        $this->error .= $this->makeReadableErrors($this->error, $attrs);
      }
      return ($r ? $r : 0);
    } else {
      $this->error = "Could not connect to LDAP server";
      return "";
    }
  }

  /*
   * $target is a dn, i.e. "ou=example,ou=orga,dc=base"
   *
   * Creates missing trees, in our example ou=orga,dc=base will get created if not existing, same thing for ou=example,ou=orga,dc=base
   * */
  function create_missing_trees($srp, $target, $ignoreReferralBases = TRUE)
  {
    $real_path = substr($target, 0, strlen($target) - strlen($this->basedn) - 1);

    if ($target == $this->basedn) {
      $l = array("dummy");
    } else {
      $l = array_reverse(gosa_ldap_explode_dn($real_path));
    }
    unset($l['count']);
    $cdn = $this->basedn;

    /* Load schema if available... */
    $classes = $this->get_objectclasses();

    foreach ($l as $part) {
      if ($part != "dummy") {
        $cdn = "$part,$cdn";
      }

      /* Ignore referrals */
      if ($ignoreReferralBases) {
        $found = FALSE;
        foreach ($this->referrals as $ref) {
          $base = preg_replace('!^[^:]+://[^/]+/([^?]+).*$!', '\\1', $ref['URI']);
          if ($base == $cdn) {
            $found = TRUE;
            break;
          }
        }
        if ($found) {
          continue;
        }
      }

      $this->cat ($srp, $cdn);
      $attrs = $this->fetch($srp);

      /* Create missing entry? */
      if (!count($attrs)) {
        $type   = preg_replace('/^([^=]+)=.*$/', '\\1', $cdn);
        $param  = LDAP::fix(preg_replace('/^[^=]+=([^,]+).*$/', '\\1', $cdn));
        $param  = preg_replace(array('/\\\\,/','/\\\\"/'), array(',','"'), $param);

        $na = array();

        /* Automatic or traditional? */
        if (count($classes)) {
          /* Get name of first matching objectClass */
          $ocname = "";
          foreach ($classes as $class) {
            if (isset($class['MUST']) && in_array($type, $class['MUST'])) {

              /* Look for first classes that is structural... */
              if (isset($class['STRUCTURAL'])) {
                $ocname = $class['NAME'];
                break;
              }

              /* Look for classes that are auxiliary... */
              if (isset($class['AUXILIARY'])) {
                $ocname = $class['NAME'];
              }
            }
          }

          /* Bail out, if we've nothing to do... */
          if ($ocname == '') {
            msg_dialog::display(_('Internal error'), sprintf(_('Cannot automatically create subtrees with RDN "%s": no object class found!'), $type), FATAL_ERROR_DIALOG);
            exit();
          }

          /* Assemble_entry */
          $na['objectClass'] = array($ocname);
          if (isset($classes[$ocname]['AUXILIARY'])) {
            $na['objectClass'][] = $classes[$ocname]['SUP'];
          }
          if ($type == 'dc') {
            /* This is bad actually, but - tell me a better way? */
            $na['objectClass'][] = 'organization';
            $na['o'] = $param;
          }
          $na[$type] = $param;

          // Fill in MUST values - but do not overwrite existing ones.
          if (is_array($classes[$ocname]['MUST'])) {
            foreach ($classes[$ocname]['MUST'] as $attr) {
              if (isset($na[$attr]) && !empty($na[$attr])) continue;
              $na[$attr] = 'filled';
            }
          }
        } else {
          /* Use alternative add... */
          switch ($type) {
            case 'ou':
              $na['objectClass']  = 'organizationalUnit';
              $na['ou']           = $param;
              break;
            case 'dc':
              $na['objectClass']  = array('dcObject', 'top', 'organization');
              $na['dc']           = $param;
              $na['o']            = $param;
              break;
            default:
              msg_dialog::display(_('Internal error'), sprintf(_('Cannot automatically create subtrees with RDN "%s": not supported'), $type), FATAL_ERROR_DIALOG);
              exit();
          }

        }
        $this->cd($cdn);
        $this->add($na);

        if (!$this->success()) {
          @DEBUG(DEBUG_LDAP, __LINE__, __FUNCTION__, __FILE__, $cdn, 'dn');
          @DEBUG(DEBUG_LDAP, __LINE__, __FUNCTION__, __FILE__, $na, 'Content');
          @DEBUG(DEBUG_LDAP, __LINE__, __FUNCTION__, __FILE__, $this->get_error(), 'LDAP error');

          msg_dialog::display(_('LDAP error'), msgPool::ldaperror($this->get_error(), $cdn, LDAP_ADD, get_class()), LDAP_ERROR);
          return FALSE;
        }
      }
    }

    return TRUE;
  }

  /*!
   * \brief Recursive remove
   *
   * \param integer $srp
   */
  function recursive_remove($srp)
  {
    $delarray = array();

    /* Get sorted list of dn's to delete */
    $this->search ($srp, "(objectClass=*)");
    while ($this->fetch($srp)) {
      $deldn            = $this->getDN($srp);
      $delarray[$deldn] = strlen($deldn);
    }
    arsort($delarray);
    reset($delarray);

    /* Delete all dn's in subtree */
    foreach (array_keys($delarray) as $key) {
      $this->rmdir($key);
    }
  }

  /*!
   * \brief Read a entry from a directory
   *
   * \param string $dn The DN
   *
   * \param string $name The name of the entry
   *
   * \param integer $r_array Initialized at 0
   */
  function get_attribute($dn, $name, $r_array = 0)
  {
    $data = "";
    if ($this->reconnect) {
      $this->connect();
    }
    $sr = @ldap_read($this->cid, LDAP::fix($dn), "objectClass=*", array("$name"));

    /* fill data from LDAP */
    if ($sr) {
      $ei = @ldap_first_entry($this->cid, $sr);
      if ($ei) {
        if ($info = @ldap_get_values_len($this->cid, $ei, "$name")) {
          $data = $info[0];
        }
      }
    }
    if ($r_array == 0) {
      return $data;
    } else {
      return $info;
    }
  }


  /*!
   * \brief Get the LDAP additional error
   *
   * \return string $error containts LDAP_OPT_ERROR_STRING
   */
  function get_additional_error()
  {
    $error = "";
    @ldap_get_option ($this->cid, LDAP_OPT_ERROR_STRING, $error);
    return $error;
  }

  /*!
   * \brief Success
   *
   * \return boolean TRUE if Success is found in $error, else return FALSE
   */
  function success()
  {
    return preg_match('/Success/i', $this->error);
  }

  /*!
   * \brief Get the error
   */
  function get_error()
  {
    if ($this->error == 'Success') {
      return $this->error;
    } else {
      $adderror = $this->get_additional_error();
      if ($adderror != "") {
        $error = $this->error." (".$this->get_additional_error().", ".sprintf(_("while operating on '%s' using LDAP server '%s'"), $this->basedn, $this->hostname).")";
      } else {
        $error = $this->error." (".sprintf(_("while operating on LDAP server %s"), $this->hostname).")";
      }
      return $error;
    }
  }

  function get_credentials($url, $referrals = NULL)
  {
    $ret    = array();
    $url    = preg_replace('!\?\?.*$!', '', $url);
    $server = preg_replace('!^([^:]+://[^/]+)/.*$!', '\\1', $url);

    if ($referrals === NULL) {
      $referrals = $this->referrals;
    }

    if (isset($referrals[$server])) {
      return $referrals[$server];
    } else {
      $ret['ADMINDN']       = LDAP::fix($this->binddn);
      $ret['ADMINPASSWORD'] = $this->bindpw;
    }

    return $ret;
  }


  /*!
   * \brief  Generates an ldif for all entries matching the filter settings, scope and limit.
   *
   * \param  $dn           The entry to export.
   *
   * \param  $filter       Limit the exported object to those maching this filter.
   *
   * \param  array $attributes Attributes
   *
   * \param  $scope        'base', 'sub' .. see manpage for 'ldapmodify' for details.
   *
   * \param  $limit        Limits the result.
   */
  function generateLdif ($dn, $filter = "(objectClass=*)", $scope = 'sub', $limit = 0)
  {
    // Ensure that limit is numeric if not skip here.
    if (!empty($limit) && !is_numeric($limit)) {
        trigger_error(sprintf("Invalid parameter for limit '%s', a numeric value is required."), $limit);
        return NULL;
    }
    $limit = (!$limit)?'':' -z '.$limit;

    // Check scope values
    $scope = trim($scope);
    if (!empty($scope) && !in_array($scope, array('base', 'one', 'sub', 'children'))) {
        trigger_error(sprintf("Invalid parameter for scope '%s', please use 'base', 'one', 'sub' or 'children'."), $scope);
        return NULL;
    }
    $scope = (!empty($scope))?' -s '.$scope: '';

    // Prepare parameters to be valid for shell execution
    $dn     = escapeshellarg($dn);
    $pwd    = escapeshellarg($this->bindpw);
    $host   = escapeshellarg($this->hostname);
    $admin  = escapeshellarg($this->binddn);
    $filter = escapeshellarg($filter);

    $cmd = "ldapsearch -x -LLLL -D {$admin} {$filter} {$limit} {$scope} -H {$host} -b {$dn} -w {$pwd} ";

    // Create list of process pipes
    $descriptorspec = array(
      0 => array("pipe", "r"),  // stdin
      1 => array("pipe", "w"),  // stdout
      2 => array("pipe", "w")   // stderr
    );

    // Try to open the process
    $process = proc_open($cmd, $descriptorspec, $pipes);
    if (is_resource($process)) {
      // Write the password to stdin
      fclose($pipes[0]);

      // Get results from stdout and stderr
      $res = stream_get_contents($pipes[1]);
      $err = stream_get_contents($pipes[2]);
      fclose($pipes[1]);

      // Close the process and check its return value
      if (proc_close($process) != 0) {
        $this->error = $err;
        return NULL;
      }
    } else {
      $this->error = _("proc_open failed to execute ldapsearch");
      return NULL;
    }
    return $res;
  }


  function dn_exists($dn)
  {
    return @ldap_list($this->cid, LDAP::fix($dn), "(objectClass=*)", array("objectClass"));
  }



  /*!
   * \brief Function to imports ldifs
   *
   * If DeleteOldEntries is TRUE, the destination entry will be deleted first.
   * If JustModify is TRUE the destination entry will only be touched by the attributes specified in the ldif.
   * if JustMofify is FALSE the destination dn will be overwritten by the new ldif.
   *
   * \param integer $srp
   *
   * \param string $str_attr
   *
   * \param string $error
   *
   * \param boolean $JustModify
   *
   * \param boolean $DeleteOldEntries
   */
  function import_complete_ldif($srp, $str_attr, $JustModify, $DeleteOldEntries)
  {
    if ($this->reconnect) {
      $this->connect();
    }

    /* First we split the string into lines */
    $fileLines = preg_split("/\n/", $str_attr);

    /* Joining lines */
    $line   = NULL;
    $entry  = array();
    $entryStart = -1;
    foreach ($fileLines as $lineNumber => $fileLine) {
      if (preg_match('/^ /', $fileLine)) {
        if ($line === NULL) {
          throw new LDIFImportException(sprintf(_('Error line %s, first line of an entry cannot start with a space'), $lineNumber));
        }
        /* Append to current line */
        $line .= substr($fileLine, 1);
      } elseif (preg_match('/^#/', $fileLine)) {
        /* Ignore comment */
        // TODO handle folded comment
      } elseif (preg_match('/^version:/', $fileLine) && empty($entry)) {
        /* Ignore version number */
      } else {
        if ($line !== NULL) {
          /* Line has ended */
          list ($key, $value) = explode(':', $line, 2);
          $value = trim($value);
          if (preg_match('/^:/', $value)) {
            $value = base64_decode(trim(substr($value, 1)));
          }
          if (preg_match('/^</', $value)) {
            throw new LDIFImportException(sprintf(_('Error line %s, references to an external file are not supported'), $lineNumber));
          }
          if ($value === '') {
            throw new LDIFImportException(sprintf(_('Error line %s, attribute "%s" has no value'), $lineNumber, $key));
          }
          if ($key == 'dn') {
            if (!empty($entry)) {
              throw new LDIFImportException(sprintf(_('Error line %s, an entry bloc can only have one dn'), $lineNumber));
            }
            $entry['dn']  = $value;
            $entryStart   = $lineNumber;
          } elseif (empty($entry)) {
            throw new LDIFImportException(sprintf(_('Error line %s, an entry bloc should start with the dn'), $lineNumber));
          } else {
            if (!isset($entry[$key])) {
              $entry[$key] = array();
            }
            $entry[$key][] = $value;
          }
        }
        /* Start new line */
        $line = trim($fileLine);
        if ($line == '') {
          if (!empty($entry)) {
            /* Entry is finished */
            $entries[$entryStart] = $entry;
          }
          /* Start a new entry */
          $entry      = array();
          $entryStart = -1;
          $line       = NULL;
        }
      }
    }

    foreach ($entries as $startLine => $entry) {
      /* Delete before insert */
      $usermdir = ($this->dn_exists($entry['dn']) && $DeleteOldEntries);
      /* Should we use Modify instead of Add */
      $usemodify = ($this->dn_exists($entry['dn']) && $JustModify);

      /* If we can't Import, return with a file error */
      if (!$this->import_single_entry($srp, $entry, $usemodify, $usermdir)) {
        throw new LDIFImportException(sprintf(_('Error while importing dn: "%s", please check your LDIF from line %s on!'), $entry['dn'][0], $startLine));
      }
    }
  }

  /*! \brief Function to Imports a single entry
   *
   * If $delete is TRUE;  The old entry will be deleted if it exists.
   * if $modify is TRUE;  All variables that are not touched by the new ldif will be kept.
   * if $modify is FALSE; The new ldif overwrites the old entry, and all untouched attributes get lost.
   *
   * \param integer $srp
   *
   * \param array $data
   *
   * \param boolean $modify
   *
   * \param boolean $delete
   */
  protected function import_single_entry($srp, $data, $modify, $delete)
  {
    global $config;

    if (!$config) {
      trigger_error("Can't import ldif, can't read config object.");
    }

    if ($this->reconnect) {
      $this->connect();
    }

    $ret  = FALSE;

    /* If dn is an index of data, we should try to insert the data */
    if (isset($data['dn'])) {
      /* Fix dn */
      $tmp = gosa_ldap_explode_dn($data['dn']);
      unset($tmp['count']);
      $dn = '';
      foreach ($tmp as $tm) {
        $dn .= trim($tm).',';
      }
      $dn = preg_replace('/,$/', '', $dn);
      unset($data['dn']);

      /* Creating Entry */
      $this->cd($dn);

      /* Delete existing entry */
      if ($delete) {
        $this->rmdir_recursive($srp, $dn);
      }

      /* Create missing trees */
      $this->cd($config->current['BASE']);
      $this->create_missing_trees($srp, preg_replace('/^[^,]+,/', '', $dn));
      $this->cd($dn);

      if (!$modify) {
        $this->cat($srp, $dn);
        if ($this->count($srp)) {
          /* The destination entry exists, overwrite it with the new entry */
          $attrs = $this->fetch($srp);
          foreach (array_keys($attrs) as $name) {
            if (!is_numeric($name)) {
              if (in_array($name, array('dn','count'))) {
                continue;
              }
              if (!isset($data[$name])) {
                $data[$name] = array();
              }
            }
          }
          $ret = $this->modify($data);
        } else {
          /* The destination entry doesn't exists, create it */
          $ret = $this->add($data);
        }
      } else {
        /* Keep all vars that aren't touched by this ldif */
        $ret = $this->modify($data);
      }
    }

    if (!$this->success()) {
      msg_dialog::display(_('LDAP error'), msgPool::ldaperror($this->get_error(), $dn, '', get_class()), LDAP_ERROR);
    }

    return $ret;
  }

  /*!
   * \brief Get the object classes
   *
   * \param boolean $force_reload FALSE
   */
  function get_objectclasses($force_reload = FALSE)
  {
    $objectclasses = array();

    /* Return the cached results. */
    if (class_available('session') && session::global_is_set('LDAP_CACHE::get_objectclasses') && !$force_reload) {
      $objectclasses = session::global_get('LDAP_CACHE::get_objectclasses');
      return $objectclasses;
    }

    // Get base to look for schema
    $sr   = @ldap_read($this->cid, '', 'objectClass=*', array('subschemaSubentry'));
    $attr = @ldap_get_entries($this->cid, $sr);
    if (!isset($attr[0]['subschemasubentry'][0])) {
      return array();
    }

    /* Get list of objectclasses and fill array */
    $nb = $attr[0]['subschemasubentry'][0];
    $objectclasses = array();
    $sr = ldap_read ($this->cid, $nb, 'objectClass=*', array('objectclasses'));
    $attrs = ldap_get_entries($this->cid, $sr);
    if (!isset($attrs[0])) {
      return array();
    }
    foreach ($attrs[0]['objectclasses'] as $val) {
      if (preg_match('/^[0-9]+$/', $val)) {
        continue;
      }
      $name = "OID";
      $pattern = explode(' ', $val);
      $ocname = preg_replace("/^.* NAME\s+\(*\s*'([^']+)'\s*\)*.*$/", '\\1', $val);
      $objectclasses[$ocname] = array();

      foreach ($pattern as $chunk) {
        switch ($chunk) {

          case '(':
            $value = '';
            break;

          case ')':
            if ($name != '') {
              $v = $this->value2container($value);
              if (in_array($name, array('MUST', 'MAY')) && !is_array($v)) {
                $v = array($v);
              }
              $objectclasses[$ocname][$name] = $v;
            }
            $name   = '';
            $value  = '';
            break;

          case 'NAME':
          case 'DESC':
          case 'SUP':
          case 'STRUCTURAL':
          case 'ABSTRACT':
          case 'AUXILIARY':
          case 'MUST':
          case 'MAY':
            if ($name != '') {
              $v = $this->value2container($value);
              if (in_array($name, array('MUST','MAY')) && !is_array($v)) {
                $v = array($v);
              }
              $objectclasses[$ocname][$name] = $v;
            }
            $name   = $chunk;
            $value  = '';
            break;

          default:  $value .= $chunk.' ';
        }
      }

    }
    if (class_available('session')) {
      session::global_set('LDAP_CACHE::get_objectclasses', $objectclasses);
    }

    return $objectclasses;
  }


  function value2container($value)
  {
    /* Set emtpy values to "TRUE" only */
    if (preg_match('/^\s*$/', $value)) {
      return TRUE;
    }

    /* Remove ' and " if needed */
    $value = preg_replace('/^[\'"]/', '', $value);
    $value = preg_replace('/[\'"] *$/', '', $value);

    /* Convert to array if $ is inside... */
    if (preg_match('/\$/', $value)) {
      $container = preg_split('/\s*\$\s*/', $value);
    } else {
      $container = chop($value);
    }

    return $container;
  }

  /*!
   * \brief Add a string in log file
   *
   * \param stri if(ng $string
   */
  function log($string)
  {
    if (session::global_is_set('config')) {
      $cfg = session::global_get('config');
      if (isset($cfg->current['LDAPSTATS']) && preg_match('/true/i', $cfg->current['LDAPSTATS'])) {
        syslog (LOG_INFO, $string);
      }
    }
  }

  /* added by Guido Serra aka Zeph <zeph@purotesto.it> */

  /*!
   * \brief Function to get cn
   *
   * \param $dn The DN
   */
  function getCn($dn)
  {
    $simple = explode(",", $dn);

    foreach ($simple as $piece) {
      $partial = explode("=", $piece);

      if ($partial[0] == "cn") {
        return $partial[1];
      }
    }
  }


  function get_naming_contexts($server, $admin = "", $password = "")
  {
    /* Build LDAP connection */
    $ds = ldap_connect ($server);
    if (!$ds) {
      die ("Can't bind to LDAP. No check possible!");
    }
    ldap_set_option($ds, LDAP_OPT_PROTOCOL_VERSION, 3);
    ldap_bind ($ds, $admin, $password);

    /* Get base to look for naming contexts */
    $sr   = @ldap_read ($ds, "", "objectClass=*", array("+"));
    $attr = @ldap_get_entries($ds, $sr);

    return $attr[0]['namingcontexts'];
  }


  function get_root_dse($server, $admin = "", $password = "")
  {
    /* Build LDAP connection */
    $ds = ldap_connect ($server);
    if (!$ds) {
      die ("Can't bind to LDAP. No check possible!");
    }
    ldap_set_option($ds, LDAP_OPT_PROTOCOL_VERSION, 3);
    ldap_bind ($ds, $admin, $password);

    /* Get base to look for naming contexts */
    $sr   = @ldap_read ($ds, "", "objectClass=*", array("+"));
    $attr = @ldap_get_entries($ds, $sr);

    /* Return empty array, if nothing was set */
    if (!isset($attr[0])) {
      return array();
    }

    /* Rework array... */
    $result = array();
    for ($i = 0; $i < $attr[0]['count']; $i++) {
      $result[$attr[0][$i]] = $attr[0][$attr[0][$i]];
      unset($result[$attr[0][$i]]['count']);
    }

    return $result;
  }

}
?>
