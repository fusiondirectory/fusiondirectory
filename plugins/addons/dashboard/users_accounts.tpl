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
        <colgroup>
          <col width="10%"/>
          <col width="15%"/>
          <col width="15%"/>
          <col width="15%"/>
          <col width="15%"/>
          <col width="10%"/>
        </colgroup>
        <thead>
          <tr style="background-color: white; text-align:center;">
            <th colspan="4" style="background-color: white; padding:5px;">
              {t}Expired accounts{/t}
            </th>
            <th colspan="3" style="background-color: white; padding:5px;">
              {t}Manager concerned{/t}
            </th>
          </tr>


          <tr style="background-color: #E8E8E8; height:26px;font-weight:bold;">
        <!-- uid/cn/telephonNumber/mail/shadowExpire/sambaKickoffTime -->
          <th>{t}uid{/t}</th><th>{t}cn{/t}</th><th>{t}telephoneNumber{/t}</th><th>{t}shadowExpire{/t}</th>
          <th>{t}manager{/t}</th><th>{t}mail{/t}</th><th>{t}telephoneNumber{/t}</th>
          </tr>
        </thead>
        <tbody>
          {foreach from=$attributes.expired.accounts item=account}
            <tr>
              <td>&nbsp;{$account.uid}</td>
              <td>&nbsp;{$account.cn}</td>
              <td>&nbsp;{$account.telephoneNumber}</td>
              <td>&nbsp;{$account.shadowExpire}</td>
              <td>&nbsp;{$account.manager_cn}</td>
              <td><a href="mailto:{$account.manager_mail}">{$account.manager_mail}</a></td>
              <td>&nbsp;{$account.manager_phone}</td>
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
        <colgroup>
          <col width="10%"/>
          <col width="15%"/>
          <col width="15%"/>
          <col width="15%"/>
          <col width="15%"/>
          <col width="10%"/>
        </colgroup>
        <thead>
          <tr style="background-color: white; text-align:center;">
            <th colspan="4" style="background-color: white; padding:5px;">
              {t}Next expired accounts{/t}
            </th>
            <th colspan="3" style="background-color: white; padding:5px;">
              {t}Manager concerned{/t}
            </th>
          </tr>
          <tr>
          <!-- uid/cn/telephonNumber/mail/shadowExpire/sambaKickoffTime -->
          <th>{t}uid{/t}</th><th>{t}cn{/t}</th><th>{t}telephoneNumber{/t}</th><th>{t}shadowExpire{/t}</th>
          <th>{t}manager{/t}</th><th>{t}mail{/t}</th><th>{t}telephoneNumber{/t}</th>
          </tr>
        </thead>
        <tbody>
          {foreach from=$attributes.expired.accounts_next_days item=account}
            <tr>
              <td>&nbsp;{$account.uid}</td>
              <td>&nbsp;{$account.cn}</td>
              <td>&nbsp;{$account.telephoneNumber}</td>
              <td>&nbsp;{$account.shadowExpire}</td>
              <td>&nbsp;{$account.manager_cn}</td>
              <td><a href="mailto:{$account.manager_mail}">{$account.manager_mail}</a></td>
              <td>&nbsp;{$account.manager_phone}</td>
            </tr>
          {/foreach}
        </tbody>
      </table>
    {/if}
  </div>
</div>
