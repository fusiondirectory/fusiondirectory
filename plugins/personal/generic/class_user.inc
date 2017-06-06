<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2016  FusionDirectory

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

/* Handle a password and its hash method */
class UserPasswordAttribute extends CompositeAttribute
{
  protected $needPassword;
  protected $previousMethod;

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "")
  {
    $temp = passwordMethod::get_available_methods();

    /* Create password methods array */
    $pwd_methods = array();
    $this->needPassword = array();
    foreach ($temp['name'] as $id => $name) {
      $this->needPassword[$name] = $temp[$id]['object']->need_password();
      $pwd_methods[$name] = $name;
      if (!empty($temp[$id]['desc'])) {
        $pwd_methods[$name] .= " (".$temp[$id]['desc'].")";
      }
    }

    parent::__construct (
      $description, $ldapName,
      array(
        new SelectAttribute(
          _('Password method'), _('Password hash method to use'),
          $ldapName.'_pwstorage', TRUE,
          array_keys($pwd_methods), '', array_values($pwd_methods)
        ),
        new PasswordAttribute(
          _('Password'), _('Password (Leave empty if you do not wish to change it)'),
          $ldapName.'_password', $required
        ),
        new PasswordAttribute(
          _('Password again'), _('Same password as above, to avoid errors'),
          $ldapName.'_password2', $required
        ),
        new HiddenAttribute(
          $ldapName.'_hash'
        ),
        new HiddenAttribute(
          $ldapName.'_locked', FALSE,
          FALSE
        )
      ),
      '', '', $acl, $label
    );
    $this->attributes[0]->setSubmitForm(TRUE);
  }

  public function setParent(&$plugin)
  {
    global $config;
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      $hash = $config->get_cfg_value('passwordDefaultHash', 'ssha');
      $this->attributes[0]->setDefaultValue($hash);
      if ($config->get_cfg_value('forcePasswordDefaultHash', 'FALSE') == 'TRUE') {
        $this->attributes[0]->setValue($hash);
        $this->attributes[0]->setDisabled(TRUE);
      }
      $this->checkIfMethodNeedsPassword();
    }
  }

  /* We need to handle method select disabling manually */
  function renderAttribute(&$attributes, $readOnly)
  {
    global $config;
    if ($this->visible) {
      if ($this->linearRendering) {
        parent::renderAttribute($attributes, $readOnly);
      } else {
        foreach ($this->attributes as $key => &$attribute) {
          if (is_object($this->plugin) && $this->plugin->is_template && ($key == 2)) {
            /* Do not display confirmation field in template mode */
            continue;
          }
          if (($key == 0) && ($config->get_cfg_value('forcePasswordDefaultHash', 'FALSE') == 'TRUE')) {
            $attribute->setDisabled(TRUE);
          } else {
            $attribute->setDisabled($this->disabled);
          }
          $attribute->renderAttribute($attributes, $readOnly);
        }
        unset($attribute);
      }
    }
  }

  /*! \brief Loads this attribute value from the attrs array
   */
  protected function loadAttrValue ($attrs)
  {
    if (isset($attrs[$this->getLdapName()])) {
      $this->setValue($this->inputValue($attrs[$this->getLdapName()][0]));
      $this->setRequired(FALSE);
      $this->attributes[1]->setRequired(FALSE);
      $this->attributes[2]->setRequired(FALSE);
    } else {
      $this->setRequired(TRUE);
      $this->attributes[0]->resetToDefault();
      $this->attributes[1]->setRequired(TRUE);
      $this->attributes[2]->setRequired(TRUE);
      $this->checkIfMethodNeedsPassword();
    }
  }

  function setValue ($value)
  {
    reset($value);
    $key = key($value);
    if ($this->attributes[0]->isDisabled() || ($value[$key] == '')) {
      $value[$key] = $this->attributes[0]->getValue();
    }
    parent::setValue($value);
    $this->checkIfMethodNeedsPassword();
  }

  function applyPostValue ()
  {
    parent::applyPostValue();
    $this->checkIfMethodNeedsPassword();
  }

  function checkIfMethodNeedsPassword()
  {
    $method = $this->attributes[0]->getValue();
    if ($method != $this->previousMethod) {
      if ($this->needPassword[$method]) {
        $this->attributes[1]->setVisible(TRUE);
        $this->attributes[2]->setVisible(TRUE);
      } else {
        $this->attributes[1]->setVisible(FALSE);
        $this->attributes[1]->setValue('');
        $this->attributes[2]->setVisible(FALSE);
        $this->attributes[2]->setValue('');
      }
    }
    $this->previousMethod = $method;
  }

  function readValues($value)
  {
    global $config;
    $pw_storage = $config->get_cfg_value('passwordDefaultHash', 'ssha');
    $locked     = FALSE;
    $password   = '';
    if ($this->plugin->is_template) {
      list($value, $password) = explode('|', $value, 2);
    }
    if (preg_match ('/^{[^}]+}/', $value)) {
      $tmp = passwordMethod::get_method($value);
      if (is_object($tmp)) {
        $pw_storage = $tmp->get_hash();
        $locked     = $tmp->is_locked($this->plugin->dn);
        if ($this->plugin->is_template) {
          $value = $tmp->generate_hash($password);
        }
      }
    } else {
      if ($value != '') {
        $pw_storage = 'clear';
      }
    }
    return array($pw_storage, $password, $password, $value, $locked);
  }

  function writeValues($values)
  {
    if ($this->needPassword[$values[0]] && ($values[1] == '')) {
      if ($this->plugin->is_template) {
        return '';
      } else {
        return $values[3];
      }
    }
    $temp = passwordMethod::get_available_methods();
    if (!isset($temp[$values[0]])) {
      trigger_error('Unknown password method '.$values[0]);
      return $values[3];
    }
    $test = new $temp[$values[0]]($this->plugin->dn, $this->plugin);
    $test->set_hash($values[0]);
    if ($this->plugin->is_template) {
      return $test->generate_hash($values[1]).'|'.$values[1];
    } else {
      return $test->generate_hash($values[1]);
    }
  }

  function check()
  {
    $method = $this->attributes[0]->getValue();
    if (!$this->needPassword[$method]) {
      $this->attributes[1]->setRequired(FALSE);
      $this->attributes[2]->setRequired(FALSE);
    }
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    }
    if (($this->attributes[1]->getValue() != '') || ($this->attributes[2]->getValue() != '')) {
      return user::reportPasswordProblems($this->plugin->dn, $this->attributes[1]->getValue(), $this->attributes[2]->getValue());
    }
  }

  function getMethod()
  {
    return $this->attributes[0]->getValue();
  }

  function getClear()
  {
    return $this->attributes[1]->getValue();
  }

  function isLocked()
  {
    return $this->attributes[4]->getValue();
  }
}

