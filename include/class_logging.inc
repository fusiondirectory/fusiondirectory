<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007  Fabian Hickert
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
 * \file class_logging.inc
 * Source code for the class logging
 */

/*!
 * \brief This is the base class for the FusionDirectory logging functionality.
 * All logging should lead to this class.
 *
 * \author  Fabian Hickert <hickert@gonicus.de>
 * \version 2.6
 * \date    11.04.2007
 */
class logging
{
  static $validActions = ['modify','create','remove','copy','snapshot','security','error'];

  /*!
   * \brief logging method
   *
   * \param string $action      One of these values (modify|create|remove|copy|snapshot|security|error)
   *
   * \param string $objecttype  represents the current edited objecttype, like user/user, or the event, like logout
   *
   * \param string $object      represents the current edited object dn, or the target of the operation
   *
   * \param array $changes      An array containing names of all touched attributes
   *
   * \param string $result      A status message, containing errors or success messages
   */
  static function log (string $action, string $objecttype, string $object, array $changes = [], string $result = '')
  {
    global $config, $ui;

    /* Create data object */
    $entry = [
      'timestamp'   => microtime(TRUE),
      'action'      => $action,
      'objecttype'  => $objecttype,
      'object'      => $object,
      'changes'     => $changes,
      'result'      => $result
    ];
    if (isset($ui->dn) && !empty($ui->dn)) {
      $entry['user']  = $ui->dn;
    } elseif (isset($_SERVER['REMOTE_ADDR'])) {
      $entry['user'] = $_SERVER['REMOTE_ADDR'];
    } else {
      $entry['user'] = 'unknown';
    }

    /* Check if all given values are valid */
    $msgs = static::check($entry);
    if (count($msgs)) {
      foreach ($msgs as $msg) {
        trigger_error('Logging failed, reason was: '.$msg);
        msg_dialog::display(_('Internal error'), sprintf(_('Logging failed: %s'), $msg), ERROR_DIALOG);
      }
    } else {
      if (is_object($config) && preg_match('/true/i', $config->get_cfg_value('logging', ''))) {
        static::log_into_syslog($entry);
        if (in_array($action, $config->get_cfg_value('auditActions', []))) {
          static::log_into_ldap($entry);
        }
      }
    }
  }

  /*!
   * \brief Debug output method
   *
   * Print a DEBUG level if specified debug level of the level matches the
   * the configured debug level.
   *
   * \param int $level        The log level of the message (one of the DEBUG_* constants)
   *
   * \param int $line         Line of origin (using __LINE__ is common)
   *
   * \param string $function  Function of origin (using __FUNCTION__ is common)
   *
   * \param string $file      File of origin (using __FILE__ is common)
   *
   * \param mixed $data       Operation result. Can be a message or an array, which is printed with print_a.
   *
   * \param string $info      Operation description
   */
  static function debug (int $level, int $line, string $function, string $file, $data, string $info = '')
  {
    global $config;

    static $first = TRUE;
    if (($_SERVER['REQUEST_METHOD'] == 'POST') && preg_match('/index.php$/', $_SERVER['REQUEST_URI'])) {
      return;
    }

    if (session::get('DEBUGLEVEL') & $level) {
      if ($first) {
        echo '<div id="debug-handling" class="notice">'.
              '<img src="geticon.php?context=status&amp;icon=dialog-information&amp;size=22" alt="Information" style="vertical-align:middle;margin-right:.2em;"/>'.
              'There is some debug output '.
              '<button onClick="javascript:$$(\'div.debug_div\').each(function (a) { a.toggle(); });">Toggle</button>'.
            '</div>';
        $first = FALSE;
      }
      $output = "DEBUG[$level] ";
      if ($function != '') {
        $output .= "($file:$function():$line) - $info: ";
      } else {
        $output .= "($file:$line) - $info: ";
      }
      echo '<div class="debug_div">';
      echo $output;
      $logline = $output;
      if (is_array($data)) {
        print_a($data);
        $logline .= print_r($data, TRUE);
      } else {
        echo "'$data'";
        $logline .= "'$data'";
      }
      echo "</div>\n";

      if (is_object($config) && preg_match('/true/i', $config->get_cfg_value('debugLogging', ''))) {
        fusiondirectory_log($logline);
      }
    }
  }

  /*!
   * \brief Check the options
   *
   * \param Array $entry to be checked
   */
  static protected function check ($entry = [])
  {
    $msgs = [];

    if (!isset($entry['action']) || !in_array($entry['action'], static::$validActions)) {
      $msgs[] = sprintf(_('Invalid option "%s" specified!'), $entry['action']);
    }

    if (!isset($entry['objecttype']) || empty($entry['objecttype'])) {
      $msgs[] = _('Specified objectType is empty or invalid!');
    }

    return $msgs;
  }

  /*
   * \brief This function is used to into the systems syslog
   *
   * \param Array $entry Entry to be loged
   */
  static protected function log_into_syslog ($entry)
  {
    $str = '';
    if (empty($entry['object']) && empty($entry['changes'])) {
      $str = '('.$entry['action'].') '.$entry['objecttype'].': '.$entry['result'];
    } else {
      $str = '('.$entry['action'].') '.$entry['object'].' of type '.$entry['objecttype'].' '.implode(',', $entry['changes']).': '.$entry['result'];
    }
    fusiondirectory_log($str);
  }

  /*
   * \brief This function is used to into the ldap for audit plugin
   *
   * \param Array $entry Entry to be loged
   */
  static protected function log_into_ldap ($entry)
  {
    global $config;
    if ($entry['objecttype'] == 'plugin/auditEvent') {
      /* Avoid infinite loop */
      return;
    }
    if (empty($entry['object'])) {
      $entry['object'] = 'none';
    }
    try {
      $tabObject = objects::create('auditEvent');
      $baseObject = $tabObject->getBaseObject();
      $baseObject->fdAuditDateTime    = DateTime::createFromFormat('U.u', number_format($entry['timestamp'], 6, '.', ''));
      $baseObject->fdAuditAction      = $entry['action'];
      $baseObject->fdAuditAuthorDN    = $entry['user'];
      $baseObject->fdAuditObjectType  = $entry['objecttype'];
      $baseObject->fdAuditObject      = $entry['object'];
      $baseObject->fdAuditAttributes  = $entry['changes'];
      $baseObject->fdAuditResult      = $entry['result'];
      $baseObject->base               = $config->current['BASE'];
      $errors = $tabObject->save();
      if (!empty($errors)) {
        msg_dialog::displayChecks($errors);
      }
    } catch (FusionDirectoryException $e) {
      msg_dialog::display(_('Error'), sprintf(_('Failed to log event (%s - %s): %s'), $entry['action'], $entry['objecttype'], $e->getMessage()), ERROR_DIALOG);
    }
  }
}
