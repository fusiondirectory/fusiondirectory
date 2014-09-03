<div>
  <div class="default">
    <h2>{$infos.title}</h2>

    {if $infos.outside}
      <p>
      {t}This dialog allows moving a couple of entries to the configured tree. Doing this may straighten your LDAP service.{/t}
      </p>
      <p style="color:red">
      {t}Be careful with this option! There may be references pointing to these entries. The FusionDirectory setup can't migrate references, so you may want to cancel the migration in this case.{/t}
      </p>
      <p>
        {t}Move selected entries into this tree{/t}:
        <select name="destination">
          {html_options values=$infos.ous output=$infos.ous selected=$infos.destination}
        </select>
      </p>
    {else}
      <p>{t}The listed entries are currently invisible in the FusionDirectory interface. If you want to change this for a couple of entries only, just select them and use the 'Migrate' button below.{/t}</p>
      <p>{t}If you want to know what will be done when migrating the selected entries, use the 'Show changes' button to see the LDIF.{/t}</p>
    {/if}

    {foreach from=$infos.entries item=entry key=key}
      {if $entry.checked}
        <input type="checkbox" name="migrate_{$key}" checked="checked" id="migrate_{$key}"/>
        <label for="migrate_{$key}">{$entry.dn}</label>
        {if $entry.after != ""}
          <div class="step2_entry_container_info">
            {t}Current{/t}
            <div style="padding-left:20px;">
<pre>
dn: {$entry.dn}
{$entry.before}
</pre>
            </div>
            {t}After migration{/t}
            <div style="padding-left:20px;">
<pre>
dn: {$entry.dn}
{$entry.after}
</pre>
            </div>
          </div>
        {elseif $entry.ldif != ""}
          <div class="step2_entry_container_info">
            <div style="padding-left:20px;">
              <pre>{$entry.ldif}</pre>
            </div>
          </div>
        {/if}
      {else}
        <input type="checkbox" name="migrate_{$key}" id="migrate_{$key}"/>
        <label for="migrate_{$key}">{$entry.dn}</label>
      {/if}
      <br>
    {/foreach}
    <input type="checkbox" id="toggle_calue" onClick="toggle_all_('^migrate_','toggle_calue')"/>
    <label for="toggle_calue">{t}Select all{/t}</label>
    <br/>

    <input type="submit" name="dialog_showchanges" value="{t}Show changes{/t}"/>

    <hr/>

    <div style="width:99%; text-align:right; padding-top:5px;">
      <input type="submit" name="dialog_confirm" value="{t}Apply{/t}"/>
      &nbsp;
      <input type="submit" name="dialog_cancel" value="{t}Cancel{/t}"/>
    </div>
  </div>
</div>
