<div class='default' style='margin:12px; '>
  <div class='step4_container'>
    <div class='step4_name'>
      <label for='timezone'>{t}Timezone{/t}</label>
    </div>
    <div class='step4_value'>
      <select id='timezone' name='timezone' title='{t}Please choose your preferred timezone here{/t}' style="width:100%">
        {foreach from=$timezones item=val}
          <option {if $val == $timezone}selected{/if} value="{$val}">{$val}</option>
        {/foreach}
      </select>
    </div>
  </div>

  <p><b>{t}Snapshots{/t}</b></p>
  <div class='step4_container'>
    <div class='step4_name'>
      <label for='snapshot_active'>{t}Enable snapshots{/t}</label>
    </div>
    <div class='step4_value'>
      <input type='checkbox' value='1'
              id='snapshot_active' name='snapshot_active'
              onClick='changeState("snapshot_base");'
{if $generic_settings.snapshot_active}
              checked='checked'
{/if}
              />
    </div>
  </div>
  <div class='step4_container'>
    <div class='step4_name'>
      <label for='snapshot_base'>{t}Snapshot base{/t}</label>
    </div>
    <div class='step4_value'>
      <input style='width:220' id='snapshot_base' name='snapshot_base'
              type='text' value='{$generic_settings.snapshot_base}'
{if $generic_settings.snapshot_active == FALSE}
                disabled='disabled'
{/if}
              />
    </div>
  </div>
</div>

<input type='hidden' value='1' name='step6_posted'/>
<div style="clear:both"></div>
