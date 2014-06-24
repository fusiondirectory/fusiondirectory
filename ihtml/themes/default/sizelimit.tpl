<h1>{$warning}</h1>
<p>
  {t}The size limit option makes LDAP operations faster and saves the LDAP server from getting too much load. The easiest way to handle big databases without long timeouts would be to limit your search to smaller values and use filters to get the entries you are looking for.{/t}
</p>
<p>
  <b>{t}Please choose the way to react for this session{/t}:</b>
</p>

<input type="radio" name="action" value="ignore" id="ignore"/>
  <label for="ignore">{t}ignore this error and show all entries the LDAP server returns{/t}</label><br/>
<input type="radio" name="action" value="limited" checked="checked" id="limited"/>
  <label for="limited">{t}ignore this error and show all entries that fit into the defined sizelimit and let me use filters instead{/t}</label><br/>
<input type="radio" name="action" value="newlimit" id="newlimit"/>
  {$limit_message}

<p class="plugbottom">
 <input type=submit name="set_size_action" value="{t}Set{/t}"/>
</p>
