<div style="font-size:18px;">
 <img alt="" src="images/warning.png"/>&nbsp;{t}Warning{/t}
</div>
<p>
 {$info}
 {t}This includes 'all' accounts, systems, etc. in this subtree. Please double check if your really want to do this since there is no way for FusionDirectory to get your data back.{/t}
</p>

<p>
 {t}Best thing to do before performing this action would be to save the current contents of your LDAP tree in a file. So - if you've done so - press 'Delete' to continue or 'Cancel' to abort.{/t}
</p>

<p class="plugbottom">
  <input type=submit name="delete_confirmed" value="{msgPool type=delButton}">
  &nbsp;
  <input type=submit name="delete_cancel" value="{msgPool type=cancelButton}">
</p>

