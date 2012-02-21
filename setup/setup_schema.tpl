<div class='default'>
    <p>
     <b>{t}Schema specific settings{/t}</b>
    </p>
    <div class='step4_container'>
        <div class='step4_name' style='width:30%'>
            {t}Enable schema validation when logging in{/t}
        </div>
        <div class='step4_value'>
      <select name='enable_schema_check'>
        {html_options options=$bool selected=$enable_schema_check}
            </select>
        </div>
    </div>
    <p>
     <b>{t}Check status{/t}</b>
    </p>
  <div>
    {if $failed_checks == 0}
      <font style="color:green">{t}Schema check succeeded{/t}</font>
    {else}
      <img src='images/small_warning.png' class='center'>
        <font style="color:red">{t}Schema check failed{/t}</font>


    {/if}
  </div>
  <div style="margin-left:20px;">

    {if $found_ocs == 0}
      <b>{t}Could not read any schema informations, all checks skipped. Adjust your ldap acls.{/t}</b>
      <br>
      {if !$database_initialised}
      <br>
        {t}It seems that your ldap database wasn't initialized yet. This maybe the reason, why FusionDirectory can't read your schema configuration!{/t}
      {/if}
    {else}
      {foreach from=$checks item=val key=key}
          {if !$checks[$key].STATUS}
          <br>
            {if $checks[$key].IS_MUST_HAVE}
              <font color='red'>{$checks[$key].MSG}</font>
            {else}
              {$checks[$key].MSG}
            {/if}
          <br>
          {/if}
      {/foreach}
    {/if}
  </div>
</div>
<input type='hidden' value='1' name='step7_posted'>
