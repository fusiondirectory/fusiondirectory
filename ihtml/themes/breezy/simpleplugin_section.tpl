<fieldset id="{$sectionId}" class="plugin-section{$sectionClasses}">
  <legend><span>{$section|escape}</span></legend>
  <div>
  <table>
    {foreach from=$attributes item=attribute key=id}
      <tr class="
        {if $attribute.subattribute}subattribute{/if}
        {if $attribute.required}required{/if}
        {if !$attribute.readable}nonreadable{/if}
        {if !$attribute.writable}nonwritable{/if}
      ">
        <td title="{$attribute.description|escape}"><label for="{$attribute.htmlid}">{eval var=$attribute.label}</label></td>
        <td>{eval var=$attribute.input}</td>
      </tr>
    {/foreach}
  </table>
  </div>
</fieldset>
