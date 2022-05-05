<?php

class mailTemplateUserReminder extends simplePlugin
{
  protected $displayHeader = TRUE;

  static function plInfo (): array
  {
    return [
      'plShortName'   => _('Mail Template User Reminder'),
      'plDescription' => _('Mail Template User Reminder'),
      'plIcon'        => 'geticon.php?context=applications&icon=supann&size=48',
      'plSmallIcon'   => 'geticon.php?context=applications&icon=supann-status&size=16',
      'plPriority'    => 32,
      'plObjectClass' => ['fdMailTemplateUserReminder'],
      'plFilter'      => '(objectClass=fdMailTemplateUserReminder)',
      'plObjectType'  => ['mailTemplate'],
      'plConflicts'   => ['mailTemplateUserRecovery'],
      'plProvidedAcls'  => parent::generatePlProvidedAcls(static::getAttributesInfo())
    ];
  }

  static function getAttributesInfo (): array
  {
    return [
      'emailppolicy' => [
        'name'  => _('Ppolicy email settings'),
        'attrs' => [
          new BooleanAttribute(
            _('Forward alerts to the manager'), _('Forward ppolicy alerts to the manager'),
            'fdMTUserReminderForwardPpolicyAlert', FALSE,
            TRUE
          ),
          new StringAttribute(
            _('Subject'), _('Subject of the ppolicy alert email'),
            'fdMTUserReminderPpolicyAlertSubject', FALSE,
            _('[FusionDirectory] Your password is about to expire')
          ),
          new TextAreaAttribute(
            _('Body (%s are cn and login)'),
            _('Body of the ppolicy alert email, sent when the user password is about to expire.'),
            'fdMTUserReminderPpolicyAlertBody', FALSE,
            _('Dear %sn%,'."\n".
              'your password for account %login% is about to expire, please change your password: '."\n".
              'https://'.$_SERVER['SERVER_NAME'].
              (($_SERVER['SERVER_PORT'] != '80') ? ':'.$_SERVER['SERVER_PORT'] : '').
              (empty($_SERVER['REQUEST_URI']) ? $_SERVER['PATH_INFO'] : $_SERVER['REQUEST_URI']).
              "\n")
          )
        ]
      ],
      'alert_mail_reminder' => [
        'name'  => _('Alert email settings'),
        'attrs' => [
          new BooleanAttribute(
            _('Forward alerts to the manager'), _('Forward alerts to the manager'),
            'fdMTUserReminderForwardAlert', TRUE,
            TRUE
          ),
          new StringAttribute(
            _('Subject'), _('Subject of the alert email'),
            'fdMTUserReminderAlertSubject', FALSE,
            _('[FusionDirectory] Your account is about to expire')
          ),
          new TextAreaAttribute(
            _('Body'),
            _('Body of the alert email, sent when the user is about to expire.'),
            'fdMTUserReminderAlertBody', FALSE,
            _('Dear %sn%,'."\n".
              'your account %login% is about to expire, please use this link to avoid this: '."\n".
              'https://'.$_SERVER['SERVER_NAME'].
              (($_SERVER['SERVER_PORT'] != '80') ? ':'.$_SERVER['SERVER_PORT'] : '').
              (empty($_SERVER['REQUEST_URI']) ? $_SERVER['PATH_INFO'] : $_SERVER['REQUEST_URI']).
              '/expired_postpone.php?uid=%login%&token=%token%'."\n")
          )
        ]
      ],
      'alert_mail_confirmation' => [
        'name'  => _('Confirmation email settings'),
        'attrs' => [
          new BooleanAttribute(
            _('Forward confirmation to the manager'), _('Forward account extension confirmation to the manager'),
            'fdMTUserReminderForwardConfirmation', TRUE,
            TRUE
          ),
          new StringAttribute(
            _('Subject'), _('Subject of the confirmation email'),
            'fdMTUserReminderConfirmationSubject', FALSE,
            _('[FusionDirectory] Your account expiration has been postponed')
          ),
          new TextAreaAttribute(
            _('Body'),
            _('Body of the confirmation email, sent when the user has postponed expiration.'),
            'fdMTUserReminderConfirmationBody', FALSE,
            _('Dear %sn%,'."\n".' your account %login% expiration has been successfully postponed.'."\n")
          ),
        ]
      ],
      'alert_mail_expiration' => [
        'name'  => _('Expiration email settings'),
        'attrs' => [
          new BooleanAttribute(
            _('Forward to the manager'), _('Forward account expiration to the manager'),
            'fdMTUserReminderForwardExpiration', TRUE,
            TRUE
          ),
          new StringAttribute(
            _('Subject'), _('Subject of the expiration email'),
            'fdMTUserReminderExpirationSubject', FALSE,
            _('[FusionDirectory] Your account has expired')
          ),
          new TextAreaAttribute(
            _('Body (%s are cn and login)'),
            _('Body of the expiration email, sent when the user has expired.'),
            'fdMTUserReminderExpirationBody', FALSE,
            _('Dear %sn%,'."\n".' your account %login% has expired.'."\n")
          ),
        ]
      ],
    ];
  }

  function __construct (string $dn = NULL, $object = NULL, $parent = NULL, bool $mainTab = FALSE, array $attributesInfo = NULL)
  {
    parent::__construct($dn, $object, $parent, $mainTab, $attributesInfo);

    if (!class_available('ppolicy')) {
      $this->attributesAccess['fdMTUserReminderForwardPpolicyAlert']->setDisabled(TRUE);
      $this->attributesAccess['fdMTUserReminderPpolicyAlertSubject']->setDisabled(TRUE);
      $this->attributesAccess['fdMTUserReminderPpolicyAlertBody']->setDisabled(TRUE);
    }
  }

  protected function shouldSave (): bool
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search('(&(objectClass=fdMailTemplateUserReminder))', ['cn','fdMTUserReminderAlertSubject']);
    while ($attrs = $ldap->fetch()) {
      if (isset($this->attrs['fdMTUserReminderAlertSubject'])) {
        $error = new FusionDirectoryError(htmlescape(sprintf(_('User Reminder Mail Template Is Already Existing!'))));
        $error->display();

        return FALSE;
      }
    }

    return TRUE;
  }
}

