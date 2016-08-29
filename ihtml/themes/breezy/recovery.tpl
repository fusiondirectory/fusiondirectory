<body onLoad="javascript:$$('div.debug_div').each(function (a) { a.hide(); });">
  {* FusionDirectory recovery - smarty template *}
  {$php_errors}
  <div>
    {include file={filePath file="setup_header.tpl"}}
  </div>
  {$msg_dialogs}

<div id="window-container">

<div id="window-div">

  <form action='recovery.php{$params}' method='post' name='mainform' onSubmit='js_check(this);return true;'>

  <div id="window-titlebar">
    <p>
      <img class="center" src="geticon.php?context=status&amp;icon=dialog-password&amp;size=48" alt="{t}Password{/t}" title="{t}Password{/t}"/>
      {t}Lost password{/t}
    </p>
  </div>
  <div id="window-content">
    <div>
{if $activated}
      <!-- Display SSL warning message on demand -->
      <span class="warning"> {$ssl} </span>

      <!-- Display error message on demand -->
      <span class="warning"> {$message} </span>

{if $step==3}
    <p class="infotext">
      {t 1=$uid 2=$email_address}Informations to reset password for %1 have been sent to email address %2{/t}<br/>
      <span class="warning">{t 1=$delay_allowed}Warning : this email is only valid for %1 minutes.{/t}</span>
    </p>
  </div>
  </div>
{elseif $step==4}
    <p class="infotext">
      {t}This dialog provides a simple way to change your password. Enter the new password (twice) in the fields below and press the 'Change' button.{/t}
    </p>

    <br/>
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
       <td><label for="uid">{t}Login{/t}</label></td>
       <td>
           <i>{$uid}</i>
       </td>
      </tr>
      <tr>
       <td><label for="new_password">{t}New password{/t}</label></td>
       <td><input type="password" name="new_password" id="new_password" value="" title="{t}New password{/t}" onkeyup="testPasswordCss(document.getElementById('new_password').value);" /></td>
      </tr>
      <tr>
       <td><label for="new_password_repeated">{t}New password repeated{/t}</label></td>
       <td><input type="password" name="new_password_repeated" id="new_password_repeated" maxlength="40" value="" title="{t}New password repeated{/t}" /></td>
      </tr>
      <tr>
       <td>{t}Password strength{/t}</td>
       <td>
        <span id="meterEmpty" style="padding:0;margin:0;width:100%;background-color:#DC143C;display:block;height:5px;">
        <span id="meterFull" style="padding:0;margin:0;z-index:100;width:0;background-color:#006400;display:block;height:5px;"></span></span>
       </td>
      </tr>
    </table>
  </div>
  </div>
  <div id="window-footer" class="plugbottom">
    <div>
    </div>
    <div>
      <input type="submit" name="change" value="{t}Change{/t}" title="{t}Click here to change your password{/t}"/>
    </div>
  </div>
{elseif $changed}
    <div class="success">
      <img class="center" src="geticon.php?context=status&amp;icon=task-complete&amp;size=16" alt="{t}Success{/t}" title="{t}Success{/t}">&nbsp;<b>{t}Your password has been changed successfully.{/t}</b><br/>
      <br/><a href="./">Return to login screen</a>
    </div>
  </div>
  </div>
{else}
    <p class="infotext">
      {t}Enter your current e-mail address in the field below and press the 'Change' button.{/t}<br/>
      <strong>{t}=> Use your e-mail in the long format, e.g : John Doe => john.doe@example.com{/t}</strong>
    </p>

    <br/>
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
       <td><label for="email_address">{t}Email address{/t}</label></td>
       <td>
           <input type="text" name="email_address" id="email_address" value="{$email_address}" title="{t}Email{/t}" onFocus=""/>
       </td>
      </tr>
    </table>
  </div>
  </div>
  <div id="window-footer" class="plugbottom">
    <div>
    </div>
    <div>
      <input type="submit" name="apply" value="{t}Change{/t}" title="{t}Click here to change your password{/t}"/>
    </div>
  </div>
{/if}
{else}
    <!-- Display error message on demand -->
    <p class="warning"> {$message} </p>
    <p>{t}Password recovery is not activated. If you have lost your password, please contact your administrator{/t}</p>
  </div>
  </div>
{/if}
  </form>
</div>
</div>

  <!-- Place cursor in email field -->
  <script type="text/javascript">
    <!-- // First input field on page
    focus_field('error_accept','uid','directory','username','current_password','email_address');
    next_msg_dialog();
    -->
  </script>

</body>
</html>