class PostalAddressAttribute extends TextAreaAttribute
{
  function inputValue ($ldapValue)
  {
    return str_replace(
      array('$',  '\24','\5C'),
      array("\n", '$',  '\\'),
      $ldapValue
    );
  }

  function computeLdapValue ()
  {
    return str_replace(
      array("\r\n", "\n", "\r"),
      '$',
      str_replace(
        array('\\', '$'),
        array('\5C','\24'),
        $this->getValue()
      )
    );
  }
}

class user extends simplePlugin
{
  var $objectclasses  = array('inetOrgPerson','organizationalPerson','person');

  private $was_locked;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('User'),
      'plDescription' => _('User account information'),
      'plIcon'        => 'geticon.php?context=applications&icon=user-info&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=user-info&size=16',
      'plSelfModify'  => TRUE,
      'plObjectType'  => array('user' => array(
        'name'        => _('User'),
        'description' => _('User account'),
        'filter'      => '(objectClass=inetOrgPerson)',
        'mainAttr'    => 'uid',
        'nameAttr'    => 'cn',
        'icon'        => 'geticon.php?context=types&icon=user&size=16',
        'ou'          => get_ou('userRDN'),
      )),
      'plForeignKeys'  => array(
        'manager' => array('user','dn')
      ),

      'plProvidedAcls' => array_merge(
        parent::generatePlProvidedAcls(static::getAttributesInfo()),
        array('userLock' => _('User lock status'))
      )
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    global $config;
    $languages = array_merge(array('' => ''), get_languages(TRUE));
    $attributesInfo = array(
      'perso' => array(
        'name'  => _('Personal information'),
        'icon'  => 'geticon.php?context=types&icon=user&size=16',
        'attrs' => array(
          new HiddenAttribute('cn'),
          new StringAttribute (
            _('Last name'), _('Last name of this user'),
            'sn', TRUE,
            '', '', '/[^,+"?()=<>;\\\\]/'
          ),
          new StringAttribute (
            _('First name'), _('First name of this user'),
            'givenName', TRUE,
            '', '', '/[^,+"?()=<>;\\\\]/'
          ),
          new TextAreaAttribute (
            _('Description'), _('Short description of the user'),
            'description', FALSE
          ),
          new ImageAttribute (
            _('Picture'), _('The avatar for this user'),
            'jpegPhoto', FALSE,
            150, 200, 'jpeg'
          ),
        )
      ),
      'contact' => array(
        'name'  => _('Organizational contact information'),
        'icon'  => 'geticon.php?context=types&icon=contact&size=16',
        'attrs' => array(
          new StringAttribute (
            _('Location'), _('Location'),
            'l', FALSE
          ),
          new StringAttribute (
            _('State'), _('State'),
            'st', FALSE
          ),
          new PostalAddressAttribute (
            _('Address'), _('Business postal address'),
            'postalAddress', FALSE
          ),
          new StringAttribute (
            _('Room No.'), _('Room number'),
            'roomNumber', FALSE
          ),
          new PhoneNumberAttribute (
            _('Phone'), _('Business phone number'),
            'telephoneNumber', FALSE
          ),
          new PhoneNumberAttribute (
            _('Mobile'), _('Business mobile number'),
            'mobile', FALSE
          ),
          new PhoneNumberAttribute (
            _('Pager'), _('Business pager number'),
            'pager', FALSE
          ),
          new PhoneNumberAttribute (
            _('Fax'), _('Business fax number'),
            'facsimileTelephoneNumber', FALSE
          ),
          new URLAttribute (
            _('Homepage'), _('Personal homepage'),
            'labeledURI', FALSE
          ),
        )
      ),
      'account' => array(
        'name'  => _('Account information'),
        'icon'  => 'geticon.php?context=applications&icon=ldap&size=16',
        'attrs' => array(
          new BaseSelectorAttribute (get_ou("userRDN")),
          new UidAttribute (
            _('Login'), _('Login of this user'),
            'uid', TRUE
          ),
          new SelectAttribute (
            _('Preferred language'), _('Preferred language'),
            'preferredLanguage', FALSE,
            array_keys($languages), '', array_values($languages)
          ),
          new UserPasswordAttribute(
            _('Password'), _('Password of the user'),
            'userPassword', FALSE
          ),
        )
      ),
      'homecontact' => array(
        'name'  => _('Personal contact information'),
        'icon'  => 'geticon.php?context=types&icon=contact&size=16',
        'attrs' => array(
          new StringAttribute (
            _('Display name'), _('Name this user should appear as. Used by Exchange.'),
            'displayName', FALSE
          ),
          new PostalAddressAttribute (
            _('Home address'), _('Home postal address'),
            'homePostalAddress', FALSE
          ),
          new StringAttribute (
            _('Private phone'), _('Home phone number'),
            'homePhone', FALSE
          ),
        )
      ),
      'organization' => array(
        'name'  => _('Organizational information'),
        'icon'  => 'geticon.php?context=places&icon=folder&size=16',
        'attrs' => array(
          new SetAttribute (
            new StringAttribute (
              _('Title'), _('Title of a person in their organizational context. Each title is one value of this multi-valued attribute'),
              'title', FALSE
            )
          ),
          new StringAttribute (
            _('Organization'), _('Organization'),
            'o', FALSE
          ),
          new StringAttribute (
            _('Department'), _('Department to which the user belongs'),
            'ou', FALSE
          ),
          new StringAttribute (
            _('Department No.'), _('Department number'),
            'departmentNumber', FALSE
          ),
          new StringAttribute (
            _('Employee No.'), _('Employee number'),
            'employeeNumber', FALSE
          ),
          new StringAttribute (
            _('Employee type'), _('Employee type'),
            'employeeType', FALSE
          ),
          new UserAttribute(
            _('Manager'), _('Manager'),
            'manager', FALSE
          ),
        )
      ),
    );
    if ($config->get_cfg_value('SplitPostalAddress') == 'TRUE') {
      $attributesInfo['contact']['attrs'][2]->setVisible(FALSE);
      array_splice($attributesInfo['contact']['attrs'], 3, 0, array(
        new StringAttribute (
          _('Street'), _('Street part of the address'),
          'street', FALSE
        ),
        new StringAttribute (
          _('Post office box'), _('Post office box'),
          'postOfficeBox', FALSE
        ),
        new IntAttribute (
          _('Postal code'), _('Postal code'),
          'postalCode', FALSE,
          0, FALSE
        ),
      ));
    }
    return $attributesInfo;
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['uid']->setUnique('whole');
    $this->attributesAccess['uid']->setAutocomplete(FALSE);
    $this->attributesAccess['uid']->setDisabled($this->initially_was_account && !$this->is_template);

