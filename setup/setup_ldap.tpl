<div class='default' style="width:100%; margin-right:20px;">

  {if $resolve_user}

  <div style='float:left; width:99%; margin-bottom:20px;'>
    <p>
    <b>{t}Please choose the LDAP user to be used by FusionDirectory{/t}</b>
    </p>
    <select name='admin_to_use' size=20 style="width:100%; margin-bottom:10px;">
      {html_options options=$resolved_users selected=$admin}
    </select>
    <input type='text' value='{$resolve_filter}' name='resolve_filter'>
    <input type='submit' value='{t}Search{/t}' name='resolve_search'>
    <p class='seperator' style='margin-bottom:10px;'>&nbsp;</p>
    <div style='text-align:right'>
    <input type='submit' value='{t}Apply{/t}' name='use_selected_user'>
    <input type='submit' value='{t}Cancel{/t}' name='resolve_user'>
    </div>
  </div>

  <div style="clear:both;"></div>

  {else}
  <p>
  <b>{t}LDAP connection{/t}</b>
  </p>
  <div class='step4_container'>
    <div class='step4_name'>
      {t}Location name{/t}
    </div>
    <div class='step4_value'>
      <input type='text' name='location' maxlength='80' size='40' value='{$location}'>
    </div>
  </div>
  <div class='step4_container'>
    <div class='step4_name'>
      <div style='vertical-align:middle;height:100%;'>{t}Connection URI{/t}</div>
    </div>
    <div class='step4_value'>
      <input type='text' name='connection' maxlength='80' size='40' value='{$connection}'>
    </div>
  </div>
        <div class='step4_container'>
          <div class='step4_name'>
            {t}TLS connection{/t}
          </div>
          <div class='step4_value'>
            <select name="tls" size="1" title="">
      {html_options options=$bool selected=$tls}
            </select>
          </div>
        </div>
  <div class='step4_container'>
    <div class='step4_name'>
      {t}Base{/t}
    </div>
    <div class='step4_value'>
      {if $namingContextsCount >= 1}
        <select name='base'>
          {html_options values=$namingContexts output=$namingContexts selected=$base}
        </select>
      {else}
        <input type='text' name='base' maxlength='80' size='40' value='{$base}'>
      {/if}
      <input type='image' class='center' src='images/lists/reload.png' title='{t}Reload{/t}' name='reload' alt='{t}Reload{/t}'>
    </div>
  </div>
  <p style="margin-top:20px">
  <b>{t}Authentication{/t}</b>
  </p>
  <div class='step4_container'>
    <div class='step4_name'>
      {t}Admin DN{/t}
    </div>
    <div class='step4_value'>
      <input type='text' name='admin_given' maxlength='160' size='40' value='{$admin_given}'>
      {if $append_base_to_admin_dn},{$base_to_append}{/if}
      <input type='image' class='center' src='images/lists/folder.png' title='{t}Select user{/t}' name='resolve_user' alt='{t}Select user{/t}'>
    </div>
  </div>

  <div class='step4_container'>
    <div class='step4_name'>
    </div>
    <div class='step4_value'>
      <input onClick='document.mainform.submit();' {if $append_base_to_admin_dn} checked {/if} type='checkbox' name='append_base_to_admin_dn' id='append_base_to_admin_dn' value='1'/><label for="append_base_to_admin_dn">&nbsp;{t}Automatically append LDAP base to admin DN{/t}</label>
    </div>
  </div>


  <div class='step4_container'>
    <div class='step4_name'>
      {t}Admin password{/t}
    </div>
    <div class='step4_value'>
      <input type='password' name='password' value='{$password}'>
    </div>
  </div>

  <p style="margin-top:20px">
  <b>{t}Schema based settings{/t}</b>
  </p>
      <div class='step4_container'>
    <div class='step4_name'>
        {t}Use rfc2307bis compliant groups{/t}
    </div>
    <div class='step4_value'>
        <select name="rfc2307bis" size="1" title="">
        {html_options options=$bool selected=$rfc2307bis}
        </select>
    </div>
      </div>
  {/if}

  {if ! $resolve_user}
  <p style="margin-top:20px">
  <b>{t}Current status{/t}</b>
  </p>
    <div class='step4_container' style='height:45px;'>
        <div class='step4_name' style='height:45px;'>
            {t}Information{/t}
        </div>
        <div class='step4_value' style='height:45px;'>
      {$connection_status}
        </div>
    </div>
  {/if}

</div>
<!-- Place cursor -->
<script type="text/javascript">
  <!-- // First input field on page
  focus_field('location');
  -->
</script>

