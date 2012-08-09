{render acl=$recoveryConfigACL}
<table style="background-color:#F8F8F8;width:100%;border:1px solid #AAA">
  <tr><td colspan="4"><h2>Password recovery settings:</h2></td></tr>
  <tr>
    <td><label for="passwordRecoveryActivated">Activate password recovery</label></td>
    <td><input id="passwordRecoveryActivated" name="passwordRecoveryActivated" {if $passwordRecoveryActivated}checked="checked" {/if}type="checkbox"/></td>
  </tr>
  <tr>
    <td><label for="passwordRecoveryEmail">Sender email address</label></td>
    <td><input id="passwordRecoveryEmail" name="passwordRecoveryEmail" value="{$passwordRecoveryEmail}" type="text"/></td>
  </tr>
  <tr>
    <td><label for="passwordRecoveryValidity">Link validity (minutes)</label></td>
    <td><input id="passwordRecoveryValidity" name="passwordRecoveryValidity" value="{$passwordRecoveryValidity}" type="number"/></td>
  </tr>
  <tr>
    <td><label for="passwordRecoverySalt">Salt for tokens</label></td>
    <td><input id="passwordRecoverySalt" name="passwordRecoverySalt" value="{$passwordRecoverySalt}" type="text"/></td>
  </tr>
  <tr>
    <td><label for="passwordRecoveryUseAlternate">Allow the use of alternate addresses for recovery</label></td>
    <td><input id="passwordRecoveryUseAlternate" name="passwordRecoveryUseAlternate" {if $passwordRecoveryUseAlternate}checked="checked" {/if}type="checkbox"/></td>
  </tr>
  <tr><td colspan="4"><p class="seperator">&nbsp;</p></td></tr>
  <tr>
    <td colspan="2" style="width:50%;border-right:1px solid black;">
      <h3>First email</h3>
      <label for="passwordRecoveryMailSubject">Subject:</label>
      <input id="passwordRecoveryMailSubject" name="passwordRecoveryMailSubject" value="{$passwordRecoveryMailSubject}" type="text" size="60"/><br/>
      <label for="passwordRecoveryMailBody">Body (first %s is login, second is link)</label><br/>
      <textarea id="passwordRecoveryMailBody" name="passwordRecoveryMailBody" cols="80" rows="6">{$passwordRecoveryMailBody}</textarea>
    </td>
    <td>
      <h3>Second email</h3>
      <label for="passwordRecoveryMail2Subject">Subject:</label>
      <input id="passwordRecoveryMail2Subject" name="passwordRecoveryMail2Subject" value="{$passwordRecoveryMail2Subject}" type="text" size="60"/><br/>
      <label for="passwordRecoveryMail2Body">Body (first %s is login, second is link)</label><br/>
      <textarea id="passwordRecoveryMail2Body" name="passwordRecoveryMail2Body" cols="80" rows="6">{$passwordRecoveryMail2Body}</textarea>
    </td>
  </tr>
  <tr>
    <td colspan="4">
    <p class="seperator">&nbsp;</p>
    <div style="width:100%; text-align:right;padding-top:10px;padding-bottom:3px;">
      <input type='submit' name='Save' value='{msgPool type=saveButton}'>
    </div>
    <input type="hidden" name="recoveryConfigPosted" value="1">
    </td>
  </tr>
</table>
{/render}
