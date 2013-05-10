<body style="background-color: #8B858B;background-image:none;">
{* FusionDirectory login - smarty template *}
{$php_errors}
{$errors}

<!-- Spacer for some browsers -->
<div class='gosaLoginSpacer'></div>

<table summary="" class='gosaLoginMask' style="text-align:right;" align='center'>
 <tr>
  <td class='gosaAccountExpiredBack' style="width:550px;">
   <div class='gosaAccountExpiredBack'>

    <p class='gosaAccountExpiredHeader'>
    {t}Your password has expired. Please choose a new one!{/t}
    </p>

    <!-- Formular data, containing a table to center fields -->
    <form action='index.php' method='post' name='mainform' onSubmit='js_check(this);return true;'>
     <table summary="" style='vertical-align:middle; text-align:left;' cellspacing='7' align='center'>
      <tr>
        <td>
          <p> {t}Old password{/t}</p>
        </td>
        <td>
    <input type="password" name="oldpassword" maxlength="25" value=""
     title="{t}Old password{/t}" onFocus="nextfield= 'newpassword';">
        </td>
      </tr>
      <tr>
        <td>
          <p> {t}New password{/t}</p>
        </td>
        <td>
    <input type="password" name="newpassword" maxlength="25" value=""
     title="{t}New password{/t}" onFocus="nextfield= 'verifypassword';">
        </td>
      </tr>
      <tr>
        <td>
          <p> {t}Verify password{/t}</p>
        </td>
        <td>
    <input type="password" name="verifypassword" maxlength="25" value=""
     title="{t}Verify password{/t}" onFocus="nextfield= 'changepassword';">
       </td>
      </tr>
      <tr>
       <td colspan="2" style="text-align:center;">
       <br />
    <input type="submit" name="changepassword" value="{t}Change password{/t}"
       title="{t}Click here to change your password{/t}">
       </td>
      </tr>
     </table>
    <input type="hidden" name="javascript" value="false"/>
    </form>

   </div>
  </td>
 </tr>
</table>

<!-- Place cursor in username field -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
  nextfield= "{$nextfield}";
  focus_field({$nextfield});
  -->
</script>

<!-- Spacer for some browsers -->
<div class="gosaLoginSpacer"></div>
</body>
