<div class="contentboxh">
 <p>{t}Filter{/t}</p>
</div>

<div class="contentboxb">
  {$PRIMARY}&nbsp;<label for="PRIMARY">{t}Show primary groups{/t}</label><br/>
  {$SAMBA}&nbsp;<label for="SAMBA">{t}Show samba groups{/t}</label><br/>
  {$MAIL}&nbsp;<label for="MAIL">{t}Show mail groups{/t}</label><br/>

  <hr/>
  {$SCOPE}
  <hr/>

  <label for="NAME"><img src="geticon.php?context=actions&amp;icon=system-search&amp;size=16"/></label>{$NAME}

  <div>
    {$APPLY}
  </div>
</div>
