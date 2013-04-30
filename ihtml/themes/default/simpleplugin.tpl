<div class="plugin_sections">
  {foreach from=$sections key=section item=display}
    {$display}
  {/foreach}
</div>

<input name="{$hiddenPostedInput}" value="1" type="hidden"/>

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
    focus_field('{$focusedField}');
  -->
</script>
