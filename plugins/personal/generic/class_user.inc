<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2018  FusionDirectory

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

class PostalAddressAttribute extends TextAreaAttribute
{
  function inputValue ($ldapValue)
  {
    return str_replace(
      ['$', '\24', '\5C'],
      ["\n", '$', '\\'],
      $ldapValue
    );
  }

  function computeLdapValue ()
  {
    return str_replace(
      ["\r\n", "\n", "\r"],
      '$',
      str_replace(
        ['\\', '$'],
        ['\5C', '\24'],
        $this->getValue()
      )
    );
  }
}

class user extends simplePlugin
{
  // This is used to see if the password is locked. The "was" is better interpreted as "is" - it is historical here.
  private $was_locked;

  static function plInfo (): array
  {
    return [
      'plShortName' => _('User'),
      'plDescription' => _('User account information'),
      'plIcon' => 'geticon.php?context=applications&icon=user-info&size=48',
      'plSmallIcon' => 'geticon.php?context=applications&icon=user-info&size=16',
      'plSelfModify' => TRUE,
      'plObjectClass' => ['inetOrgPerson', 'organizationalPerson', 'person'],
      'plFilter' => '(objectClass=inetOrgPerson)',
      'plObjectType' => ['user' => [
        'name' => _('User'),
        'description' => _('User account'),
        'mainAttr' => 'uid',
        'nameAttr' => 'cn',
        'icon' => 'geticon.php?context=types&icon=user&size=16',
        'ou' => get_ou('userRDN'),
      ]],
      'plForeignKeys' => [
        'manager' => ['user', 'dn', 'manager=%oldvalue%', '*']
      ],
      'plSearchAttrs' => ['uid', 'description'],

      'plProvidedAcls' => array_merge(
        parent::generatePlProvidedAcls(static::getAttributesInfo()),
        ['userLock' => _('User lock status')]
      )
    ];
  }

