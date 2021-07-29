<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2012-2019 FusionDirectory

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

class configInLdap extends simplePlugin
{
  static function plInfo (): array
  {
    return [
      'plShortName'     => _('Configuration'),
      'plTitle'         => _('FusionDirectory configuration'),
      'plDescription'   => _('Configuration screen of FusionDirectory'),
      'plIcon'          => 'geticon.php?context=categories&icon=settings&size=48',
      'plObjectClass'   => ['fusionDirectoryConf'],
      'plObjectType'    => [
        'configuration' => [
          'name'      => _('FusionDirectory configuration'),
          'filter'    => 'objectClass=fusionDirectoryConf',
          'tabClass'  => 'configInLdapTabs',
          'icon'      => 'geticon.php?context=categories&icon=settings&size=16',
          'mainAttr'  => FALSE,
          'ou'        => preg_replace('/^[^,]+,/', '', CONFIGRDN)
        ]
      ],
      'plSection'       => ['conf' => ['name' => _('Configuration'), 'priority' => 20]],
      'plManages'       => ['configuration'],
      'plPriority'      => 0,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    global $config;

    return [
      'look_n_feel' => [
        'name'  => _('Look and feel'),
        'attrs' => [
          new SelectAttribute(
            _('Language'), _('Language of the application. If \'automatic\' or not available, the one asked by the browser will be used. This setting can be overriden per user.'),
            'fdLanguage', FALSE,
            ['']
          ),
          new SelectAttribute(
            _('Theme'), _('Theme to be used'),
            'fdTheme', TRUE,
            static::get_themes(),
            'breezy'
          ),
          new SelectAttribute(
            _('Timezone'), _('Timezone to be used'),
            'fdTimezone', TRUE,
            ['America/New_York']
          ),
          new HiddenAttribute('fusionConfigMd5'),
          new HiddenAttribute('fdIncrementalModifierStates'),
        ]
      ],
      'core_settings' => [
        'name'  => _('Core settings'),
        'attrs' => [
          new IntAttribute(
            _('LDAP size limit'), _('Defines the number of entries to get from LDAP by default.'),
            'fdLdapSizeLimit', FALSE,
            0 /*min*/, FALSE /*no max*/, 200
          ),
          new SelectAttribute(
            _('Edit locking'),
            _('Check if a entry currently being edited has been modified outside of FusionDirectory in the meantime.'),
            'fdModificationDetectionAttribute', FALSE,
            ['', 'entryCSN', 'contextCSN'], 'entryCSN'
          ),
          new BooleanAttribute(
            _('Enable logging'),
            _('Event logging on FusionDirectory side.'),
            'fdLogging', FALSE,
            TRUE
          ),
          new BooleanAttribute(
            _('Schema validation'),
            _('Enables schema checking during login.'),
            'fdSchemaCheck', FALSE,
            TRUE
          ),
          new BooleanAttribute(
            _('Enable snapshots'), _('This enables you to save certain states of entries and restore them later on.'),
            'fdEnableSnapshots', FALSE,
            TRUE
          ),
          new StringAttribute(
            _('Snapshot base'), _('The base where snapshots should be stored inside of the LDAP.'),
            'fdSnapshotBase', FALSE,
            'ou=snapshots,'.$config->current['BASE']
          ),
          new BooleanAttribute(
            _('Wildcard foreign keys'), _('Enables wildcard searches like member=* when moving a whole department. This will open all existing groups and roles to make sure foreign keys are respected. Slow on big trees.'),
            'fdWildcardForeignKeys', FALSE,
            TRUE
          ),
        ]
      ],
      'password' => [
        'name'  => _('Password settings'),
        'attrs' => [
          new SetAttribute(
            new SelectAttribute(
              _('Allowed password hashes'), _('Password hashes which may be used for user passwords'),
              'fdPasswordAllowedHashes', TRUE,
              ['ssha']
            )
          ),
          new SelectAttribute(
            _('Password default hash'), _('Default hash to be used'),
            'fdPasswordDefaultHash', TRUE,
            ['ssha']
          ),
          new BooleanAttribute(
            _('Force default hash'), _('Force the use of the default password hash'),
            'fdForcePasswordDefaultHash'
          ),
          new IntAttribute(
            _('Password minimum length'), _('Minimum length of user passwords'),
            'fdPasswordMinLength', FALSE,
            0 /*min*/, FALSE /*no max*/
          ),
          new IntAttribute(
            _('Password minimum differs'), _('Minimum number of different characters from last password'),
            'fdPasswordMinDiffer', FALSE,
            0 /*min*/, FALSE /*no max*/
          ),
          new BooleanAttribute(
            _('Use account expiration'),
            _('Enables shadow attribute tests during the login to FusionDirectory and forces password renewal or account locking'),
            'fdHandleExpiredAccounts'
          ),
          new StringAttribute(
            _('SASL Realm'), _('SASL Realm'),
            'fdSaslRealm'
          ),
          new StringAttribute(
            _('SASL Exop'), _('Attribute to be stored in the userPassword attribute'),
            'fdSaslExop'
          ),
        ]
      ],
      'login' => [
        'name'  => _('Login and session'),
        'attrs' => [
          new SelectAttribute(
            _('Login attribute'),
            _('Which LDAP attribute should be used as the login name during login.'),
            'fdLoginAttribute', TRUE,
            ['uid', 'mail', 'uid,mail'], 'uid',
            ['uid', 'mail', 'both']
          ),
          new BooleanAttribute(
            _('Enforce encrypted connections'),
            _('Enables PHP security checks to force encrypted access (https) to the web interface.'),
            'fdForceSSL'
          ),
          new BooleanAttribute(
            _('Warn if session is not encrypted'),
            _('will display a warning to the user when http is used instead of https.'),
            'fdWarnSSL', FALSE,
            TRUE
          ),
          new IntAttribute(
            _('Session lifetime'), _('Defines when a session will expire in seconds (0 to disable).'),
            'fdSessionLifeTime', TRUE,
            0 /*min*/, FALSE /*no max*/, 1800
          ),
          new SelectAttribute(
            _('Login method'),
            _('Which login method should be used for connecting to FusionDirectory'),
            'fdLoginMethod', TRUE
          ),
          new StringAttribute(
            _('Header name'), _('Name of the header containing user identifier.'),
            'fdHttpHeaderAuthHeaderName', FALSE,
            'AUTH_USER'
          ),
        ]
      ],
      'ssl' => [
        'name'  => _('SSL'),
        'attrs' => [
          new TrimmedStringAttribute(
            _('Key path'), _('Path to FusionDirectory private key. Unused for now.'),
            'fdSslKeyPath', FALSE,
            '/etc/ssl/private/fd.key'
          ),
          new TrimmedStringAttribute(
            _('Certificate path'), _('Path to FusionDirectory certificate. Unused for now.'),
            'fdSslCertPath', FALSE,
            '/etc/ssl/certs/fd.cert'
          ),
          new TrimmedStringAttribute(
            _('CA certificate path'), _('Path to the CA certificate. Used for validating Argonaut Server host.'),
            'fdSslCaCertPath', FALSE,
            '/etc/ssl/certs/ca.cert'
          ),
        ]
      ],
      'cas' => [
        'name'  => _('CAS'),
        'attrs' => [
          new TrimmedStringAttribute(
            _('CA certificate path'), _('Path to the CA certificate of the CAS server'),
            'fdCasServerCaCertPath', FALSE,
            '/etc/ssl/certs/ca.cert'
          ),
          new StringAttribute(
            _('Host'), _('Host of the CAS server'),
            'fdCasHost', FALSE,
            'localhost'
          ),
          new IntAttribute(
            _('Port'), _('Port the CAS server is listening on'),
            'fdCasPort', FALSE,
            0 /*min*/, FALSE /*no max*/, 443
          ),
          new StringAttribute(
            _('CAS context'), _('CAS context to be used'),
            'fdCasContext', FALSE,
            '/cas'
          ),
          new BooleanAttribute(
            _('Verbose error'), _('Activate verbose errors in phpCAS. Avoid in production.'),
            'fdCasVerbose', FALSE
          ),
        ]
      ],
      'people_and_group' => [
        'name'  => _('People and group storage'),
        'class' => ['critical'],
        'attrs' => [
          new SelectAttribute(
            _('People DN attribute'), _('Attribute to use at the beginning of users dn'),
            'fdAccountPrimaryAttribute', TRUE,
            ['uid', 'cn']
          ),
          new StringAttribute(
            _('CN pattern'), _('The pattern to use to build the common name field'),
            'fdCnPattern', TRUE,
            '%givenName% %sn%'
          ),
          new BooleanAttribute(
            _('Strict naming policy'),
            _('Enables strict checking of user and group names'),
            'fdStrictNamingRules', FALSE,
            TRUE
          ),
          new StringAttribute(
            _('Users RDN'), _('The branch where users are stored.'),
            'fdUserRDN', TRUE,
            'ou=people'
          ),
          new StringAttribute(
            _('ACL role RDN'), _('The branch where ACL roles are stored.'),
            'fdAclRoleRDN', TRUE,
            'ou=aclroles'
          ),
          new BooleanAttribute(
            _('Restrict role members'), _('When enabled only users from the same branch or members of groups from the same branch can be added to a role.'),
            'fdRestrictRoleMembers'
          ),
          new BooleanAttribute(
            _('Separate address fields'), _('Expose street, postOfficeBox and postalCode fields instead of postalAddress.'),
            'fdSplitPostalAddress'
          ),
          new PostalAddressAttribute(
            _('Postal address pattern'), _('When using separate address fields, you can use a pattern to fill postalAddress field.'),
            'fdPostalAddressPattern'
          ),
          new IntAttribute(
            _('Avatar max size'), _('Maximum user picture width and height in pixels. Bigger uploaded pictures will be resized.'),
            'fdMaxAvatarSize', FALSE,
            1, FALSE, 200
          ),
        ]
      ],
      'debug' => [
        'name'  => _('Debugging'),
        'attrs' => [
          new BooleanAttribute(
            _('Display PHP errors'),
            _('Shows PHP errors in the upper part of the screen. This should be disabled in productive deployments, because there might be some passwords in it.'),
            'fdDisplayErrors'
          ),
          new IntAttribute(
            _('Maximum LDAP query time'), _('Stop LDAP actions if there is no answer within the specified number of seconds.'),
            'fdLdapMaxQueryTime', FALSE,
            0 /*min*/, FALSE /*no max*/
          ),
          new BooleanAttribute(
            _('Log LDAP statistics'),
            _('Track LDAP timing statistics to the syslog. This may help to find indexing problems or bad search filters.'),
            'fdLdapStats'
          ),
          new DebugLevelAttribute(
            new SelectAttribute(
              _('Debug level'),
              _('Display certain information on each page load.'),
              'fdDebugLevel', FALSE,
              [DEBUG_TRACE,  DEBUG_LDAP, DEBUG_DB,   DEBUG_SHELL,  DEBUG_POST,
                    DEBUG_SESSION,  DEBUG_ACL,  DEBUG_SI, DEBUG_MAIL],
                    DEBUG_TRACE,
              ['Trace',      'LDAP',     'Database', 'Shell',      'POST',
                    'SESSION',      'ACL',      'SI',     'Mail']
            )
          ),
          new BooleanAttribute(
            _('Log debug messages'),
            _('Sends debug output to syslog as well'),
            'fdDebugLogging'
          ),
        ]
      ],
      'miscellaneous' => [
        'name'  => _('Miscellaneous'),
        'attrs' => [
          new BooleanAttribute(
            _('Display summary in listings'),
            _('Determines whether a status bar will be shown on the bottom of lists, displaying a short summary of type and number of elements in the list.'),
            'fdListSummary', FALSE,
            TRUE
          ),
          new BooleanAttribute(
            _('Show ACL tab on all objects'),
            _('For very specific ACL rights setting where you might need to give right on a single object.'),
            'fdAclTabOnObjects'
          ),
          new SetAttribute(
            new StringAttribute(
              _('Available department categories'), _('Available categories in the departments dropdown'),
              'fdDepartmentCategories', FALSE
            ),
            []
          ),
          new OrderedArrayAttribute(
            new PipeSeparatedCompositeAttribute(
              _('Use this to hide some menu entry to specific groups of users'),
              'fdPluginsMenuBlacklist',
              [
                new SelectAttribute(
                  '', _('Group or role'),
                  'blacklistGroup', TRUE,
                  []
                ),
                new SelectAttribute(
                  '', _('Plugin to blacklist'),
                  'blacklistPlugin', TRUE,
                  []
                ),
              ],
              '',
              _('Plugin menu blacklist')
            ),
            // no order
            FALSE,
            []
          ),
          // Needed here for ACLs
          new HiddenAttribute('fdManagementConfig'),
          new IntAttribute(
            _('ACL target filter limit'), _('Defines the maximum number of entries an ACL target filter is allowed to return'),
            'fdAclTargetFilterLimit', FALSE,
            0 /*min*/, FALSE /*no max*/, 100
          ),
        ]
      ],
    ];
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE, $attributesInfo = NULL)
  {
    global $config;
    $attributesInfo = static::getAttributesInfo();
    /* Languages */
    $languages = Language::getList(TRUE);
    asort($languages);
    $languages = array_merge(["" => _("Automatic")], $languages);
    $attributesInfo['look_n_feel']['attrs'][0]->setChoices(array_keys($languages), array_values($languages));
    /* Timezones */
    $attributesInfo['look_n_feel']['attrs'][2]->setChoices(timezone::_get_tz_zones());
    /* Password methods */
    $methods = passwordMethod::get_available_methods();
    $methods = $methods['name'];
    if (!in_array('sasl', $methods)) {
      $methods[] = 'sasl';
    }
    $attributesInfo['password']['attrs'][0]->attribute->setChoices($methods);
    $attributesInfo['password']['attrs'][0]->setDefaultValue($methods);
    $attributesInfo['password']['attrs'][0]->resetToDefault();
    $attributesInfo['password']['attrs'][1]->setChoices($methods);
    /* Login methods */
    $methods = LoginMethod::getMethods();
    $attributesInfo['login']['attrs'][4]->setChoices(array_keys($methods), array_values($methods));

    $groupsAndRoles = array_merge(
      array_map(
        function ($group)
        {
          return sprintf(_('Group %s'), $group);
        },
        objects::ls('ogroup')
      ),
      array_map(
        function ($role)
        {
          return sprintf(_('Role %s'), $role);
        },
        objects::ls('role')
      )
    );
    $attributesInfo['miscellaneous']['attrs'][3]->attribute->attributes[0]->setChoices(
      array_keys($groupsAndRoles),
      array_values($groupsAndRoles)
    );
    $menuPlugins = [];
    $plist = session::get('plist');
    foreach ($config->data['SECTIONS'] as $section => $section_infos) {
      foreach ($config->data['MENU'][$section] as $info) {
        if (isset($info['CLASS'])) {
          list ($plHeadline, , , ) = $plist->get_infos($info['CLASS']);
          $menuPlugins[$info['CLASS']] = $plHeadline;
        }
      }
    }
    asort($menuPlugins);
    $attributesInfo['miscellaneous']['attrs'][3]->attribute->attributes[1]->setChoices(
      array_keys($menuPlugins),
      array_values($menuPlugins)
    );

    try {
      parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);
    } catch (NonExistingLdapNodeException $e) {
      parent::__construct('new', $object, $parent, $mainTab, $attributesInfo);
      $this->dn = $dn;
    }

    $this->fusionConfigMd5 = md5_file(CACHE_DIR."/".CLASS_CACHE);

    $this->attributesAccess['fdEnableSnapshots']->setManagedAttributes(
      [
        'disable' => [
          FALSE => [
            'fdSnapshotBase',
          ]
        ]
      ]
    );

    $this->attributesAccess['fdForceSSL']->setManagedAttributes(
      [
        'disable' => [
          TRUE => [
            'fdWarnSSL',
          ]
        ]
      ]
    );
    $this->attributesAccess['fdSplitPostalAddress']->setManagedAttributes(
      [
        'disable' => [
          FALSE => [
            'fdPostalAddressPattern',
          ]
        ]
      ]
    );
    $this->attributesAccess['fdLoginMethod']->setManagedAttributes(
      [
        'multiplevalues' => [
          'noncas'    => [
            'LoginPost',
            'LoginHTTPAuth',
            'LoginHTTPHeader',
          ],
          'nonheader' => [
            'LoginPost',
            'LoginCAS',
            'LoginHTTPAuth'
          ],
        ],
        'disable' => [
          'noncas' => [
            'fdCasServerCaCertPath',
            'fdCasHost',
            'fdCasPort',
            'fdCasContext',
            'fdCasVerbose',
          ],
          'nonheader' => [
            'fdHttpHeaderAuthHeaderName',
          ]
        ]
      ]
    );
    if (empty($this->attrs['fdLoginMethod'][0])) {
      // Reading OBSOLETE attributes from FD<1.4 to ease migration
      if (isset($this->attrs['fdHttpAuthActivated'][0]) && ($this->attrs['fdHttpAuthActivated'][0] == 'TRUE')) {
        $this->fdLoginMethod = 'LoginHTTPAuth';
      } elseif (isset($this->attrs['fdCasActivated'][0]) && ($this->attrs['fdCasActivated'][0] == 'TRUE')) {
        $this->fdLoginMethod = 'LoginCAS';
      } elseif (isset($this->attrs['fdHttpHeaderAuthActivated'][0]) && ($this->attrs['fdHttpHeaderAuthActivated'][0] == 'TRUE')) {
        $this->fdLoginMethod = 'LoginHTTPHeader';
      }
    }

    $this->attributesAccess['fdPasswordDefaultHash']->setChoices(
      $this->attributesAccess['fdPasswordAllowedHashes']->getValue()
    );
  }