    $filename = './plugins/users/images/default.jpg';
    $fd       = fopen ($filename, 'rb');
    $this->attributesAccess['jpegPhoto']->setPlaceholder(fread ($fd, filesize($filename)));

    $this->was_locked = $this->attributesAccess['userPassword']->isLocked();
  }

  function is_this_account($attrs)
  {
    /* Only inetOrgPerson is needed, it has the two others as SUP classes */
    return (isset($attrs['objectClass']) && in_array_ics('inetOrgPerson', $attrs['objectClass']));
  }

  function resetCopyInfos()
  {
    parent::resetCopyInfos();
    $this->attributesAccess['uid']->setDisabled($this->initially_was_account && !$this->is_template);
  }

  private function update_cn ()
  {
    global $config;
    $pattern  = $config->get_cfg_value('CnPattern', '%givenName% %sn%');
    $this->attributesAccess['cn']->setValue($this->applyPattern($pattern));
  }

  private function applyPattern ($pattern)
  {
    $fields   = templateHandling::listFields($pattern);
    $attrs    = array();
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
      trigger_error('Could not find field '.$field.' in any tab!');
    }

    return templateHandling::parseString($pattern, $attrs);
  }

  function compute_dn()
  {
    global $config;
    if ($this->is_template) {
      $dn = 'cn='.ldap_escape_dn($this->_template_cn).',ou=templates,'.get_ou('userRDN').$this->base;
      return $dn;
    }

    $this->update_cn();
    $attribute = $config->get_cfg_value('accountPrimaryAttribute', 'uid');

    return $this->create_unique_dn($attribute, get_ou('userRDN').$this->base);
  }

  function execute()
  {
    $smarty = get_smarty();
    $smarty->append('css_files', 'plugins/users/style/user_tab.css');
    return parent::execute();
  }

  protected function prepare_save ()
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

  function ldap_save()
  {
    $errors = parent::ldap_save();

    if (!empty($errors)) {
      return $errors;
    }

    if (!$this->is_template && $this->was_locked && $this->attributesAccess['userPassword']->hasChanged()) {
      $methods  = passwordMethod::get_available_methods();
      $pmethod  = new $methods[$this->attributesAccess['userPassword']->getMethod()]($this->dn);
      $pmethod->lock_account($this->dn);
    }

    return $errors;
  }

  function post_save()
  {
    global $ui;

    /* Update current locale settings, if we have edited ourselves */
    if (isset($this->attrs['preferredLanguage']) && ($this->dn == $ui->dn)) {
      $ui->language = $this->preferredLanguage;
      session::set('ui', $ui);
      session::set('Last_init_lang', 'update');
    }

    return parent::post_save();
  }

  function adapt_from_template($attrs, $skip = array())
  {
    if ($this->uid != '') {
      $skip[] = 'uid';
    }
    parent::adapt_from_template($attrs, $skip);
    if (isset($this->attrs['userPassword']) && !in_array('userPassword', $skip)) {
      list($hash,$password) = explode('|', $this->attrs['userPassword'][0], 2);
      if (preg_match ('/^{[^}]+}/', $hash)) {
        $tmp = passwordMethod::get_method($hash);
        if (is_object($tmp)) {
          $hash = $tmp->generate_hash($password);
        }
      }
      $this->userPassword = array(
        '',
        $password,
        $password,
        $hash,
        $this->attributesAccess['userPassword']->isLocked()
      );
    }
  }

  function callHook($cmd, array $addAttrs = array(), &$returnOutput = array(), &$returnCode = NULL)
  {
    $addAttrs['passwordMethod'] = $this->attributesAccess['userPassword']->getMethod();
    $addAttrs['userLocked']     = $this->attributesAccess['userPassword']->isLocked();
    $addAttrs['passwordClear']  = $this->attributesAccess['userPassword']->getClear();
    return parent::callHook($cmd, $addAttrs, $returnOutput, $returnCode);
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

    $ldap = $config->get_ldap_link();
    $ldap->cat($user, array('pwdPolicySubentry', 'pwdHistory', 'pwdChangedTime', 'userPassword'));
    $attrs = $ldap->fetch();
    $ppolicydn = '';
    if (isset($attrs['pwdPolicySubentry'][0])) {
      $ppolicydn = $attrs['pwdPolicySubentry'][0];
    } else {
      $ppolicydn = $config->get_cfg_value('ppolicyDefaultCn', '');
      if (!empty($ppolicydn)) {
        $ppolicydn = 'cn='.$ppolicydn.','.get_ou('ppolicyRDN').$config->current['BASE'];
      }
    }
    if (!empty($ppolicydn)) {
      $ldap->cat($ppolicydn, array('pwdAllowUserChange', 'pwdMinLength', 'pwdMinAge', 'pwdSafeModify'));
      $policy = $ldap->fetch();
      if (!$policy) {
        return sprintf(_('Ppolicy "%s" could not be found in the LDAP!'), $ppolicydn);
      }
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
        $now  = new DateTime('now', timezone::utc());
        if ($now->getTimeStamp() < $date->getTimeStamp() + $policy['pwdMinAge'][0]) {
          return sprintf(_('You must wait %d seconds before changing your password again'), $policy['pwdMinAge'][0] - ($now->getTimeStamp() - $date->getTimeStamp()));
        }
      }
      if (isset($policy['pwdSafeModify'][0]) && ($policy['pwdSafeModify'][0] == 'FALSE')) {
        if (empty($current_password)) {
          $current_password = NULL;
        }
      }
      if (isset($attrs['pwdHistory'][0])) {
        unset($attrs['pwdHistory']['count']);
        foreach ($attrs['pwdHistory'] as $pwdHistory) {
          $pwdHistory = explode('#', $pwdHistory, 4);
          $method = passwordMethod::get_method($pwdHistory[3], $user);
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
    } elseif ($check_differ && (substr($current_password, 0, $differ) == substr($new_password, 0, $differ))) {
      return _('The password used as new and current are too similar.');
    } elseif ($check_length && (strlen($new_password) < $length)) {
      return _('The password used as new is too short.');
    } elseif (!passwordMethod::is_harmless($new_password)) {
      return _('The password contains possibly problematic Unicode characters!');
    }

    return FALSE;
  }
}
?>
