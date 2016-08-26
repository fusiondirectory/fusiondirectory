<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2016  FusionDirectory

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

  /*! \brief The constructor of DateAttribute
   *
   *  \param string $label The label to show for this attribute
   *  \param string $description A more detailed description for the attribute
   *  \param string $ldapName The name of the attribute in the LDAP (If it's not in the ldap, still provide a unique name)
   *  \param boolean $required Is this attribute mandatory or not
   *  \param string $format The date format. It can be any format recognized by DateTime::format. see http://www.php.net/manual/fr/function.date.php
   *  \param mixed $defaultValue The default value for this attribute
   *  \param string $acl The name of the acl for this attribute if he does not use its own. (Leave empty if he should use its own like most attributes do)
   */
  function __construct ($label, $description, $ldapName, $required, $format, $defaultValue = 'now', $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->format = $format;
  }

  function inputValue ($value)
  {
    if ($value === "" && !$this->isRequired()) {
      return $value;
    } elseif (is_object($this->plugin) && $this->plugin->is_template && preg_match('/%/', $value)) {
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
        if (is_object($this->plugin) && $this->plugin->is_template && preg_match('/%/', $this->value)) {
          return $this->value;
        } else {
          return '';
        }
      }
    }
  }

  protected function ldapToDate($ldapValue)
  {
    $date = DateTime::createFromFormat($this->format, $ldapValue, new DateTimeZone('UTC'));
    if ($date !== FALSE) {
      return $date;
    } else {
      trigger_error('LDAP value for '.$this->getLdapName().' was not in the right date format.');
      return new DateTime($ldapValue, new DateTimeZone('UTC'));
    }
  }

  protected function dateToLdap($dateValue)
  {
    return $dateValue->format($this->format);
  }

  function getDateValue()
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
        if (is_object($this->plugin) && $this->plugin->is_template && preg_match('/%/', $this->value)) {
          return $this->value;
        } else {
          throw $e;
        }
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
          if (is_object($this->plugin) && $this->plugin->is_template && preg_match('/%/', $this->value)) {
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
      array(
        'value' => '{literal}'.$this->getValue().'{/literal}',
        'class' => 'date'
      )
    );
    $display  .= '{if $'.$this->getAcl().'ACL|regex_replace:"/[cdmr]/":"" == "w"}'.
        '<script type="text/javascript">
          {literal}
          var datepicker  = new DatePicker({ relative : \''.$id.'\', language : \'{/literal}{$lang}{literal}\', keepFieldEmpty : true,
                                             enableCloseEffect : false, enableShowEffect : false });
          {/literal}
        </script>
        {/if}';
    return $this->renderAcl($display);
  }
}

class GeneralizedTimeDateAttribute extends DateAttribute
{
  function __construct ($label, $description, $ldapName, $required, $defaultValue = 'now', $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, '', $defaultValue, $acl);
  }

  protected function ldapToDate($ldapValue)
  {
    try {
      return LdapGeneralizedTime::fromString($ldapValue);
    } catch (LdapGeneralizedTimeBadFormatException $e) {
      trigger_error('LDAP value "'.$ldapValue.'" for '.$this->getLdapName().' is not in the right date format.');
      return new DateTime($ldapValue, timezone::utc());
    }
  }

  protected function dateToLdap($dateValue)
  {
    return LdapGeneralizedTime::toString($dateValue);
  }
}
