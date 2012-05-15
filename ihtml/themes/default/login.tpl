<body style="background-color:white;background-image:none;">

{$php_errors}
<div>
    <div class='setup_header'>
      <div style="float:left;"><img src='themes/default/images/go_logo.png' class='center' alt='FusionDirectory' /></div>
      <div style="padding-top:8px;text-align:right;height:38px;color:#FFFFFF">{$version}</div>
    </div>
</div>

{* FusionDirectory login - smarty template *}

<!-- Spacer for some browsers -->
<div class='gosaLoginSpacer'></div>

<div style='float:left; width:25%;'>&nbsp;</div>
<div style='float:left; width:50%; border:1px solid #AAAAAA;background-color:white'>
<form action='index.php' method='post' name='mainform' onSubmit='js_check(this);return true;'>

{$msg_dialogs}
  <div style='border-bottom:1px solid #AAAAAA'>
    <div style='padding:3px; background-color:#DBE6C6'>
    <p class="center" style="margin:0px 0px 0px 5px;padding:5px;font-size:24px;font-weight:bold;">
      {t}Login screen{/t}
    </p>
    </div>
  </div>
  <div style='border-bottom:1px solid #AAAAAA'>

      <div style='padding:12px;text-align:center;'>
    {t}Please use your username and your password to log into the site administration system.{/t}<br />
    {if $ssl}<b>{$ssl}</b>{/if}
    {if $lifetime}<b><font style="color:red";>{$lifetime}</font></b>{/if}
      </div>
  

        <input id='focus' name='focus' type='image' src='images/empty.png' style='width:0px; height:0px;' />
    <div style='text-align:center; padding:10px;'>  
    <label for='username'><img class='center' align='middle' src='{$personal_img}' alt='{t}Username{/t}' title='{t}Username{/t}' />&nbsp;</label>
    <input type='text' name='username' id='username' maxlength='40' value='{$username}'
       title='{t}Username{/t}' onFocus="nextfield= 'password';" />
    <br />
    <br />
    <label for='password'><img class='center' align='middle' src='{$password_img}' alt='{t}Password{/t}' title='{t}Password{/t}' />&nbsp;</label>
    <input type='password' name='password' id='password' maxlength='40' value=''
       title='{t}Password{/t}' onFocus="nextfield= 'login';" />
    </div>  
    <div style='text-align:center; padding:15px;'>
            <img class='center' align='middle' src='{$directory_img}' alt='{t}Directory{/t}' title='{t}Directory{/t}' />&nbsp;
      <select name='server'  title='{t}Directory{/t}'>
        {html_options options=$server_options selected=$server_id}
      </select>
    </div>

      <!-- check, if cookies are enabled -->
      <p class='gosaLoginWarning'>
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
  </div>
  <div style='padding:10px;'>
    <div style='float:left; color:red; font-weight:bold'>
    {$message}
    </div>
    <div style='float:right; text-align:right;'>
      <!-- Display error message on demand -->
        <input type='submit' name='login' value='{t}Sign in{/t}'
         title='{t}Click here to log in{/t}' onFocus="nextfield='login';" />
      <input type='hidden' name='javascript' value='false' />   
      <input type='hidden' name='login' value='{t}Sign in{/t}' />   
    </div>
    <div style="clear:both"></div>
  </div>

</form>
</div>

<div style="clear:both"></div>

{$errors}

<script language="JavaScript" type="text/javascript">
<!-- 
  enable_keyPress = false;
  nextfield= "{$nextfield}";
  focus_field("{$nextfield}");
  next_msg_dialog();
-->
</script>
</body>
</html>
