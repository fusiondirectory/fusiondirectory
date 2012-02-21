<div class='default'>
  <p>
    <b>{t}Create your configuration file{/t}</b>
  </p>
  <p> 
    {$msg2}
  </p>

      {if $webgroup == ""}
{t}Depending on the user name your webserver is running on:{/t}
<tt>
<pre> chown root:www-data {$CONFIG_DIR}/{$CONFIG_FILE}
 chmod 640 {$CONFIG_DIR}/{$CONFIG_FILE}

or

 chown root:apache {$CONFIG_DIR}/{$CONFIG_FILE}
 chmod 640 {$CONFIG_DIR}/{$CONFIG_FILE}
</pre>
{else}
<pre>
 chown root:{$webgroup} {$CONFIG_DIR}/{$CONFIG_FILE}
 chmod 640 {$CONFIG_DIR}/{$CONFIG_FILE}
</pre>
{/if} 
  </tt>
  <p> 
    <input type='submit' name='getconf' value='{t}Download configuration{/t}'>
  </p>
    {if $err_msg != ""}
      <p class="seperator">&nbsp;</p>
      <br>
      {t}Status: {/t}
      <a style='color:red ; font-weight:bold '>{$err_msg}</a>
    {/if}

</div>
<input type='hidden' value='1' name='step8_posted'>
