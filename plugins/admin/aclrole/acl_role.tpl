{if $dialogState eq 'create'}
  <h1>{t}List of available ACL categories{/t}</h1>
  {$aclList}

  <hr/>
  <div style="text-align:right;margin-top:5px">
    {render acl=$gosaAclTemplateACL}
      <input type="submit" name="submit_new_acl" value="{msgPool type=applyButton}"/>
      &nbsp;
    {/render}
    <input type="submit" name="cancel_new_acl" value="{msgPool type=cancelButton}"/>
  </div>
{/if}

{if $dialogState eq 'edit'}
  <h1>{$headline}</h1>

  {render acl=$gosaAclTemplateACL}
    {$aclSelector}
  {/render}

  <hr/>
  <div style="text-align:right;margin-top:5px">
    {render acl=$gosaAclTemplateACL}
      <input type="submit" name="submit_edit_acl" value="{msgPool type=applyButton}"/>
      &nbsp;
    {/render}
    <input type="submit" name="cancel_edit_acl" value="{msgPool type=cancelButton}"/>
  </div>
{/if}
