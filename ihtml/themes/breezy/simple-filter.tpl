<div class="contentboxh">
 <p>{t}Filter{/t}</p>
</div>

<div class="contentboxb">
  {if isset($objectFilters)}
    {foreach from=$objectFilters item="ofilter"}
      <label for="{$ofilter.id}">
        {${$ofilter.id}}&nbsp;{$ofilter.label}
      </label>
    {/foreach}
  {/if}

  <hr/>
  {$SCOPE}
  <hr/>

  <label for="NAME" title="{$NAMEDESC}">
    <img src="geticon.php?context=actions&amp;icon=system-search&amp;size=16" alt="Search"/>
    {$NAME}
  </label>

  <div>
    {$APPLY}
  </div>
</div>
