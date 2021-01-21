<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2020  FusionDirectory

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

class Lock
{
  public $dn;
  public $objectDn;
  public $userDn;
  public $timestamp;

  public function __construct (string $dn, string $objectDn, string $userDn, DateTime $timestamp)
  {
    $this->dn         = $dn;
    $this->objectDn   = $objectDn;
    $this->userDn     = $userDn;
    $this->timestamp  = $timestamp;
  }

  /*!
   *  \brief Add a lock for object(s)
   *
   * Adds a lock by the specified user for one ore multiple objects.
   * If a lock for that object already exists from another user, an error is triggered.
   *
   * \param array $object The object or array of objects to lock
   *
   * \param string $user  The user who shall own the lock
   */
  public static function add ($object, string $user = NULL)
  {
    global $config, $ui;

    /* Remember which entries were opened as read only, because we
        don't need to remove any locks for them later */
    if (!session::is_set('LOCK_CACHE')) {
      session::set('LOCK_CACHE', ['']);
    }
    if (is_array($object)) {
      foreach ($object as $obj) {
        static::add($obj, $user);
      }
      return;
    }

    if ($user === NULL) {
      $user = $ui->dn;
    }

    $cache = &session::get_ref('LOCK_CACHE');
    if (isset($_POST['open_readonly'])) {
      $cache['READ_ONLY'][$object] = TRUE;
      return;
    }
    if (isset($cache['READ_ONLY'][$object])) {
      unset($cache['READ_ONLY'][$object]);
    }

    /* Just a sanity check... */
    if (empty($object) || empty($user)) {
      throw new FusionDirectoryError(htmlescape(_('Error while adding a lock. Contact the developers!')));
    }

    /* Check for existing entries in lock area */
    $ldap = $config->get_ldap_link();
    $ldap->cd(get_ou('lockRDN').get_ou('fusiondirectoryRDN').$config->current['BASE']);
    $ldap->search('(&(objectClass=fdLockEntry)(fdUserDn='.ldap_escape_f($user).')(fdObjectDn='.base64_encode($object).'))',
        ['fdUserDn']);
    if ($ldap->get_errno() == 32) {
      /* No such object, means the locking branch is missing, create it */
      $ldap->cd($config->current['BASE']);
      try {
        $ldap->create_missing_trees(get_ou('lockRDN').get_ou('fusiondirectoryRDN').$config->current['BASE']);
      } catch (FusionDirectoryError $error) {
        $error->display();
      }
      $ldap->cd(get_ou('lockRDN').get_ou('fusiondirectoryRDN').$config->current['BASE']);
      $ldap->search('(&(objectClass=fdLockEntry)(fdUserDn='.ldap_escape_f($user).')(fdObjectDn='.base64_encode($object).'))',
        ['fdUserDn']);
    }
    if (!$ldap->success()) {
      throw new FusionDirectoryError(
        sprintf(
          htmlescape(_('Cannot create locking information in LDAP tree. Please contact your administrator!')).
            '<br><br>'.htmlescape(_('LDAP server returned: %s')),
          '<br><br><i>'.htmlescape($ldap->get_error()).'</i>'
        )
      );
    }

    /* Add lock if none present */
    if ($ldap->count() == 0) {
      $attrs  = [];
      $name   = md5($object);
      $dn     = 'cn='.$name.','.get_ou('lockRDN').get_ou('fusiondirectoryRDN').$config->current['BASE'];
      $ldap->cd($dn);
      $attrs = [
        'objectClass'     => 'fdLockEntry',
        'cn'              => $name,
        'fdUserDn'        => $user,
        'fdObjectDn'      => base64_encode($object),
        'fdLockTimestamp' => LdapGeneralizedTime::toString(new DateTime('now')),
      ];
      $ldap->add($attrs);
      if (!$ldap->success()) {
        throw new FusionDirectoryLdapError($dn, LDAP_ADD, $ldap->get_error(), $ldap->get_errno());
      }
    }
  }

