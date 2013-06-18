
{if $feedback_send}

  <div class='default' style='margin:12px; '>
    <div style='color:green;'>{t}Feedback successfully send{/t}</div>
  </div>

{else}

<div class='default' style='margin:12px; '>

  <h2>{$additional_info}</h2>

  <h2>
    <input {if $subscribe} checked {/if} type='checkbox' name='subscribe' value='1' class='center'>&nbsp;{t}Subscribe to the fusiondirectory-users mailinglist{/t}
  </h2>
  <table>
    <tr>
      <td>
        {t}Organization{/t}
      </td>
      <td>
        <input name='organization' type='text' value='{$organization}' style='width:300px;'>
      </td>
    </tr>
    <tr>
      <td>
        {t}Name{/t}
      </td>
      <td>
        <input name='name' type='text' value='{$name}' style='width:300px;'>
      </td>
    </tr>
    <tr>
      <td>
        {t}Mail address{/t}{$must}
      </td>
      <td>
        <input name='eMail' type='text' value='{$eMail}' style='width:300px;'>
      </td>
    </tr>
  </table>


  <p>&nbsp;</p>
  <h2>
      <input {if $use_gosa_announce} checked {/if} type='checkbox' name='use_gosa_announce' value='1' class='center'>&nbsp;{t}Send feedback to the FusionDirectory project team{/t}
  </h2>
  <p>
  {t}When checking this option, FusionDirectory will try to connect http://www.fusiondirectory.org in order to submit your form anonymously.{/t}
  </p>
  <b>{t}Generic{/t}</b>
  <table>
    <tr>
      <td style='vertical-align:top'>
        {t}Did the setup procedure help you to get started?{/t}
      </td>
      <td>
        <input {if $get_started} checked {/if} type='radio' name='get_started' value='1'>{t}Yes{/t}
        <br>
        <input {if !$get_started} checked {/if} type='radio' name='get_started' value='0'>{t}No{/t}
      </td>
    </tr>
    <tr>
      <td style='vertical-align:top'>
        {t}If not, what problems did you encounter{/t}:
      </td>
      <td>
        <textarea name='problems_encountered' rows='4' cols='50' style='width:100%'>{$problems_encountered}</textarea>
      </td>
    </tr>
    <tr>
      <td style='vertical-align:top'>
        {t}Is this the first time you use FusionDirectory?{/t}
      </td>
      <td>
        <input {if $first_use} checked {/if} type='radio' name='first_use' value='1'>{t}Yes{/t}
        <br>
        <input {if !$first_use} checked {/if} type='radio' name='first_use' value='0'>{t}No{/t},
        {t}I use it since{/t}
        <select name='use_since' title='{t}Select the year since when you are using FusionDirectory{/t}'>
          {html_options options=$years}
        </select>
      </td>
    </tr>
    <tr>
      <td style='vertical-align:top'>
        {t}What operating system / distribution do you use?{/t}
      </td>
      <td>
        <input type='text' name='distribution' size=50 value='{$distribution}'>
      </td>
    </tr>
    <tr>
      <td style='vertical-align:top'>
        {t}What web server do you use?{/t}
      </td>
      <td>
        <input type='text' size=50 name='web_server' value='{$web_server}'>
      </td>
    </tr>
    <tr>
      <td style='vertical-align:top'>
        {t}What PHP version do you use?{/t}
      </td>
      <td>
        <input type='text' size=50 name='php_version' value='{$php_version}'>
      </td>
    </tr>
    <tr>
      <td style='vertical-align:top'>
        {t}FusionDirectory version{/t}</td>
      </td>
      <td>
        {$gosa_version}
      </td>
    </tr>
  </table>

  <br><b>{t}LDAP{/t}</b>
  <table>
    <tr>
      <td style='vertical-align:top'>
      {t}What kind of LDAP server(s) do you use?{/t}
      </td>
      <td><input type='text' name='ldap_server' size=50 value='{$ldap_server}'>
      </td>
    </tr>
    <tr>
      <td>{t}How many objects are in your LDAP?{/t}
      </td>
      <td><input type='text' name='object_count' size=50 value='{$object_count}'>
      </td>
    </tr>
  </table>

  <br><b>{t}Features{/t}</b>
  <table>
    <tr>
      <td style='vertical-align:top'>{t}What features of FusionDirectory do you use?{/t}
      </td>
      <td>
        {foreach from=$features_used item=data key=key}
          <input type='checkbox' name='feature_{$key}' {if $data.USED} checked {/if}>
          {$data.NAME}<br>
        {/foreach}
      </td>
    </tr>
    <tr>
      <td style="vertical-align:top">{t}What features do you want to see in future versions of FusionDirectory?{/t}
      </td>
      <td><textarea name='want_to_see_next' cols=50 rows=3>{$want_to_see_next}</textarea>
      </td>
    </tr>
  </table>
</div>
<input type='submit' name='send_feedback' value='{t}Send feedback{/t}'>
{/if}
<input type='hidden' name='step_feedback' value='1'>
