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

/*! \class FusionDirectoryError
    \brief Parent class for all errors in FusionDirectory
*/
class FusionDirectoryError extends Error
{
  protected $htmlMessage;

  public function __construct (string $htmlMessage = '', int $code = 0, Throwable $previous = NULL)
  {
    $this->htmlMessage = $htmlMessage;
    parent::__construct(htmlunescape(strip_tags($htmlMessage)), $code, $previous);
  }

  public function getHtmlMessage ()
  {
    return $this->htmlMessage;
  }

  public function toArray (): array
  {
    return [
      'class'   => get_class($this),
      'message' => $this->getMessage(),
      'line'    => $this->getLine(),
      'file'    => $this->getFile(),
    ];
  }

  public function __toString ()
  {
    return $this->getMessage();
  }

  public function computeMsgDialogParameters (): array
  {
    return [_('Error'), $this->htmlMessage, ERROR_DIALOG, static::formatTrace($this)];
  }

  public function display ()
  {
    msg_dialog::display(...$this->computeMsgDialogParameters());
  }

  public static function formatTrace (Throwable $throwable): array
  {
    $trace = $throwable->getTrace();

    foreach ($trace as &$traceStep) {
      if (isset($traceStep['function']) && isset($traceStep['class']) && isset($traceStep['type'])) {
        $traceStep['function'] = $traceStep['class'].$traceStep['type'].$traceStep['function'];
        unset($traceStep['class']);
        unset($traceStep['type']);
      }
    }
    unset($traceStep);

    array_unshift(
      $trace,
      [
        'file'  => $throwable->getFile(),
        'line'  => $throwable->getLine(),
      ]
    );

    if ($previous = $throwable->getPrevious()) {
      $trace[] = static::formatTrace($previous);
    }

    return $trace;
  }
}
