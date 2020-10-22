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

  <div class="acledition">

    <div class="togglebuttons">
      <input type="button" name="toggle_all_create" onClick="acl_toggle_all('_0_c$');" value="Toggle C"/>
      <input type="button" name="toggle_all_move"   onClick="acl_toggle_all('_0_m$');" value="Toggle M"/>
      <input type="button" name="toggle_all_remove" onClick="acl_toggle_all('_0_d$');" value="Toggle D"/>
       -
      <input type="button" name="toggle_all_read"   onClick="acl_toggle_all('_0_r$');" value="Toggle R"/>
      <input type="button" name="toggle_all_write"  onClick="acl_toggle_all('_0_w$');" value="Toggle W"/>
       -
      <input type="button" name="toggle_all_sub_read"   onClick="acl_toggle_all('[^0]_r$');" value="R+"/>
      <input type="button" name="toggle_all_sub_write"  onClick="acl_toggle_all('[^0]_w$');" value="W+"/>
    </div>
    <div class="setbuttons">
      <input type="button" name="set_true_all_create"  onClick="acl_set_all('_0_c$',true);"  value="C+"/><input type="button" name="set_false_all_create" onClick="acl_set_all('_0_c$',false);" value="C-"/>
      <input type="button" name="set_true_all_move"    onClick="acl_set_all('_0_m$',true);"  value="M+"/><input type="button" name="set_false_all_move"   onClick="acl_set_all('_0_m$',false);" value="M-"/>
      <input type="button" name="set_true_all_remove"  onClick="acl_set_all('_0_d$',true);"  value="D+"/><input type="button" name="set_false_all_remove" onClick="acl_set_all('_0_d$',false);" value="D-"/>
       -
      <input type="button" name="set_true_all_read"    onClick="acl_set_all('_0_r$',true);"  value="R+"/><input type="button" name="set_false_all_read"   onClick="acl_set_all('_0_r$',false);" value="R-"/>
      <input type="button" name="set_true_all_write"   onClick="acl_set_all('_0_w$',true);"  value="W+"/><input type="button" name="set_false_all_write"  onClick="acl_set_all('_0_w$',false);" value="W-"/>
       -
      <input type="button" name="set_true_all_read"   onClick="acl_set_all('[^0]_r$',true);"  value="R+"/><input type="button" name="set_false_all_read"  onClick="acl_set_all('[^0]_r$',false);" value="R-"/>
      <input type="button" name="set_true_all_write"  onClick="acl_set_all('[^0]_w$',true);"  value="W+"/><input type="button" name="set_false_all_write" onClick="acl_set_all('[^0]_w$',false);" value="W-"/>
    </div>

    {foreach from=$aclObjects key=$key item=$infos}
      <table {if $infos.expand}class="expand"{/if}>
        <tbody>
          <tr>
            <td colspan="2">
             <b>{t}Object{/t}: {$infos.name}</b>
            </td>
            <td align="right">
              {if !empty($infos.attrs)}
                <input id="show{$infos.tname}" type="button" onclick="$('{$infos.tname}').toggle();" value="{t}Show/hide advanced settings{/t}"/>
              {/if}
            </td>
          </tr>
          <tr>
            <td colspan="2">
              {if $infos.rights.create}
                {makeCheckbox id={$infos.tname|cat:'_0_c'} label=_("Create objects")  checked=preg_match('/c/', $infos.globalAcl)}&nbsp;&nbsp;
              {/if}
              {if $infos.rights.move}
                {makeCheckbox id={$infos.tname|cat:'_0_m'} label=_("Move objects")    checked=preg_match('/m/', $infos.globalAcl)}&nbsp;&nbsp;
              {/if}
              {if $infos.rights.remove}
                {makeCheckbox id={$infos.tname|cat:'_0_d'} label=_("Remove objects")  checked=preg_match('/d/', $infos.globalAcl)}&nbsp;&nbsp;
              {/if}
              {if $infos.rights.self}
                {makeCheckbox id={$infos.tname|cat:'_0_s'} label=_("Grant permission to owner")  checked=preg_match('/s/', $infos.globalAcl)}&nbsp;&nbsp;
              {/if}
            </td>
            <td>
              {if ($infos.rights.read||$infos.rights.write)}
                &nbsp;{t}Complete object{/t}:
                {if $infos.rights.read}
                  {makeCheckbox id={$infos.tname|cat:'_0_r'} label=_("read")  checked=preg_match('/r/', $infos.globalAcl)}&nbsp;&nbsp;
                {/if}
                {if $infos.rights.write}
                  {makeCheckbox id={$infos.tname|cat:'_0_w'} label=_("write") checked=preg_match('/w/', $infos.globalAcl)}
                {/if}
              {/if}
            </td>
          </tr>
          <tr id="tr_{$infos.tname}">
            <td colspan="3">
              <div id="{$infos.tname}" style="display:none;">
                <table>
                  <tbody>
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
                      <td>
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
                      <td>&nbsp;</td>
                      {$cnt = $cnt+1}
                    {/while}
                  </tbody>
                </table>
              </div>
            </td>
          </tr>
        </tbody>
      </table><br/>
    {/foreach}

  </div>

  <p class="plugbottom">
    <input type="submit" name="submit_edit_acl" value="{msgPool type=applyButton}"/>
    <input type="submit" formnovalidate="formnovalidate" name="cancel_edit_acl" value="{msgPool type=cancelButton}"/>
  </>
{/if}
