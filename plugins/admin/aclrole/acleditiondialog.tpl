  <h1>{$headline|escape}</h1>

{if $dialogState eq 'create'}
  {$aclEdition}
  <p class="plugbottom">
    <input type="submit" name="add_finish" value="{msgPool type=applyButton}"/>
    <input type="submit" formnovalidate="formnovalidate" name="add_cancel" value="{msgPool type=cancelButton}"/>
  </p>
{elseif $dialogState eq 'edit'}
  {function makeCheckbox id='' label='' checked=false}
    <input id="acl_{$id}" type="checkbox" name="acl_{$id}"{if $checked} checked="checked"{/if}/>
    <label for="acl_{$id}">{$label}</label>
  {/function}

  <input type="hidden" name="acl_dummy_0_0_0" value="1"/>

  <input style="width:100px;" type="button" name="toggle_all_create" onClick="acl_toggle_all('_0_c$');" value="Toggle C"/><input style="width:100px;" type="button" name="toggle_all_move"   onClick="acl_toggle_all('_0_m$');" value="Toggle M"/><input style="width:100px;" type="button" name="toggle_all_remove" onClick="acl_toggle_all('_0_d$');" value="Toggle D"/>
   -
  <input style="width:100px;" type="button" name="toggle_all_read"   onClick="acl_toggle_all('_0_r$');" value="Toggle R"/><input style="width:100px;" type="button" name="toggle_all_write"  onClick="acl_toggle_all('_0_w$');" value="Toggle W"/>
   -
  <input style="width:100px;" type="button" name="toggle_all_sub_read"   onClick="acl_toggle_all('[^0]_r$');" value="R+"/><input style="width:100px;" type="button" name="toggle_all_sub_write"  onClick="acl_toggle_all('[^0]_w$');" value="W+"/>

  <br/>

  <input style="width:50px;" type="button" name="set_true_all_create"  onClick="acl_set_all('_0_c$',true);"  value="C+"/><input style="width:50px;" type="button" name="set_false_all_create" onClick="acl_set_all('_0_c$',false);" value="C-"/><input style="width:50px;" type="button" name="set_true_all_move"    onClick="acl_set_all('_0_m$',true);"  value="M+"/><input style="width:50px;" type="button" name="set_false_all_move"   onClick="acl_set_all('_0_m$',false);" value="M-"/><input style="width:50px;" type="button" name="set_true_all_remove"  onClick="acl_set_all('_0_d$',true);"  value="D+"/><input style="width:50px;" type="button" name="set_false_all_remove" onClick="acl_set_all('_0_d$',false);" value="D-"/>
   -
  <input style="width:50px;" type="button" name="set_true_all_read"    onClick="acl_set_all('_0_r$',true);"  value="R+"/><input style="width:50px;" type="button" name="set_false_all_read"   onClick="acl_set_all('_0_r$',false);" value="R-"/><input style="width:50px;" type="button" name="set_true_all_write"   onClick="acl_set_all('_0_w$',true);"  value="W+"/><input style="width:50px;" type="button" name="set_false_all_write"  onClick="acl_set_all('_0_w$',false);" value="W-"/>
   -
  <input style="width:50px;" type="button" name="set_true_all_read"   onClick="acl_set_all('[^0]_r$',true);"  value="R+"/><input style="width:50px;" type="button" name="set_false_all_read"  onClick="acl_set_all('[^0]_r$',false);" value="R-"/><input style="width:50px;" type="button" name="set_true_all_write"  onClick="acl_set_all('[^0]_w$',true);"  value="W+"/><input style="width:50px;" type="button" name="set_false_all_write" onClick="acl_set_all('[^0]_w$',false);" value="W-"/>

  {foreach from=$aclObjects key=$key item=$infos}
    {if $infos.expand}
      {$back_color = '#C8C8FF'}
    {else}
      {$back_color = '#C8C8C8'}
    {/if}
    <table style="width:100%;border:1px solid #A0A0A0;border-spacing:0;border-collapse:collapse;">
      <tr>
        <td style="background-color:{$back_color};height:1.8em;" colspan="2">
         <b>{t}Object{/t}: {$infos.name}</b>
        </td>
        <td align="right" style="background-color:{$back_color};height:1.8em;">
        <input id="show{$infos.tname}" type="button" onclick="$('{$infos.tname}').toggle();" value="{t}Show/hide advanced settings{/t}"/></td>
      </tr>
      <tr>
        <td style="background-color:#E0E0E0" colspan="2">
          {makeCheckbox id={$infos.tname|cat:'_0_c'} label=_("Create objects")  checked=preg_match('/c/', $infos.globalAcl)}&nbsp;&nbsp;
          {makeCheckbox id={$infos.tname|cat:'_0_m'} label=_("Move objects")    checked=preg_match('/m/', $infos.globalAcl)}&nbsp;&nbsp;
          {makeCheckbox id={$infos.tname|cat:'_0_d'} label=_("Remove objects")  checked=preg_match('/d/', $infos.globalAcl)}&nbsp;&nbsp;
          {if $infos.self}
            {makeCheckbox id={$infos.tname|cat:'_0_s'} label=_("Grant permission to owner")  checked=preg_match('/s/', $infos.globalAcl)}&nbsp;&nbsp;
          {/if}
        </td>
        <td style="background-color:#D4D4D4">
          &nbsp;{t}Complete object{/t}:
          {makeCheckbox id={$infos.tname|cat:'_0_r'} label=_("read")  checked=preg_match('/r/', $infos.globalAcl)}&nbsp;&nbsp;
          {makeCheckbox id={$infos.tname|cat:'_0_w'} label=_("write") checked=preg_match('/w/', $infos.globalAcl)}
        </td>
      </tr>
      <tr id="tr_{$infos.tname}" style="vertical-align:top;height:0px;">
        <td colspan="3">
          <div id="{$infos.tname}" style="overflow:hidden; display:none;vertical-align:top;width:100%;">
            <table style="width:100%;border-collapse: collapse;" border="1">
              {* Walk through the list of attributes *}
              {$cnt    = 1}
              {foreach from=$infos.attrs key=$attr item=$dsc}
                {* Skip pl* attributes, they are internal... *}
                {if preg_match('/^pl[A-Z]+.*$/', $attr)}
                  continue;
                {/if}

                {* Open table row *}
                {if ($cnt == 1)}
                  <tr>
                {/if}

                {* Collect list of attributes *}
                {$state = ''}
                {if isset($infos.acl[$attr])}
                  {$state = $infos.acl[$attr]}
                {/if}
                <td style="border:1px solid #A0A0A0;width:33%">
                  <b>{$dsc}</b> ({$attr})<br/>
                  {$rchecked = preg_match('/r/', $state)}
                  {$wchecked = preg_match('/w/', $state)}

                  <input id="acl_{$infos.tname}_{$attr}_r" type="checkbox" name="acl_{$key}_{$attr}_r"{if $rchecked} checked="checked"{/if}/>
                  <label for="acl_{$infos.tname}_{$attr}_r">{t}read{/t}</label>
                  <input id="acl_{$infos.tname}_{$attr}_w" type="checkbox" name="acl_{$key}_{$attr}_w"{if $wchecked} checked="checked"{/if}/>
                  <label for="acl_{$infos.tname}_{$attr}_w">{t}write{/t}</label>
                </td>

                {* Close table row *}
                {if ($cnt == 3)}
                  {$cnt = 1}
                  </tr>
                {else}
                  {$cnt = $cnt+1}
                {/if}
              {/foreach}
              {* Fill missing td's if needed *}
              {while (($cnt == 3) || ($cnt == 2))}
                <td style="border:1px solid #A0A0A0;width:33%">&nbsp;</td>
                {$cnt = $cnt+1}
              {/while}
            </table>
          </div>
        </td>
      </tr>
    </table><br/>
  {/foreach}

  <p class="plugbottom">
    <input type="submit" name="submit_edit_acl" value="{msgPool type=applyButton}"/>
    <input type="submit" formnovalidate="formnovalidate" name="cancel_edit_acl" value="{msgPool type=cancelButton}"/>
  </>
{/if}
