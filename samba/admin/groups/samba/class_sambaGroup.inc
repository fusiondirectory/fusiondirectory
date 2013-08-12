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

class sambaGroup extends simplePlugin
{
  var $displayHeader  = TRUE;
  var $objectclasses  = array("sambaGroupMapping");

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Samba'),
      'plDescription' => _('Samba group settings'),
      'plSelfModify'  => FALSE,
      'plObjectType'  => array('group'),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Domain'),
        'attrs' => array(
          new SelectAttribute(
            _('Domain'), _('Samba domain'),
            'sambaDomainName', TRUE
          ),
          new CompositeAttribute(
            _('Samba informations'),
            'sambaSID',
            array(
              new HiddenAttribute(
                'SID'
              ),
              new SelectAttribute(
                _('Group type'), _('Samba group type'),
                'GID', TRUE,
                array(0,                512,                513,                514),
                0,
                array(_("Samba group"), _("Domain admins"), _("Domain users"),  _("Domain guests"))
              )
            ),
            '/^([^ ]+)-([0-9]+)$/',
            '%s-%d'
          ),
          new HiddenAttribute(
            'sambaGroupType', 2
          )
        )
      )
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    // User wants me to fake the idMappings? This is useful for
    //  making winbind resolve the user names in a reasonable amount
    //  of time in combination with larger databases.
    if ($config->get_cfg_value('sambaidmapping') == 'TRUE') {
      $this->objectclasses[] = 'sambaIdmapEntry';
    }

    parent::__construct($config, $dn, $object);

    $this->attributesAccess['sambaDomainName']->setInLdap(FALSE);
    $this->attributesAccess['sambaDomainName']->setChoices(array_keys($this->config->data['SERVERS']['SAMBA']));

    /* Get samba Domain in case of samba 3 */
    if ($this->sambaSID != "") {
      /* Get domain name for SID */
      $this->sambaDomainName = "DEFAULT";

      foreach ($this->config->data['SERVERS']['SAMBA'] as $key => $val) {
        if ($val['SID'] == $this->getSID()) {
          $this->sambaDomainName = $key;
          break;
        }
      }
    } else {
      if ($this->config->get_cfg_value("sid") != "") {
        $this->sambaDomainName  = "DEFAULT";
        $this->setSID($this->config->get_cfg_value("sid"));
      } else {
        msg_dialog::display(_("Configuration error"), _("Cannot find group SID in your configuration!"), ERROR_DIALOG);
      }
    }

    $this->attributesAccess['sambaDomainName']->setInitialValue($this->sambaDomainName);

    if ($this->getGID() < 500 || $this->getGID() > 553) {
      $this->setGID(0);
    }

    /* Don't loose special groups! If not key'ed above, add it to the combo box */
    if (!in_array($this->getGID(), $this->attributesAccess['sambaSID']->attributes[1]->getChoices())) {
      $this->attributesAccess['sambaSID']->attributes[1]->setChoices(
        array(0,                512,                513,                514,                $this->getGID()),
        array(_("Samba group"), _("Domain admins"), _("Domain users"),  _("Domain guests"), sprintf(_("Special group (%d)"), $this->getGID()))
      );
    }
    $this->attributesAccess['sambaSID']->attributes[1]->setInitialValue($this->getGID());

    $this->updateAttributesValues();
  }

  function prepare_save()
  {
    parent::prepare_save();

    // Generate rid / primaryGroupId
    if (!isset($this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['SID'])) {
      msg_dialog::display(_("Warning"), _("Undefined Samba SID detected. Please fix this problem manually!"), WARNING_DIALOG);
    } else {
      $this->setSID($this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['SID']);
    }

    if ($this->getGID() == 0) {
      if (($this->getSID() == "") ||
          $this->attributesAccess['sambaDomainName']->hasChanged() ||
          $this->attributesAccess['sambaSID']->attributes[1]->hasChanged()) {
        $sid                      = $this->getSambaSID();
        $this->attrs['sambaSID']  = $sid;
        $this->sambaSID           = $sid;
      } else {
        unset($this->attrs['sambaSID']);
      }
    } else {
      $this->attrs['sambaSID'] = $this->sambaSID;
    }
  }

  /*! \brief Get a new SambaSID for a group */
  function getSambaSID()
  {
    $ldap       = $this->config->get_ldap_link();
    $gidNumber  = $this->parent->getBaseObject()->gidNumber;
    do {
      $sid = $this->getSID()."-".($gidNumber * 2 + $this->getRidBase() + 1);
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(sambaSID=$sid)", array("sambaSID"));
      $gidNumber++;
    } while ($ldap->count() != 0);

    return $sid;
  }

  function resetCopyInfos()
  {
    parent::resetCopyInfos();

    /* Set a new SID */
    $this->sambaSID = "";
  }

  function getRidBase()
  {
    return $this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['RIDBASE'];
  }

  function getSID()
  {
    return $this->attributesAccess['sambaSID']->attributes[0]->getValue();
  }

  function setSID($value)
  {
    $this->attributesAccess['sambaSID']->attributes[0]->setValue($value);
  }

  function getGID()
  {
    return $this->attributesAccess['sambaSID']->attributes[1]->getValue();
  }

  function setGID($value)
  {
    $this->attributesAccess['sambaSID']->attributes[1]->setValue($value);
  }

}
?>
