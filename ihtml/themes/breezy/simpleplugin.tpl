<div class="plugin-sections">
  {foreach from=$sections key=section item=display}
    {$display}
  {/foreach}
</div>

{if is_array($hiddenPostedInput)}
  {foreach from=$hiddenPostedInput item=hiddenPostedInput_item}
    <input name="{$hiddenPostedInput_item|escape}" value="1" type="hidden"/>
  {/foreach}
{else}
  <input name="{$hiddenPostedInput|escape}" value="1" type="hidden"/>
{/if}

{if isset($focusedField)}
  <!-- Place cursor -->
  <script>
    <!-- // First input field on page
      focus_field('{$focusedField|escape}');
    -->
  </script>
{/if}
