<div>
  <div class="default">
    <h2>{t}Add required object classes to the LDAP base{/t}</h2>

    <b>{t}Current{/t}</b>
    <div class="step2-entry-container-info">
      <div style='padding-left:20px;'>
        <pre>{$infos.current}</pre>
      </div>
    </div>
    <br/>
    <b>{t}After migration{/t}</b>
    <div class="step2-entry-container-info">
      <div style='padding-left:20px;'>
        <pre>{$infos.target}</pre>
      </div>
    </div>

    <br/>
    <input type='submit' name='dialog_confirm' value='{t}Migrate{/t}'>
    </p>

    <hr/>
    <div style='width:100%; text-align:right; padding:5px;'>
      <input type='submit' name='dialog_cancel' value='{t}Close{/t}'>
    </div>
  </div>
</div>
