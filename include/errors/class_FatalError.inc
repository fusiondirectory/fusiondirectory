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

/*! \class FatalError
    \brief Fatal error class. Does not extend FusionDirectoryError.
*/
class FatalError extends Error
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
      'message' => $this->getMessage(),
      'line'    => $this->getLine(),
      'file'    => $this->getFile(),
      'fatal'   => TRUE,
    ];
  }

  public function display ()
  {
    restore_error_handler();
    error_reporting(E_ALL);
    echo $this->renderFatalErrorDialog();
  }

  /*!
   * \brief Render fatal error screen
   */
  protected function renderFatalErrorDialog ()
  {
    global $config;

    $display =
      '<!DOCTYPE html>
      <html><head>
      <title>'.htmlescape(_('FusionDirectory Fatal Error')).'</title>
      </head><body>';

    $display .=
      '<table style="width:100%; border:2px solid red;">
        <tr>
          <td style="vertical-align:top;padding:10px">
            <img src="geticon.php?context=status&amp;icon=dialog-error&amp;size=32" alt="'.htmlescape(_('Error')).'"/>
          </td>
          <td style="width:100%">
            <h3>'.htmlescape(_('Fatal Error')).'</h3>
            '.$this->getHtmlMessage().'
          </td>
        </tr>
      </table>';

    if (isset($config) && is_object($config) &&
      $config->get_cfg_value('displayerrors') == 'TRUE') {
      $trace    = FusionDirectoryError::formatTrace($this);
      $display  .= print_a($trace, TRUE);
    }

    $display .= '</body></html>';

    return $display;
  }
}
