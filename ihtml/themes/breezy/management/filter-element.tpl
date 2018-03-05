<fieldset><legend>{$NAME}</legend>
  {foreach from=$INPUTS key="key" item="input"}
    <label for="{$key}" title="{$input.desc}">
      <input type="checkbox" id="{$key}" name="{$key}" value="1"
        {if ($input.checked)}checked="checked"{/if}/>
      {if isset($input.icon)}
        <img src="{$input.icon|escape}" alt="{$key}"/>
      {else}
        <img src="images/empty.png" alt=""/>
      {/if}
      {$input.name|escape}
    </label>
  {/foreach}
</fieldset>
