<div style="font-size:18px;">
  {t}Applying a template{/t}
</div>

<p>
 {t}Applying a template to several users will replace all user attributes defined in the template.{/t}
</p>

<p class="seperator">&nbsp;</p>
<br>

{if $templates}
<table summary="" style="vertical-align:top; text-align:left;" cellpadding=4 border=0>
  <tr>
    <td><b><LABEL for="template">{t}Template{/t}</LABEL></b></td>
    <td>
      <select size="1" name="template" id="template">
       {html_options options=$templates}
      </select>
    </td>
  </tr>
</table>

<br>
<p class="plugbottom">
  <input type=submit name="templatize_continue" value="{msgPool type=applyButton}">
  &nbsp;
  <input type=submit name="edit_cancel" value="{msgPool type=cancelButton}">
</p>

{else}

{t}No templates available!{/t}

<p class="plugbottom">
  <input type=submit name="edit_cancel" value="{msgPool type=cancelButton}">
</p>
{/if}


<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('template');
  -->
</script>
