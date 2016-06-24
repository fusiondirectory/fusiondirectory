<div class="notice">
  <div class="msgtitle">
    <h2>
      <img alt="" src="geticon.php?context=status&amp;icon=dialog-warning&amp;size=32" class="center"/>&nbsp;{t}Warning: you are about to delete the following objects{/t}
    </h2>
  </div>
  <div>
    <p>
      <ul>
        {foreach from=$objects item=object}
          <li style="list-style-image:url('{$object.icon}');" title="{$object.type}">
            {$object.name}&nbsp;(<i>{$object.dn}</i>)
          </li>
        {/foreach}
      </ul>
      {t}Please double check if you really want to do this since there is no way for FusionDirectory to get your data back.{/t}
    </p>

    <p>
     {t}So - if you're sure - press 'Delete' to continue or 'Cancel' to abort.{/t}
    </p>

    <p class="plugbottom">
      <input type="submit" name="delete_confirmed" value="{msgPool type=delButton}"/>
      <input type="submit" name="delete_cancel" value="{msgPool type=cancelButton}"/>
    </p>
  </div>
</div>
