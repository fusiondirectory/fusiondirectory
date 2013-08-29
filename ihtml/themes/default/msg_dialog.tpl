{if $frame}
  <div  id='e_layer2'
    style="
      position: absolute;
      left: 0px;
      top: 0px;
      right:0px;
      bottom:0px;
      z-index:100;
      background-image: url(images/opacity_black.png);">
{else}


  {if !$JS}

    {if $i_Type == $smarty.const.INFO_DIALOG || $i_Type == $smarty.const.CONFIRM_DIALOG || $i_Type == $smarty.const.OK_CANCEL_DIALOG}
    <div id='e_layer{$i_ID}' class="infoMsgDialog" style='top:200px;left:200px;'>
    {else}
    <div id='e_layer{$i_ID}' class="errorMsgDialog" style='top:200px;left:200px;'>
    {/if}

      <div id="e_layerTitle{$i_ID}" class="errorMsgTitle">
      <table><tr><td>
    {if $i_Type == $smarty.const.ERROR_DIALOG}
            <img src="images/error.png" alt="{t}Error{/t}">
    {elseif $i_Type == $smarty.const.WARNING_DIALOG}
            <img src="images/warning.png"  alt="{t}Warning{/t}">
    {elseif $i_Type == $smarty.const.INFO_DIALOG || $i_Type == $smarty.const.CONFIRM_DIALOG || $i_Type == $smarty.const.OK_CANCEL_DIALOG}
            <img src="images/info.png" alt="{t}Information{/t}">
    {/if}
      </td><td style="font-size: 1.1em; font-weight:bold;">{$s_Title}</td></tr>
      </table>
      </div>
      <table style="width:100%; border-top: solid 1px #BBBBBB;" cellspacing=0 cellpadding=2 border=0>
        <tr>
          <td style="width:100%;padding:7px; padding-bottom:14px">
            {$s_Message}
            <br>
          </td>
        </tr>
        <tr>
          <td colspan="2" align="center">
    {if $i_Type == $smarty.const.ERROR_DIALOG || $i_Type == $smarty.const.WARNING_DIALOG || $i_Type == $smarty.const.INFO_DIALOG}
            <input type="button" type="submit" name="MSG_OK{$i_ID}" value="{t}Ok{/t}">
    {elseif $i_Type == $smarty.const.CONFIRM_DIALOG || $i_Type == $smarty.const.OK_CANCEL_DIALOG}
            <input type="button" type="submit" name="MSG_OK{$i_ID}" value="{t}Ok{/t}">
            <input type="button" type="submit" name="MSG_CANCEL{$i_ID}" value="{t}Cancel{/t}">
    {/if}
          </td>
        </tr>
      </table>
    </div>

  {else}

    {if $s_Trace != "" && $i_TraceCnt != 0}
    <div id="trace_{$i_ID}" style="display:none;">
      {$s_Trace}
    </div>

    {/if}

    {if $i_Type == $smarty.const.INFO_DIALOG || $i_Type == $smarty.const.CONFIRM_DIALOG || $i_Type == $smarty.const.OK_CANCEL_DIALOG}
    <div id="e_layer{$i_ID}" class="infoMsgDialog" style="top:200px;left:200px;">
    {else}
    <div id="e_layer{$i_ID}" class="errorMsgDialog" style="top:200px;left:200px;">
    {/if}

    <div id="e_layerTitle{$i_ID}" class="errorMsgTitle">
      <table><tr><td>
    {if $i_Type == $smarty.const.ERROR_DIALOG}
            <img src="images/error.png" alt="{t}Error{/t}">
    {elseif $i_Type == $smarty.const.WARNING_DIALOG}
            <img src="images/warning.png"  alt="{t}Warning{/t}">
    {elseif $i_Type == $smarty.const.INFO_DIALOG || $i_Type == $smarty.const.CONFIRM_DIALOG || $i_Type == $smarty.const.OK_CANCEL_DIALOG}
            <img src="images/info.png" alt="{t}Information{/t}">
    {/if}
      </td><td style="font-size: 1.1em; font-weight:bold;">{$s_Title}</td></tr>
    </table>
    </div>

      <table style="z-index:250;width:100%; border-top: solid 1px #BBBBBB;" cellspacing=0 cellpadding=2 border=0>
        <tr>
          <td style="width:100%; padding:7px; padding-bottom:14px;">
            {$s_Message}
            <br>
          </td>
          {if $s_Trace != "" && $i_TraceCnt != 0}
          <td style="width:20px; vertical-align:top; cursor:pointer;">
            <div onClick="$('trace_{$i_ID}').toggle();"><u>Trace</u></div>
          </td>
          {/if}
        </tr>
        <tr>
          {if $s_Trace != "" && $i_TraceCnt != 0}
          <td colspan="3" align="center" style="border-top: solid 1px #BBBBBB;">
          {else}
          <td colspan="2" align="center" style="border-top: solid 1px #BBBBBB;">
          {/if}
    {if $i_Type == $smarty.const.ERROR_DIALOG || $i_Type == $smarty.const.WARNING_DIALOG || $i_Type == $smarty.const.INFO_DIALOG}
            <input type="button" id="MSG_OK{$i_ID}" type="button" name="MSG_OK{$i_ID}"
              onClick="next_msg_dialog();" value="{t}Ok{/t}">
    {elseif $i_Type == $smarty.const.CONFIRM_DIALOG}
            <input type="button" id="MSG_OK{$i_ID}" type="submit" name="MSG_OK{$i_ID}"
              onClick="next_msg_dialog();" value="{t}Ok{/t}">
            <input type="button" type="button" name="MSG_CANCEL{$i_ID}"
              onClick="next_msg_dialog();" value="{t}Cancel{/t}">
    {elseif $i_Type == $smarty.const.OK_CANCEL_DIALOG}
            <input type="button" id="MSG_OK{$i_ID}" type="submit" name="MSG_OK{$i_ID}"
              onClick="next_msg_dialog();" value="{t}Ok{/t}">
            <input type="button" type="submit" name="MSG_CANCEL{$i_ID}"
              onClick="next_msg_dialog();" value="{t}Cancel{/t}">
    {/if}
          </td>
        </tr>
      </table>
      </div>
    <script type="text/javascript">
      focus_field('MSG_OK{$i_ID}');
    </script>

  {/if}
{/if}
