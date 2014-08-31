{if $RestoreMode}

<h2>{t}Restoring object snapshots{/t}</h2>
<hr/>
<br>
{t}This procedure will restore a snapshot of the selected object. It will replace the existing object after pressing the folder icon.{/t}
<br>
<br>
{t}Remember that DNS configuration and database entries could not be restored. For some objects it is only necessary to open and save them again (goFon), but some entries must be recreated manually.{/t}
<br>
<br>
{t}Don't forget to check references to other objects, for example does the selected printer still exists ?{/t}
<br>
<hr/>
<br>
<table style="width:100%">
  {if !$restore_deleted}
  <tr>
    <td>
    <b>{t}Object{/t}</b>&nbsp;
    {$CurrentDN}
    </td>
  </tr>
  {/if}
  <tr>
    <td>
      <br>
      {if $CountSnapShots==0}
        {t}There is no snapshot available that could be restored{/t}
      {else}
        {t}Choose a snapshot and click the folder image, to restore the snapshot{/t}
      {/if}
    </td>
  </tr>
  <tr>
    <td>
      {$SnapShotDivlist}
    </td>
  </tr>
</table>

<hr/>
<div style="width:100%; text-align:right;">
    <input type="submit" name="CancelSnapshot" value="{t}Cancel{/t}">
</div>

{else}

<h2>{t}Creating object snapshots{/t}</h2>
<hr/>
<br>
{t}This procedure will create a snapshot of the selected object. It will be stored inside a special branch of your directory system and can be restored later on.{/t}
<br>
<br>
{t}Remember that database entries, DNS configurations and possibly created zones in server extensions will not be stored in the snapshot.{/t}
<br>
<hr/>
<br>
<table style="width:100%">
  <tr>
    <td>
      <b>{t}Object{/t}</b>
    </td>
    <td style="width:95%">
       {$CurrentDN}
    </td>
  </tr>
  <tr>
    <td>
      <b>{t}Timestamp{/t}</b>
    </td>
    <td>
       {$CurrentDate}
    </td>
  </tr>
  <tr>
    <td style="vertical-align:top;" colspan="2">
      <br>
      {t}Reason for generating this snapshot{/t}<br>
      <textarea name="CurrentDescription" style="width:100%;height:160px;" rows=10 cols=100>{$CurrentDescription}</textarea>
    </td>
  </tr>
</table>

<hr/>
<div style="width:100%; text-align:right;">
    <input type="submit" name="CreateSnapshot" value="{t}Continue{/t}">
    &nbsp;
    <input type="submit" name="CancelSnapshot" value="{t}Cancel{/t}">
</div>

<script type="text/javascript">
  <!-- // First input field on page
  document.mainform.CurrentDescription.focus();
  -->
</script>
{/if}
