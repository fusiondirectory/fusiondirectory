<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org)
  Copyright (C) 2010 Antoine Gallavardin
  Copyright (C) 2011-2016 FusionDirectory project

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

class dashboardUsers extends simplePlugin
{
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Users'),
      'plDescription' => _('Statistics about users'),
      'plObjectType'  => array('dashboard'),
      'plPriority'    => 10,

      'plProvidedAcls'    => array()
    );
  }

  static function getAttributesInfo()
  {
    return array(
      'userstats' => array(
        'name'  => _('Users statistics'),
        'attrs' => array(new FakeAttribute('users_stats')),
        'template' => get_template_path('users_stats.tpl', TRUE, dirname(__FILE__)),
      ),
      'groupstats' => array(
        'name'  => _('Groups statistics'),
        'attrs' => array(new FakeAttribute('groups_stats')),
        'template' => get_template_path('groups_stats.tpl', TRUE, dirname(__FILE__)),
      ),
      'expired_accounts' => array(
        'name'  => _('Expired accounts'),
        'attrs' => array(new FakeAttribute('expired')),
        'template' => get_template_path('users_accounts.tpl', TRUE, dirname(__FILE__)),
      ),
    );
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->expiredAccountsColumns = array(
      'user' => array(
        'uid'             => _('Login'),
        'cn'              => _('Name'),
        'telephoneNumber' => _('Phone number'),
        'expirationDate'  => _('Expiration date'),
      ),
      'manager' => array(
        'manager_cn'      => _('Name'),
        'manager_mail'    => _('Email'),
        'manager_phone'   => _('Phone number'),
      )
    );
    $this->users_stats  = $this->computeUsersStats();
    $this->groups_stats = $this->computeGroupsStats();
    $this->expired      = $this->expired_accounts_info();
  }

  function computeUsersStats ()
  {
    global $config;
    /* User statistics */
    try {
      $nb_accounts        = count(objects::ls('user', NULL, NULL, '',                               TRUE));
      $nb_mail_accounts   = count(objects::ls('user', NULL, NULL, '(objectClass=gosaMailAccount)',  TRUE));
      $nb_posix_accounts  = count(objects::ls('user', NULL, NULL, '(objectClass=posixAccount)',     TRUE));
      $nb_samba_accounts  = count(objects::ls('user', NULL, NULL, '(objectClass=sambaSamAccount)',  TRUE));
    } catch (LDAPFailureException $e) {
      msg_dialog::display(
        _('LDAP error'),
        sprintf(_('User statistics could not be computed because of the following LDAP error: %s'), $e->getMessage()),
        ERROR_DIALOG
      );
      $nb_accounts        = 0;
      $nb_mail_accounts   = 0;
      $nb_posix_accounts  = 0;
      $nb_samba_accounts  = 0;
    }

    return array(
      'accounts' => array(
        array('name' => 'mail',   'nb' => $nb_mail_accounts,  'img' => 'geticon.php?context=applications&icon=internet-mail&size=16'),
        array('name' => 'posix',  'nb' => $nb_posix_accounts, 'img' => 'geticon.php?context=applications&icon=os-linux&size=16'),
        array('name' => 'samba',  'nb' => $nb_samba_accounts, 'img' => 'geticon.php?context=applications&icon=os-windows&size=16')
      ),
      'nb'  => $nb_accounts,
      'img' => 'geticon.php?context=types&icon=user&size=16',
    );
  }

  function computeGroupsStats ()
  {
    global $config;
    /* Group statistics */
    try {
      $nb_groups        = count(objects::ls('group', NULL, NULL, '',                              TRUE));
      $nb_mail_groups   = count(objects::ls('group', NULL, NULL, '(objectClass=fdGroupMail)',     TRUE));
      $nb_samba_groups  = count(objects::ls('group', NULL, NULL, '(objectClass=sambaGroupMapping)', TRUE));
    } catch (LDAPFailureException $e) {
      msg_dialog::display(
        _('LDAP error'),
        sprintf(_('Group statistics could not be computed because of the following LDAP error: %s'), $e->getMessage()),
        ERROR_DIALOG
      );
      $nb_groups        = 0;
      $nb_mail_groups   = 0;
      $nb_samba_groups  = 0;
    } catch (NonExistingObjectTypeException $e) {
      /* Posix plugin is not installed */
      $nb_groups        = 0;
      $nb_mail_groups   = 0;
      $nb_samba_groups  = 0;
    }

    return array(
      'groups' => array(
        array('name' => 'mail',   'nb' => $nb_mail_groups,  'img' => 'geticon.php?context=applications&icon=internet-mail&size=16'),
        array('name' => 'samba',  'nb' => $nb_samba_groups, 'img' => 'geticon.php?context=applications&icon=os-windows&size=16')
      ),
      'nb'  => $nb_groups,
      'img' => 'geticon.php?context=types&icon=user-group&size=16',
    );
  }

  function expired_accounts_info ()
  {
    global $config;

    /* Begin of code for selecting expired account */

    /* getting the date in TIMESTAMP UNIX format */
    /* 24 * 60 * 60 = 86400 */
    $today = floor(time() / 86400);

    /* Fetch global value from fusiondirectory.conf */
    $next_expired_days = $config->get_cfg_value('dashboardExpiredAccountsDays', 15);
    $next_expired_date = ($today + $next_expired_days);

    if (!class_available('posixAccount')) {
      /* if shadowExpire is not available no way to check expiration */
      return array(
        'columns'             => $this->expiredAccountsColumns,
        'accounts'            => array(),
        'accounts_next_days'  => array(),
        'next_days'           => $next_expired_days,
      );
    }

    /* search all account with all date, mail, telephone */
    try {
      $attributes = array(
        'dn'                    => 'raw',
        'uid'                   => 'raw',
        'cn'                    => 'raw',
        'telephoneNumber'       => 'raw',
        'manager'               => 'raw',
        'shadowExpire'          => 1,
      );
      if (class_available('mailAccount')) {
        $attributes['mail'] = 'raw';
      }
      $users = objects::ls('user', $attributes, NULL, '(shadowExpire=*)', TRUE);
    } catch (LDAPFailureException $e) {
      msg_dialog::display(
        _('LDAP error'),
        sprintf(_('Expired user information could not be computed because of the following LDAP error: %s'), $e->getMessage()),
        ERROR_DIALOG
      );
      $users = array();
    } catch (FusionDirectoryException $e) {
      msg_dialog::display(
        _('Error'),
        sprintf(_('Expired user information could not be computed because of the following error: %s'), $e->getMessage()),
        ERROR_DIALOG
      );
      $users = array();
    }

    $expired_accounts       = array();
    $next_expired_accounts  = array();
    foreach ($users as $user) {
      // Test if account is expired now
      if ($user['shadowExpire'] <= $today) {
        $expired_accounts[] = static::get_user_infos($user);
      } elseif ($user['shadowExpire'] <= $next_expired_date) {
        $next_expired_accounts[] = static::get_user_infos($user);
      }
    }

    uasort($expired_accounts,       array('dashboardUsers','compareUsers'));
    uasort($next_expired_accounts,  array('dashboardUsers','compareUsers'));

    return array(
      'columns'             => $this->expiredAccountsColumns,
      'accounts'            => $expired_accounts,
      'accounts_next_days'  => $next_expired_accounts,
      'next_days'           => $next_expired_days,
    );
  }

  static function get_user_infos($attrs)
  {
    global $config;

    if (isset($attrs['manager'][0])) {
      $ldap = $config->get_ldap_link();
      $ldap->cat($attrs['manager'][0], array('cn', 'mail', 'telephoneNumber'));
      if ($manager_attrs = $ldap->fetch()) {
        $manager_cn     = (isset($manager_attrs['cn'][0]) ? $manager_attrs['cn'][0] : '');
        $manager_mail   = (isset($manager_attrs['mail'][0]) ? $manager_attrs['mail'][0] : '');
        $manager_phone  = (isset($manager_attrs['telephoneNumber'][0]) ? $manager_attrs['telephoneNumber'][0] : '');
      } else {
        $manager_cn     = sprintf(_('Not found (%s)'), $attrs['manager'][0]);
        $manager_mail   = '';
        $manager_phone  = '';
      }
    } else {
      $manager_cn     = '';
      $manager_mail   = '';
      $manager_phone  = '';
    }

    if (isset($attrs['shadowExpire'][0])) {
      /* 24 * 60 * 60 = 86400 */
      $human_shadowExpire = date('d.m.Y', $attrs['shadowExpire'][0] * 86400);
    } else {
      $human_shadowExpire = '';
    }

    return array(
      'uid'             => $attrs['uid'][0],
      'cn'              => $attrs['cn'][0],
      'telephoneNumber' => (isset($attrs['telephoneNumber'][0]) ? $attrs['telephoneNumber'][0] : ''),
      'mail'            => (isset($attrs['mail'][0]) ? $attrs['mail'][0] : ''),
      'manager_cn'      => $manager_cn,
      'manager_mail'    => $manager_mail,
      'manager_phone'   => $manager_phone,
      'expirationDate'  => $human_shadowExpire,
    );
  }

  static function compareUsers ($a, $b)
  {
    if ($a['expirationDate'] < $b['expirationDate']) {
      return -1;
    } elseif ($a['expirationDate'] > $b['expirationDate']) {
      return 1;
    } elseif ($a['uid'] < $b['uid']) {
      return -1;
    } elseif ($a['uid'] > $b['uid']) {
      return 1;
    } else {
      return 0;
    }
  }
}
?>
