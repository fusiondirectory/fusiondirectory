<!-- Headline -->
<body>
{$php_errors}
<form action='main.php{$plug}' name='mainform' method='post' enctype='multipart/form-data'>
<div class='setup_header'>
  <div style="float:left;"><img src='themes/default/images/go_logo.png' class='center' alt='FusionDirectory' style="padding-left:2px; padding-right:25px;">
   {if !$javascript}
    <a class="maintitlebar" href='main.php?reset=1' onClick='return question("{t}You are currently editing a database entry. Do you want to dismiss the changes?{/t}", "main.php?reset=1");' ><img src='{$go_home}' class='center' border="0" alt="Main">&nbsp;{t}Main{/t}
    </a>
   {else}
    <a class="maintitlebar" onClick='return question("{t}You are currently editing a database entry. Do you want to dismiss the changes?{/t}", "main.php?reset=1");' ><img src='{$go_home}' class='center' border="0" alt="Main">&nbsp;{t}Main{/t}
    </a>
   {/if}
    &nbsp;
   {if !$javascript}
    <a class="maintitlebar" href='logout.php?request' onClick='return question("{t}You are currently editing a database entry. Do you want to dismiss the changes?{/t}", "logout.php?request");' style="margin-left:35px;">
      <img src='{$go_out}' class='center' border="0" alt="Sign out">&nbsp;{t}Sign out{/t}
    </a>
    {else}
    <a class="maintitlebar" onClick='return question("{t}You are currently editing a database entry. Do you want to dismiss the changes?{/t}", "logout.php?request");' style="margin-left:35px;">
      <img src='{$go_out}' class='center' border="0" alt="Sign out">&nbsp;{t}Sign out{/t}
    </a>
    {/if}
    <a class="plugtop" style="margin-left:35px;">
      <img src="{$headline_image}" class="center" align="middle" alt="{t}{$headline}{/t}">&nbsp;
      {t}{$headline}{/t}
    </a>
  </div>
  <div style="padding-top:8px;text-align:right;height:28px;color:white;"><a>{t}Signed in:{/t} <b>{$username}</b></a></div>
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
                <table style="vertical-align:top; text-align:justify; 
                    background-color: white; border:1px solid #AAA; 
                    margin-top:0px; margin-left:7px; margin-right:7px; 
                    width:99%" summary="">
                    <tr>
                        <td>
                            {$msg_dialogs}
                            {$contents}
                        </td>
                    </tr>
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

{$errors}{$focus}<input type="hidden" name="php_c_check" value="1"></form></body></html>
