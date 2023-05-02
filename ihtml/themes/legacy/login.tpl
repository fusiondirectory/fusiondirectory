<body>

  {$php_errors}
  {include file={filePath file="setup_header.tpl"}}

{* FusionDirectory login - smarty template *}

<div id="window-container">

<div id="window-div">
<form action="index.php" method="post" id="loginform" name="loginform" onSubmit="js_check(this);return true;">

{$msg_dialogs}
  <div id="window-titlebar">
    <p>
      {t}Sign in{/t}
    </p>
  </div>
  <div id="window-content">

    <div class="optional">
      {t}Please use your username and your password to log into the site administration system.{/t}<br />
      {if $ssl}<span class="warning">{$ssl}</span>{/if}
      {if $lifetime}<span class="warning">{$lifetime|escape}</span>{/if}
    </div>

    <div>
      <label for="username">
        <img class="center" src="geticon.php?context=types&amp;icon=user&amp;size=48" alt="{t}Username{/t}" title="{t}Username{/t}"/>&nbsp;
      </label>
      <input type="text" name="username" id="username" value="{$username|escape}" title="{t}Username{/t}" />
      <br />
      <br />
      <label for="password">
        <img class="center" src="geticon.php?context=status&amp;icon=dialog-password&amp;size=48" alt="{t}Password{/t}" title="{t}Password{/t}" />&nbsp;
      </label>
      <input type="password" name="password" id="password" value="" title="{t}Password{/t}"/>
      <br />
      <a href="recovery.php">{t}I forgot my password{/t}</a>
    </div>
    <div>
      <img class="center" src="geticon.php?context=places&amp;icon=network-server&amp;size=48" alt="{t}Directory{/t}" title="{t}Directory{/t}" />&nbsp;
      <select name="server" title="{t}Directory{/t}" onchange="javascript:document.loginform.submit();">
        {html_options options=$server_options selected=$server_id}
      </select>
    </div>

      <!-- check, if cookies are enabled -->
      <p class="fusiondirectoryLoginWarning">
       <script>
        <!--
        document.cookie = "fdtest=empty;path=/";
        if (document.cookie.indexOf( "fdtest=") > -1 )
          document.cookie = "fdtest=empty;path=/;expires=Thu, 01-Jan-1970 00:00:01 GMT";
        else
          document.write("<b>{t}Warning{/t}</b> {t}Your browser has cookies disabled. Please enable cookies and reload this page before logging in!{/t}");
        -->
       </script>
      </p>
  </div>
  <div id="window-footer" class="plugbottom">
    <div>
      <!-- Display error message on demand -->
      {$message|escape}
    </div>
    <div>
      <input type="submit" name="login" value="{t}Sign in{/t}" title="{t}Click here to log in{/t}"/>
      <input type="hidden" name="javascript" value="false" />
    </div>
  </div>

</form>
</div>

</div>

{include file={filePath file="copynotice.tpl"}}

<script>
<!--
  enable_keyPress = false;
  focus_field("{$focusfield}");
  next_msg_dialog();
-->
</script>
</body>
</html>
