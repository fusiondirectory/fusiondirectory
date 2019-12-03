<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2012-2019  FusionDirectory

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

/*! \brief This class allow to handle easily an Date LDAP attribute
 *
 * We are using UTC timezone because we don't care about time, we just want date.
 */
class DateAttribute extends Attribute
{
  protected $format;
  protected $minDate = NULL;
  protected $maxDate = NULL;

  /*! \brief The constructor of DateAttribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param string $format The date format. It can be any format recognized by DateTime::format. see http://www.php.net/manual/fr/function.date.php
   *  \param mixed $defaultValue The default value for this attribute
   *  \param mixed $min Minimum valid value
   *  \param mixed $max Maximum valid value
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct (string $label, string $description, string $ldapName, bool $required, string $format, $defaultValue = 'today', $min = NULL, $max = NULL, string $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->format = $format;
    if ($min !== NULL) {
      try {
        $this->minDate = new DateTime($min, new DateTimeZone('UTC'));
      } catch (Exception $e) {
        // Failed to parse min date, ignore
      }
    }
    if ($max !== NULL) {
      try {
        $this->maxDate = new DateTime($max, new DateTimeZone('UTC'));
      } catch (Exception $e) {
        // Failed to parse max date, ignore
      }
    }
  }

  function inputValue ($value)
  {
    if (
        (($value === '') && !$this->isRequired()) ||
        ($this->isTemplate() && preg_match('/%/', $value))
      ) {
      return $value;
    } else {
      return $this->ldapToDate($value);
    }
  }

  function getValue ()
  {
    if ($this->value === "" && !$this->isRequired()) {
      return $this->value;
    } else {
      try {
        return $this->getDateValue()->format('Y-m-d');
      } catch (Exception $e) {
        return $this->value;
      }
    }
  }

  protected function ldapToDate ($ldapValue)
  {
    $date = DateTime::createFromFormat('!'.$this->format, $ldapValue, new DateTimeZone('UTC'));
    if ($date !== FALSE) {
      return $date;
    } else {
      return $ldapValue;
    }
  }

  protected function dateToLdap (DateTime $dateValue)
  {
    return $dateValue->format($this->format);
  }

  function getDateValue ()
  {
    $value = $this->value;
    if (!($value instanceof DateTime)) {
      $value = new DateTime($value, new DateTimeZone('UTC'));
    }
    return $value;
  }

  function computeLdapValue ()
  {
    if ($this->value === "" && !$this->isRequired()) {
      return $this->value;
    } elseif (!($this->value instanceof DateTime)) {
      try {
        $this->setValue($this->getDateValue());
      } catch (Exception $e) {
        return $this->value;
      }
    }
    return $this->dateToLdap($this->value);
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    } else {
      if (empty($this->value)) {
        return '';
      }
      try {
        $dateValue = $this->getDateValue();
        if (($this->minDate !== NULL) && ($dateValue < $this->minDate)) {
          return sprintf(_('Invalid date in %s, should be newer than: %s'), $this->getLabel(), $this->minDate->format('Y-m-d'));
        }
        if (($this->maxDate !== NULL) && ($dateValue > $this->maxDate)) {
          return sprintf(_('Invalid date in %s, should be older than: %s'), $this->getLabel(), $this->maxDate->format('Y-m-d'));
        }
      } catch (Exception $e) {
        if ($this->isTemplate() && preg_match('/%/', $this->value)) {
          return '';
        } else {
          return sprintf(_('Error, incorrect date: %s'), $e->getMessage());
        }
      }
      return '';
    }
  }

  function renderFormInput (): string
  {
    $attributes = [
      'value'   => '{literal}'.$this->getValue().'{/literal}',
      'pattern' => '{literal}[0-9]{4}-[0-9]{2}-[0-9]{2}{/literal}',
    ];
    if ($this->minDate !== NULL) {
      $attributes['min'] = $this->minDate->format('Y-m-d');
    }
    if ($this->maxDate !== NULL) {
      $attributes['max'] = $this->maxDate->format('Y-m-d');
    }
    $display = $this->renderInputField('date', $this->getHtmlId(), $attributes);
    return $this->renderAcl($display);
  }

  function renderTemplateInput (): string
  {
    $id = $this->getHtmlId();
    $attributes = [
      'value' => '{literal}'.htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8').'{/literal}'
    ];
    if ($this->isSubAttribute) {
      $attributes['class'] = 'subattribute';
    }
    $display = $this->renderInputField('text', $id, $attributes);
    return $this->renderAcl($display);
  }
}

/*!
 * \brief Date attribute storing standard LDAP GeneralizedTime format
 */
class GeneralizedTimeDateAttribute extends DateAttribute
{
  function __construct (string $label, string $description, string $ldapName, bool $required, $defaultValue = 'today', $min = NULL, $max = NULL, string $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, '', $defaultValue, $min, $max, $acl);
  }

  protected function ldapToDate ($ldapValue)
  {
    try {
      return LdapGeneralizedTime::fromString($ldapValue);
    } catch (LdapGeneralizedTimeBadFormatException $e) {
      return $ldapValue;
    }
  }

  protected function dateToLdap (DateTime $dateValue)
  {
    return LdapGeneralizedTime::toString($dateValue);
  }
}

/*!
 * \brief Attribute storing time as His format
 */
class TimeHisAttribute extends CompositeAttribute
{
  protected $convert;

