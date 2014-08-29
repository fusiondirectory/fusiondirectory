<div>
  <div class="default">
    <h2>User migration</h2>

    <p>{t}The listed users are currently invisible in the FusionDirectory user interface. If you want to change this for a couple of users, just select them and use the 'Migrate' button below.{/t}</p>
    <p>{t}If you want to know what will be done when migrating the selected entries, use the 'Show changes' button to see the LDIF.{/t}</p>
    {foreach from=$infos item=user key=key}
      {if $user.checked}
        <input type='checkbox' name='migrate_{$key}' checked id='migrate_{$key}'>
        {$user.dn}
        {if $user.after != ""}
          <div class="step2_entry_container_info" id="sol_8">
            {t}Current{/t}
            <div style='padding-left:20px;'>
              <pre>
              dn: {$user.dn}
              {$user.before}
              </pre>
            </div>
            {t}After migration{/t}
            <div style='padding-left:20px;'>
              <pre>
              dn: {$user.dn}
              {$user.after}
              </pre>
            </div>
          </div>
        {/if}
      {else}
        <input type='checkbox' name='migrate_{$key}' id='migrate_{$key}'>
        {$user.dn}
      {/if}
      <br>
    {/foreach}
    <input type='checkbox' id='toggle_calue' onClick="toggle_all_('^migrate_','toggle_calue')">
    {t}Select all{/t}
    <br/>

    {if $user_details}
      <input type='submit' name='dialog_hidechanges' value='{t}Hide changes{/t}'>
    {else}
      <input type='submit' name='dialog_showchanges' value='{t}Show changes{/t}'>
    {/if}

    <hr/>

    <div style='width:99%; text-align:right; padding-top:5px;'>
      <input type='submit' name='dialog_confirm' value='{t}Apply{/t}'>
      &nbsp;
      <input type='submit' name='dialog_cancel' value='{t}Cancel{/t}'>
    </div>
  </div>
</div>
