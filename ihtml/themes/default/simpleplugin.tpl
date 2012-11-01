{foreach from=$sections key=section item=display}
  {$display}
{/foreach}

<input name="{$hiddenPostedInput}" value="1" type="hidden"/>
