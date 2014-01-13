<div class="contentboxh">
 <p class="contentboxh">
  <img src="images/launch.png" align="right" alt="[F]">{t}Filter{/t}
 </p>
</div>

<div class="contentboxb">

 {$PRIMARY}&nbsp;<LABEL for='PRIMARY'>{t}Show primary groups{/t}</LABEL><br>
 {$SAMBA}&nbsp;<LABEL for='SAMBA'>{t}Show samba groups{/t}</LABEL><br>
 {$MAIL}&nbsp;<LABEL for='MAIL'>{t}Show mail groups{/t}</LABEL><br>

 {$SCOPE}

 <table summary="" style="width:100%;border-top:1px solid #B0B0B0;">
  <tr>
   <td>
    <label for="NAME">
     <img src="geticon.php?context=actions&icon=system-search&size=16" align=middle>&nbsp;{t}Name{/t}
    </label>
   </td>
   <td>
    {$NAME}
   </td>
  </tr>
 </table>

 <table summary=""  width="100%"  style="background:#EEEEEE;border-top:1px solid #B0B0B0;">
  <tr>
   <td width="100%" align="right">
    {$APPLY}
   </td>
  </tr>
 </table>
</div>
