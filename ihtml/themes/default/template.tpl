<h1>
  {t}Creating a new object using templates{/t}
</h1>

<hr/>

{if isset($template_dialog)}
  {$template_dialog}
{else}
<table border="0">
  <tr>
    <td><label for="template">{t}Template{/t}</label></td>
    <td>
      <select name="template" id="template">
       {html_options options=$templates}
      </select>
    </td>
  </tr>
</table>
{/if}

<br/>
<p class="plugbottom">
  <input type="submit" name="template_continue" value="{t}Continue{/t}"/>
  &nbsp;
  <input type="submit" name="template_cancel" value="{msgPool type=cancelButton}"/>
</p>
