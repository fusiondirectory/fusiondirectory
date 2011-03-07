{if $objectList ne ""}
<table summary="" style="width:100%; vertical-align:top; text-align:left; border:1px solid #B0B0B0;"  cellpadding=4 cellspacing=0 border=0 rules="cols">

 <colgroup>
  <col width="20%">
  <col width="20%">
  <col width="60%">
 </colgroup>

 <tr style="background-color: #E8E8E8; height:26px;font-weight:bold;">
  <td>{t}Object name{/t}</td><td>{t}Description{/t}</td><td>{t}Contents{/t}</td>
 </tr>

 {$objectList}

</table>
{else}
<b>{t}This object has no relationship to other objects.{/t}</b>
{/if}
