<div class='default' style='margin:12px; '>
    <p><b>{t}Samba settings{/t}</b></p>

    <div class='step4_container'>
        <div class='step4_name'>
            {t}Windows workstation branch{/t}
        </div>
        <div class='step4_value'>
            {if $generic_settings.wws_ou_active == FALSE}
                <input type='checkbox' value='1' name='wws_ou_active'
                    onClick='changeState("wws_ou");'>
                <input size=40 id='wws_ou' name='wws_ou' type='text' value='{$generic_settings.wws_ou}' disabled>
            {else}
                <input type='checkbox' value='1' name='wws_ou_active' checked>
                <input size=40 id='wws_ou' name='wws_ou' type='text' value='{$generic_settings.wws_ou}'  >
            {/if}
        </div>
    </div>
    <div class='step4_container'>
        <div class='step4_name'>
            {t}Samba SID mapping{/t}
        </div>
        <div class='step4_value'>
            <select name="sambaidmapping" size="1" title="">
      {html_options options=$bool selected=$sambaidmapping}
            </select>
        </div>
    </div>
  <div class='step4_container'>
        <div class='step4_name'>
            {t}Timezone{/t}
        </div>
        <div class='step4_value'>
            <select name='timezone' title='{t}Please choose your preferred timezone here{/t}' style="width:100%">
    {foreach from=$timezones item=val}
                <option {if $val == $timezone}selected{/if} value="{$val}">{$val}</option>
        {/foreach}

            </select>
        </div>
    </div>

    <p><b>{t}Mail settings{/t}</b></p>

    <div class='step4_container'>
        <div class='step4_name'>
            {t}Mail method{/t}
        </div>
      <div class='step4_value'>
            <select name="mail" size="1" title="" onChange="document.mainform.submit();">
                <option  value="disabled">{t}disabled{/t}</option>
                {foreach from=$mail_methods item=item}
                    <option value="{$item.name}" {if $mail == $item.name} selected {/if}>{$item.name}</option>
                {/foreach}
            </select>
        </div>
        <div class='step4_status'>
        </div>
    </div>

    <div class='step4_container'>
        <div class='step4_name'>
            {t}Account identification attribute{/t}
        </div>
        <div class='step4_value'>
            <select name="mail_attrib" size="1" {if $mail == "disabled"} disabled {/if}>
                <option value="mail" {if $mail_attrib == "mail"} selected {/if} >mail</option>
                <option value="uid" {if $mail_attrib == "uid"} selected {/if} >uid</option>
            </select>
        </div>
        <div class='step4_status'>
        </div>
    </div>

    <div class='step4_container'>
        <div class='step4_name'>
            {t}Vacation templates{/t}
        </div>
        <div class='step4_value'>
            {if $mail_settings.vacationdir_active == FALSE}
                <input type='checkbox' value='1' name='vacationdir_active'
                    onClick='changeState("vacationdir");'>
                <input size=40 id='vacationdir' name='vacationdir' type='text' value='{$mail_settings.vacationdir}' disabled>
            {else}
                <input type='checkbox' value='1' name='vacationdir_active' checked>
                <input size=40 id='vacationdir' name='vacationdir' type='text' value='{$mail_settings.vacationdir}'  >
            {/if}
        </div>
    </div>

    <div class='step4_container'>
        <div class='step4_name'>
            {t}Use Cyrus UNIX style{/t}
        </div>
        <div class='step4_value'>
            <select name="cyrusunixstyle" size="1" title="">
            {html_options options=$bool selected=$cyrusunixstyle}
            </select>
        </div>
    </div>


        <p><b>{t}Snapshots / Undo{/t}</b></p>
            {if $generic_settings.snapshot_active == FALSE}
        <div class='step4_container'>
         <div class='step4_name'>
                <input type='checkbox' value='1' name='snapshot_active'
                    onClick='
                                                                changeState("snapshot_base");
                                                                changeState("snapshot_server");
                                                                changeState("snapshot_user");
                                                                changeState("snapshot_password");'>{t}Enable snapshots{/t}
         </div>
        </div>
        <div style='margin-left:10px;'>
        <div class='step4_container'>
         <div class='step4_name'>
                {t}Snapshot base{/t}
         </div>
         <div class='step4_value'>
                <input style='width:220' id='snapshot_base' name='snapshot_base'
                                        type='text' value='{$generic_settings.snapshot_base}' disabled>
         </div>
        </div>
        <div class='step4_container'>
         <div class='step4_name'>
                    {t}Server{/t}
         </div>
         <div class='step4_value'>
                                <input style='width:220' id='snapshot_server' name='snapshot_server'
                                        type='text' value='{$generic_settings.snapshot_server}' disabled>
        </div>
        </div>
        <div class='step4_container'>
         <div class='step4_name'>
                        {t}User{/t}
         </div>
         <div class='step4_value'>
                                <input style='width:220' id='snapshot_user' name='snapshot_user'
                                        type='text' value='{$generic_settings.snapshot_user}' disabled>
        </div>
        </div>
        <div class='step4_container'>
         <div class='step4_name'>
                        {t}Password{/t}
         </div>
         <div class='step4_value'>
                                <input style='width:220' id='snapshot_password' name='snapshot_password'
                                        type='password' value='{$generic_settings.snapshot_password}' disabled>
        </div>
        </div>
        </div>

            {else}
        <div class='step4_container'>
         <div class='step4_name'>
                <input type='checkbox' value='1' name='snapshot_active' checked>{t}Enable snapshots{/t}
        </div>
        </div>
        <div style='margin-left:10px;'>
        <div class='step4_container'>
         <div class='step4_name'>
                    {t}Snapshot base{/t}
        </div>
         <div class='step4_value'>
           <input style='width:220' id='snapshot_base' name='snapshot_base'
                                        type='text' value='{$generic_settings.snapshot_base}'>
        </div>
        </div>
        <div class='step4_container'>
         <div class='step4_name'>
                        {t}Server{/t}
        </div>
         <div class='step4_value'>
                                <input style='width:220' id='snapshot_server' name='snapshot_server'
                                        type='text' value='{$generic_settings.snapshot_server}'>
        </div>
        </div>
        <div class='step4_container'>
         <div class='step4_name'>
                    {t}User{/t}
        </div>
         <div class='step4_value'>
                                <input style='width:220' id='snapshot_user' name='snapshot_user'
                                        type='text' value='{$generic_settings.snapshot_user}'>
        </div>
        </div>
        <div class='step4_container'>
         <div class='step4_name'>
                        {t}Password{/t}
        </div>
         <div class='step4_value'>
                                <input style='width:220;' id='snapshot_password' name='snapshot_password'
                                        type='password' value='{$generic_settings.snapshot_password}'>
        </div>
        </div>
        </div>
            {/if}
</div>

<input type='hidden' value='1' name='step6_posted'>
<div style="clear:both"></div>
