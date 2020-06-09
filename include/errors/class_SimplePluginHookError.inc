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

/*! \class SimplePluginHookError
    \brief Error returned by a hook called from SimplePlugin
*/
class SimplePluginHookError extends SimplePluginError
{
  protected $hookType;

  public function __construct ($origin, string $type, string $output = '', int $code = 0, Throwable $previous = NULL)
  {
    $this->hookType = $type;

    parent::__construct($origin, htmlescape($output), $code, $previous);
  }

  public function toArray (): array
  {
    $array = parent::toArray();

    $array['hookType']  = $this->hookType;
    $array['hookCode']  = $this->getCode();

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

    $html .= htmlescape(sprintf(_('Trigger "%s" returned an error!'), $this->hookType));

    $html .= '<br/><br/>'."\n";
    $html .= htmlescape(sprintf(_('Exit code: %d'), $this->getCode())).'<br/>'."\n";
    if (!empty($this->htmlMessage)) {
      $html .= 'Result: '.$this->htmlMessage."\n";
    }

    $trace = $this->getTrace();

    array_unshift(
      $trace,
      [
        'file' => $this->getFile(),
        'line' => $this->getLine(),
      ]
    );

    return [_('Error'), $html, ERROR_DIALOG, $trace];
  }
}
