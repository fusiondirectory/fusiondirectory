<div id="{$sectionId}"  class="plugin-section fullwidth">
  <span class="legend">
    {$section}
  </span>
  <div>
    <h1>
    {if $attributes.expired.accounts|@count > 0}
      {t count=$attributes.expired.accounts|@count 1=$attributes.expired.accounts|@count plural="There are %1 expired accounts"}There is one expired account{/t}
    {else}
      {t}There is no expired account{/t}
    {/if}
    </h1>
    {if $attributes.expired.accounts|@count > 0}
      <table style="width: 100%;" class="listingTable">
        <thead>
          <tr style="background-color: white; text-align:center;">
            <th colspan="{$attributes.expired.columns.user|@count}" style="background-color: white; padding:5px;">
              {t}Expired accounts{/t}
            </th>
            <th colspan="{$attributes.expired.columns.manager|@count}" style="background-color: white; padding:5px;">
              {t}Manager concerned{/t}
            </th>
          </tr>


          <tr>
          {foreach from=$attributes.expired.columns.user item=colname}
            <th>{$colname}</th>
          {/foreach}
          {foreach from=$attributes.expired.columns.manager item=colname}
            <th>{$colname}</th>
          {/foreach}
          </tr>
        </thead>
        <tbody>
          {foreach from=$attributes.expired.accounts item=account}
            <tr>
              {foreach from=$attributes.expired.columns.user key=colkey item=colname}
                <td>&nbsp;{$account.$colkey}</td>
              {/foreach}
              {foreach from=$attributes.expired.columns.manager key=colkey item=colname}
                {if $colkey==manager_mail}
                  <td><a href="mailto:{$account.$colkey}">{$account.$colkey}</a></td>
                {else}
                  <td>&nbsp;{$account.$colkey}</td>
                {/if}
              {/foreach}
            </tr>
          {/foreach}
        </tbody>
      </table>
     {/if}

    <h1>
    {if $attributes.expired.accounts_next_days|@count > 0}
      {t count=$attributes.expired.accounts_next_days|@count 1=$attributes.expired.next_days 2=$attributes.expired.accounts_next_days|@count plural="There are %2 accounts expiring in the next %1 days"}There is one account expiring in the next %1 days{/t}
    {else}
      {t 1=$attributes.expired.next_days}There is no account expiring in the next %1 days{/t}
    {/if}
    </h1>
    {if $attributes.expired.accounts_next_days|@count > 0}
      <table style="width: 100%;" class="listingTable">
        <thead>
          <tr style="background-color: white; text-align:center;">
            <th colspan="{$attributes.expired.columns.user|@count}" style="background-color: white; padding:5px;">
              {t}Next expired accounts{/t}
            </th>
            <th colspan="{$attributes.expired.columns.manager|@count}" style="background-color: white; padding:5px;">
              {t}Manager concerned{/t}
            </th>
          </tr>


          <tr>
          {foreach from=$attributes.expired.columns.user item=colname}
            <th>{$colname}</th>
          {/foreach}
          {foreach from=$attributes.expired.columns.manager item=colname}
            <th>{$colname}</th>
          {/foreach}
          </tr>
        </thead>
        <tbody>
          {foreach from=$attributes.expired.accounts_next_days item=account}
            <tr>
              {foreach from=$attributes.expired.columns.user key=colkey item=colname}
                <td>&nbsp;{$account.$colkey}</td>
              {/foreach}
              {foreach from=$attributes.expired.columns.manager key=colkey item=colname}
                {if $colkey==manager_mail}
                  <td><a href="mailto:{$account.$colkey}">{$account.$colkey}</a></td>
                {else}
                  <td>&nbsp;{$account.$colkey}</td>
                {/if}
              {/foreach}
            </tr>
          {/foreach}
        </tbody>
      </table>
    {/if}
  </div>
</div>
