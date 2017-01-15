<div class="contentboxh">
 <p>{t}Filter{/t}</p>
</div>

<div class="contentboxb">
  {if isset($objectFilters)}
    {foreach from=$objectFilters item="ofilter"}
      {${$ofilter.id}}<label for="{$ofilter.id}">&nbsp;{$ofilter.label}</label><br/>
    {/foreach}
  {/if}

  <hr/>
  {$SCOPE}
  <hr/>

  <label for="NAME"><img src="geticon.php?context=actions&amp;icon=system-search&amp;size=16" alt="Search"/></label>{$NAME}

  <div>
    {$APPLY}
  </div>
</div>
