<div class="contentboxh">
 <p class="contentboxh">
  {t}Filter{/t}
 </p>
</div>

<div class="contentboxb">

{$TEMPLATES}&nbsp;<label for='TEMPLATES'>{t}Show templates{/t}</label><br/>
{$FUNCTIONAL}&nbsp;<label for='FUNCTIONAL'>{t}Show functional users{/t}</label><br/>
{$POSIX}&nbsp;<label for='POSIX'>{t}Show POSIX users{/t}</label><br/>
{if $USE_MAIL}
  {$MAIL}&nbsp;<label for='MAIL'>{t}Show Mail users{/t}</label><br/>
{/if}
{if $USE_SAMBA}
  {$SAMBA}&nbsp;<label for='SAMBA'>{t}Show Samba users{/t}</label><br/>
{/if}

 <div style="width:100%;border-top:1px solid #AAAAAA"></div>
 {$SCOPE}

 <table style="width:100%;border-top:1px solid #B0B0B0;">
  <tr>
   <td>
    <label for="NAME">
     <img src="geticon.php?context=actions&amp;icon=system-search&amp;size=16" alt='search'/>&nbsp;{t}Name{/t}
    </label>
   </td>
   <td>
    {$NAME}
   </td>
  </tr>
 </table>

 <table style="width:100%;background:#EEEEEE;border-top:1px solid #B0B0B0;">
  <tr>
   <td style="width:100%;text-align:right;">
    {$APPLY}
   </td>
  </tr>
 </table>
</div>
