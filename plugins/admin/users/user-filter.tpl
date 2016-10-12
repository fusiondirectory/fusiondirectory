<div class="contentboxh">
 <p>{t}Filter{/t}</p>
</div>

<div class="contentboxb">
  {$TEMPLATES}&nbsp;<label for='TEMPLATES'>{t}Show templates{/t}</label><br/>
  {$FUNCTIONAL}&nbsp;<label for='FUNCTIONAL'>{t}Show functional users{/t}</label><br/>
  {if $USE_POSIX}
    {$POSIX}&nbsp;<label for='POSIX'>{t}Show POSIX users{/t}</label><br/>
  {/if}
  {if $USE_MAIL}
    {$MAIL}&nbsp;<label for='MAIL'>{t}Show Mail users{/t}</label><br/>
  {/if}
  {if $USE_SAMBA}
    {$SAMBA}&nbsp;<label for='SAMBA'>{t}Show Samba users{/t}</label><br/>
  {/if}

  <hr/>
  {$SCOPE}
  <hr/>

  <label for="NAME"><img src="geticon.php?context=actions&amp;icon=system-search&amp;size=16" alt="Search"/></label>{$NAME}

  <div>
    {$APPLY}
  </div>
</div>
