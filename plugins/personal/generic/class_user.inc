<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013  FusionDirectory

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
  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = "", $acl = "")
  {
    $temp = passwordMethod::get_available_methods();

    /* Create password methods array */
    $pwd_methods = array();
    foreach ($temp['name'] as $id => $name) {
      if (!$temp[$id]['object']->need_password()) {
        continue;
      }
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
        )
      ),
      '', '', $acl, $label
    );
  }

  public function setParent(&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      $hash = $this->plugin->config->get_cfg_value('passwordDefaultHash', 'ssha');
      $this->attributes[0]->setDefaultValue($hash);
      if ($this->plugin->config->get_cfg_value('forcePasswordDefaultHash', 'FALSE') == 'TRUE') {
        $this->attributes[0]->setValue($hash);
        $this->attributes[0]->setDisabled(TRUE);
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
    }
  }

  function readValues($value)
  {
    $pw_storage = $this->plugin->config->get_cfg_value('passwordDefaultHash', 'ssha');
    if (preg_match ('/^{[^}]+}/', $value)) {
      $tmp = passwordMethod::get_method($value);
      if (is_object($tmp)) {
        $pw_storage = $tmp->get_hash();
      }
    } else {
      if ($value != '') {
        $pw_storage = 'clear';
      }
    }
    return array($pw_storage, '', '', $value);
  }

  function writeValues($values)
  {
    if ($values[1] == '') {
      return $values[3];
    }
    $temp = passwordMethod::get_available_methods();
    $test = new $temp[$values[0]]($this->plugin->config, $this->plugin->dn);
    $test->set_hash($values[0]);
    return $test->generate_hash($values[1]);
  }

  function check()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    }
    if ($this->attributes[1]->getValue() != $this->attributes[2]->getValue()) {
      return _('Passwords does not match');
    }
  }
}

