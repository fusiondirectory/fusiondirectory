<div class="contentboxh">
 <p class="contentboxh">
  <img src="images/launch.png" alt="[F]"/>{t}Filter{/t}
 </p>
</div>

<div class="contentboxb">

{$ACL}&nbsp;<LABEL for='ACL'>{t}Show access control lists{/t}</LABEL><br>
{$ROLE}&nbsp;<LABEL for='ROLE'>{t}Show roles{/t}</LABEL><br>

 <div style="border-top:1px solid #AAAAAA"></div>
 {$SCOPE}

 <table summary="" style="width:100%;border-top:1px solid #B0B0B0;">
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

 <table style="width:100%;background:#EEEEEE;border-top:1px solid #B0B0B0;">
  <tr>
   <td style="width:100%;text-align:right;">
    {$APPLY}
   </td>
  </tr>
 </table>
</div>
