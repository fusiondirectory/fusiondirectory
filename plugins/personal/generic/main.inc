<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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

/* Remove locks created by this plugin */
$lock_msg="";
if ($remove_lock || isset($_POST['edit_cancel']) || isset($_POST['password_cancel'])){
  if(session::is_set('user') && session::is_set('edit')){
    del_lock($ui->dn);
  }
}

/* Remove this plugin from session */
if ( $cleanup ){
  session::un_set('user');
  session::un_set('edit');

}else{

  /* Reset requested? */
  if (isset($_POST['edit_cancel']) || isset($_POST['password_cancel'])|| isset($_POST['password_changed'])){
    session::un_set ('edit');
    session::un_set ('user');
  }

  /* Create user object on demand */
  if (!session::is_set('user') || (isset($_GET['reset']) && $_GET['reset'] == 1)){
    $user= new user ($config, $ui->dn);
    $user->enable_CSN_check();
    $user->set_acl_base($ui->dn);
    $user->set_acl_category("users");
    session::set('user',$user);
  }
  $user = session::get('user');

  /* save changes back to object */
  if (session::is_set('edit')){
    $user->save_object ();
  }

  /* Enter edit mode? */
  if ((isset($_POST['edit'])) && (!session::is_set('edit')) &&preg_match("/w/",$ui->get_permissions($ui->dn,"users/user"))){

    /* Check locking */
    if (($username= get_lock($ui->dn)) != ""){
      session::set('back_plugin',$plug);
      session::set('LOCK_VARS_TO_USE',array("/^edit$/","/^plug$/"));
      $lock_msg =  gen_locked_message ($username, $ui->dn);

    }else{

      /* Lock the current entry */
      add_lock ($ui->dn, $ui->dn);
      session::set('dn',$ui->dn);
      session::set('edit',TRUE);
    }
  }

  /* save changes to LDAP and disable edit mode */
  $info = "";
  if (isset($_POST['edit_finish'])){

      /* Perform checks */
      $message= $user->check ();

      /* No errors, save object */
      if (count ($message) == 0){
          $user->save ();
          del_lock ($ui->dn);
          session::un_set ('edit');

          /* Need to reset the password? */
          if($user->password_change_needed()){

              $user->passwordClass= new password($config, $ui->dn);
              $user->passwordClass->forceHash($user->pw_storage);

              session::set('user',$user);
              $display.=$user->passwordClass->execute();
          } else {
            $display.= $user->execute ();
            session::un_set ('user');
            session::un_set ('edit');
          }

      } else {
          /* Errors found, show message */
          msg_dialog::displayChecks($message);
      }
  }elseif(isset($user->passwordClass)){

    $display.=$user->passwordClass->execute();

  }else{
      $passwordClass = NULL;
      /* Execute formular */
      if($lock_msg){
          $display.= $lock_msg;
      }else{
          $display.= $user->execute ();
      }
  }

  /* Store changes  in session */
  if (session::is_set('edit')){
      session::set('user',$user);
  }

  /* Show page footer depending on the mode */
  if (!$user->passwordClass && !$user->picture_dialog && !$user->dialog && $user->is_account && empty($lock_msg)){
      $display.= "<p class=\"plugbottom\">";

      /* Are we in edit mode? */
      if (session::is_set('edit')){
          $display.= "<input type=submit name=\"edit_finish\" style=\"width:80px\" value=\"".msgPool::okButton()."\">\n";
          $display.= "&nbsp;";
          $display.= "<input type=submit name=\"edit_cancel\" value=\"".msgPool::cancelButton()."\">\n";

          $info= "<img class=\"center\" alt=\"\" align=\"middle\" src=\"".get_template_path('images/lists/locked.png').
              "\"> ".$ui->dn."&nbsp;";
      } else {
          /* 'viewid' may be set by i.e. the phonelist plugin. This
             means that we want to go back... */
          if (isset ($viewid)){
              $str= msgPool::backButton();
              $fn = "back";
          } else {
              $str= msgPool::editButton();
              $fn = "edit";
              $info= "<img class=\"center\" alt=\"\" align=\"middle\" src=\"".get_template_path('images/lists/unlocked.png').
                  "\"> ".$ui->dn."&nbsp;";
          }

          /* Only display edit button if there is at least one attribute editable */
          if(preg_match("/r/",$ui->get_permissions($ui->dn,"users/user"))){
              if ($fn == "edit"){
                  $info.= "<div class=\"optional\"><img class=\"center\" alt=\"\" align=\"middle\" src=\"".get_template_path('images/lists/on.png')."\"> ".
                      msgPool::clickEditToChange()."</div>";
                  $display.= "<input type=submit name=\"$fn\" value=\"$str\">\n";
              }
          }

          $display.= "<input type=\"hidden\" name=\"ignore\">\n";
      }
      $display.= "</p>\n";
  }


  /* Page header*/
  $display= print_header(get_template_path('plugins/generic/images/icon.png'),
      _("Generic user information"), $info).$display;

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
