<div class="contentboxh">
 <p>{t}Filter{/t}</p>
</div>

<div class="contentboxb">
  <label for="PRIMARY">{$PRIMARY}&nbsp;{t}Show primary groups{/t}</label><br/>
  <label for="SAMBA">{$SAMBA}&nbsp;{t}Show samba groups{/t}</label><br/>
  <label for="MAIL">{$MAIL}&nbsp;{t}Show mail groups{/t}</label><br/>

  <hr/>
  {$SCOPE}
  <hr/>

  <label for="NAME"><img src="geticon.php?context=actions&amp;icon=system-search&amp;size=16"/>{$NAME}</label>

  <div>
    {$APPLY}
  </div>
</div>
