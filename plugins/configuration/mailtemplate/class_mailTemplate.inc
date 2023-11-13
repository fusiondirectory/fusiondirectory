<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2012-2022 FusionDirectory

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

class mailTemplate extends simplePlugin
{
  var $displayHeader = FALSE;

  public static function plInfo (): array
  {
    return [
      'plShortName' => _('Mail Template'),
      'plDescription' => _('Mail Template'),
      'plObjectClass' => ['fdMailTemplate'],
      'plFilter' => '(objectClass=fdMailTemplate)',
      'plObjectType' => ['mailTemplate' => [
        'name' => _('Mail Template'),
        'ou' => get_ou('mailTemplateRDN'),
        'icon' => 'geticon.php?context=applications&icon=mail-template&size=16',
      ]],
      'plProvidedAcls' => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    global $config;
    return [
      // Attributes are grouped by section
      'mailTemplate' => [
        'name' => _('Mail Object'),
        'attrs' => [
          new StringAttribute(
            _('Mail Template Name'), _('Mail Template Name'),
            'cn', TRUE
          ),
          new StringAttribute(
            _('Mail Subject'), _('Mail Subject'),
            'fdMailTemplateSubject', TRUE
          ),
          new TextAreaAttribute(
            _('Mail Template Body'), _('Text to be sent to recipient'),
            'fdMailTemplateBody', TRUE
          ),
          new TextAreaAttribute(
            _('Mail Signature'), _('Mail Signature'),
            'fdMailTemplateSignature', FALSE
          ),
          new BooleanAttribute(
            _('Read Receipt'),
            _('Read Receipt'),
            'fdMailTemplateReadReceipt', FALSE
          ),
          // Management of attachments files in a sub-node ldap format.
          new SubNodesAttribute(
            '', _('Attachments'),
            'attachments', ['fdMailAttachments'],
            [
              new StringAttribute(
                _('Name'), _('The name of the attachment file'),
                'cn', TRUE
              ),
              new FileDownloadAttribute(
                _('Attachment'), _('Import a file for this e-mail'),
                'fdMailAttachmentsContent', FALSE, '', TRUE
              ),
            ],
            FALSE, /* no order */
            [],
            TRUE /* edit enabled */
          ),
        ]
      ],
    ];
  }


  function __construct ($dn = NULL, $object = NULL, $parent = NULL, $mainTab = FALSE)
  {
    global $config;
    parent::__construct($dn, $object, $parent, $mainTab);

  }

  protected function shouldSave (): bool
  {
    // Trigger a save method required due to sub nodes not being triggered by default. (Modification and suppression).
    return TRUE;
  }


  function ldap_save (): array
  {
    global $config;
    $errors = parent::ldap_save();
    $this->attributesAccess['attachments']->postLdapSave($config->get_ldap_link());
    return $errors;
  }


}
