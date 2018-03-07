<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2018  FusionDirectory

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

class LockAction extends Action
{
  function __construct($name, $targets, $callable, array $acl = array(), $inmenu = TRUE, $inline = TRUE)
  {
    parent::__construct(
      $name,
      array('lock' => _('Lock users'), 'unlock' => _('Unlock users')),
      array(
        'lock'    => 'geticon.php?context=status&icon=object-locked&size=16',
        'unlock'  => 'geticon.php?context=status&icon=object-unlocked&size=16',
      ),
      $targets, $callable, $acl, $inmenu, $inline);
  }

  function isLocked(ListingEntry $entry)
  {
    if (isset($entry['userPassword']) && preg_match('/^\{[^\}]/', $entry['userPassword'])) {
      if (preg_match('/^[^\}]*+\}!/', $entry['userPassword'])) {
        return TRUE;
      } else {
        return FALSE;
      }
    }
    return NULL;
  }

  function fillMenuItems(&$actions)
  {
    if (!$this->inmenu) {
      return;
    }

    $actions[] = array(
      'name'      => $this->name.'_lock',
      'icon'      => $this->icon['lock'],
      'label'     => $this->label['lock'],
      'enabled'   => $this->isEnabledFor(),
      'separator' => $this->separator,
    );

    $actions[] = array(
      'name'      => $this->name.'_unlock',
      'icon'      => $this->icon['unlock'],
      'label'     => $this->label['unlock'],
      'enabled'   => $this->isEnabledFor(),
      'separator' => FALSE,
    );
  }

  function getRowClasses(ListingEntry $entry)
  {
    $lockStatus = $this->isLocked($entry);
    if ($lockStatus === TRUE) {
      return array('entry-locked');
    } else {
      return array();
    }
  }

  function renderColumnIcons(ListingEntry $entry)
  {
    if (!$this->inline) {
      return '';
    }

    // Skip the entry completely if there's no permission to execute it
    if (!$this->hasPermission($entry->dn, $entry->getTemplatedType(), $entry->isTemplate())) {
      return '<img src="images/empty.png" alt=" " class="center optional"/>';
    }

    $lockStatus = $this->isLocked($entry);
    if ($lockStatus === NULL) {
      return '<img src="images/empty.png" alt=" " class="center optional"/>';
    } elseif ($lockStatus) {
      // Render
      return '<input type="image" src="'.htmlentities($this->icon['lock'], ENT_COMPAT, 'UTF-8').'"'.
              ' title="'.$this->label['unlock'].'" alt="'.$this->label['unlock'].'" name="listing_'.$this->name.'_unlock_'.$entry->row.'"/>';
    } else {
      return '<input type="image" src="'.htmlentities($this->icon['unlock'], ENT_COMPAT, 'UTF-8').'"'.
              ' title="'.$this->label['lock'].'" alt="'.$this->label['lock'].'" name="listing_'.$this->name.'_lock_'.$entry->row.'"/>';
    }
  }
}

class userManagement extends management
{
  public $neededAttrs = array('userPassword' => '1');

  public static $skipTemplates = FALSE;

  public static $columns = array(
    array('ObjectTypeColumn', array()),
    array('LinkColumn',       array('attribute' => 'sn',        'label' => 'Last name')),
    array('LinkColumn',       array('attribute' => 'givenName', 'label' => 'First name')),
    array('Column',           array('attribute' => 'uid',       'label' => 'Login')),
    array('PropertiesColumn', array('label' => 'Properties')),
    array('ActionsColumn',    array('label' => 'Actions')),
  );

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Users'),
      'plTitle'       => _('Manage users'),
      'plDescription' => _('Manage user accounts and their properties'),
      'plIcon'        => 'geticon.php?context=types&icon=user&size=48',
      'plSection'     => 'accounts',
      'plManages'     => array('user'),
      'plPriority'    => 10,

      'plProvidedAcls' => array()
    );
  }


  protected function configureActions()
  {
    parent::configureActions();

    $this->registerAction(
      new LockAction(
        'lockUsers',
        '+', 'lockUsers',
        array('userPassword:rw'), TRUE, TRUE
      )
    );
    $this->actions['lockUsers']->setSeparator(TRUE);
  }

  /* !\brief  Lock/unlock multiple users.
   */
  function lockUsers(array $action)
  {
    global $config, $ui;
    @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action, 'Lock');

    // Filter out entries we are not allowed to modify
    $disallowed = array();
    $allowed    = array();
    foreach ($action['targets'] as $dn) {
      if (strpos($ui->get_permissions($dn, 'user/user', 'userLock'), 'w') === FALSE) {
        $disallowed[] = $dn;
      } else {
        $allowed[] = $dn;
      }
    }
    if (count($disallowed)) {
      msg_dialog::display(_('Permission'), msgPool::permModify($disallowed), INFO_DIALOG);
    }

    // Try to lock/unlock the rest of the entries.
    foreach ($allowed as $dn) {
      // We can't lock empty passwords.
      $entry = $this->listing->getEntry($dn);
      if (!isset($entry['userPassword'])) {
        continue;
      }

      // Detect the password method and try to lock/unlock.
      $pwd      = $entry['userPassword'];
      $method   = passwordMethod::get_method($pwd, $dn);
      $success  = TRUE;
      if ($method instanceOf passwordMethod) {
        if (!$method->is_lockable()) {
          $hn = $method->get_hash_name();
          if (is_array($hn)) {
            $hn = $hn[0];
          }
          msg_dialog::display(_('Account locking'),
              sprintf(_('Password method "%s" does not support locking. Account "%s" has not been locked!'),
                $hn, $dn), ERROR_DIALOG);
          return;
        }
        if (($action['subaction'] == 'lock') && !$method->is_locked($dn)) {
          $success = $method->lock_account($dn);
        } elseif (($action['subaction'] == 'unlock') && $method->is_locked($dn)) {
          $success = $method->unlock_account($dn);
        }

        // Check if everything went fine.
        if (!$success) {
          $hn = $method->get_hash_name();
          if (is_array($hn)) {
            $hn = $hn[0];
          }
          msg_dialog::display(_('Account locking'),
              sprintf(_('Locking failed using password method "%s". Account "%s" has not been locked!'),
                $hn, $dn), ERROR_DIALOG);
        }
      }
    }
  }
}
?>