class user extends simplePlugin
{
  var $objectclasses  = array('top','person','organizationalPerson','inetOrgPerson','gosaAccount');
  var $mainTab        = TRUE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Generic'),
      'plDescription' => _('Generic user settings'),
      'plSelfModify'  => TRUE,
      'plObjectType'  => array('user' => array(
        'description' => _('Users'),
        'name'        => _('User'),
        'filter'      => 'objectClass=gosaAccount',
        'mainAttr'    => 'cn',
        'icon'        => 'geticon.php?context=types&icon=user&size=16',
        'ou'          => get_ou('userRDN'),
      )),
      'plForeignKeys'  => array(
        'manager' => array('user','dn')
      ),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  // The main function : information about attributes
  static function getAttributesInfo ()
  {
    $languages = array_merge(array('' => ''), get_languages(TRUE));
    return array(
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
          new StringAttribute (
            _('Personal title'), _('Personal title'),
            'personalTitle', FALSE
          ),
          new StringAttribute (
            _('Academic title'), _('Academic title'),
            'academicTitle', FALSE
          ),
          new DateAttribute (
            _('Date of birth'), _('Date of birth'),
            'dateOfBirth', FALSE,
            'Y-m-d', ''
          ),
          new SelectAttribute (
            _('Sex'), _('Gender'),
            'gender', FALSE,
            array('', 'M', 'F'), '', array('', 'male', 'female')
          ),
          new StringAttribute (
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
      'account' => array(
        'name'  => _('Account information'),
        'icon'  => 'images/lists/search-subtree.png',
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
          new SetAttribute(
            new IpAttribute(
              _('Restrict FD login to'), _('The user will only be able to connect to FusionDirectory from these hosts or networks'),
              'gosaLoginRestriction', FALSE
            )
          )
        )
      ),
      'contact' => array(
        'name'  => _('Contact information'),
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
          new TextAreaAttribute (
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
          new PhoneNumberAttribute ( // TODO: mobile attribute
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
        )
      ),
      'organization' => array(
        'name'  => _('Organizational information'),
        'icon'  => 'geticon.php?context=places&icon=folder&size=16',
        'attrs' => array(
          new StringAttribute (
            _('Organization'), _('Organization'),
            'o', FALSE
          ),
          new StringAttribute (
            _('Department'), _('Department'),
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
      'homecontact' => array(
        'name'  => _('Personal contact information'),
        'icon'  => 'geticon.php?context=types&icon=contact&size=16',
        'attrs' => array(
          new TextAreaAttribute (
            _('Home address'), _('Home postal address'),
            'homePostalAddress', FALSE
          ),
          new StringAttribute (
            _('Private phone'), _('Home phone number'),
            'homePhone', FALSE
          ),
          new URLAttribute (
            _('Homepage'), _('Personal homepage'),
            'labeledURI', FALSE
          ),
        )
      )
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesAccess['uid']->setUnique('whole');
    $this->attributesAccess['uid']->setDisabled($this->initially_was_account && !$this->is_template);

    $filename = './plugins/users/images/default.jpg';
    $fd       = fopen ($filename, 'rb');
    $this->attributesAccess['jpegPhoto']->setPlaceholder(fread ($fd, filesize($filename)));
  }

  function postCopyHook()
  {
    $this->attributesAccess['uid']->setDisabled($this->initially_was_account);
  }

  private function update_cn()
  {
    // Fill cn (it may be overridden by accountRDN)
    $pt = "";
    if ($this->config->get_cfg_value('personalTitleInDN') == 'TRUE') {
      if (!empty($this->personalTitle)) {
        $pt = $this->personalTitle." ";
      }
    }

    $this->attributesAccess['cn']->setValue(
      $pt.$this->attributesAccess['givenName']->getValue()." ".$this->attributesAccess['sn']->getValue()
    );
  }

  function compute_dn()
  {
    if ($this->is_template) {
      $dn = 'cn='.$this->_template_cn.',ou=templates,'.get_ou("userRDN").$this->base;
      return $dn;
    }

    $this->update_cn();
    // Handle accountRDN if filled
    $attribute  = '';
    $pattern    = $this->config->get_cfg_value('accountRDN', '');
    if ($pattern != '') {
      $attribute  = preg_replace('/^([^=]+)=/', '\1', $pattern);
      $pattern    = preg_replace('/^[^=]+=/',   '',   $pattern);
      $this->attributesAccess[$attribute]->setValue($this->tpl_parse($pattern));
    } else {
      $attribute = $this->config->get_cfg_value('accountPrimaryAttribute', 'uid');
    }

    return $this->create_unique_dn($attribute, get_people_ou().$this->base);
  }

  function execute()
  {
    $smarty = get_smarty();
    $smarty->append('css_files', 'plugins/users/style/user_tab.css');
    return parent::execute();
  }

  function save()
  {
    parent::save();

    /* Update current locale settings, if we have edited ourselves */
    $ui = session::get('ui');
    if (isset($this->attrs['preferredLanguage']) && ($this->dn == $ui->dn)) {
      $ui->language = $this->preferredLanguage;
      session::set('ui', $ui);
      session::set('Last_init_lang', 'update');
    }

    /* Remove ACL dependencies too */
    if (($this->dn != $this->orig_dn) && ($this->orig_dn != 'new') && (!$this->is_template)) {
      acl::update_acl_membership($this->orig_dn, $this->dn);
    }
  }

  function ldap_remove()
  {
    parent::ldap_remove();

    $fixed_dn = LDAP::prepare4filter($this->dn);

    $ldap = $this->config->get_ldap_link();

    /* Delete references to object groups */
    $ldap->cd ($this->config->current['BASE']);
    $ldap->search ('(&(objectClass=gosaGroupOfNames)(member='.$fixed_dn.'))', array('cn'));
    while ($ldap->fetch()) {
      $og = new ogroup($this->config, $ldap->getDN());
      unset($og->member[$this->dn]);
      $og->save();
    }

    // Update 'manager' attributes from gosaDepartment and inetOrgPerson
    $filter = '(&(objectClass=inetOrgPerson)(manager='.$fixed_dn.'))';
    $filter = '(|'.$filter.'(&(objectClass=gosaDepartment)(manager='.$fixed_dn.')))';
    $leaf_deps = get_list($filter, array('all'), $this->config->current['BASE'],
        array('manager','dn','objectClass'), GL_SUBSEARCH | GL_NO_ACL_CHECK);
    foreach ($leaf_deps as $entry) {
      $update = array('manager' => array());
      $ldap->cd($entry['dn']);
      $ldap->modify($update);
      if (!$ldap->success()) {
        trigger_error(sprintf("Failed to update manager for '%s', error was '%s'", $entry['dn'], $ldap->get_error()));
      }
    }

    if (class_available('roleGeneric')) {
      /* Delete references to roles */
      $ldap->cd ($this->config->current['BASE']);
      $ldap->search ('(&(objectClass=organizationalRole)(roleOccupant='.$fixed_dn.'))', array('cn'));
      while ($ldap->fetch()) {
        $role = new roleGeneric($this->config, $ldap->getDN());
        $key  = array_search($this->dn, $role->roleOccupant);
        if ($key !== FALSE) {
          unset($role->roleOccupant[$key]);
          $role->roleOccupant = array_values($role->roleOccupant);
          $role->save ();
        }
      }
    }

    /* Remove ACL dependencies too */
    acl::remove_acl_for($this->dn);
  }
}
?>
