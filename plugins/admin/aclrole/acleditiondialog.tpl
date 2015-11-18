  <h1>{$headline}</h1>
  {$aclEdition}

{if $dialogState eq 'create'}
  <p class="plugbottom">
    <input type="submit" name="add_finish" value="{msgPool type=applyButton}"/>
    <input type="submit" name="add_cancel" value="{msgPool type=cancelButton}"/>
  </p>
{/if}

{if $dialogState eq 'edit'}
  <p class="plugbottom">
    <input type="submit" name="submit_edit_acl" value="{msgPool type=applyButton}"/>
    <input type="submit" name="cancel_edit_acl" value="{msgPool type=cancelButton}"/>
  </>
{/if}
