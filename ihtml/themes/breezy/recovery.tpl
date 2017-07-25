<body onLoad="javascript:$$('div.debug_div').each(function (a) { a.hide(); });">
  {* FusionDirectory recovery - smarty template *}
  {$php_errors}
  {$msg_dialogs}

<div id="window-container">

<div id="window-div">

  <form action='recovery.php{$params}' method='post' name='mainform' onSubmit='js_check(this);return true;'>

  <div id="window-titlebar">
    <img id="fd-logo" src="geticon.php?context=applications&amp;icon=fusiondirectory&amp;size=48" alt="FusionDirectory"/>
    <p>
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
      {t 1=$login 2=$email_address}Information to reset password for %1 has been sent to email address %2{/t}<br/>
      <span class="warning">{t 1=$delay_allowed}Warning : this email is only valid for %1 minutes.{/t}</span>
    </p>
  </div>
  </div>
{elseif $step==4}
    <p class="infotext">
      {t}This dialog provides a simple way to change your password. Enter the new password (twice) in the fields below and press the 'Change' button.{/t}
    </p>

    <br/>
    <div style="text-align: center;">
      <label for="login">
        <img class="center" src="geticon.php?context=types&amp;icon=user&amp;size=48" alt="{t}Username{/t}" title="{t}Username{/t}"/>&nbsp;
      </label>
      <input type="text" name="login" value="{$login}" title="{t}Username{/t}" readonly />
      <br />
      <label for="new_password">
        <img class="center" src="geticon.php?context=status&amp;icon=dialog-password&amp;size=48" alt="{t}New password{/t}" title="{t}New password{/t}"/>&nbsp;
      </label>
      <input type="password" name="new_password" id="new_password" value="" title="{t}New password{/t}" onkeyup="testPasswordCss(document.getElementById('new_password').value);" />
      <br />
      <label for="new_password_repeated">
        <img class="center" src="geticon.php?context=status&amp;icon=dialog-password&amp;size=48" alt="{t}New password repeated{/t}" title="{t}New password repeated{/t}"/>&nbsp;
      </label>
      <input type="password" name="new_password_repeated" id="new_password_repeated" maxlength="40" value="" title="{t}New password repeated{/t}" />
      <br>
      <div style="width: 190px; display: inline-block; margin-left: 54px;" title="{t}Password strength{/t}">
        <span id="meterEmpty" style="padding:0;margin:0;width:100%;background-color:#DC143C;display:block;height:5px;">
        <span id="meterFull" style="padding:0;margin:0;z-index:100;width:0;background-color:#006400;display:block;height:5px;"></span></span>
      </div>
    </div>
    {if $show_directory_chooser}
    <div>
      <img class="center" src="geticon.php?context=places&amp;icon=network-server&amp;size=48" alt="{t}Directory{/t}" title="{t}Directory{/t}" />&nbsp;
      <select name="server" title="{t}Directory{/t}" onchange="javascript:document.loginform.submit();">
        {html_options options=$server_options selected=$server_id}
      </select>
    </div>
    {/if}
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
      <br/><a href="index.php">{t}Return to login screen{/t}</a>
    </div>
  </div>
  </div>
{else}
    <p class="infotext">
      {t}Enter your current e-mail address in the field below and press the 'Change' button.{/t}<br/>
      <strong>{t}=> Use your e-mail in the long format, e.g : John Doe => john.doe@example.com{/t}</strong>
    </p>

    <div>
      <label for="email_address">
         <img class="center" src="geticon.php?context=applications&amp;icon=internet-mail&amp;size=48" alt="{t}Email address{/t}" title="{t}Email address{/t}" />&nbsp;
      </label>
      <input type="text" name="email_address" id="email_address" value="{$email_address}" title="{t}Email{/t}" onFocus=""/>
    </div>
    {if $show_directory_chooser}
    <div>
      <img class="center" src="geticon.php?context=places&amp;icon=network-server&amp;size=48" alt="{t}Directory{/t}" title="{t}Directory{/t}" />&nbsp;
      <select name="server" title="{t}Directory{/t}" onchange="javascript:document.loginform.submit();">
        {html_options options=$server_options selected=$server_id}
      </select>
    </div>
    {/if}
    <br />
    <a href="index.php">{t}Return to login screen{/t}</a>
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
{include file={filePath file="copynotice.tpl"}}

  <!-- Place cursor in email field -->
  <script type="text/javascript">
    <!-- // First input field on page
    focus_field('error_accept','login','directory','username','current_password','email_address');
    next_msg_dialog();
    -->
  </script>

</body>
</html>
