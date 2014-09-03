<div class="default">
  <p>
    <b>{t}Create your configuration file{/t}</b>
  </p>
  <p>
    {$msg2}
  </p>
  <p>
    {t escape=no 1="<strong>fusiondirectory-setup --check-config</strong>"}Run %1 to put the correct right on fusiondirectory.conf{/t}
  </p>
  <p>
    <input type="submit" name="getconf" value="{t}Download configuration{/t}"/>
  </p>
    {if $err_msg != ""}
      <hr/>
      <br/>
      {t}Status: {/t}
      <strong>{$err_msg}</strong>
    {/if}
</div>
