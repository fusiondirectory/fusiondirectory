<h2>{t}User settings{/t}</h2>
<table width="100%">
  <tr>
  <td style="vertical-align:top; border-right:1px solid #AAA; width:50%">
    <table>
    <tr>
      <td><label for="sn">{t}Last name{/t}</label></td>
      <td><input type='text' id="sn" name="sn" size=25 maxlength=60  value="{$sn}"></td>
    </tr>
    <tr>
      <td><label for="givenName">{t}First name{/t}</label></td>
      <td><input type='text' id="givenName" name="givenName" size=25 maxlength=60 value="{$givenName}"></td>
    </tr>
    <tr>
      <td><label for="uid">{t}Login{/t}</label></td>
      <td><input type='text' id="uid" name="uid" size=25 maxlength=60 value="{$uid}"></td>
    </tr>
    <tr>
      <td style='vertical-align:top;'>
        {t}Password{/t}
      </td>
      <td>
        <input type="radio" {if $passwordTodo=="clear"} checked{/if} name="passwordTodo" value="clear">{t}Clear password{/t}<br>
        <input type="radio" {if $passwordTodo=="new"}   checked{/if} name="passwordTodo" value="new">{t}Set new password{/t}
      </td>
    </tr>
    </table>
  </td>
  <td style="vertical-align:top ">
    <table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding=4 border=0>
    <tr>
    <!-- Image container -->
      <td colspan="2">
      <table>
        <tr>
        <td width="147" height="200" bgcolor="gray">
          <img align="middle" border="0" width="100%" src="getbin.php?rand={$rand|escape}" alt="{t}Personal picture{/t}">
        </td>
        </tr>
      </table>
      </td>
    </tr>
    <tr>
      <!-- Name, ... -->
      <td style="vertical-align:top;">
      <input type="hidden" name="MAX_FILE_SIZE" value="2000000">
       {t}User picture{/t}
      </td>
      <td>
      <input id="picture_file" name="picture_file" type="file" size="20" maxlength="255" accept="image/*.jpg">
      &nbsp;
      <input type=submit name="picture_remove" value="{t}Remove picture{/t}">
      </td>
    </tr>
    </table>
  </td>
  </tr>
</table>
<br>
