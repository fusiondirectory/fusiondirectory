<fieldset id="{$sectionId}" class="plugin_section{$sectionClasses}">
  <legend><span>{$section}</span></legend>
  <div>
  <table>
    {foreach from=$attributes item=attribute key=id}
      <tr>
        <td title="{$attribute.description}"><label for="{$attribute.htmlid}">{eval var=$attribute.label}</label></td>
        <td>{eval var=$attribute.input}</td>
      </tr>
    {/foreach}
  </table>
  </div>
</fieldset>