  /*!
   * \brief Remove a lock for object(s)
   *
   * Remove a lock for object(s)
   *
   * \param mixed $object object or array of objects for which a lock shall be removed
   */
  public static function deleteByObject ($object)
  {
    global $config;

    if (is_array($object)) {
      foreach ($object as $obj) {
        static::deleteByObject($obj);
      }
      return;
    }

    /* Sanity check */
    if ($object == '') {
      return;
    }

    /* If this object was opened in read only mode then
        skip removing the lock entry, there wasn't any lock created.
      */
    if (session::is_set('LOCK_CACHE')) {
      $cache = &session::get_ref('LOCK_CACHE');
      if (isset($cache['READ_ONLY'][$object])) {
        unset($cache['READ_ONLY'][$object]);
        return;
      }
    }

    /* Check for existance and remove the entry */
    $ldap = $config->get_ldap_link();
    $dn   = get_ou('lockRDN').get_ou('fusiondirectoryRDN').$config->current['BASE'];
    $ldap->cd($dn);
    $ldap->search('(&(objectClass=fdLockEntry)(fdObjectDn='.base64_encode($object).'))', ['fdObjectDn']);
    if (!$ldap->success()) {
      throw new FusionDirectoryLdapError($dn, LDAP_SEARCH, $ldap->get_error(), $ldap->get_errno());
    } elseif ($attrs = $ldap->fetch()) {
      $ldap->rmdir($attrs['dn']);
      if (!$ldap->success()) {
        throw new FusionDirectoryLdapError($attrs['dn'], LDAP_DEL, $ldap->get_error(), $ldap->get_errno());
      }
    }
  }

  /*!
   * \brief Remove all locks owned by a specific userdn
   *
   * For a given userdn remove all existing locks. This is usually
   * called on logout.
   *
   * \param string $userdn the subject whose locks shall be deleted
   */
  public static function deleteByUser (string $userdn)
  {
    global $config;

    /* Get LDAP ressources */
    $ldap = $config->get_ldap_link();
    $ldap->cd(get_ou('lockRDN').get_ou('fusiondirectoryRDN').$config->current['BASE']);

    /* Remove all objects of this user, drop errors silently in this case. */
    $ldap->search('(&(objectClass=fdLockEntry)(fdUserDn='.ldap_escape_f($userdn).'))', ['fdUserDn']);
    while ($attrs = $ldap->fetch()) {
      $ldap->rmdir($attrs['dn']);
    }
  }

  /*!
   * \brief Get locks for objects
   *
   * \param mixed $objects Array of dns for which a lock will be searched or dn of a single object
   *
   * \param boolean $allow_readonly TRUE if readonly access should be permitted,
   * FALSE if not (default).
   *
   * \return A numbered array containing all found locks as an array with key 'object'
   * and key 'user', or FALSE if an error occured.
   */
  public static function get ($objects, bool $allow_readonly = FALSE): array
  {
    global $config;

    if (is_array($objects) && (count($objects) == 1)) {
      $objects = reset($objects);
    }
    if (is_array($objects)) {
      if ($allow_readonly) {
        throw new FusionDirectoryException('Read only is not possible for several objects');
      }
      $filter = '(&(objectClass=fdLockEntry)(|';
      foreach ($objects as $obj) {
        $filter .= '(fdObjectDn='.base64_encode($obj).')';
      }
      $filter .= '))';
    } else {
      if ($allow_readonly && isset($_POST['open_readonly'])) {
        /* If readonly is allowed and asked and there is only one object, bypass lock detection */
        return [];
      }
      $filter = '(&(objectClass=fdLockEntry)(fdObjectDn='.base64_encode($objects).'))';
    }

    /* Get LDAP link, check for presence of the lock entry */
    $ldap = $config->get_ldap_link();
    $dn   = get_ou('lockRDN').get_ou('fusiondirectoryRDN').$config->current['BASE'];
    $ldap->cd($dn);
    $ldap->search($filter, ['fdUserDn','fdObjectDn', 'fdLockTimestamp']);
    if (!$ldap->success()) {
      throw new FusionDirectoryLdapError($dn, LDAP_SEARCH, $ldap->get_error(), $ldap->get_errno());
    }

    $locks = [];
    $sessionLifetime = $config->get_cfg_value('sessionLifetime', 1800);
    if ($sessionLifetime > 0) {
      $expirationDate = (new DateTime())->sub(new DateInterval('PT'.$sessionLifetime.'S'));
    }
    while ($attrs = $ldap->fetch()) {
      $date = LdapGeneralizedTime::fromString($attrs['fdLockTimestamp'][0]);
      if (isset($expirationDate) && ($date < $expirationDate)) {
        /* Delete expired locks */
        $ldap->rmdir($attrs['dn']);
      } else {
        $locks[] = new Lock(
          $attrs['dn'],
          base64_decode($attrs['fdObjectDn'][0]),
          $attrs['fdUserDn'][0],
          $date
        );
      }
    }

    if (!is_array($objects) && (count($locks) > 1)) {
      /* Hmm. We're removing broken LDAP information here and issue a warning. */
      $warning = new FusionDirectoryWarning(htmlescape(_('Found multiple locks for object to be locked. This should not happen - cleaning up multiple references.')));
      $warning->display();

      /* Clean up these references now... */
      foreach ($locks as $lock) {
        $ldap->rmdir($lock->dn);
      }

      return [];
    }

    return $locks;
  }


