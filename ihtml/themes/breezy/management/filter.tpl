<input type="hidden" name="FILTER_PID" value="{$FILTER_PID}"/>

<div class="contentboxh">
 <p>{t}Filter{/t}</p>
</div>

<div class="contentboxb">
  <fieldset><legend>{t}Types{/t}</legend>
  {foreach from=$TYPES key="key" item="type"}
    <label for="{$key}">
      <input type="checkbox" id="{$key}" name="{$key}" value="1"
        {if ($type.show)}checked="checked"{/if}/>
      <img src="{$type.infos.icon|escape}" alt=""/>
      {$type.infos.name|escape}
    </label>
  {/foreach}
  {if isset($TEMPLATES)}
    <label for="filter_type_TEMPLATE">
      <input type="checkbox" id="filter_type_TEMPLATE" name="filter_type_TEMPLATE" value="1"
        {if ($TEMPLATES)}checked="checked"{/if}/>
      <img src="geticon.php?context=devices&amp;icon=template&amp;size=16" alt=""/>
      {t}Template{/t}
    </label>
  {/if}
  </fieldset>
  {foreach from=$FILTERS key="key" item="element"}
    {$element}
  {/foreach}

  <hr/>

  {if $SHOWSCOPE}
  <label for="SCOPE">
    <input type="checkbox" id="SCOPE" name="SCOPE" value="1"
    {if ($SCOPE == 'sub')}checked="checked"{/if}/>
    &nbsp;{t}Search in subtrees{/t}
  </label>
  <hr/>
  {/if}

  <label for="SEARCH" title="{$SEARCHDESC|escape}"><img src="geticon.php?context=actions&amp;icon=system-search&amp;size=16" alt="Search"/>
    <input class="filter_textfield" id="SEARCH" name="SEARCH" type="search" value="{$SEARCH}"/>
  </label>
  <div>
    <input type="submit" name="apply" value="{t}Apply filter{/t}"/>
  </div>
</div>
