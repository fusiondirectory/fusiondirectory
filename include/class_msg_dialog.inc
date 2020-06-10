<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2020  FusionDirectory

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
/*!
 * \file class_msg_dialog.inc
 * Source code for class message dialog
 */

define('INFO_DIALOG',         10001);
define('WARNING_DIALOG',      10002);
define('ERROR_DIALOG',        10003);
// LDAP_ERROR is the same as ERROR_DIALOG
define('LDAP_ERROR',          10003);
define('CONFIRM_DIALOG',      10004);
define('FATAL_ERROR_DIALOG',  10006);

/*!
 * \brief This class contains all the function needed to make messages
 *  dialogs
 */
class msg_dialog
{
  private $s_Title;
  private $s_Message;
  private $i_Type;
  private $i_ID;
  private $a_Trace;

  /*!
   * \brief Message dialog constructor
   *
   * \param string $title The title of the message dialog (plain string)
   *
   * \param string $message The message of the message dialog (HTML)
   *
   * \param int $type The type of the message dialog, by default = INFO_DIALOG
   *
   * \param array $trace The trace from where the message was built
   */
  public function __construct (string $title, string $message, int $type = INFO_DIALOG, array $trace = [])
  {
    if (!in_array($type, [INFO_DIALOG,WARNING_DIALOG,ERROR_DIALOG,CONFIRM_DIALOG,FATAL_ERROR_DIALOG])) {
      trigger_error('Invalid msg_dialog type.');
      $type = INFO_DIALOG;
    }

    $this->i_ID       = (int)preg_replace('/[^0-9]*/', '', microtime());
    $this->s_Title    = $title;
    $this->s_Message  = $message;
    $this->i_Type     = $type;
    $this->a_Trace    = $trace;
  }

  protected function show ()
  {
    global $config;

    if (empty($this->s_Message)) {
      return;
    }

    if ((!session::is_set('errorsAlreadyPosted')) || !is_array(session::get('errorsAlreadyPosted'))) {
      session::set('errorsAlreadyPosted', []);
    }

    $errorsAlreadyPosted = session::get('errorsAlreadyPosted');
    if (!isset($errorsAlreadyPosted[$this->s_Title.$this->s_Message])) {
      $errorsAlreadyPosted[$this->s_Title.$this->s_Message] = 0;
    }
    $errorsAlreadyPosted[$this->s_Title.$this->s_Message]++;

    session::set('errorsAlreadyPosted', $errorsAlreadyPosted);

    if ($errorsAlreadyPosted[$this->s_Title.$this->s_Message] > 1) {
      /* Skip if the same message was already reported once */
      return;
    }

    /* Append trace information, only if error messages are enabled */
    if (isset($config) && is_object($config) &&
      $config->get_cfg_value('displayerrors') == 'TRUE') {
      if (empty($this->a_Trace)) {
        $this->a_Trace = debug_backtrace();
      }
    } else {
      $this->a_Trace = [];
    }
    if ($this->i_Type == FATAL_ERROR_DIALOG) {
      restore_error_handler();
      error_reporting(E_ALL);
      echo $this->renderFatalErrorDialog();
    } else {
      if (session::is_set('msg_dialogs')) {
        $msg_dialogs = session::get('msg_dialogs');
      } else {
        $msg_dialogs = [];
      }
      $msg_dialogs[] = $this;
      session::set('msg_dialogs', $msg_dialogs);
    }
  }

  /*!
   * \brief Display a message dialog
   *
   * \param string $title The title of the message dialog
   *
   * \param string $message The message of the message dialog
   *
   * \param int $type The type of the message dialog, by default = INFO_DIALOG
   *
   * \param array $trace The trace from where the message was built
   */
  public static function display (string $title, string $message, int $type = INFO_DIALOG, array $trace = [])
  {
    $dialog = new msg_dialog($title, $message, $type, $trace);
    $dialog->show();
  }

  /*
   * \brief Display checks
   *
   * \param array $messages Contains messages
   */
  public static function displayChecks ($messages)
  {
    foreach ($messages as $error) {
      if ($error instanceof FusionDirectoryError) {
        static::display(...$error->computeMsgDialogParameters());
      } else {
        static::display(_('Error'), $error, ERROR_DIALOG);
      }
    }
  }

  /*
   * \brief Accessor of message dialog's identifier
   *
   * \return The identifier of the message dialog
   */
  public function getId (): int
  {
    return $this->i_ID;
  }

  /*!
   * \brief Run the message dialog
   */
  protected function renderFatalErrorDialog ()
  {
    global $config;

    $display =
      '<!DOCTYPE html>
      <html><head>
      <title>FusionDirectory startup failed</title>
      </head><body>';
    if (isset($config) && is_object($config) &&
      $config->get_cfg_value('displayerrors') == 'TRUE') {
      list($trace, ) = html_trace();
      $display .= $trace;
    }
    $display .=
      '<table style="width:100%; border:2px solid red;">
        <tr>
          <td style="vertical-align:top;padding:10px">
            <img src="geticon.php?context=status&amp;icon=dialog-error&amp;size=32" alt="'.htmlescape(_('Error')).'"/>
          </td>
          <td style="width:100%">
            <b>'.htmlescape($this->s_Title).'</b><br/>
            '.$this->s_Message.'<br/><br/>
            '.htmlescape(_('Please fix the above error and reload the page.')).'
          </td>
        </tr>
      </table></body></html>';
    return $display;
  }

  /*!
   * \brief Check if the message is confirmed by user
   *
   * \return True if is confirmed, else return false
   */
  public function is_confirmed (): bool
  {
    return isset($_POST['MSG_OK'.$this->i_ID]);
  }

  /*!
   * \brief Return an array with infos for the template
   */
  protected function getDialogInfos (): array
  {
    return [
      'id'          => $this->i_ID,
      'type'        => $this->i_Type,
      'title'       => $this->s_Title,
      'message'     => $this->s_Message,
      'trace'       => ((count($this->a_Trace) > 0) ? print_a($this->a_Trace, TRUE) : ''),
    ];
  }

  /*!
   * \brief Accessor of the message dialog rendered HTML
   */
  public static function get_dialogs (): string
  {
    if (session::is_set('msg_dialogs') &&
        is_array(session::get('msg_dialogs')) &&
        count(session::get('msg_dialogs'))) {
      $smarty = get_smarty();

      $msg_dialogs  = session::get('msg_dialogs');
      $dialogInfos  = [];
      $dialogIds    = [];
      foreach ($msg_dialogs as $dialog) {
        $dialogInfos[]  = $dialog->getDialogInfos();
        $dialogIds[]    = $dialog->getId();
      }
      session::set('msg_dialogs', []);

      $smarty->assign('dialogInfos',  $dialogInfos);
      $smarty->assign('dialogIds',    $dialogIds);

      return $smarty->fetch(get_template_path('msg_dialog.tpl'));
    } else {
      return '';
    }
  }
}
