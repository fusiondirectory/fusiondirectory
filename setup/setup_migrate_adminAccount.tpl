<div>
  <div class="default">
    <h2>{t}Create a new FusionDirectory administrator account{/t}</h2>

    <p>
      {t}This dialog will automatically add a new super administrator to your LDAP tree.{/t}
    </p>
    <table>
      <tr>
        <td>
          {t}Name{/t}:&nbsp;
        </td>
        <td>
          <i>System administrator</i>
        </td>
      </tr>
      <tr>
        <td>
          {t}User ID{/t}:&nbsp;
        </td>
        <td>
          <input type='text' value='{$infos.uid}' name='uid'><br>
        </td>
      </tr>
      <tr>
        <td>
          {t}Password{/t}:&nbsp;
        </td>
        <td>
          <input type='password' value='{$infos.password}' name='userPassword_password'><br>
        </td>
      </tr>
      <tr>
        <td>
          {t}Password (again){/t}:&nbsp;
        </td>
        <td>

          <input type='password' value='{$infos.password2}' name='userPassword_password2'><br>
        </td>
      </tr>
    </table>

    <!-- Place cursor -->
    <script type="text/javascript">
      <!-- // First input field on page
      focus_field('new_user_password');
      -->
    </script>

    <hr/>
    <div style='width:99%; text-align:right; padding:5px;'>
      <input type='submit' name='dialog_confirm' value='{t}Apply{/t}'/>
      <input type='submit' name='dialog_cancel' value='{t}Cancel{/t}'/>
    </div>
  </div>
</div>