  static function getAttributesInfo (): array
  {
    global $config;
    $languages = Language::getList(TRUE);
    asort($languages);
    $languages      = array_merge(['' => ''], $languages);
    $attributesInfo = [
      'perso' => [
        'name' => _('Personal information'),
        'icon' => 'geticon.php?context=types&icon=user&size=16',
        'attrs' => [
          new HiddenAttribute('cn'),
          new StringAttribute(
            _('Last name'), _('Last name of this user'),
            'sn', TRUE,
            '', '', '/^[^,+"?()=<>;\\\\]+$/'
          ),
          new StringAttribute(
            _('First name'), _('First name of this user'),
            'givenName', ($config->get_cfg_value('GivenNameRequired', 'TRUE') === 'TRUE'),
            '', '', '/^[^,+"?()=<>;\\\\]+$/'
          ),
          new StringAttribute(
            _('Initials'), _('The initials of some or all of the individual\'s names, but not the surname(s)'),
            'initials', FALSE,
            '', '', '/^[^,+"?()=<>;\\\\]+$/'
          ),
          new TextAreaAttribute(
            _('Description'), _('Short description of the user'),
            'description', FALSE
          ),
          new ImageAttribute(
            _('Picture'), _('The avatar for this user'),
            'jpegPhoto', FALSE,
            $config->get_cfg_value('MaxAvatarSize', 200), $config->get_cfg_value('MaxAvatarSize', 200), 'jpeg'
          ),
        ]
      ],
      'contact' => [
        'name' => _('Organizational contact information'),
        'icon' => 'geticon.php?context=types&icon=user&size=16',
        'attrs' => [
          new StringAttribute(
            _('Location'), _('Location'),
            'l', FALSE
          ),
          new StringAttribute(
            _('State'), _('State'),
            'st', FALSE
          ),
          new PostalAddressAttribute(
            _('Address'), _('Business postal address'),
            'postalAddress', FALSE
          ),
          new StringAttribute(
            _('Room No.'), _('Room number'),
            'roomNumber', FALSE
          ),
          new PhoneNumberButtonAttribute(
            _('Phone'), _('Business phone number'),
            'telephoneNumber', FALSE,
            '',
            'phone'
          ),
          new PhoneNumberButtonAttribute(
            _('Mobile'), _('Business mobile number'),
            'mobile', FALSE,
            '',
            'mobile'
          ),
          new PhoneNumberAttribute(
            _('Pager'), _('Business pager number'),
            'pager', FALSE
          ),
          new PhoneNumberAttribute(
            _('Fax'), _('Business fax number'),
            'facsimileTelephoneNumber', FALSE
          ),
          new URLAttribute(
            _('Homepage'), _('Personal homepage'),
            'labeledURI', FALSE
          ),
        ]
      ],
      'account' => [
        'name' => _('Account information'),
        'icon' => 'geticon.php?context=applications&icon=ldap&size=16',
        'attrs' => [
          new BaseSelectorAttribute(get_ou("userRDN")),
          new UidAttribute(
            _('Login'), _('Login of this user'),
            'uid', TRUE
          ),
          new SelectAttribute(
            _('Preferred language'), _('Preferred language'),
            'preferredLanguage', FALSE,
            array_keys($languages), '', array_values($languages)
          ),
          new UserPasswordAttribute(
            _('Password'), _('Password of the user'),
            'userPassword', FALSE
          ),
        ]
      ],
      'homecontact' => [
        'name' => _('Personal contact information'),
        'icon' => 'geticon.php?context=types&icon=user&size=16',
        'attrs' => [
          new StringAttribute(
            _('Display name'), _('Name this user should appear as. Used by Exchange.'),
            'displayName', FALSE
          ),
          new PostalAddressAttribute(
            _('Home address'), _('Home postal address'),
            'homePostalAddress', FALSE
          ),
          new PhoneNumberAttribute(
            _('Private phone'), _('Home phone number'),
            'homePhone', FALSE
          ),
        ]
      ],
      'organization' => [
        'name' => _('Organizational information'),
        'icon' => 'geticon.php?context=places&icon=folder&size=16',
        'attrs' => [
          new SetAttribute(
            new StringAttribute(
              _('Title'), _('Title of a person in their organizational context. Each title is one value of this multi-valued attribute'),
              'title', FALSE
            )
          ),
          new StringAttribute(
            _('Organization'), _('Organization'),
            'o', FALSE
          ),
          new StringAttribute(
            _('Unit'), _('Organizational unit this user belongs to'),
            'ou', FALSE
          ),
          new StringAttribute(
            _('Department No.'), _('Department number'),
            'departmentNumber', FALSE
          ),
          new StringAttribute(
            _('Employee No.'), _('Employee number'),
            'employeeNumber', FALSE
          ),
          new StringAttribute(
            _('Employee type'), _('Employee type'),
            'employeeType', FALSE
          ),
          new UserAttribute(
            _('Manager'), _('Manager'),
            'manager', FALSE
          ),
        ]
      ],
    ];
    if ($config->get_cfg_value('SplitPostalAddress') == 'TRUE') {
      $attributesInfo['contact']['attrs'][2]->setVisible(FALSE);
      array_splice($attributesInfo['contact']['attrs'], 3, 0, [
        new StringAttribute(
          _('Street'), _('Street part of the address'),
          'street', FALSE
        ),
        new StringAttribute(
          _('Post office box'), _('Post office box'),
          'postOfficeBox', FALSE
        ),
        new IntAttribute(
          _('Postal code'), _('Postal code'),
          'postalCode', FALSE,
          0, FALSE
        ),
      ]);
    }
    return $attributesInfo;
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

    // verify if the attribute password is locked
    $this->was_locked = $this->attributesAccess['userPassword']->isLocked();

    if ($this->was_locked) {
      $this->read_only = TRUE;
      // This will update the parent class (simplePlugin) via class-level state allowing children to get read only state.
      self::setUserLocked(TRUE);
    }

    if ($this->is_template && !$this->initially_was_account) {
      $this->attributesAccess['userPassword']->setValue('%askme%');
    }

    $this->attributesAccess['uid']->setUnique('whole');
    $this->attributesAccess['uid']->setAutocomplete(FALSE);
    $this->attributesAccess['uid']->setDisabled($this->initially_was_account && !$this->is_template);

    $filename = './plugins/users/images/default.jpg';
    $fd       = fopen($filename, 'rb');
    $this->attributesAccess['jpegPhoto']->setPlaceholder(fread($fd, filesize($filename)));


    // Do not apply automatic snap on templates nor if the DN is not yet processed (new creation from template)
    if ($this->is_template !== TRUE && $this->dn !== 'new') {
      // Verification is snapshot is enabled and automatic.
      if (isset($config->current['ENABLEAUTOMATICSNAPSHOTS']) && isset($config->current['ENABLESNAPSHOTS'])) {
        if (strtolower($config->current['ENABLEAUTOMATICSNAPSHOTS']) === 'true' && strtolower($config->current['ENABLESNAPSHOTS']) === 'true') {
          $this->generateAutomaticSnapshot();
        }
      }
    }
  }

  function resetCopyInfos ()
  {
    parent::resetCopyInfos();
    $this->attributesAccess['uid']->setDisabled($this->initially_was_account && !$this->is_template);
  }

  private function update_cn ()
  {
    global $config;
    $pattern = $config->get_cfg_value('CnPattern', '%givenName% %sn%');
    $this->attributesAccess['cn']->setValue($this->applyPattern($pattern));
  }

