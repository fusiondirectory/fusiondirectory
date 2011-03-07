<script type="text/javascript" src="include/pwdStrength.js"></script>

<p>
 {t}To change the user password use the fields below. The changes take effect immediately. Please memorize the new password, because the user wouldn't be able to login without it.{/t}
</p>

<p>
 {t}Changing the password affects your authentification on mail, proxy, samba and unix services.{/t}
</p>

<hr>

{if !$proposalEnabled}

  <table summary="" style="vertical-align:top; text-align:left;" cellpadding=4 border=0>
    <tr>
      <td><b><LABEL for="new_password">{t}New password{/t}</LABEL></b></td>
      <td><input type="password" id="new_password" name="new_password" onFocus="nextfield= 'repeated_password';" onkeyup="testPasswordCss(document.getElementById('new_password').value);"></td>
    </tr>
    <tr>
      <td><b><LABEL for="repeated_password">{t}Repeat new password{/t}</LABEL></b></td>
      <td><input type="password" id="repeated_password" name="repeated_password" onFocus="nextfiled= 'password_finish'"></td>
    </tr>
    <tr>
      <td><b>{t}Strength{/t}</b></td>
      <td>
          <span id="meterEmpty" style="padding:0;margin:0;width:100%;background-color:#DC143C;display:block;height:5px;">
          <span id="meterFull" style="padding:0;margin:0;z-index:100;width:0;background-color:#006400;display:block;height:5px;"></span></span>
      </td>
    </tr>
  </table>

{else}

  <table summary="{t}Password input dialog{/t}" cellpadding=4 border=0>
    <tr>
      <td>
        <input type='radio' value='1' name='proposalSelected' id='proposalSelected' onClick='updateFields();'
            {if $proposalSelected} checked {/if}>&nbsp;<b>{t}Use proposal{/t}</b>
      </td>
      <td>
        <div id='proposalText' style='
                  width:180px;
                  border:1px solid #BBB;
                  padding-top:3px;
                  padding-bottom:3px;
            '>{$proposal}</div>
       </td>
       <td>
        &nbsp;<input type='submit' name='refreshProposal' value='{t}Refresh{/t}'>
      </td>
    </tr>
    <tr>
      <td>
        <input type='radio' value='0' name='proposalSelected' onClick='updateFields();'
            {if !$proposalSelected} checked {/if}>&nbsp;<b>{t}Manually specify a password{/t}</b>
      </td>
    </tr>
    <tr>
      <td style='padding-left:40px;'><b><LABEL for="new_password">{t}New password{/t}</LABEL></b></td>
      <td><input type="password" id="new_password" name="new_password" onFocus="nextfield= 'repeated_password';"     onkeyup="testPasswordCss(document.getElementById('new_password').value);"></td>
    </tr>
    <tr>
      <td style='padding-left:40px;'><b><LABEL for="repeated_password">{t}Repeat new password{/t}</LABEL></b></td>
      <td><input type="password" id="repeated_password" name="repeated_password" onFocus="nextfiled='password_finish'"></td>
    </tr>
    <tr>
      <td style='padding-left:40px;'><b>{t}Strength{/t}</b></td>
      <td>
        <span id="meterEmpty" style="padding:0;margin:0;width:100%;
          background-color:#DC143C;display:block;height:7px;">
        <span id="meterFull" style="padding:0;margin:0;z-index:100;width:0;
          background-color:#006400;display:block;height:7px;"></span></span>
      </td>
    </tr>
  </table>

{/if}

{if $passwordChangeForceable}
    <hr>
    <input type='checkbox' name='enforcePasswordChange' value='1' id='enforcePasswordChange'
        {if $enforcePasswordChange} checked {/if}>&nbsp;
            <LABEL for='enforcePasswordChange'>{t}Enforce password change on next login.{/t}</LABEL>
{/if}



<br>
<p class="plugbottom">
  <input type=submit name="password_finish" value="{t}Set password{/t}">
  &nbsp;
  <input type=submit name="password_cancel" value="{msgPool type=cancelButton}">
  <input type='hidden' id='formSubmit'>
</p>
<script language="JavaScript" type="text/javascript">
    nextfield= 'current_password';
    focus_field('current_password');


    function updateFields()
    {
        if($('proposalSelected').checked){
            $('new_password').disable();
            $('repeated_password').disable();
            $('proposalText').setStyle(
                 'background-color:#FFF;' +
                 'color:#000;' +
                 'width:180px;' +
                 'border:1px solid #CCC;' +
                 'padding:3px;' );
        }else{
            $('new_password').enable();
            $('repeated_password').enable();
            $('proposalText').setStyle(
                 'background-color:#DDD;' +
                 'color:#888;' +
                 'width:180px;' +
                 'border:1px solid #CCC;' +
                 'padding:3px;');
        }
    }
    updateFields();
</script>



