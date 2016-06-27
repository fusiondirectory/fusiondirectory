{include file={filePath file="simple-list.tpl"}}

<p class="plugbottom">
{if $MULTISELECT}
  <input type="submit" name="add_finish" value="{msgPool type=addButton}"/>
  &nbsp;
{/if}
  <input type="submit" name="add_cancel" value="{msgPool type=cancelButton}"/>
</p>
