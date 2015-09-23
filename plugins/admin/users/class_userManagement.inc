<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
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

class userManagement extends simpleManagement
{
  protected $objectTypes  = array('user');

  protected $departmentBrowser      = TRUE;
  protected $departmentRootVisible  = FALSE;
  protected $baseMode               = TRUE;
  protected $multiSelect            = TRUE;

  protected $filterXMLPath    = NULL;
  protected $listXMLPath      = NULL;
  protected $autoFilter       = FALSE;
  protected $autoActions      = FALSE;

  protected $skipTemplates = FALSE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Users'),
      'plDescription' => _('Manage users'),
      'plIcon'        => 'geticon.php?context=types&icon=user&size=48',
      'plSection'     => 'accounts',
      'plManages'     => array('user'),
      'plPriority'    => 10,

      'plProvidedAcls' => array()
    );
  }

  function __construct($config, $ui)
  {
    $this->filterXMLPath  = get_template_path('user-filter.xml',  TRUE, dirname(__FILE__));
    $this->listXMLPath    = get_template_path('user-list.xml',    TRUE, dirname(__FILE__));

    parent::__construct($config, $ui);
  }

  function configureHeadpage ()
  {
    parent::configureHeadpage();
    $this->registerAction('new_user',               'newEntry');
    $this->registerAction('new_template_user',      'newEntryTemplate');
    $this->registerAction('template_apply_to_user', 'applyTemplateToEntry');
    $this->registerAction('template_apply_user',    'newEntryFromTemplate');

    $this->registerAction('lock',         'lockEntry');
    $this->registerAction('lockUsers',    'lockUsers');
    $this->registerAction('unlockUsers',  'lockUsers');

    $this->headpage->registerElementFilter('lockLabel',         'userManagement::filterLockLabel');
    $this->headpage->registerElementFilter('lockImage',         'userManagement::filterLockImage');
    $this->headpage->registerElementFilter('filterProperties',  'userManagement::filterProperties');
  }

  function configureFilter ()
  {
    parent::configureFilter();

    if (!class_available('mailAccount')) {
      /* If mail plugin is not installed, ignore mail filter */
      $this->filter->elements['MAIL']['unset']      = '';
      $this->filter->elements['MAIL']['set']        = '';
      /* The FUNCTIONAL filter must not use inexisting gosaMailAccount class */
      $this->filter->elements['FUNCTIONAL']['set']  = '(!(|(objectClass=posixAccount)(objectClass=sambaSamAccount)))';
    }
  }

  function renderList ()
  {
    $smarty = get_smarty();
    $smarty->assign('USE_MAIL', class_available('mailAccount'));
    return parent::renderList();
  }

  /* !\brief  Lock/unlock multiple users.
   */
  function lockUsers($action, $target, $all)
  {
    if (!count($target)) {
      return;
    }
    if ($action == 'lockUsers') {
      $this->lockEntry($action, $target, $all, 'lock');
    } else {
      $this->lockEntry($action, $target, $all, 'unlock');
    }
  }


  /* !\brief  Locks/unlocks the given user(s).
   */
  function lockEntry($action, $entry, $all, $type = 'toggle')
  {
    // Filter out entries we are not allowed to modify
    $disallowed = array();
    $dns        = array();
    foreach ($entry as $dn) {
      if (!preg_match('/w/', $this->ui->get_permissions($dn, 'user/password'))) {
        $disallowed[] = $dn;
      } else {
        $allowed[] = $dn;
      }
    }
    if (count($disallowed)) {
      msg_dialog::display(_('Permission'), msgPool::permDelete($disallowed), INFO_DIALOG);
    }

    // Try to lock/unlock the rest of the entries.
    $ldap = $this->config->get_ldap_link();
    foreach ($allowed as $dn) {
      $ldap->cat($dn, array('userPassword'));
      if ($ldap->count() == 1) {

        // We can't lock empty passwords.
        $val = $ldap->fetch();
        if (!isset($val['userPassword'])) {
          continue;
        }

        // Detect the password method and try to lock/unlock.
        $pwd      = $val['userPassword'][0];
        $method   = passwordMethod::get_method($pwd, $val['dn']);
        $success  = TRUE;
        if ($method instanceOf passwordMethod) {
          if ($type == 'toggle') {
            if ($method->is_locked($this->config, $val['dn'])) {
              $success = $method->unlock_account($this->config, $val['dn']);
            } else {
              $success = $method->lock_account($this->config, $val['dn']);
            }
          } elseif ($type == 'lock' && !$method->is_locked($this->config, $val['dn'])) {
            $success = $method->lock_account($this->config, $val['dn']);
          } elseif ($type == 'unlock' && $method->is_locked($this->config, $val['dn'])) {
            $success = $method->unlock_account($this->config, $val['dn']);
          }

          // Check if everything went fine.
          if (!$success) {
            $hn = $method->get_hash_name();
            if (is_array($hn)) {
              $hn = $hn[0];
            }
            msg_dialog::display(_('Account locking'),
                sprintf(_('Password method "%s" does not support locking. Account "%s" has not been locked!'),
                  $hn, $dn), WARNING_DIALOG);
          }
        } else {
          // Can't lock unknown methods.
        }
      }
    }
  }

  static function filterLockImage($userPassword)
  {
    $image = 'images/empty.png';
    if (isset($userPassword[0]) && preg_match('/^\{[^\}]/', $userPassword[0])) {
      if (preg_match('/^[^\}]*+\}!/', $userPassword[0])) {
        $image = 'geticon.php?context=status&icon=object-locked&size=16';
      } else {
        $image = 'geticon.php?context=status&icon=object-unlocked&size=16';
      }
    }
    return $image;
  }

  static function filterLockLabel($userPassword)
  {
    $label = '';
    if (isset($userPassword[0]) && preg_match('/^\{[^\}]/', $userPassword[0])) {
      if (preg_match('/^[^\}]*+\}!/', $userPassword[0])) {
        $label = _('Unlock account').'<rowClass:entry-locked/>';
      } else {
        $label = _('Lock account');
      }
    }
    return $label;
  }

  static function filterProperties($row, $dn)
  {
    global $config;
    static $usertabs = array();
    if (empty($usertabs)) {
      foreach ($config->data['TABS']['USERTABS'] as $plug) {
        if ($plug['CLASS'] == 'user') {
          continue;
        }
        if (class_available($plug['CLASS'])) {
          $name = $plug['CLASS'];

          $usertabs[$name] = new $name($config, $dn);
        }
      }
    }

    // Load information if needed
    $ldap = $config->get_ldap_link();
    $ldap->cat($dn);
    $result = '<input class="center" type="image" src="geticon.php?context=applications&amp;icon=user-info&amp;size=16" '.
                       'alt="'._('User account').'" title="'._('User account information').'" '.
                       'name="listing_edit_tab_user_'.$row.'"/>';
    if ($attrs = $ldap->fetch()) {
      if (in_array_ics ('fdTemplate', $attrs['objectClass'])) {
        $attrs = plugin::tpl_template_to_attrs($attrs);
      }
      foreach ($usertabs as $class => $usertab) {
        if ($usertab->is_this_account($attrs)) {
          $infos = pluglist::pluginInfos($class);
          if (isset($infos['plSmallIcon'])) {
            $result .= '<input class="center" type="image" src="'.htmlentities($infos['plSmallIcon'], ENT_COMPAT, 'UTF-8').'" '.
                       'alt="'.$infos['plShortName'].'" title="'.$infos['plShortName'].'" '.
                       'name="listing_edit_tab_'.$class.'_'.$row.'"/>';
          } else {
            @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $infos['plShortName']." ($class)", "No icon for");
          }
        } else {
          $result .= '<img src="images/empty.png" alt="" class="center optional '.$class.'"/>';
        }
      }
    }

    return $result;
  }
}
?>
