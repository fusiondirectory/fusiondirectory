<input type="hidden" name="FILTER_PID" value="{$FILTER_PID}"/>

<div class="contentboxh">
 <p>{t}Filter{/t}</p>
</div>

<div class="contentboxb">
  {if isset($objectFilters)}
    {foreach from=$objectFilters item="ofilter"}
      {${$ofilter.id}}<label for="{$ofilter.id}">&nbsp;{$ofilter.label}</label><br/>
    {/foreach}
  {/if}
  {foreach from=$TYPES key="key" item="type"}
    <label for="{$key}">
      <input type="checkbox" id="{$key}" name="{$key}" value="1" onClick="document.mainform.submit();"
        {if ($type.show)}checked="checked"{/if}>
      <img src="{$type.infos.icon|escape}">
      {$type.infos.name}
    </label>
  {/foreach}
  {if isset($TEMPLATES)}
    <label for="TEMPLATES">
      <input type="checkbox" id="TEMPLATES" name="TEMPLATES" value="1" onClick="document.mainform.submit();"
        {if ($TEMPLATES)}checked="checked"{/if}>
      <img src="geticon.php?context=devices&amp;icon=template&amp;size=16">
      {t}Template{/t}
    </label>
  {/if}

  <hr/>
  <label for="SCOPE">
    <input type="checkbox" id="SCOPE" name="SCOPE" value="1" onClick="document.mainform.submit();"
    {if ($SCOPE == 'sub')}checked="checked"{/if}>
    &nbsp;{t}Search in subtrees{/t}
  </label>
  <hr/>

  <label for="SEARCH" title="{$SEARCHDESC|escape}"><img src="geticon.php?context=actions&amp;icon=system-search&amp;size=16" alt="Search"/>
    <input class="filter_textfield" id="SEARCH" name="SEARCH" type="text" value=""/>
  </label>
  <div>
    <input type="submit" name="apply" value="{t}Apply filter{/t}"/>
  </div>
</div>
