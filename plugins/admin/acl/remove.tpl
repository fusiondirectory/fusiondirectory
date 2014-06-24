<div style="padding:5px;">
  <div style="font-size:18px;">
   <img alt="" src="geticon.php?context=status&icon=dialog-warning&size=48" class="center"/>&nbsp;{t}Warning{/t}
  </div>
  <p>
    <ul>
      {foreach from=$objects item=object}
        <li style="list-style-image:url('{$object.icon}');" title="{$object.type}">
          {$object.name}&nbsp;(<i>{$object.dn}</i>)
        </li>
      {/foreach}
    </ul>
    {t}This includes all ACL assignments made on this node(s). If you want the list of these assignments, please cancel and open the objects.{/t}
  </p>

  <p>
    {t}Press 'Delete' to continue or 'Cancel' to abort.{/t}
  </p>

  <p class="plugbottom">
    <input type="submit" name="delete_confirmed" value="{msgPool type=delButton}"/>
    <input type="submit" name="delete_cancel" value="{msgPool type=cancelButton}"/>
  </p>
</div>
