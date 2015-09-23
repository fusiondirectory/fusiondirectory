<div id="{$sectionId}" class="plugin_section">
  <span class="legend">
    {$section}
  </span>
  <div>
    {foreach from=$attributes item=infos}

      {foreach from=$infos item=val key=key}
        <!-- Entry {$key} -->
        {if $val.SOLUTION != "" && !$val.RESULT}

          <!-- Add ability to display info popup -->
          <div class='step2_entry_container_info' style='background-color:{cycle values="#F0F0F0, #FFF"}'>
        {else}

          <!-- Normal entry everything is fine -->
          <div class='step2_entry_container' style='background-color:{cycle values="#F0F0F0, #FFF"}'>
        {/if}

        <div class='step2_entry_name'>{$val.NAME}</div>
        <div class='step2_entry_status'>

        {if $val.RESULT}
          <div class='step2_successful'>{t}Ok{/t}</div>
        {else}
          {if $val.MUST}
            <div class='step2_failed'>{t}Error{/t}</div>
          {else}
            <div class='step2_warning'>{t}Warning{/t}</div>
          {/if}
          {if $val.SOLUTION != ""}

          <!-- Information popup -->
          <div class='solution_visible' id='sol_{$key}'>
            {if $val.MUST}
              <div class='step2_failed_text' style="background-image: url('geticon.php?context=status&icon=dialog-error&size=16')">
              {$val.SOLUTION}<br/>
              <b>{t}FusionDirectory will NOT run without fixing this.{/t}</b>
            {else}
              <div class="step2_warning_text" style="background-image: url('geticon.php?context=status&icon=dialog-warning&size=16')">
              {$val.SOLUTION}<br/>
              <b>{t}FusionDirectory will run without fixing this.{/t}</b>
            {/if}
            </div>
          </div>
          {/if}
        {/if}
          </div>
        </div>
      {/foreach}

    {/foreach}
  </div>
</div>
