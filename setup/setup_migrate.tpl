<div id="{$sectionId}" class="plugin-section{$sectionClasses}">
  <span class="legend">
    {$section}
  </span>
  <div>
    {foreach from=$attributes item=checks}
      <p>{t}During the LDAP inspection, we're going to check for several common pitfalls that may occur when migration to FusionDirectory base LDAP administration. You may want to fix the problems below, in order to provide smooth services.{/t}
      </p>

      {foreach from=$checks item=check key=key}
        <div style="width:98%; padding:4px; background-color:{cycle values="#F0F0F0, #FFF"}">
          {if $check->error}
            <!-- Add ability to display info popup -->
            <div class="step2-entry-container-info">
          {else}
            <!-- Normal entry everything is fine -->
            <div class="step2-entry-container">
          {/if}
          <div class="step2-entry-name"><b>{$check->title}</b></div>
          <div class="step2-entry-status">
            {if $check->status}
              <div class="step2-successful">{$check->msg}</div>
            {else}
              <div class="step2-failed">{$check->msg}</div>
            {/if}
          </div>
            {if $check->error}
              {$check->error}
            {/if}
          </div>
        </div>
      {/foreach}
    {/foreach}
    <br/>
    <input type="submit" name="reload" value="{t}Check again{/t}"/>
  </div>
</div>
