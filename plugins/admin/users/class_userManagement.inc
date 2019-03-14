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

class userManagement extends management
{
  public $neededAttrs = ['userPassword' => '1'];

  public static $skipTemplates = FALSE;

  public static $columns = [
    ['ObjectTypeColumn', []],
    ['LinkColumn',       ['attributes' => 'sn',        'label' => 'Last name']],
    ['LinkColumn',       ['attributes' => 'givenName', 'label' => 'First name']],
    ['Column',           ['attributes' => 'uid',       'label' => 'Login']],
    ['PropertiesColumn', ['label' => 'Properties']],
    ['ActionsColumn',    ['label' => 'Actions']],
  ];

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Users'),
      'plTitle'       => _('Manage users'),
      'plDescription' => _('Manage user accounts and their properties'),
      'plIcon'        => 'geticon.php?context=types&icon=user&size=48',
      'plSection'     => 'accounts',
      'plManages'     => ['user'],
      'plPriority'    => 10,

      'plProvidedAcls' => []
    ];
  }


  protected function configureActions ()
  {
    parent::configureActions();

    $this->registerAction(
      new LockAction(
        'lockUsers',
        '+', 'lockUsers',
        ['userPassword:rw'], TRUE, TRUE
      )
    );
    $this->actions['lockUsers']->setSeparator(TRUE);
  }

  /* !\brief  Lock/unlock multiple users.
   */
  function lockUsers (array $action)
  {
    global $config, $ui;
    @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $action, 'Lock');

    // Filter out entries we are not allowed to modify
    $disallowed = [];
    $allowed    = [];
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

      // Detect the password method and try to lock/unlock.
      static::lockUser($action['subaction'], $entry['userPassword'], $dn);
    }
  }

  /* !\brief  Lock/unlock a user
   *
   * \param string $action  'lock' or 'unlock'
   * \param string $pwd     userPassword value
   * \param string $dn      dn of the LDAP node
   */
  static function lockUser (string $action, string $pwd, string $dn)
  {
    $method = passwordMethod::get_method($pwd, $dn);
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

      $success = TRUE;
      if (($action == 'lock') && !$method->is_locked($dn)) {
        $success = $method->lock_account($dn);
      } elseif (($action == 'unlock') && $method->is_locked($dn)) {
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
