<div
  id="e_layer2"
  class="msgbackground">

  {foreach from=$dialogInfos item=dialog}

    {if ($dialog.trace != "")}
    <div id="trace_{$dialog.id}"
      class="msgtrace"
      style="display:none;">
      {$dialog.trace}
    </div>
    {/if}

    {if $dialog.type == $smarty.const.INFO_DIALOG || $dialog.type == $smarty.const.CONFIRM_DIALOG}
    <div id="e_layer{$dialog.id}" class="info msgdialog">
    {elseif $dialog.type == $smarty.const.WARNING_DIALOG}
    <div id="e_layer{$dialog.id}" class="notice msgdialog">
    {else}
    <div id="e_layer{$dialog.id}" class="error msgdialog">
    {/if}

      <div id="e_layerTitle{$dialog.id}" class="msgtitle">
        <h2>
        {if $dialog.type == $smarty.const.ERROR_DIALOG}
          <img src="geticon.php?context=status&amp;icon=dialog-error&amp;size=32" class="center" alt="{t}Error{/t}"/>
        {elseif $dialog.type == $smarty.const.WARNING_DIALOG}
          <img src="geticon.php?context=status&amp;icon=dialog-warning&amp;size=32" class="center"  alt="{t}Warning{/t}"/>
        {elseif $dialog.type == $smarty.const.INFO_DIALOG || $dialog.type == $smarty.const.CONFIRM_DIALOG}
          <img src="geticon.php?context=status&amp;icon=dialog-information&amp;size=32" class="center" alt="{t}Information{/t}"/>
        {/if}
          {$dialog.title}
        </h2>
      </div>

      <div class="msgcontent">
        {$dialog.message}
      </div>
      <div class="plugbottom">
        {if ($dialog.trace != "")}
          <input type="button" id="Trace" name="Trace"
            onClick="$('trace_{$dialog.id}').toggle();" value="{t}Trace{/t}"/>
        {/if}
        <input type="button" id="MSG_OK{$dialog.id}" name="MSG_OK{$dialog.id}"
          onClick="next_msg_dialog();" value="{t}Ok{/t}"/>
        {if $dialog.type == $smarty.const.CONFIRM_DIALOG}
          <input type="button" name="MSG_CANCEL{$dialog.id}"
            onClick="next_msg_dialog();" value="{t}Cancel{/t}"/>
        {/if}
      </div>
    </div>
    <script type="text/javascript">
      focus_field('MSG_OK{$dialog.id}');
    </script>
  {/foreach}

</div>
<input type="hidden" name="pending_msg_dialogs"  id="pending_msg_dialogs" value="{','|implode:$dialogIds}"/>
<input type="hidden" name="closed_msg_dialogs"   id="closed_msg_dialogs" value=""/>
<input type="hidden" name="current_msg_dialogs"  id="current_msg_dialogs" value=""/>
<input type="hidden" name="js_debug"             id="js_debug"/>
