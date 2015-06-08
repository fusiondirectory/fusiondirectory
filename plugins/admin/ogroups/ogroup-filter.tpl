<div class="contentboxh">
 <p class="contentboxh">
  {t}Filter{/t}
 </p>
</div>

<div class="contentboxb">

 {$USER}&nbsp;<LABEL for='USER'>{t}user groups{/t}</LABEL><br>
 {$GROUP}&nbsp;<LABEL for='GROUP'>{t}nested groups{/t}</LABEL><br>
 {$APPLICATION}&nbsp;<LABEL for='APPLICATION'>{t}application groups{/t}</LABEL><br>
 {$DEPARTMENT}&nbsp;<LABEL for='DEPARTMENT'>{t}department groups{/t}</LABEL><br>
 {$SERVER}&nbsp;<LABEL for='SERVER'>{t}server groups{/t}</LABEL><br>
 {$WORKSTATION}&nbsp;<LABEL for='WORKSTATION'>{t}workstation groups{/t}</LABEL><br>
 {$WINDOWS}&nbsp;<LABEL for='WINDOWS'>{t}windows workstation groups{/t}</LABEL><br>
 {$TERMINAL}&nbsp;<LABEL for='TERMINAL'>{t}terminal groups{/t}</LABEL><br>
 {$PRINTER}&nbsp;<LABEL for='PRINTER'>{t}printer groups{/t}</LABEL><br>
 {$PHONE}&nbsp;<LABEL for='PHONE'>{t}phone groups{/t}</LABEL><br>


 {$SCOPE}

 <table style="width:100%;border-top:1px solid #B0B0B0;">
  <tr>
   <td>
    <label for="NAME">
     <img src="geticon.php?context=actions&icon=system-search&size=16" alt="search"/>&nbsp;{t}Name{/t}
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
