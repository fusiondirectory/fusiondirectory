{if $frame}
  <div
    id="e_layer2"
    style="
      position:absolute;
      left:0px;
      top:0px;
      right:0px;
      bottom:0px;
      z-index:100;
      overflow:hidden;
      background-image:url(images/opacity_black.png);">
{else}

  {if $s_Trace != "" && $i_TraceCnt != 0}
  <div id="trace_{$i_ID}"
    style="
      display:none;
      position:absolute;
      left:0px;
      top:0px;
      right:0px;">
    {$s_Trace}
  </div>
  {/if}

  {if $i_Type == $smarty.const.INFO_DIALOG || $i_Type == $smarty.const.CONFIRM_DIALOG}
  <div id="e_layer{$i_ID}" class="info msgdialog">
  {elseif $i_Type == $smarty.const.WARNING_DIALOG}
  <div id="e_layer{$i_ID}" class="notice msgdialog">
  {else}
  <div id="e_layer{$i_ID}" class="error msgdialog">
  {/if}

    <div id="e_layerTitle{$i_ID}" class="msgtitle">
      <h2>
      {if $i_Type == $smarty.const.ERROR_DIALOG}
        <img src="geticon.php?context=status&amp;icon=dialog-error&amp;size=32" class="center" alt="{t}Error{/t}"/>
      {elseif $i_Type == $smarty.const.WARNING_DIALOG}
        <img src="geticon.php?context=status&amp;icon=dialog-warning&amp;size=32" class="center"  alt="{t}Warning{/t}"/>
      {elseif $i_Type == $smarty.const.INFO_DIALOG || $i_Type == $smarty.const.CONFIRM_DIALOG}
        <img src="geticon.php?context=status&amp;icon=dialog-information&amp;size=32" class="center" alt="{t}Information{/t}"/>
      {/if}
        {$s_Title}
      </h2>
    </div>

    <div style="z-index:250;width:100%;">
      {$s_Message}
    </div>
    <div class="plugbottom">
      {if $s_Trace != "" && $i_TraceCnt != 0}
        <input type="button" id="Trace" name="Trace"
          onClick="$('trace_{$i_ID}').toggle();" value="{t}Trace{/t}"/>
      {/if}
      <input type="button" id="MSG_OK{$i_ID}" name="MSG_OK{$i_ID}"
        onClick="next_msg_dialog();" value="{t}Ok{/t}"/>
      {if $i_Type == $smarty.const.CONFIRM_DIALOG}
        <input type="button" name="MSG_CANCEL{$i_ID}"
          onClick="next_msg_dialog();" value="{t}Cancel{/t}"/>
      {/if}
    </div>
  </div>
  <script type="text/javascript">
    focus_field('MSG_OK{$i_ID}');
  </script>

{/if}
