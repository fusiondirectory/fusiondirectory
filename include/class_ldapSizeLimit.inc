<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018  FusionDirectory

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
 * \file class_ldapSizeLimit.inc
 * Source code for the class ldapSizeLimit
 */

/*!
 * \brief Class ldapSizeLimit
 * This class contains all informations and functions to handle the LDAP size limit dialogs, configuration and bypass
 */

class ldapSizeLimit
{
  /*! \brief Current size limit */
  protected $sizeLimit;

  /*! \brief Ignore dialogs */
  protected $ignore;

  /*! \brief Limit was exceeded */
  protected $limitExceeded;

  function __construct ()
  {
    global $config;

    $this->sizeLimit  = $config->get_cfg_value('LDAPSIZELIMIT', 200);
    $this->ignore     = preg_match('/true/i', $config->get_cfg_value('LDAPSIZEIGNORE', 'TRUE'));
  }

  function getSizeLimit ()
  {
    return $this->sizeLimit;
  }

  function setSizeLimit ($limit)
  {
    $this->sizeLimit = $limit;
  }

  function setLimitExceeded ($exceeded = TRUE)
  {
    $this->limitExceeded = $exceeded;
  }

  /*!
   * \brief Handle sizelimit dialog related posts
   */
  function update ()
  {
    if (isset($_POST['set_size_action']) && isset($_POST['action'])) {
      switch ($_POST['action']) {
        case 'newlimit':
          if (isset($_POST['new_limit']) && tests::is_id($_POST['new_limit'])) {
            if (($error = static::checkMaxInputVars($_POST['new_limit'])) !== FALSE) {
              $error->display();
            } else {
              $this->sizeLimit  = intval($_POST['new_limit']);
              $this->ignore     = FALSE;
            }
          }
          break;
        case 'ignore':
          $this->sizeLimit  = 0;
          $this->ignore     = TRUE;
          break;
        case 'limited':
          $this->ignore     = TRUE;
          break;
        default:
          break;
      }
    }

    /* Allow fallback to dialog */
    if (isset($_POST['edit_sizelimit'])) {
      $this->ignore = FALSE;
    }
  }

  /*!
   * \brief Show sizelimit configuration dialog
   *
   * Show sizelimit configuration dialog when number
   * of entries exceeded the sizelimit
   */
  function check ()
  {
    global $config;

    /* Ignore dialog? */
    if ($this->ignore) {
      return '';
    }

    /* Eventually show dialog */
    if ($this->limitExceeded) {
      $smarty = get_smarty();
      $smarty->assign('sizelimit', $this->sizeLimit);
      return $smarty->fetch(get_template_path('sizelimit.tpl'));
    }

    return '';
  }

  /*!
   * \brief Print a sizelimit warning
   *
   * Print a sizelimit warning when number
   * of entries exceeded the sizelimit
   */
  function renderWarning ()
  {
    if ($this->limitExceeded) {
      $config = '<input type="submit" name="edit_sizelimit" value="'.htmlescape(_('Configure')).'" formnovalidate="formnovalidate"/>';
      return '('.htmlescape(_('incomplete')).') '.$config;
    }
    return '';
  }

  /**
   * Checks if a new limit or a number of entries is too high regarding to max_input_vars.
   *
   * If there are more items in $_POST than max_input_vars, PHP will discard some of them and will cause a CSRF error.
   */
  static public function checkMaxInputVars (int $newLimit, string $messageTemplate = NULL)
  {
    $maxInputVars = ini_get('max_input_vars');

    if (($maxInputVars != '') && (($newLimit + 10) >= intval($maxInputVars))) {
      return new FusionDirectoryError(
        htmlescape(sprintf(
          $messageTemplate ?? _('Limit %d is greater than or too close to configured max_input_vars PHP ini setting of %d. Please change max_input_vars ini setting to a higher value if you wish to set the limit higher.'),
          $newLimit,
          $maxInputVars
        ))
      );
    }

    return FALSE;
  }
}
