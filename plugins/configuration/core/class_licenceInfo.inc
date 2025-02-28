<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2020-2021  FusionDirectory

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

class licenceInfo extends simplePlugin
{
  /* ldap attribute => ini variable */
  public static $subscriptionAttributes = [
    'fdSubscriptionName'        => 'Name',
    'uid'                       => 'Id',
    'fdSubscriptionType'        => 'Type',
    'fdSubscriptionContractId'  => 'Contract',
    'fdSubscriptionStartDate'   => 'StartDate',
    'fdSubscriptionEndDate'     => 'EndDate',
  ];

  public static $sectionName = 'FusionDirectory Subscription';

  public static function plInfo (): array
  {
    return [
      'plShortName'   => _('Subscription'),
      'plTitle'       => _('Subscription information'),
      'plDescription' => _('Information about your FusionDirectory subscription'),
      'plIcon'        => 'geticon.php?context=applications&icon=fusiondirectory&size=48',
      'plObjectClass' => ['fdSubscriptionInformation'],
      'plObjectType'  => [
        'subscriptionInfo' => [
          'name'      => _('Subscription information'),
          'ou'        => get_ou('fusiondirectoryRDN'),
        ]
      ],
      'plSection'     => 'conf',
      'plPriority'    => 1,

      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo()),
    ];
  }

  public static function getAttributesInfo (): array
  {
    return [
      'info' => [
        'name'  => _('Subscription information'),
        'attrs' => [
          new HiddenAttribute(
            'cn', TRUE, 'subscription'
          ),
          new DisplayAttribute(
            '', _('Information text for subscription'),
            'fdSubscriptionText', FALSE
          ),
          new DisplayLDAPAttribute(
            _('Name'), _('Subscription name'),
            'fdSubscriptionName', TRUE
          ),
          new DisplayLDAPAttribute(
            _('Id'), _('Subscription id'),
            'uid', TRUE
          ),
          new DisplayLDAPAttribute(
            _('Type'), _('Subscription type'),
            'fdSubscriptionType', TRUE
          ),
          new DisplayLDAPAttribute(
            _('Contract'), _('Contract reference'),
            'fdSubscriptionContractId', TRUE
          ),
          new GeneralizedTimeDisplayAttribute(
            _('Start date'), _('Start date of this subscription'),
            'fdSubscriptionStartDate', TRUE,
            '', 'Y-m-d'
          ),
          new GeneralizedTimeDisplayAttribute(
            _('End date'), _('End date of this subscription'),
            'fdSubscriptionEndDate', TRUE,
            '', 'Y-m-d'
          ),
        ],
      ],
      'import' => [
        'name'  => _('Import your subscription'),
        'attrs' => [
          new DisplayAttribute(
            '', _('Information text for expired subscription'),
            'fdSubscriptionTextExpired', FALSE
          ),
          new FileAttribute(
            '', _('Import subscription'),
            'import_file', FALSE
          ),
          new ButtonAttribute(
            '', '',
            'import',
            _('Import'),
            NULL, '',
            'import_file'
          ),
        ],
      ],
    ];
  }

  public function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    parent::__construct($dn, $object, $parent, $mainTab);

    $this->attributesAccess['import_file']->setInLdap(FALSE);
    $this->attributesAccess['import']->setInLdap(FALSE);

    $this->fdSubscriptionText = '<p>'.nl2br(sprintf(
      htmlescape(_("You do not have a valid subscription for this instance of FusionDirectory.\nPlease visit %s for a list of available options.")),
      '<a href="https://www.fusiondirectory.org/support/" target="_blank"><strong>https://www.fusiondirectory.org/support/</strong></a>'
    )).'</p>';
    $this->attributesAccess['fdSubscriptionText']->setAllowHTML(TRUE);
    $this->fdSubscriptionTextExpired = '<p>'.nl2br(sprintf(
      htmlescape(_("Your subscription is expired for this instance of FusionDirectory.\nPlease visit %s to renew it.")),
      '<a href="https://www.fusiondirectory.org/support/" target="_blank"><strong>https://www.fusiondirectory.org/support/</strong></a>'
    )).'</p>';
    $this->attributesAccess['fdSubscriptionTextExpired']->setAllowHTML(TRUE);
  }

  public function update (): bool
  {
    parent::update();

    if ($this->uid !== '') {
      $this->attributesAccess['fdSubscriptionText']->setVisible(FALSE);
      $this->attributesAccess['fdSubscriptionName']->setVisible(TRUE);
      $this->attributesAccess['uid']->setVisible(TRUE);
      $this->attributesAccess['fdSubscriptionType']->setVisible(TRUE);
      $this->attributesAccess['fdSubscriptionContractId']->setVisible(TRUE);
      $this->attributesAccess['fdSubscriptionStartDate']->setVisible(TRUE);
      $this->attributesAccess['fdSubscriptionEndDate']->setVisible(TRUE);
      if ($this->attributesAccess['fdSubscriptionEndDate']->getDateValue() < new DateTime()) {
        $this->attributesAccess['fdSubscriptionTextExpired']->setVisible(TRUE);
      } else {
        $this->attributesAccess['fdSubscriptionTextExpired']->setVisible(FALSE);
      }
    } else {
      $this->attributesAccess['fdSubscriptionText']->setVisible(TRUE);
      $this->attributesAccess['fdSubscriptionName']->setVisible(FALSE);
      $this->attributesAccess['uid']->setVisible(FALSE);
      $this->attributesAccess['fdSubscriptionType']->setVisible(FALSE);
      $this->attributesAccess['fdSubscriptionContractId']->setVisible(FALSE);
      $this->attributesAccess['fdSubscriptionStartDate']->setVisible(FALSE);
      $this->attributesAccess['fdSubscriptionEndDate']->setVisible(FALSE);
      $this->attributesAccess['fdSubscriptionTextExpired']->setVisible(FALSE);
    }

    return TRUE;
  }

  public function handle_import ()
  {
    $data = $this->import_file;
    if (empty($data)) {
      /* No file or empty file */
      $error = new SimplePluginError(
        $this->attributesAccess['import_file'],
        htmlescape(_('No data. Did you forgot to upload a file?'))
      );
      $error->display();
    } elseif (($data = parse_ini_string($data, TRUE)) === FALSE) {
      /* Import of INI failed */
      $error = new SimplePluginError(
        $this->attributesAccess['import_file'],
        htmlescape(_('Failed to parse imported file'))
      );
      $error->display();
    } else {
      /* Check content of ini file */
      $errorText = NULL;
      if (!isset($data[static::$sectionName])) {
        $errorText = sprintf(_('Missing section "%s" in imported file'), static::$sectionName);
      } else {
        $data = $data[static::$sectionName];
        foreach (static::$subscriptionAttributes as $attr => $iniVar) {
          if ($this->attributesAccess[$attr]->isRequired() && (!isset($data[$iniVar]) || ($data[$iniVar] === ''))) {
            $errorText = sprintf(_('Missing attribute "%s" in imported file'), $iniVar);
            break;
          }
        }
      }
      if ($errorText !== NULL) {
        $error = new SimplePluginError(
          $this->attributesAccess['import_file'],
          htmlescape($errorText)
        );
        $error->display();
      } else {
        /* Import data and save it to the LDAP */
        foreach (static::$subscriptionAttributes as $attr => $iniVar) {
          $this->attributesAccess[$attr]->setValue($data[$iniVar] ?? '');
        }
        $errors = $this->parent->save();
        if (count($errors) > 0) {
          msg_dialog::displayChecks($errors);
          /* Reset fields to LDAP value */
          foreach (array_keys(static::$subscriptionAttributes) as $attr) {
            $this->attributesAccess[$attr]->setValue($this->attributesAccess[$attr]->getInitialValue());
          }
        } else {
          /* The object will stay open so it needs to appear as loaded from LDAP */
          $this->is_account             = TRUE;
          $this->initially_was_account  = TRUE;
        }
      }
    }
    /* Avoid double import */
    $this->import_file = '';
  }

  public static function mainInc ($classname = NULL, $entry_dn = NULL, $tabs = FALSE, $edit_mode = FALSE, $objectType = FALSE)
  {
    global $config;

    if ($entry_dn === NULL) {
      $entry_dn = 'cn=subscription,'.get_ou('fusiondirectoryRDN').$config->current['BASE'];
    }

    parent::mainInc($classname, $entry_dn, $tabs, $edit_mode, $objectType);
  }
}
