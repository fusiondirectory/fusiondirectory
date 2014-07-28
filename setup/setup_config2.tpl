<div class='default' style='margin:12px; '>
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

        <p><b>{t}Snapshots / Undo{/t}</b></p>
            {if $generic_settings.snapshot_active == FALSE}
        <div class='step4_container'>
         <div class='step4_name'>
            <input type='checkbox' value='1' name='snapshot_active'
                onClick='changeState("snapshot_base");'/>{t}Enable snapshots{/t}
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
        </div>
            {/if}
</div>

<input type='hidden' value='1' name='step6_posted'>
<div style="clear:both"></div>
