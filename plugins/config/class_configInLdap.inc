<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2016 FusionDirectory

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

/* Debug level is an OR combination of several values */
class DebugLevelAttribute extends SetAttribute
{
  protected function loadAttrValue ($attrs)
  {
    if (isset($attrs[$this->getLdapName()])) {
      $value = $attrs[$this->getLdapName()][0];
      $this->value = array();
      foreach ($this->attribute->getChoices() as $choice) {
        if ($value & $choice) {
          $this->value[] = $choice;
        }
      }
    } else {
      $this->resetToDefault();
    }
  }

  function computeLdapValue()
  {
    $value = 0;
    foreach ($this->value as $v) {
      $value |= $v;
    }
    return $value;
  }
}

class configInLdap extends simplePlugin
{
  var $objectclasses  = array("fusionDirectoryConf");
  var $mainTab        = TRUE;

  static function plInfo()
  {
    return array(
      'plShortName'     => _('Configuration'),
      'plTitle'         => _('FusionDirectory configuration'),
      'plDescription'   => _('Configuration screen of FusionDirectory'),
      'plIcon'          => 'geticon.php?context=categories&icon=settings&size=48',
      'plObjectType'    => array(
        'configuration' => array(
          'name'      => _('FusionDirectory configuration'),
          'filter'    => 'objectClass=fusionDirectoryConf',
          'icon'      => 'geticon.php?context=categories&icon=settings&size=16',
          'mainAttr'  => FALSE,
          'ou'        => preg_replace('/^[^,]+,/', '', CONFIGRDN)
        )
      ),
      'plSection'       => array('conf' => array('name' => _('Configuration'), 'priority' => 20)),
      'plManages'       => array('configuration'),
      'plPriority'      => 0,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    global $config;
    $plugins = array();
    if (session::global_is_set('plist')) {
      $plugins = array_keys(session::global_get('plist')->info);
    }
    sort($plugins);
    return array(
      'look_n_feel' => array(
        'name'  => _('Look and feel'),
        'attrs' => array(
          new SelectAttribute (
            _('Language'), _('Language of the application. If \'automatic\' or not available, the one asked by the browser will be used. This setting can be overriden per user.'),
            'fdLanguage', FALSE,
            array('')
          ),
          new SelectAttribute (
            _('Theme'), _('Theme to be used'),
            'fdTheme', TRUE,
            self::get_themes(),
            'default'
          ),
          new SelectAttribute (
            _('Timezone'), _('Timezone to be used'),
            'fdTimezone', TRUE,
            array('America/New_York')
          ),
          new HiddenAttribute ('fusionConfigMd5'),
        )
      ),
      'schema_setup' => array(
        'name'  => _('Schema setup'),
        'attrs' => array(
          new BooleanAttribute (
            _('Schema validation'),
            _('Enables schema checking during login.'),
            'fdSchemaCheck', FALSE,
            TRUE
          ),
        )
      ),
      'password' => array(
        'name'  => _('Password settings'),
        'attrs' => array(
          new SelectAttribute (
            _('Password default hash'), _('Default hash to be used'),
            'fdPasswordDefaultHash', TRUE,
            array('ssha')
          ),
          new BooleanAttribute (
            _('Force default hash'), _('Force the use of the default password hash'),
            'fdForcePasswordDefaultHash'
          ),
          new IntAttribute (
            _('Password minimum length'), _('Minimum length of user passwords'),
            'fdPasswordMinLength', FALSE,
            0 /*min*/, FALSE /*no max*/
          ),
          new IntAttribute (
            _('Password minimum differs'), _('Minimum number of different characters from last password'),
            'fdPasswordMinDiffer', FALSE,
            0 /*min*/, FALSE /*no max*/
          ),
          new BooleanAttribute (
            _('Use account expiration'),
            _('Enables shadow attribute tests during the login to FusionDirectory and forces password renewal or account locking'),
            'fdHandleExpiredAccounts'
          ),
          new StringAttribute (
            _('SASL Realm'), _('SASL Realm'),
            'fdSaslRealm'
          ),
          new StringAttribute (
            _('SASL Exop'), _('Attribute to be stored in the userPassword attribute'),
            'fdSaslExop'
          ),
        )
      ),
      'core_settings' => array(
        'name'  => _('Core settings'),
        'attrs' => array(
          new BooleanAttribute (
            _('Enable primary group filter'),
            _('It is time consuming to evaluate which groups are primary and which are not, so you may want to disable it if your group plugin is slow.'),
            'fdPrimaryGroupFilter', FALSE,
            TRUE
          ),
          new BooleanAttribute (
            _('Display summary in listings'),
            _('Determines whether a status bar will be shown on the bottom of lists, displaying a short summary of type and number of elements in the list.'),
            'fdListSummary', FALSE,
            TRUE
          ),
          new SelectAttribute (
            _('Edit locking'),
            _('Check if a entry currently being edited has been modified outside of FusionDirectory in the meantime.'),
            'fdModificationDetectionAttribute', FALSE,
            array('', 'entryCSN', 'contextCSN'), 'entryCSN'
          ),
          new BooleanAttribute (
            _('Enable logging'),
            _('Event logging on FusionDirectory side.'),
            'fdLogging', FALSE,
            TRUE
          ),
          new IntAttribute (
            _('LDAP size limit'), _('Defines the number of entries to get from LDAP by default.'),
            'fdLdapSizeLimit', FALSE,
            0 /*min*/, FALSE /*no max*/, 200
          ),
        )
      ),
      'login' => array(
        'name'  => _('Login and session'),
        'attrs' => array(
          new SelectAttribute (
            _('Login attribute'),
            _('Which LDAP attribute should be used as the login name during login.'),
            'fdLoginAttribute', TRUE,
            array('uid', 'mail', 'uid,mail'), 'uid',
            array('uid', 'mail', 'both')
          ),
          new BooleanAttribute (
            _('Enforce encrypted connections'),
            _('Enables PHP security checks to force encrypted access (https) to the web interface.'),
            'fdForceSSL'
          ),
          new BooleanAttribute (
            _('Warn if session is not encrypted'),
            _('will display a warning to the user when http is used instead of https.'),
            'fdWarnSSL', FALSE,
            TRUE
          ),
          new IntAttribute (
            _('Session lifetime'), _('Defines when a session will expire in seconds.'),
            'fdSessionLifeTime', TRUE,
            0 /*min*/, FALSE /*no max*/, 1800
          ),
        )
      ),
      'snapshots' => array(
        'name'  => _('Snapshots'),
        'attrs' => array(
          new BooleanAttribute (
            _('Enable snapshots'), _('This enables you to save certain states of entries and restore them later on.'),
            'fdEnableSnapshots', FALSE,
            TRUE
          ),
          new StringAttribute (
            _('Snapshot base'), _('The base where snapshots should be stored inside of the LDAP.'),
            'fdSnapshotBase', FALSE,
            'ou=snapshots,'.$config->current['BASE']
          ),
        )
      ),
      'ssl' => array(
        'name'  => _('SSL'),
        'attrs' => array(
          new StringAttribute (
            _('Key path'), _('Path to FusionDirectory private key. Unused for now.'),
            'fdSslKeyPath', FALSE,
            '/etc/ssl/private/fd.key'
          ),
          new StringAttribute (
            _('Certificate path'), _('Path to FusionDirectory certificate. Unused for now.'),
            'fdSslCertPath', FALSE,
            '/etc/ssl/certs/fd.cert'
          ),
          new StringAttribute (
            _('CA certificate path'), _('Path to the CA certificate. Used for validating Argonaut Server host.'),
            'fdSslCaCertPath', FALSE,
            '/etc/ssl/certs/ca.cert'
          ),
        )
      ),
      'cas' => array(
        'name'  => _('CAS'),
        'attrs' => array(
          new BooleanAttribute (
            _('Enable CAS'), _('CAS login will be used instead of LDAP bind'),
            'fdCasActivated', FALSE,
            FALSE
          ),
          new StringAttribute (
            _('CA certificate path'), _('Path to the CA certificate of the CAS server'),
            'fdCasServerCaCertPath', FALSE,
            '/etc/ssl/certs/ca.cert'
          ),
          new StringAttribute (
            _('Host'), _('Host of the CAS server'),
            'fdCasHost', FALSE,
            'localhost'
          ),
          new IntAttribute (
            _('Port'), _('Port the CAS server is listening on'),
            'fdCasPort', FALSE,
            0 /*min*/, FALSE /*no max*/, 443
          ),
          new StringAttribute (
            _('CAS context'), _('CAS context to be used'),
            'fdCasContext', FALSE,
            '/cas'
          ),
        )
      ),
      'people_and_group' => array(
        'name'  => _('People and group storage'),
        'class' => array('critical'),
        'attrs' => array(
          new SelectAttribute (
            _('People DN attribute'), _('Attribute to use at the beginning of users dn'),
            'fdAccountPrimaryAttribute', TRUE,
            array('uid', 'cn')
          ),
          new StringAttribute (
            _('CN pattern'), _('The pattern to use to build the common name field'),
            'fdCnPattern', TRUE,
            '%givenName% %sn%'
          ),
          new BooleanAttribute (
            _('Strict naming policy'),
            _('Enables strict checking of user and group names'),
            'fdStrictNamingRules', FALSE,
            TRUE
          ),
          new IntAttribute (
            _('Group/user min id'),
            _('The minimum assignable user or group id to avoid security leaks with id 0 accounts.'),
            'fdMinId', FALSE,
            0, FALSE, 100
          ),
          new StringAttribute (
            _('Next id hook'), _('A script to be called for finding the next free id number for users or groups.'),
            'fdNextIdHook', FALSE
          ),
          new IntAttribute (
            _('Base number for user id'),
            _('Where to start looking for a new free user id.'),
            'fdUidNumberBase', FALSE,
            0, FALSE, 1100
          ),
          new IntAttribute (
            _('Base number for group id'),
            _('Where to start looking for a new free group id.'),
            'fdGidNumberBase', FALSE,
            0, FALSE, 1100
          ),
          new StringAttribute (
            _('Users RDN'), _('The branch where users are stored.'),
            'fdUserRDN', TRUE,
            'ou=people'
          ),
          new StringAttribute (
            _('Groups RDN'), _('The branch where groups are stored.'),
            'fdGroupRDN', TRUE,
            'ou=groups'
          ),
          new StringAttribute (
            _('ACL role RDN'), _('The branch where ACL roles are stored.'),
            'fdAclRoleRDN', TRUE,
            'ou=aclroles'
          ),
          new SelectAttribute (
            _('Id allocation method'), _('Method to allocate user/group ids'),
            'fdIdAllocationMethod', TRUE,
            array('traditional', 'pool'), 'traditional',
            array(_('Traditional'), _('Samba unix id pool'))
          ),
          new IntAttribute (
            _('Pool user id min'), _('Minimum value for user id when using pool method'),
            'fdUidNumberPoolMin', FALSE,
            0, FALSE, 10000
          ),
          new IntAttribute (
            _('Pool user id max'), _('Maximum value for user id when using pool method'),
            'fdUidNumberPoolMax', FALSE,
            0, FALSE, 40000
          ),
          new IntAttribute (
            _('Pool group id min'), _('Minimum value for group id when using pool method'),
            'fdGidNumberPoolMin', FALSE,
            0, FALSE, 10000
          ),
          new IntAttribute (
            _('Pool group id max'), _('Maximum value for group id when using pool method'),
            'fdGidNumberPoolMax', FALSE,
            0, FALSE, 40000
          ),
        )
      ),
      'debug' => array(
        'name'  => _('Debugging'),
        'attrs' => array(
          new BooleanAttribute (
            _('Display errors'),
            _('Shows PHP errors in the upper part of the screen. This should be disabled in productive deployments, because there might be some passwords in it.'),
            'fdDisplayErrors'
          ),
          new IntAttribute (
            _('Maximum LDAP query time'), _('Stop LDAP actions if there is no answer within the specified number of seconds.'),
            'fdLdapMaxQueryTime', FALSE,
            0 /*min*/, FALSE /*no max*/
          ),
          new BooleanAttribute (
            _('Log LDAP statistics'),
            _('Track LDAP timing statistics to the syslog. This may help to find indexing problems or bad search filters.'),
            'fdLdapStats'
          ),
          new DebugLevelAttribute (
            new SelectAttribute (
              _('Debug level'),
              _('Display certain information on each page load.'),
              'fdDebugLevel', FALSE,
              array(DEBUG_TRACE,  DEBUG_LDAP, DEBUG_DB,   DEBUG_SHELL,  DEBUG_POST,
                    DEBUG_SESSION,  DEBUG_ACL,  DEBUG_SI, DEBUG_MAIL),
                    DEBUG_TRACE,
              array('Trace',      'LDAP',     'Database', 'Shell',      'POST',
                    'SESSION',      'ACL',      'SI',     'Mail')
            )
          ),
        )
      ),
      'miscellaneous' => array(
        'name'  => _('Miscellaneous'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new OrderedArrayAttribute (
            new CompositeAttribute (
              _('Hooks that are called when specific actions happens'),
              'fdTabHook',
              array(
                new SelectAttribute(
                  _('tab'), _('The tab that this hook concerns'),
                  'hookTab', TRUE,
                  $plugins
                ),
                new SelectAttribute(
                  _('mode'), _('When to call this command'),
                  'hookMode', TRUE,
                  array('postcreate', 'postremove', 'postmodify',
                        'precreate', 'preremove', 'premodify', 'check')
                ),
                new StringAttribute(
                  _('cmd'), _('The command that will be called'),
                  'hookCmd', TRUE
                )
              ),
              '/^([^\\|]+)\\|([^\\|]+)\\|(.*)$/',
              '%s|%s|%s',
              '', // acl
              _('Hooks')
            ),
            FALSE, // non-ordered
            array(),
            TRUE // edition
          ),
          new BooleanAttribute (
            _('Display hook output'),
            _('When enabled successful hook execution output is displayed to the user using a dialog.'),
            'fdDisplayHookOutput'
          ),
          new SetAttribute(
            new StringAttribute (
              _('Available shells'), _('Available POSIX shells for FD users.'),
              'fdShells', FALSE
            ),
            array('/bin/ash','/bin/bash','/bin/csh','/bin/sh','/bin/ksh',
                  '/bin/tcsh','/bin/dash','/bin/zsh','/sbin/nologin', '/bin/false')
          ),
          new BooleanAttribute (
            _('Show ACL tab on all objects'),
            _('For very specific ACL rights setting where you might need to give right on a single object.'),
            'fdAclTabOnObjects'
          ),
        )
      ),
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    $attributesInfo = self::getAttributesInfo();
    /* Languages */
    $languages = get_languages(TRUE);
    asort($languages);
    $languages = array_merge(array("" => _("Automatic")), $languages);
    $attributesInfo['look_n_feel']['attrs'][0]->setChoices(array_keys($languages), array_values($languages));
    /* Timezones */
    $attributesInfo['look_n_feel']['attrs'][2]->setChoices(timezone::_get_tz_zones());
    /* Password methods */
    $methods = passwordMethod::get_available_methods();
    $methods = $methods['name'];
    $attributesInfo['password']['attrs'][0]->setChoices($methods);

    parent::__construct($config, $dn, $object, $attributesInfo);

    $this->fusionConfigMd5 = md5_file(CACHE_DIR."/".CLASS_CACHE);

    $this->attributesAccess['fdEnableSnapshots']->setManagedAttributes(
      array(
        'disable' => array (
          FALSE => array (
            'fdSnapshotBase',
          )
        )
      )
    );
    $this->attributesAccess['fdForceSSL']->setManagedAttributes(
      array(
        'disable' => array (
          TRUE => array (
            'fdWarnSSL',
          )
        )
      )
    );
    $this->attributesAccess['fdIdAllocationMethod']->setManagedAttributes(
      array(
        'erase' => array (
          'traditional' => array (
            'fdUidNumberPoolMin','fdUidNumberPoolMax',
            'fdGidNumberPoolMin','fdGidNumberPoolMax',
          )
        )
      )
    );
    $this->attributesAccess['fdCasActivated']->setManagedAttributes(
      array(
        'disable' => array (
          FALSE => array (
            'fdCasServerCaCertPath',
            'fdCasHost',
            'fdCasPort',
            'fdCasContext',
          )
        )
      )
    );
  }

  function compute_dn()
  {
    return $this->dn;
  }

  static function get_themes()
  {
    $themesdir  = '../ihtml/themes/';
    $themes     = array_keys(session::global_get(IconTheme::$session_var));
    if ($dir = opendir("$themesdir")) {
      while (($file = readdir($dir)) !== FALSE) {
        if (is_dir("$themesdir/$file") && !preg_match("/^\./", $file)) {
          $themes[] = $file;
        }
      }
    }
    return array_unique($themes);
  }
}
?>