  function compute_dn (): string
  {
    return $this->dn;
  }

  function check (): array
  {
    $messages = parent::check();
    if (($this->fdPasswordDefaultHash == 'sasl') && ($this->fdSaslRealm == '') && ($this->fdSaslExop == '')) {
      $messages[] = new SimplePluginCheckError(
        $this,
        htmlescape(_('You need to fill saslRealm or saslExop in the configuration screen in order to use SASL'))
      );
    }
    if ($this->attributesAccess['fdLanguage']->hasChanged() && ($this->fdLanguage != '') && !Language::isAvailable($this->fdLanguage)) {
      $messages[] = new SimplePluginCheckError(
        $this->attributesAccess['fdLanguage'],
        htmlescape(sprintf(_('It seems the selected language "%s" is not installed on the system. Please install it or select an other one.'), $this->fdLanguage))
      );
    }

    if (($this->fdLdapSizeLimit !== '') && ($this->fdLdapSizeLimit > 0)) {
      $error = ldapSizeLimit::checkMaxInputVars($this->fdLdapSizeLimit);
      if ($error !== FALSE) {
        $messages[] = new SimplePluginCheckError(
          $this->attributesAccess['fdLdapSizeLimit'],
          $error->getHtmlMessage(),
          $error->getCode(),
          $error
        );
      }
    }

    return $messages;
  }

  public function update (): bool
  {
    $res = parent::update();

    $this->attributesAccess['fdPasswordDefaultHash']->setChoices(
      $this->attributesAccess['fdPasswordAllowedHashes']->getValue()
    );

    return $res;
  }

  static function get_themes ()
  {
    $themesdir  = '../ihtml/themes/';
    $themes     = array_keys(session::get(IconTheme::$session_var));
    if ($dir = opendir("$themesdir")) {
      while (($file = readdir($dir)) !== FALSE) {
        if (is_dir("$themesdir/$file") && !preg_match("/^\./", $file)) {
          $themes[] = $file;
        }
      }
    }
    return array_unique($themes);
  }

  static function mainInc ($classname = NULL, $entry_dn = NULL, $tabs = TRUE, $edit_mode = TRUE, $objectType = FALSE)
  {
    global $config;

    if ($classname === NULL) {
      $classname = get_called_class();
    }

    if ($entry_dn === NULL) {
      $entry_dn = CONFIGRDN.$config->current['BASE'];
    }

    parent::mainInc($classname, $entry_dn, $tabs, $edit_mode, $objectType);
  }
}
