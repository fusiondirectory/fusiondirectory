<fieldset><legend>{$NAME|escape}</legend>
  {foreach from=$INPUTS key="key" item="input"}
    <label title="{$input.desc|escape}">
      {if isset($input.icon)}
        <img src="{$input.icon|escape}" alt=""/>
      {/if}
      {$input.name|escape}
    </label>
  {/foreach}
</fieldset>
