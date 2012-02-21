<div class='default'>
    <p><b>{t}Look and feel{/t}</b></p>

    <div class='step4_container'>
        <div class='step4_name'>
            {t}Theme{/t}
        </div>
        <div class='step4_value'>
            <select name="theme">
                {html_options options=$themes selected=$theme}
            </select>
        </div>
    </div>

    <p><b>{t}Apache{/t}</b></p>

    <div class='step4_container'>
        <div class='step4_name'>
            {t}Compress output send to browser{/t}
        </div>
        <div class='step4_value'>
            <select name="compressed">
                {html_options options=$bool selected=$compressed}
            </select>
        </div>
    </div>
    <p><b>{t}People and group storage{/t}</b></p>
    <div class='step4_container'>
        <div class='step4_name'>
            {t}People DN attribute{/t}
        </div>
        <div class='step4_value'>
            <select size="1" name="peopledn">
                {html_options values=$peopledns output=$peopledns selected=$peopledn}
            </select>
        </div>
    </div>

  <div class='step4_container'>
        <div class='step4_name'>
            {t}People storage subtree{/t}
        </div>
        <div class='step4_value'>
            <input type='text' name='peopleou' maxlength='40' size='40' value='{$peopleou}'>
        </div>
    </div>

  <div class='step4_container'>
        <div class='step4_name'>
            {t}Group storage subtree{/t}
        </div>
        <div class='step4_value'>
            <input type='text' name='groupou' maxlength='40' size='40' value='{$groupou}'>
        </div>
    </div>

  <div class='step4_container'>
        <div class='step4_name'>
            {t}Include personal title in user DN{/t}
        </div>
        <div class='step4_value'>
            <select name="include_personal_title" size="1" title="">
            {html_options options=$bool selected=$include_personal_title}
            </select>
        </div>
    </div>

  <div class='step4_container'>
        <div class='step4_name'>
            {t}Relaxed naming policies{/t}
        </div>
        <div class='step4_value'>
            <select name="strict" size="1" title="">
            {html_options options=$bool selected=$strict}
            </select>
        </div>
    </div>

    <div class='step4_container'>
        <div class='step4_name'>
            {t}Automatic UIDs{/t}
        </div>
        <div class='step4_value'>
            {if $id_settings.idgen_active == FALSE}
                <input type='checkbox' value='1' name='idgen_active'
                    onClick='changeState("idgen");'>
                <input size='20' id='idgen' name='idgen' type='text' value='{$id_settings.idgen}' disabled>
            {else}
                <input type='checkbox' value='1' name='idgen_active' checked>
                <input size='20' id='idgen' name='idgen' type='text' value='{$id_settings.idgen}'  >
            {/if}
        </div>
    </div>

  <div class='step4_container'>
        <div class='step4_name'>
            {t}GID / UID min id{/t}
        </div>
        <div class='step4_value'>
            {if $id_settings.minid_active == FALSE}
                <input type='checkbox' value='1' name='minid_active'
                    onClick='changeState("minid");'>
                <input size='10' id='minid' name='minid' type='text' value='{$id_settings.minid}' disabled>
            {else}
                <input type='checkbox' value='1' name='minid_active' checked>
                <input size='10' id='minid' name='minid' type='text' value='{$id_settings.minid}'  >
            {/if}
        </div>
    </div>

    <div class='step4_container' style="margin-bottom:5px;">
        <div class='step4_name'>
            {t}Number base for people/groups{/t}
        </div>
        <div class='step4_value'>
            <input type='text' name='uidbase' maxlength='20' size='10' value='{$uidbase}'>
        </div>
     </div>

    <p><b>{t}Password settings{/t}</b></p>

    <div class='step4_container'>
        <div class='step4_name'>
            {t}Password encryption algorithm{/t}
        </div>
        <div class='step4_value'>
            <select name="encryption" size="1" title="">
                {html_options options=$crypt_methods selected=$encryption}
            </select>
        </div>
    </div>

    <div class='step4_container' style='height:4em;'>
        <div class='step4_name' style='height:4em;'>
            {t}Password restrictions{/t}
        </div>
        <div class='step4_value' style='height:4em;'>

      {if $pwd_rules.pwminlen_active == FALSE}
        <input type='checkbox' value='1' name='pwminlen_active'
          onClick='changeState("pwminlen");'>
          {t}Password minimum length{/t}
        <input id='pwminlen' name='pwminlen' type='text' value='{$pwd_rules.pwminlen}' size=3 disabled>
      {else}
        <input type='checkbox' value='1' name='pwminlen_active' checked>
        {t}Password minimum length{/t}
        <input id='pwminlen' name='pwminlen' type='text' value='{$pwd_rules.pwminlen}' size=3 >
      {/if}
      <br>
      {if $pwd_rules.pwdiffer_active == FALSE}
        <input type='checkbox' value='1' name='pwdiffer_active' 
          onClick='changeState("pwdiffer");'>
        {t}Different characters from old password{/t}
        <input id='pwdiffer' name='pwdiffer' type='text' value='{$pwd_rules.pwdiffer}' size=3 disabled>
      {else}
        <input type='checkbox' value='1' name='pwdiffer_active' checked>
        {t}Different characters from old password{/t}
        <input id='pwdiffer' name='pwdiffer' type='text' value='{$pwd_rules.pwdiffer}' size=3 >
      {/if}
  
        </div>
    </div>

    <div class='step4_container'>
        <div class='step4_name'>
                {t}Password change hook{/t}
        </div>
        <div class='step4_value'>
            {if $pwd_rules.externalpwdhook_active == FALSE}
                <input type='checkbox' value='1' name='externalpwdhook_active'
                    onClick='changeState("externalpwdhook");'>
                <input size=40 id='externalpwdhook' name='externalpwdhook' type='text' value='{$pwd_rules.externalpwdhook}' disabled>
            {else}
                <input type='checkbox' value='1' name='externalpwdhook_active' checked>
                <input size=40 id='externalpwdhook' name='externalpwdhook' type='text' value='{$pwd_rules.externalpwdhook}'  >
            {/if}
        </div>
    </div>

    <div class='step4_container'>
        <div class='step4_name'>
            {t}Use SASL for kerberos{/t}
        </div>
        <div class='step4_value'>
            <select name="krbsasl" size="1" title="">
            {html_options options=$bool selected=$krbsasl}
            </select>
        </div>
    </div>

    <div class='step4_container'>
        <div class='step4_name'>
            {t}Use account expiration{/t}
        </div>
        <div class='step4_value'>
            <select name="account_expiration" size="1" title="">
            {html_options options=$bool selected=$account_expiration}
            </select>
        </div>
    </div>


<input type='hidden' value='1' name='step5_posted'>
<!--
         {t}FusionDirectory supports several encryption types for your passwords. Normally this is adjustable via user templates, but you can specify a default method to be used here, too.{/t}
         {t}FusionDirectory always acts as admin and manages access rights internally. This is a workaround till OpenLDAP's in directory ACI's are    fully implemented. For this to work, we need the admin DN and the corresponding password.{/t}
         {t}Some basic LDAP parameters are tunable and affect the locations where FusionDirectory saves people and groups, including the way accounts get created. Check the values below if the fit your needs.{/t}
         {t}FusionDirectory has modular support for several mail methods. These methods provide interfaces to users mailboxes and general handling    for quotas. You can choose the dummy plugin to leave all your mail settings untouched.{/t}-->

