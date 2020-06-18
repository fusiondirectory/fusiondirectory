<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2019-2020  FusionDirectory

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

/*! \class SimplePluginLdapError
    \brief Error returned by an LDAP operation called from SimplePlugin
*/
class SimplePluginLdapError extends SimplePluginError
{
  protected $dn;
  protected $operation;

  public function __construct ($origin, string $dn = NULL, int $operation = NULL, string $ldapError = '', int $code = 0, Throwable $previous = NULL)
  {
    $this->dn         = $dn;
    $this->operation  = $operation;

    parent::__construct($origin, htmlescape($ldapError), $code, $previous);
  }

  public function toArray (): array
  {
    $array = parent::toArray();

    $typemap = [1 => 'read','add','modify','delete','search','authentication'];

    $array['dn'] = $this->dn;
    if (isset($this->operation)) {
      $array['operation'] = $typemap[$this->operation];
    }
    $array['errno'] = $this->getCode();

    return $array;
  }

  public function computeMsgDialogParameters (): array
  {
    $html = '';

    if (isset($this->object)) {
      $html .= htmlescape($this->object->getBaseObject()->dn.' > ');
    }

    if (isset($this->tab)) {
      $html .= htmlescape($this->tab->parent->by_name[get_class($this->tab)].' > ');
    }

    if (isset($this->attribute)) {
      $label = $this->attribute->getLabel();
      if (empty($label)) {
        $html .= '<i>'.htmlescape($this->attribute->getLdapName()).'</i>';
      } else {
        $html .= htmlescape($label);
      }
    }

    $html .= '<br/><br/>';

    /* Assign headline depending on type */
    $typemap = [1 => _('read operation'), _('add operation'), _('modify operation'),
        _('delete operation'), _('search operation'), _('authentication')];

    if (isset($this->operation)) {
      $html .= htmlescape(sprintf(_('LDAP %s failed!'), $typemap[$this->operation]));
    } else {
      $html .= htmlescape(_('LDAP operation failed!'));
    }

    if (!empty($this->dn)) {
      $html .= '<br/><br/><i>'.htmlescape(_('Object:')).'</i> '.htmlescape($this->dn);
    }

    $html .= '<br/><br/><i>'.htmlescape(_('Error:')).'</i> '.$this->htmlMessage;

    $trace = $this->getTrace();

    array_unshift(
      $trace,
      [
        'file' => $this->getFile(),
        'line' => $this->getLine(),
      ]
    );

    return [_('LDAP error'), $html, LDAP_ERROR, $trace];
  }
}