  private function applyPattern ($pattern)
  {
    $fields = templateHandling::listFields($pattern);
    $attrs  = [];
    foreach ($fields as $field) {
      if (in_array($field, $this->attributes)) {
        $attrs[$field] = $this->$field;
        continue;
      }

      if (isset($this->parent->by_object)) {
        foreach ($this->parent->by_object as $object) {
          if (in_array($field, $object->attributes)) {
            $attrs[$field] = $object->$field;
            continue 2;
          }
        }
      }
      trigger_error('Could not find field ' . $field . ' in any tab!');
    }

    return templateHandling::parseString($pattern, $attrs);
  }

  function compute_dn (): string
  {
    global $config;

    if ($this->is_template) {
      return 'cn=' . ldap_escape_dn($this->_template_cn) . ',ou=templates,' . get_ou('userRDN') . $this->base;
    }

    $this->update_cn();
    $attribute = $config->get_cfg_value('accountPrimaryAttribute', 'uid');

    return $this->create_unique_dn($attribute, get_ou('userRDN') . $this->base);
  }

  public function render (): string
  {
    $smarty = get_smarty();
    $smarty->append('css_files', 'plugins/users/style/user_tab.css');
    return parent::render();
  }

  protected function shouldSave (): bool
  {
    if ($this->attributesAccess['userPassword']->getClear() != '') {
      /* There may be hooks using this even if LDAP object is not modified */
      return TRUE;
    }
    return parent::shouldSave();
  }

  protected function prepare_save (): array
  {
    global $config;
    if ($config->get_cfg_value('SplitPostalAddress') == 'TRUE') {
      $pattern = $config->get_cfg_value('PostalAddressPattern', '');
      if (!empty($pattern)) {
        $this->postalAddress = $this->applyPattern($this->attributesAccess['postalAddress']->inputValue($pattern));
      }
    }

    return parent::prepare_save();
  }

  function ldap_save (): array
  {
    $errors = parent::ldap_save();

    if (!empty($errors)) {
      return $errors;
    }

    if (!$this->is_template && $this->was_locked && $this->attributesAccess['userPassword']->hasChanged()) {
      $methods = passwordMethod::get_available_methods();
      $pmethod = new $methods[$this->attributesAccess['userPassword']->getMethod()]($this->dn);
      $pmethod->lock_account($this->dn);
    }

    return $errors;
  }

  function post_save ()
  {
    global $ui, $config;

    /* Update current locale settings, if we have edited ourselves */
    if (isset($this->attrs['preferredLanguage']) && ($this->dn == $ui->dn)) {
      $ui->language = $this->preferredLanguage;
      session::set('ui', $ui);
      session::set('Last_init_lang', 'update');
    }
    return parent::post_save();
  }

  /**
   * @return void
   * Note : Create a snapshot of current data before save, verifying if data have changed before taking a snap.
   */
  public function generateAutomaticSnapshot ()
  {
    // Get the hash of current data before modifications.
    global $config;
    $ldap            = $config->get_ldap_link();
    $currentSnapHash = md5($ldap->generateLdif($this->dn, '(!(objectClass=gosaDepartment))', 'sub'));

    // Verify if current snap hash already exist in the list of existing snapshots - not taking a snap if it exists.
    $snapshotHandler   = new SnapshotHandler();
    $existingSnapshots = $snapshotHandler->getAvailableSnapsShots($this->dn);
    $sameHashExist     = FALSE;
    foreach ($existingSnapshots as $snap) {
      if (!empty($snap['fdSnapshotHash'][0]) && $snap['fdSnapshotHash'][0] === $currentSnapHash) {
        $sameHashExist = TRUE;
      }
    }
    // Create the snapshot
    if ($sameHashExist === FALSE) {
      $snapshotHandler->createSnapshot($this->dn, 'automatic snapshot', 'USER', 'FD');
      $snapshotHandler->verifySnapshotRetention($this->dn);
    }
  }

  function adapt_from_template (array $attrs, array $skip = [])
  {
    if ($this->uid != '') {
      $skip[] = 'uid';
    }

    parent::adapt_from_template($attrs, array_merge($skip, ['userPassword']));

    if (isset($attrs['userPassword']) && !in_array('userPassword', $skip)) {
      $this->userPassword = $this->attributesAccess['userPassword']->readUserPasswordValues($attrs['userPassword'][0], TRUE);
    }
  }

  function fillHookAttrs (array &$addAttrs)
  {
    parent::fillHookAttrs($addAttrs);
    $addAttrs['passwordMethod'] = $this->attributesAccess['userPassword']->getMethod();
    $addAttrs['userLocked']     = (int)($this->attributesAccess['userPassword']->isLocked());
    $addAttrs['passwordClear']  = $this->attributesAccess['userPassword']->getClear();
  }

