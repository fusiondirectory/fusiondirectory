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

    $this->users_stats  = $this->computeUsersStats();
    $this->groups_stats = $this->computeGroupsStats();
    $this->expired      = $this->expired_accounts_info();
  }

  function computeUsersStats ()
  {
    global $config;
    /* User statistics */
    $nb_accounts = objects::count('user');

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=inetOrgPerson)(objectClass=gosaMailAccount))', array('cn'));
    $nb_mail_accounts = $ldap->count();
    $ldap->search('(&(objectClass=inetOrgPerson)(objectClass=posixAccount))', array('cn'));
    $nb_posix_accounts = $ldap->count();
    $ldap->search('(&(objectClass=inetOrgPerson)(objectClass=sambaSamAccount))', array('cn'));
    $nb_samba_accounts = $ldap->count();

    return array(
      'accounts' => array(
        array('name' => 'mail',   'nb' => $nb_mail_accounts,  'img' => 'geticon.php?context=applications&icon=internet-mail&size=16'),
        array('name' => 'posix',  'nb' => $nb_posix_accounts, 'img' => 'geticon.php?context=applications&icon=os-linux&size=16'),
        array('name' => 'samba',  'nb' => $nb_samba_accounts, 'img' => 'geticon.php?context=applications&icon=os-windows&size=16')
      ),
      'nb' => $nb_accounts,
      'img' => 'geticon.php?context=types&icon=user&size=16',
    );
  }

  function computeGroupsStats ()
  {
    global $config;
    /* Group statistics */
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(objectClass=posixGroup)', array('cn'));
    $nb = $ldap->count();
    $ldap->search('(&(objectClass=posixGroup)(objectClass=fdGroupMail))', array('cn'));
    $nb_mail_groups = $ldap->count();
    $ldap->search('(&(objectClass=posixGroup)(objectClass=sambaSamAccount))', array('cn'));
    $nb_samba_groups = $ldap->count();

    return array(
      'groups' => array(
        array('name' => 'mail',   'nb' => $nb_mail_groups,  'img' => 'geticon.php?context=applications&icon=internet-mail&size=16'),
        array('name' => 'samba',  'nb' => $nb_samba_groups, 'img' => 'geticon.php?context=applications&icon=os-windows&size=16')
      ),
      'nb' => $nb,
      'img' => 'geticon.php?context=types&icon=user-group&size=16',
    );
  }

  function expired_accounts_info ()
  {
    global $config;
    /*
     * Begin of code for selecting expired account
     */

    /* getting the date in TIMESTAMP UNIX format */
    /* 24 * 60 * 60 = 86400 */
    $today        = floor(time() / 86400);

    /* Fetch global value from fusiondirectory.conf */
    $next_expired_days = $config->get_cfg_value('dashboardExpiredAccountsDays', 15);
    $next_expired_date = ($today + $next_expired_days);

    /* search all account with all date, mail, telephone */
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search(
      '(shadowExpire=*)',
      array('uid','shadowExpire','mail','telephoneNumber','cn','manager')
    );

    $expired_accounts = array();
    $next_expired_accounts = array();
    while ($attrs = $ldap->fetch()) {
      // Test if account is expired now
      if ($attrs['shadowExpire'][0] <= $today) {
        $expired_accounts[] = static::get_user_infos($attrs);
      } elseif ($attrs['shadowExpire'][0] <= $next_expired_date) {
        $next_expired_accounts[] = static::get_user_infos($attrs);
      }
    }

    return array(
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
      $ldap->cd($config->current['BASE']);
      $manager_cn     = $ldap->get_attribute($attrs['manager'][0], 'cn');
      $manager_mail   = $ldap->get_attribute($attrs['manager'][0], 'mail');
      $manager_phone  = $ldap->get_attribute($attrs['manager'][0], 'telephoneNumber');
    } else {
      $manager_cn     = '';
      $manager_mail   = '';
      $manager_phone  = '';
    }

    /* 24 * 60 * 60 = 86400 */
    $human_shadowExpire = date('d.m.Y', $attrs['shadowExpire'][0] * 86400);

    return array(
      'uid'             => $attrs['uid'][0],
      'cn'              => $attrs['cn'][0],
      'telephoneNumber' => (isset($attrs['telephoneNumber'][0])?$attrs['telephoneNumber'][0]:''),
      'mail'            => (isset($attrs['mail'][0])?$attrs['mail'][0]:''),
      'manager_cn'      => $manager_cn,
      'manager_mail'    => $manager_mail,
      'manager_phone'   => $manager_phone,
      'shadowExpire'    => $human_shadowExpire,
    );
  }
}
?>
