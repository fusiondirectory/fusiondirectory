<h1>{$warning}</h1>
<p>
{t}The size limit option makes LDAP operations faster and saves the LDAP server from getting too much load. The easiest way to handle big databases without long timeouts would be to limit your search to smaller values and use filters to get the entries you are looking for.{/t}
</p>
<p>
<b>{t}Please choose the way to react for this session{/t}:</b>
</p>

<input type="radio" name="action" value="ignore">{t}ignore this error and show all entries the LDAP server returns{/t}<br>
<input type="radio" name="action" value="limited" checked>{t}ignore this error and show all entries that fit into the defined sizelimit and let me use filters instead{/t}<br>
<input type="radio" name="action" value="newlimit">{$limit_message}

<p class="plugbottom">
 <input type=submit name="set_size_action" value="{t}Set{/t}">
</p>

<input type="hidden" name="ignore">
