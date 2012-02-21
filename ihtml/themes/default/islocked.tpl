<div style="font-size:20px;">
  <img alt="" src="images/encrypted.png" style="vertical-align:top;">&nbsp;{t}Locking conflict detected{/t}
</div>

<p>
  <b>{t}Warning{/t}:</b> {$message}
</p>
<p>
  {t}If this lock detection is false, the other person has obviously closed the webbrowser during the edit operation. You may want to take over the lock by pressing the 'Edit anyway' button.{/t}
</p>

<p class="plugbottom">
  <input type="submit" name="delete_lock" value="{$action}">
  {if $allow_readonly}
  &nbsp;
  <input type="submit" name="open_readonly" value="{t}Read only{/t}">
  {/if}
  &nbsp;
  <input type="submit" name="cancel_lock" value="{t}Cancel{/t}">
</p>
      
<input type="hidden" name="dn" value="{$dn}">

