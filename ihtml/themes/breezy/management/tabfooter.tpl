{if $readOnly}
  <p class="plugbottom">
    <input type="submit" name="edit_cancel" value="{msgPool type=cancelButton}"/>
  </p>
{else}
  <p class="plugbottom">
    <input type="submit" name="edit_finish" value="{msgPool type=okButton}"/>
    &nbsp;
    {if $showApply}
      <input type="submit" name="edit_apply" value="{msgPool type=applyButton}"/>
      &nbsp;
    {/if}
    <input type="submit" name="edit_cancel" value="{msgPool type=cancelButton}"/>
  </p>
{/if}
