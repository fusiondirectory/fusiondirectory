<div class="contentboxh">
 <p class="contentboxh">
  <img src="images/launch.png" alt="[F]">{t}Filter{/t}
 </p>
</div>

<div class="contentboxb">

  {$SERVER}&nbsp;{t}Show servers{/t}<br>
  {$WORKSTATION}&nbsp;{t}Show workstations{/t}<br>
  {$WINSTATION}&nbsp;{t}Show Windows PC{/t}<br>
  {$TERMINAL}&nbsp;{t}Show terminals{/t}<br>

  <div style="border-top:1px solid #AAAAAA"></div>

  {$SCOPE}

 <table style="width:100%;border-top:1px solid #B0B0B0;">
  <tr>
   <td>
    <label for="NAME">
     <img src="images/lists/search.png"/>&nbsp;{t}Name{/t}
    </label>
   </td>
   <td>
    {$NAME}
   </td>
  </tr>
 </table>

 <table  width="100%"  style="background:#EEEEEE;border-top:1px solid #B0B0B0;">
  <tr>
   <td style="width:100%;text-align:right;">
    {$APPLY}
   </td>
  </tr>
 </table>
</div>
