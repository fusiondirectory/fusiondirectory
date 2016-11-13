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
class logging {
  static $validActions = array('modify','create','remove','copy','snapshot','view','security','debug');

  /*!
   * \brief logging method
   *
   * \param $action         One of these values (modify|create|remove|snapshot|copy|view|security|debug)
   *
   * \param $objecttype     represents the current edited objecttype, like user/user
   *
   * \param $object         represents the current edited object dn
   *
   * \param $changes_array  An array containing names of all touched attributes
   *
   * \param $result         A status message, containing errors or success messages
   */
  static function log ($action, $objecttype, $object, $changes_array = array(), $result = '')
  {
    global $config, $ui;
    if (!is_array($changes_array)) {
      trigger_error('log(string,string,string,array(),bool). Forth parameter must be an array.');
      $changes_array = array();
    }

    /* Create data object */
    $entry = array(
      'timestamp'   => time(),
      'action'      => $action,
      'objecttype'  => $objecttype,
      'object'      => $object,
      'changes'     => $changes_array,
      'result'      => $result
    );
    if (isset($ui->dn) && !empty($ui->dn)) {
      $entry['user']  = $ui->dn;
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
        if (in_array($action, $config->get_cfg_value('auditActions', array()))) {
          static::log_into_ldap($entry);
        }
      }
    }
  }

  /*!
   * \brief Check the options
   *
   * \param Array $entry to be checked
   */
  static protected function check($entry = array())
  {
    $msgs = array();

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
  static protected function log_into_syslog($entry)
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
  static protected function log_into_ldap($entry)
  {
    global $config;
    if ($entry['objecttype'] == 'plugin/auditEvent') {
      // Avoid infinite loop
      return;
    }
    $tabObject = objects::create('auditEvent');
    $baseObject = $tabObject->getBaseObject();
    $baseObject->fdAuditDateTime    = new DateTime('@'.$entry['timestamp']);
    $baseObject->fdAuditAction      = $entry['action'];
    $baseObject->fdAuditAuthorDN    = $entry['user'];
    $baseObject->fdAuditObjectType  = $entry['objecttype'];
    $baseObject->fdAuditObject      = $entry['object'];
    $baseObject->fdAuditAttributes  = $entry['changes'];
    $baseObject->fdAuditResult      = $entry['result'];
    $baseObject->base               = $config->current['BASE'];
    $message = $tabObject->check();
    if (count($message) == 0) {
      $tabObject->save();
    } else {
      msg_dialog::displayChecks($message);
    }
  }
}
?>