  function __construct ($label, $description, $ldapName, $required, $convert = TRUE, $acl = '')
  {
    $this->convert = $convert;
    $attributes = [
      new IntAttribute(
        '',  _('Hours'),
        $ldapName.'_hours', TRUE,
        0, 23, 1
      ),
      new IntAttribute(
        ':', _('Minutes'),
        $ldapName.'_minutes', TRUE,
        0, 59, 0
      ),
      new IntAttribute(
        ':', _('Seconds'),
        $ldapName.'_seconds', TRUE,
        0, 59, 0
      )
    ];
    parent::__construct($description, $ldapName, $attributes, '/^(\d\d)(\d\d)(\d\d)$/', '%02d%02d%02d', $acl, $label);
    $this->setLinearRendering(TRUE);
  }

  function readValues (string $value): array
  {
    $values = parent::readValues($value);
    if ($this->convert) {
      $datetime = new DateTime('T'.implode(':', $values), timezone::utc());

      $datetime->setTimeZone(timezone::getDefaultTimeZone());
      if (count($values) < 3) {
        $values = explode(':', $datetime->format('H:i'));
      } else {
        $values = explode(':', $datetime->format('H:i:s'));
      }
    }
    return $values;
  }

  function writeValues (array $values)
  {
    if ($this->convert) {
      $datetime = new DateTime('T'.implode(':', $values), timezone::getDefaultTimeZone());
      $datetime->setTimeZone(timezone::utc());
      if (count($values) < 3) {
        $values = explode(':', $datetime->format('H:i'));
      } else {
        $values = explode(':', $datetime->format('H:i:s'));
      }
    }
    return parent::writeValues($values);
  }

  function displayValue ($value): string
  {
    $values = parent::readValues($value);
    $datetime = new DateTime('T'.implode(':', $values), timezone::utc());
    if ($this->convert) {
      $datetime->setTimeZone(timezone::getDefaultTimeZone());
    }
    if (count($values) < 3) {
      return $datetime->format('H:i');
    } else {
      return $datetime->format('H:i:s');
    }
  }
}

/*!
 * \brief Attribute storing time as Hi format
 */
class TimeHiAttribute extends TimeHisAttribute
{
  function __construct ($label, $description, $ldapName, $required, $convert = TRUE, $acl = '')
  {
    $this->convert = $convert;
    $attributes = [
      new IntAttribute(
        '',  _('Hours'),
        $ldapName.'_hours', TRUE,
        0, 23, 1
      ),
      new IntAttribute(
        ':', _('Minutes'),
        $ldapName.'_minutes', TRUE,
        0, 59, 0
      )
    ];
    CompositeAttribute::__construct($description, $ldapName, $attributes, '/^(\d\d)(\d\d)$/', '%02d%02d', $acl, $label);
    $this->setLinearRendering(TRUE);
  }
}

/*!
 * \brief Attribute storing both Date and Time
 */
class DateTimeAttribute extends CompositeAttribute
{
  function __construct ($label, $description, $ldapName, $required, $acl = '')
  {
    $attributes = [
      new DateAttribute(
        _('Date'), '',
        $ldapName.'_date', $required,
        'Ymd'
      ),
      /* Disabling conversion as it needs to be done on the date+time level (see argonautAction for an example) */
      new TimeHisAttribute(
        _('Time'), '',
        $ldapName.'_time', $required,
        FALSE
      )
    ];
    parent::__construct($description, $ldapName, $attributes, '/^(\d{8})(\d{6})$/', '%s%s', $acl, $label);
  }
}

/*!
 * \brief Read-only GeneralizedTimeDateAttribute. Used by audit plugin
 */
class GeneralizedTimeDisplayAttribute extends GeneralizedTimeDateAttribute
{
  function getValue ()
  {
    return $this->computeLdapValue();
  }

  function renderFormInput (): string
  {
    $date = $this->getDateValue();
    $date->setTimezone(timezone::getDefaultTimeZone());
    return htmlentities($date->format('Y-m-d, H:i:s'), ENT_COMPAT, 'UTF-8');
  }
}

/*!
 * \brief Date stored as days since Unix epoch. Used by posix plugin.
 */
class EpochDaysDateAttribute extends DateAttribute
{
  /* 24 * 60 * 60 = 86400 */
  public static $secondsPerDay = 86400;

  function __construct (string $label, string $description, string $ldapName, bool $required, $defaultValue = 'today', $min = NULL, $max = NULL, string $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, '', $defaultValue, $min, $max, $acl);
  }

  protected function ldapToDate ($ldapValue)
  {
    $date = DateTime::createFromFormat('U', $ldapValue * static::$secondsPerDay, timezone::utc());
    if ($date !== FALSE) {
      return $date;
    } else {
      trigger_error('LDAP value for '.$this->getLdapName().' was not in the right date format.');
      return new DateTime($ldapValue, timezone::utc());
    }
  }

  protected function dateToLdap (DateTime $dateValue)
  {
    return floor($dateValue->format('U') / static::$secondsPerDay);
  }

  function getEpochDays ()
  {
    if (empty($this->value)) {
      return 0;
    } else {
      try {
        return $this->dateToLdap($this->getDateValue());
      } catch (Exception $e) {
        if (is_object($this->plugin) && $this->plugin->is_template) {
          return $this->value;
        } else {
          throw $e;
        }
      }
    }
  }
}
