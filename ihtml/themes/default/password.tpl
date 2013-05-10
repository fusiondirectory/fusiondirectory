<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
  "http://www.w3.org/TR/html4/transitional.dtd">
<html>

<head>
  <title>FusionDirectory - {t}Change your password{/t}</title>

  <meta name="generator" content="my hands">
  <meta name="description" content="FusionDirectory - Password changer">
  <meta name="author" lang="de" content="FusionDirectory Project">

  <meta http-equiv="Expires" content="Mon, 26 Jul 1997 05:00:00 GMT">
  <meta http-equiv="Last-Modified" content="{$date} GMT">
  <meta http-equiv="Cache-Control" content="no-cache">
  <meta http-equiv="Pragma" content="no-cache">
  <meta http-equiv="Cache-Control" content="post-check=0, pre-check=0">
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">

  <style type="text/css">@import url('themes/default/style.css');</style>
  <style type="text/css">@import url('themes/default/password-style.css');</style>
  <link rel="shortcut icon" href="favicon.ico">

  <script language="javascript" src="include/prototype.js" type="text/javascript"></script>
  <script language="javascript" src="include/fusiondirectory.js" type="text/javascript"></script>
  <script language="javascript" src="include/pwdStrength.js" type="text/javascript"></script>
</head>

<body style='height:100%; width:100%;'>
{* FusionDirectory login - smarty template *}
{$php_errors}

{if $changed}
<div class='success'">
<img class="center" src="images/true.png" alt="{t}Success{/t}" title="{t}Success{/t}">&nbsp;<b>{t}Your password has been changed successfully.{/t}</b>
</div>
{else}
<form action="password.php{$params}" method="post" name="mainform" onSubmit="js_check(this);return true;">
    <h1 class="headline">
    <img class="center" src="{$password_img}" alt="{t}Password{/t}" title="{t}Password{/t}">
    {t}Password change{/t}
    </h1>

    <!-- Display SSL warning message on demand -->
    <p class="warning"> {$ssl} </p>
    <input type="hidden" name="javascript" value="false"/>

    <!-- Display error message on demand -->
    <p class="warning"> {$message} </p>


    <p class="infotext">
  {t}This dialog provides a simple way to change your password. Enter the current password and the new password (twice) in the fields below and press the 'Change' button.{/t}
    </p>

    <div class="ruler"></div>
    <table>
      {if $show_directory_chooser}
      <tr>
       <td>{t}Directory{/t}</td>
       <td>
          <select name="server"  title="{t}Directory{/t}">
            {html_options options=$server_options selected=$server_id}
          </select>
  </td>
      </tr>
      {/if}
      <tr>
       <td><label for="uid">{t}Username{/t}</label></td>
       <td>{if $display_username}
           <input type="text" name="uid" id="uid" maxlength="40" value="{$uid}" title="{t}Username{/t}" onFocus="nextfield= 'current_password';">
           {else}
           <i>{$uid}</i>
           {/if}
       </td>
      </tr>
      <tr>
       <td><label for="current_password">{t}Current password{/t}</label></td>
       <td><input type="password" name="current_password" id="current_password" maxlength="40" value="" title="{t}Current password{/t}" onFocus="nextfield= 'new_password';"></td>
      </tr>
      <tr>
       <td><label for="new_password">{t}New password{/t}</label></td>
       <td><input type="password" name="new_password" id="new_password" maxlength="40" value="" title="{t}New password{/t}" onFocus="nextfield= 'new_password_repeated';" onkeyup="testPasswordCss(document.getElementById('new_password').value);"></td>
      </tr>
      <tr>
       <td><label for="new_password_repeated">{t}New password repeated{/t}</label></td>
       <td><input type="password" name="new_password_repeated" id="new_password_repeated" maxlength="40" value="" title="{t}New password repeated{/t}" onFocus="nextfield= 'apply';"></td>
      </tr>
      <tr>
       <td>{t}Password strength{/t}</td>
       <td>
        <span id="meterEmpty" style="padding:0;margin:0;width:100%;background-color:#DC143C;display:block;height:5px;">
        <span id="meterFull" style="padding:0;margin:0;z-index:100;width:0;background-color:#006400;display:block;height:5px;"></span></span>
       </td>
      </tr>
    </table>

    <div class="ruler"></div>

    <div class="change">
    <input type="submit" name="apply"  value="{t}Change{/t}"
                 title="{t}Click here to change your password{/t}">
    <input type="hidden" id="formSubmit">
    </div>
    <!-- check, if cookies are enabled -->
    <p class="warning">
     <script language="JavaScript" type="text/javascript">
        <!--
            document.cookie = "gosatest=empty;path=/";
            if (document.cookie.indexOf( "gosatest=") > -1 )
                document.cookie = "gosatest=empty;path=/;expires=Thu, 01-Jan-1970 00:00:01 GMT";
            else
                document.write("{$cookies}");
        -->
     </script>
    </p>

</form>

{/if}

{$msg_dialogs}

<table class="iesucks"><tr><td>{$errors}</td></tr></table>

<!-- Place cursor in username field -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
  focus_field('error_accept','uid','directory', 'username', 'current_password');
  next_msg_dialog();
  -->
</script>

</body>
</html>
