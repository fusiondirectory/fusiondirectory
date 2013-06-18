<script type="text/javascript" src="include/pwdStrength.js"></script>

<p>
  {t}To change your personal password use the fields below. The changes take effect immediately. Please memorize the new password, because you wouldn't be able to login without it.{/t}
</p>

<p>
  {t}Changing the password affects your authentification on mail, proxy, samba and unix services.{/t}
</p>

<hr>

{if !$proposalEnabled}

  <table style="vertical-align:top; text-align:left;" cellpadding=4 border=0>
    <tr>
      <td><b><LABEL for="current_password">{t}Current password{/t}</LABEL></b></td>
      <td><input id="current_password" type="password" name="current_password"
          onFocus="nextfield= 'new_password';"></td>
    </tr>
    <tr>
      <td><b><LABEL for="new_password">{t}New password{/t}</LABEL></b></td>
      <td><input id="new_password" type="password" name="new_password"
          onFocus="nextfield= 'repeated_password';" onkeyup="testPasswordCss(document.getElementById('new_password').value);"></td>
    </tr>
    <tr>
      <td><b><LABEL for="repeated_password">{t}Repeat new password{/t}</LABEL></b></td>
      <td><input id="repeated_password" type="password" name="repeated_password"
          onFocus="nextfield= 'password_finish';"></td>
    </tr>
    <tr>
      <td><b>{t}Password strength{/t}</b></td>
      <td>
      <span id="meterEmpty" style="padding:0;margin:0;width:100%;background-color:#DC143C;display:block;height:5px;">
      <span id="meterFull" style="padding:0;margin:0;z-index:100;width:0;background-color:#006400;display:block;height:5px;"></span></span>
      </td>
    </tr>
  </table>

{else}

  <table cellpadding="4" border="0">
    <tr>
      <td><b><LABEL for="current_password">{t}Current password{/t}</LABEL></b></td>
      <td><input id="current_password" type="password" name="current_password"
          onFocus="nextfield= 'new_password';"></td>
    </tr>
    <tr>
      <td>
        <input type='radio' value='1' name='proposalSelected'
            {if $proposalSelected} checked {/if}>&nbsp;<b>{t}Use proposal{/t}</b>
      </td>
      <td>
        <div style='
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
        <input type='radio' value='0' name='proposalSelected'
            {if !$proposalSelected} checked {/if}>&nbsp;<b>{t}Manually specify a password{/t}</b>
      </td>
    </tr>
    <tr>
      <td  style='padding-left:40px;'><b><LABEL for="new_password">{t}New password{/t}</LABEL></b></td>
      <td><input id="new_password" type="password" name="new_password"
          onFocus="nextfield= 'repeated_password';" onkeyup="testPasswordCss(document.getElementById('new_password').value);"></td>
    </tr>
    <tr>
      <td  style='padding-left:40px;'><b><LABEL for="repeated_password">{t}Repeat new password{/t}</LABEL></b></td>
      <td><input id="repeated_password" type="password" name="repeated_password"
          onFocus="nextfield= 'password_finish';"></td>
    </tr>
    <tr>
      <td  style='padding-left:40px;'><b>{t}Password strength{/t}</b></td>
      <td>
      <span id="meterEmpty" style="padding:0;margin:0;width:100%;background-color:#DC143C;display:block;height:7px;">
      <span id="meterFull" style="padding:0;margin:0;z-index:100;width:0;background-color:#006400;display:block;height:7px;"></span></span>
      </td>
    </tr>
  </table>
{/if}


<br>

<p class="plugbottom">
  <input type=submit name="password_finish" value="{t}Set password{/t}">
  &nbsp;
  <input type=reset id="password_cancel" name="password_cancel" value="{t}Clear fields{/t}">
  <input type='hidden' id='formSubmit'>
</p>

<input type="hidden" name="ignore">

<!-- Place cursor -->
<script type="text/javascript">
  <!-- // First input field on page
  nextfield= 'current_password';
  focus_field('current_password');
  -->
</script>

