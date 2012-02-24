<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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

define("INFO_DIALOG"    , 10001);
define("WARNING_DIALOG"   , 10002);
define("ERROR_DIALOG"   , 10003);
define("CONFIRM_DIALOG"   , 10004);
define("OK_CANCEL_DIALOG" , 10005);
define("FATAL_ERROR_DIALOG" , 10006);

/*! \brief This class contains all the function needed to make messages
 *  dialogs
 */

class msg_dialog
{
  private $s_Title  = "Undefined";
  private $s_Message  = "Undefined";
  private $i_Type   = INFO_DIALOG ;
  private $i_ID   = 0;
  private $a_Trace  = array();

  public function __construct($s_title,$s_message,$i_type)
  {
    global $config;

    if(empty($s_message)) return;
  
    if(!in_array($i_type,array(INFO_DIALOG,WARNING_DIALOG,ERROR_DIALOG,CONFIRM_DIALOG,FATAL_ERROR_DIALOG,OK_CANCEL_DIALOG))){
      trigger_error("Invalid msg_dialog type.");
      $i_type = INFO_DIALOG;
    }

    if((!session::is_set('errorsAlreadyPosted')) || !is_array(session::get('errorsAlreadyPosted'))){
      session::set('errorsAlreadyPosted',array());
    }

    $errorsAlreadyPosted = session::get('errorsAlreadyPosted');
    if(!isset($errorsAlreadyPosted[$s_title.$s_message])){
      $errorsAlreadyPosted[$s_title.$s_message] = 0;
    }
    $errorsAlreadyPosted[$s_title.$s_message]++;

    if($errorsAlreadyPosted[$s_title.$s_message] <=1  ){

      $this->i_ID    = preg_replace("/[^0-9]*/","",microtime()); 
      $this->s_Title   = $s_title;
      $this->s_Message = $s_message;
      $this->i_Type  = $i_type;

      /* Append trace information, only if error messages are enabled */
      if( isset($config) && 
        is_object($config) && 
        $config->get_cfg_value("displayerrors") == "true" ){
        $this->a_Trace   = debug_backtrace();
      }
      if(!session::is_set('msg_dialogs')){
        session::set('msg_dialogs',array());
      }
      if($this->i_Type == FATAL_ERROR_DIALOG){
        restore_error_handler();
        error_reporting(E_ALL);
        echo $this->execute();
      }else{
        $msg_dialogs   = session::get('msg_dialogs');
        $msg_dialogs[] = $this;
        session::set('msg_dialogs',$msg_dialogs);
      }
    }
    session::set('errorsAlreadyPosted',$errorsAlreadyPosted);
  }


  public static function display($s_title,$s_message,$i_type = INFO_DIALOG)
  {
    new msg_dialog($s_title,$s_message,$i_type);  
  }


  public static function displayChecks($messages)
  {
    /* Assemble the message array to a plain string */
    foreach ($messages as $error){
      msg_dialog::display(_("Error"), $error, ERROR_DIALOG);
    }
  }


  public function get_ID()
  {
    return($this->i_ID);
  }

  public function execute()
  {
    if($this->i_Type == FATAL_ERROR_DIALOG) {
      $display = 
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"
        \"http://www.w3.org/TR/html4/transitional.dtd\">
        <html><head>
        <title>FusionDirectory startup failed</title>
        </head><body>
        <table style='width:100%; border:2px solid red;' summary=''>
        <tr><td style='vertical-align:top;padding:10px'>
        <img src='images/error.png' alt='{t}Error{/t}'>
        </td><td style='width:100%'>
        <b>".$this->s_Title."</b><br>
        ".$this->s_Message."<br><br>
        "._("Please fix the above error and reload the page.")."
        </td></tr>
        </table></body></html>";
      return($display);;
    }else{

      $smarty = get_smarty();
      $smarty->assign("s_Trace" ,print_a($this->a_Trace,TRUE));
      $smarty->assign("i_TraceCnt",count($this->a_Trace));
      $smarty->assign("i_Type",$this->i_Type);
      $smarty->assign("s_Message",$this->s_Message);
      $smarty->assign("s_Title",$this->s_Title);
      $smarty->assign("i_ID",$this->i_ID);
      $smarty->assign("frame",false);
      $smarty->assign("JS",session::global_get('js'));
      $smarty->assign("IE",preg_match("/MSIE/", $_SERVER['HTTP_USER_AGENT']));
      return($smarty->fetch(get_template_path('msg_dialog.tpl')));
    }
  }

  
  public function is_confirmed()
  {
    if(isset($_POST['MSG_OK'.$this->i_ID])){
      return(TRUE);
    }else{
      return(FALSE);
    }
  }
  

  public static function get_dialogs()
  {
    $return  ="";
    $dialog_ids= "";

    $seen = "";
    if(isset($_POST['closed_msg_dialogs'])){
#     $seen = $_POST['closed_msg_dialogs'];
    }

    if(session::is_set('msg_dialogs') && is_array(session::get('msg_dialogs')) && count(session::get('msg_dialogs'))){

      /* Get frame one */
      $smarty = get_smarty();
      $smarty->assign("frame", true);
      $smarty->assign("IE",preg_match("/MSIE/", $_SERVER['HTTP_USER_AGENT']));
      $return = $smarty->fetch(get_template_path('msg_dialog.tpl'));

      if(!session::global_get('js')){
        $dialog = array_pop(session::get('msg_dialogs'));
        $return.= $dialog->execute();
      }else{
        $msg_dialogs = session::get('msg_dialogs');
        foreach($msg_dialogs as $key => $dialog){

          if(preg_match("/".$dialog->get_ID()."/",$seen)){
            unset($msg_dialogs[$key]);
          }else{
            $return.= $dialog->execute();
            $dialog_ids= $dialog->get_ID().",".$dialog_ids;
          }
          unset($msg_dialogs[$key]);
        }
        session::set('msg_dialogs',$msg_dialogs);
      }
      $dialog_ids = preg_replace("/,$/","",$dialog_ids);

      $return .= "</div>";
      $return .="<input type='hidden' style='width:400px;' name='pending_msg_dialogs' id='pending_msg_dialogs' value='".$dialog_ids."'>";
      $return .="<input type='hidden' style='width:400px;' name='closed_msg_dialogs' id='closed_msg_dialogs' value=''>";
      $return .="<input type='hidden' style='width:400px;' name='current_msg_dialogs' id='current_msg_dialogs' value=''>";
      $return .="<input type='hidden' style='width:700px;' name='js_debug' id='js_debug'>";
    }
    return($return);
  }

}
?>