  static function fetchPpolicy (string $userdn): array
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cat($userdn, ['pwdPolicySubentry', 'pwdHistory', 'pwdChangedTime', 'userPassword']);
    $attrs     = $ldap->fetch(TRUE);
    $ppolicydn = '';
    if (isset($attrs['pwdPolicySubentry'][0])) {
      $ppolicydn = $attrs['pwdPolicySubentry'][0];
    } else {
      $ppolicydn = $config->get_cfg_value('ppolicyDefaultDn', '');
    }

    $policy = NULL;
    if (!empty($ppolicydn)) {
      $ldap->cat($ppolicydn, ['pwdAllowUserChange', 'pwdMinLength', 'pwdMinAge', 'pwdSafeModify', 'pwdExpireWarning', 'pwdMaxAge']);
      $policy = $ldap->fetch(TRUE);
      if (!$policy) {
        throw new NonExistingLdapNodeException($ppolicydn, sprintf(_('Ppolicy "%s" could not be found in LDAP!'), $ppolicydn));
      }
    }

    return [$policy, $attrs];
  }

  static function reportPasswordProblems ($user, $new_password, $repeated_password, $current_password = NULL)
  {
    global $config, $ui;

    /* Should we check different characters in new password */
    $check_differ = ($config->get_cfg_value('passwordMinDiffer') != '');
    $differ       = $config->get_cfg_value('passwordMinDiffer', 0);
    if ($current_password === NULL) {
      $check_differ = FALSE;
    }

    /* Enable length check ? */
    $check_length = ($config->get_cfg_value('passwordMinLength') != '');
    $length       = $config->get_cfg_value('passwordMinLength', 0);

    try {
      list($policy, $attrs) = static::fetchPpolicy($user);
    } catch (NonExistingLdapNodeException $e) {
      return $e->getMessage();
    }

    if (isset($policy)) {
      if (isset($policy['pwdAllowUserChange'][0]) && ($policy['pwdAllowUserChange'][0] == 'FALSE') && ($ui->dn == $user)) {
        return _('You are not allowed to change your own password');
      }
      if (isset($policy['pwdMinLength'][0])) {
        $check_length = TRUE;
        $length       = $policy['pwdMinLength'][0];
      }
      if (isset($policy['pwdMinAge'][0]) && isset($attrs['pwdChangedTime'][0])) {
        $date = LdapGeneralizedTime::fromString($attrs['pwdChangedTime'][0]);
        $date->setTimezone(timezone::utc());
        $now = new DateTime('now', timezone::utc());
        if ($now->getTimeStamp() < $date->getTimeStamp() + $policy['pwdMinAge'][0]) {
          return sprintf(_('You must wait %d seconds before changing your password again'), $policy['pwdMinAge'][0] - ($now->getTimeStamp() - $date->getTimeStamp()));
        }
      }
      if (isset($policy['pwdSafeModify'][0]) && ($policy['pwdSafeModify'][0] == 'FALSE') && empty($current_password)) {
        $current_password = NULL;
      }
      if (isset($attrs['pwdHistory'][0])) {
        unset($attrs['pwdHistory']['count']);
        foreach ($attrs['pwdHistory'] as $pwdHistory) {
          $pwdHistory = explode('#', $pwdHistory, 4);
          $method     = passwordMethod::get_method($pwdHistory[3], $user);
          if (($method !== NULL) && $method->checkPassword($new_password, $pwdHistory[3])) {
            return _('Password is in history of old passwords');
          }
        }
      }
      if (($current_password !== NULL) && ($current_password == $new_password)) {
        return _('Password is not being changed from existing value');
      } elseif (isset($attrs['userPassword'][0])) {
        $method = passwordMethod::get_method($attrs['userPassword'][0], $user);
        if (($method !== NULL) && $method->checkPassword($new_password, $attrs['userPassword'][0])) {
          return _('Password is not being changed from existing value');
        }
      }
    }

    // Perform FusionDirectory password policy checks
    if (($current_password !== NULL) && empty($current_password)) {
      return _('You need to specify your current password in order to proceed.');
    } elseif ($new_password != $repeated_password) {
      return _('The passwords you\'ve entered as "New password" and "Repeated new password" do not match.');
    } elseif ($new_password == '') {
      return msgPool::required(_('New password'));
    } elseif ($check_differ && (mb_substr($current_password, 0, $differ) == mb_substr($new_password, 0, $differ))) {
      return _('The password used as new and current are too similar.');
    } elseif ($check_length && (mb_strlen($new_password) < $length)) {
      return _('The password used as new is too short.');
    } elseif (!passwordMethod::is_harmless($new_password)) {
      return _('The password contains possibly problematic Unicode characters!');
    }

    return FALSE;
  }
}
