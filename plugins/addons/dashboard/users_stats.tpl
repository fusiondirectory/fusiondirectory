<div id="{$sectionId}"  class="plugin_section">
  <span class="legend">
    {$section}
  </span>
  <div>
    <img src="{$attributes.stats.user_img}" alt="user icon"/>
    {t count=$attributes.stats.nb_accounts 1=$attributes.stats.nb_accounts plural="There are %1 users:"}There is 1 user:{/t}
    <ul>
      {foreach from=$attributes.stats.accounts item=acc}
        <li style="list-style-image:url({$acc.img})">
        {if $acc.nb > 0}
          {t count=$acc.nb 1=$acc.name 2=$acc.nb plural="%2 of them have a %1 account"}One of them have a %1 account{/t}
        {else}
          {t 1=$acc.name}None of them have a %1 account{/t}
        {/if}
        </li>
      {/foreach}
      <li style="list-style-image:url({$attributes.stats.locked_accounts.img})">
        {if $attributes.stats.locked_accounts.nb > 0}
          {t count=$attributes.stats.locked_accounts.nb 1=$attributes.stats.locked_accounts.nb plural="%1 of them are locked"}One of them is locked{/t}
        {else}
          {t}None of them is locked{/t}
        {/if}
      </li>
    </ul>
  </div>
</div>
