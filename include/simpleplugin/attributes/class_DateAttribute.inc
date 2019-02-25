<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2018  FusionDirectory

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
  protected $defaultDate;

  /*! \brief The constructor of DateAttribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param string $format The date format. It can be any format recognized by DateTime::format. see http://www.php.net/manual/fr/function.date.php
   *  \param mixed $defaultValue The default value for this attribute
   *  \param mixed $defaultDate The default date for the date picker when attribute is empty
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct ($label, $description, $ldapName, $required, $format, $defaultValue = 'now', $defaultDate = NULL, $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->format = $format;
    if ($defaultDate !== NULL) {
      try {
        $date = new DateTime($defaultDate, new DateTimeZone('UTC'));
        $defaultDate = $date->format('d.m.Y');
      } catch (Exception $e) {
        $defaultDate = NULL;
      }
    }
    $this->defaultDate = $defaultDate;
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
        return $this->getDateValue()->format('d.m.Y');
      } catch (Exception $e) {
        return $this->value;
      }
    }
  }

  protected function ldapToDate ($ldapValue)
  {
    $date = DateTime::createFromFormat($this->format, $ldapValue, new DateTimeZone('UTC'));
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
      if ($this->value instanceof DateTime) {
        return;
      } else {
        try {
          $this->getDateValue();
        } catch (Exception $e) {
          if ($this->isTemplate() && preg_match('/%/', $this->value)) {
            return;
          } else {
            return sprintf(_('Error, incorrect date: %s'), $e->getMessage());
          }
        }
      }
    }
  }

  function renderFormInput ()
  {
    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    $id = $this->getHtmlId();
    $display = $this->renderInputField(
      'text', $id,
      [
        'value' => '{literal}'.$this->getValue().'{/literal}',
        'class' => 'date'
      ]
    );
    $display  .= '{if $'.$this->getAcl().'ACL|regex_replace:"/[cdmr]/":"" == "w"}'.
        '<script type="text/javascript">
          {literal}
          var datepicker  = new DatePicker('.
            '{ '.
              'relative : \''.$id.'\', '.
              (($this->defaultDate !== NULL) ? 'defaultDate : \''.$this->defaultDate.'\', ' : '').
              'language : \'{/literal}{$lang}{literal}\', '.
              'keepFieldEmpty : true, '.
              'enableCloseEffect : false, '.
              'enableShowEffect : false '.
            '}'.
          ');
          {/literal}
        </script>
        {/if}';
    return $this->renderAcl($display);
  }
}

/*!
 * \brief Date attribute storing standard LDAP GeneralizedTime format
 */
class GeneralizedTimeDateAttribute extends DateAttribute
{
  function __construct ($label, $description, $ldapName, $required, $defaultValue = 'now', $defaultDate = NULL, $acl = '')
  {
    parent::__construct($label, $description, $ldapName, $required, '', $defaultValue, $defaultDate, $acl);
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

  function readValues ($value)
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

  function displayValue ($value)
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
