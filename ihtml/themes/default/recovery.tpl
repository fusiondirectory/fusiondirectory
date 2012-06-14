<body style='height:100%; width:100%;'>
  {* FusionDirectory recovery - smarty template *}
  {$php_errors}
  <div>
      <div class='setup_header'>
        <div style="float:left;"><img src='themes/default/images/go_logo.png' class='center' alt='FusionDirectory' /></div>
        <div style="padding-top:8px;text-align:right;height:38px;color:#FFFFFF">{$version}</div>
      </div>
        <div class='setup_menu'>
          <b>{t}FusionDirectory password recovery{/t}</b>
        </div>
  </div>

  <!-- Spacer for some browsers -->
  <div class='gosaLoginSpacer'></div>

  <div style='float:left; width:25%;'>&nbsp;</div>
  <div style='float:left; width:50%; border:1px solid #AAAAAA;background-color:white'>

  <form action='recovery.php{$params}' method='post' name='mainform' onSubmit='js_check(this);return true;'>

{$msg_dialogs}
  <div style='padding:3px; background-color:#F8F8F8;border-bottom:1px solid #AAAAAA'>
  <p class="center" style="margin:0px 0px 0px 5px;padding:5px;font-size:24px;font-weight:bold;">
  <img class='center' src='images/password.png' alt='{t}Password{/t}' title='{t}Password{/t}'>
    {t}Lost password{/t}
  </p>
  </div>
{if $activated}
  <div style="padding-left:10px;padding-right:10px;">
    <!-- Display SSL warning message on demand -->
    <p class='warning'> {$ssl} </p>
    <input type='hidden' name='javascript' value='false'/>

    <!-- Display error message on demand -->
    <p class='warning'> {$message} </p>

{if $step==2}
    <p class="infotext">
      {t}Password recovery for email {$address_mail}{/t}
    </p>

    <p class="infotext">
      {if $other_method}
        {t}Available methods{/t}
      {/if}
      <ul>
        <li>{t}Receive by email a link that allows you to reset your password{/t} :
          <input type='submit' name='send'  value='{t}Send{/t}'
                 title='{t}Click here to send a reset link{/t}'>
          <input type='hidden' id='address_mail' maxlength='60' value='{$address_mail}'>
          <input type='hidden' id='uid' maxlength='60' value='{$uid}'>
        </li>
        {if $other_method}
          <li>
             Dummy recuperation method
          </li>
          <li>
            <font color="red">{t}If none of the above methods suits you, contact your administrator to change your password{/t}</font>
          </li>
        {/if}
      </ul>
    </p>
{elseif $step==3}
    <p class="infotext">
      {t}Informations to reset password for {$uid} have been sent to email address {$address_mail}{/t}<br/>
      <font color="red">{t}Warning : this email is only valid for {$delay_allowed} minutes.{/t}</font>
    </p>
{elseif $step==4}
    <p class="infotext">
      {t}This dialog provides a simple way to change your password. Enter the new password (twice) in the fields below and press the 'Change' button.{/t}
    </p>

    <div class="ruler"></div>
    <table>
      {if $show_directory_chooser}
      <tr>
        <td>{t}Directory{/t}</td>
        <td>
          <select name='server'  title='{t}Directory{/t}'>
            {html_options options=$server_options selected=$server_id}
          </select>
        </td>
      </tr>
      {/if}
      <tr>
       <td><label for='uid'>{t}Username{/t}</label></td>
       <td>
           <i>{$uid}</i>
       </td>
      </tr>
      <tr>
       <td><label for='new_password'>{t}New password{/t}</label></td>
       <td><input type='password' name='new_password' id="new_password" maxlength='40' value='' title='{t}New password{/t}' onFocus="nextfield= 'new_password_repeated';" onkeyup="testPasswordCss(document.getElementById('new_password').value);"></td>
      </tr>
      <tr>
       <td><label for='new_password_repeated'>{t}New password repeated{/t}</label></td>
       <td><input type='password' name='new_password_repeated' id='new_password_repeated' maxlength='40' value='' title='{t}New password repeated{/t}' onFocus="nextfield= 'apply';"></td>
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

    <div class="change" style="float:right; text-align:right;">
      <input type='submit' name='change' value='{t}Change{/t}' title='{t}Click here to change your password{/t}'>
      <input type='hidden' id='address_mail' maxlength='60' value='{$address_mail}'>
      <input type='hidden' id='uniq' maxlength='60' value='{$uniq}'>
      <input type='hidden' id='formSubmit'>
    </div>
    <div style="clear:both"></div>
{elseif $changed}
    <div class='success'">
      <img class='center' src='images/true.png' alt='{t}Success{/t}' title='{t}Success{/t}'>&nbsp;<b>{t}Your password has been changed successfully.{/t}</b>
    </div>
{else}
    <p class="infotext">
      {t}Enter your current e-mail address in the field below and press the 'Change' button.{/t}<br/>
      <strong>{t}=> Use your e-mail in the long format, e.g : John Doe => john.doe@ibcp.fr{/t}</strong>
    </p>

    <div class="ruler"></div>
    <table>
      {if $show_directory_chooser}
      <tr>
       <td>{t}Directory{/t}</td>
       <td>
          <select name='server'  title='{t}Directory{/t}'>
            {html_options options=$server_options selected=$server_id}
          </select>
        </td>
      </tr>
      {/if}
      <tr>
       <td><label for='mail'>{t}Adresse mail{/t}</label></td>
       <td>
           <input type='text' name='address_mail' id='address_mail' width='60' maxlength='60' value='{$address_mail}' title='{t}Mail{/t}' onFocus="">
       </td>
      </tr>
    </table>
    <div class="change" style="float:right; text-align:right;">
      <input type='submit' name='apply'  value='{t}Change{/t}'
                   title='{t}Click here to change your password{/t}'>
      <input type='hidden' id='formSubmit'>
    </div>
    <div style="clear:both"></div>
{/if}
{else}
  <div style="padding-left:10px;padding-right:10px;">
    <!-- Display error message on demand -->
    <p class='warning'> {$message} </p>
    <p>{t}Password recovery is not activated. If you have lost your password, please contact your administrator{/t}</p>
{/if}
  </div>
  </form>

  <!-- Place cursor in username field -->
  <script language="JavaScript" type="text/javascript">
    <!-- // First input field on page
    focus_field('error_accept','uid','directory', 'username', 'current_password');
    next_msg_dialog();
    -->
  </script>

</body>
</html>