  /*!
   *  \brief Add a lock for object(s) or fail
   *
   * Adds a lock by the specified user for one ore multiple objects.
   * If the lock for that object already exists, waits a bit and retry.
   * If a lock cannot be set, throws.
   *
   * \param array|string $object The object or array of objects to lock
   *
   * \param string $user  The user who shall own the lock
   *
   * \param int $retries  how many times we can retry (waiting a second each time)
   */
  public static function addOrFail ($object, string $user = NULL, int $retries = 10)
  {
    $wait = $retries;
    while (!empty($locks = Lock::get($object))) {
      sleep(1);

      /* Oups - timed out */
      if ($wait-- == 0) {
        throw new FusionDirectoryException(_('Timeout while waiting for lock!'));
      }
    }
    Lock::add($object, $user);
  }

  /*!
   * \brief Generate a lock message
   *
   * This message shows a warning to the user, that a certain object is locked
   * and presents some choices how the user can proceed. By default this
   * is 'Cancel' or 'Edit anyway', but depending on the function call
   * its possible to allow readonly access, too.
   *
   * Example usage:
   * \code
   * if ($locks = Lock::get($this->dn)) {
   *   return Lock::genLockedMessage($locks, TRUE);
   * }
   * \endcode
   *
   * \param string $locks the locks as returned by Lock::get
   *
   * \param boolean $allowReadonly TRUE if readonly access should be permitted,
   * FALSE if not (default).
   *
   * \param string $action Label of the action button, "Edit anyway" by default. Will be escaped.
   *
   */
  public static function genLockedMessage (array $locks, bool $allowReadonly = FALSE, string $action = NULL): string
  {
    /* Save variables from LOCK_VARS_TO_USE in session - for further editing */
    if (session::is_set('LOCK_VARS_TO_USE') && count(session::get('LOCK_VARS_TO_USE'))) {
      $LOCK_VARS_USED_GET       = [];
      $LOCK_VARS_USED_POST      = [];
      $LOCK_VARS_USED_REQUEST   = [];
      $LOCK_VARS_TO_USE         = session::get('LOCK_VARS_TO_USE');

      foreach ($LOCK_VARS_TO_USE as $name) {
        if (empty($name)) {
          continue;
        }

        foreach ($_POST as $Pname => $Pvalue) {
          if (preg_match($name, $Pname)) {
            $LOCK_VARS_USED_POST[$Pname] = $_POST[$Pname];
          }
        }

        foreach ($_GET as $Pname => $Pvalue) {
          if (preg_match($name, $Pname)) {
            $LOCK_VARS_USED_GET[$Pname] = $_GET[$Pname];
          }
        }

        foreach ($_REQUEST as $Pname => $Pvalue) {
          if (preg_match($name, $Pname)) {
            $LOCK_VARS_USED_REQUEST[$Pname] = $_REQUEST[$Pname];
          }
        }
      }
      session::set('LOCK_VARS_TO_USE',        []);
      session::set('LOCK_VARS_USED_GET',      $LOCK_VARS_USED_GET);
      session::set('LOCK_VARS_USED_POST',     $LOCK_VARS_USED_POST);
      session::set('LOCK_VARS_USED_REQUEST',  $LOCK_VARS_USED_REQUEST);
    }

    /* Prepare and show template */
    $smarty = get_smarty();
    $smarty->assign('allow_readonly', $allowReadonly);
    $smarty->assign('action',         ($action ?? _('Edit anyway')));
    $smarty->assign('locks',          $locks);

    return $smarty->fetch(get_template_path('islocked.tpl'));
  }
}
