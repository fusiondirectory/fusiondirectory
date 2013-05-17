<!-- Headline -->
<body onLoad="javascript:$$('div.debug_div').each(function (a) { a.hide(); });">
{$game_screen}
{$php_errors}
<form action='main.php{$plug}' name='mainform' id='mainform' method='post' enctype='multipart/form-data'>
<div class='setup_header'>
  <div style="float:left;"><img src='{filePath file="images/fd_logo.png"}' class='center' alt='FusionDirectory' style="padding-left:2px; padding-right:25px;">
    <a class="maintitlebar" href='main.php?reset=1'>
      <img src='{filePath file="images/home.png"}' class='center' border="0" alt="Main"/>&nbsp;{t}Main{/t}
    </a>
    &nbsp;
    <a class="maintitlebar" href='logout.php?request' style="margin-left:35px;">
      <img src='{filePath file="images/sign_out.png"}' class='center' border="0" alt="Sign out"/>&nbsp;{t}Sign out{/t}
    </a>
    <a class="plugtop" style="margin-left:35px;">
      <img src="{$headline_image}" class="center" align="middle" alt="{t}{$headline}{/t}"/>&nbsp;
      {t}{$headline}{/t}
    </a>
  </div>
  <div style="padding-top:8px;text-align:right;height:38px;color:black;">
    <a>{t}Signed in:{/t} <b>{$username}</b></a>
    <div class='logout-label'>
      <canvas id="sTimeout" width="22" height="22" title="{$sessionLifetime}|{t}Session expires in %d!{/t}"></canvas>
    </div>
  </div>
</div>
<!--<div class='setup_menu'>
  <b>{t}FusionDirectory main menu{/t}</b>
</div>-->

{if !$hideMenus}

    <table summary="" class="framework" cellspacing="0" cellpadding="0" style='margin-top:7px'>
        <tr style="vertical-align:top;">

            <!-- Menu -->
            <td style="background-color:white; vertical-align:top;" id="menucell">
                {$menu}
                <br />
                <div style="height:1px; width:140px;"></div>
            </td>

            <!-- Plugin window -->
            <td style="background-color:white; width:100%; vertical-align:top; height:15px;" colspan="2">
                <table summary="" class="plugin_window">
                  <tbody>
                    <tr>
                        <td>
                            {$msg_dialogs}
                            {$contents}
                        </td>
                    </tr>
                  </tbody>
                </table>
            </td>
        </tr>
        {if $channel != ""}
            <input type="hidden" name="_channel_" value="{$channel}">
        {/if}
    </table>

{else}

    <table summary="" class="framework" cellspacing="0" cellpadding="0" style='margin-top:7px' width="99%">
        <tr style="vertical-align:top;">
            <td style="background-color:white; vertical-align:top;" id="menucell">
                {$msg_dialogs}
                {$contents}
                {if $channel != ""}
                  <input type="hidden" name="_channel_" value="{$channel}">
                {/if}
            </td>
        </tr>
    </table>

{/if}

{$errors}
{$focus}
<input type="hidden" name="php_c_check" value="1">
</form>


  <!-- Automatic logout when session is expired -->
  <script type='text/javascript'>
  {literal}
   function logout()
   {
    document.location = 'logout.php';
   }
  {/literal}

   logout.delay({$sessionLifetime});

  </script>

</body>
</html>
