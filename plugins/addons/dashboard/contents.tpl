<div class="plugin_section">
  <span class="legend">
    {t}Statistics{/t}
  </span>
  <div>
    <ul>
    {foreach from=$stats item=stat}
    <li style="list-style-image:url({$stat.img})">
      {$stat.nb} {$stat.name}
    </li>
    {/foreach}
    </ul>
    {if $nb_argonaut_server > 1}
      <img src="images/small_warning.png" style="vertical-align:middle;margin-right:3px;" alt="warning"/>{t}There are several argonaut servers! (this is not yet supported){/t}
    {elseif $nb_argonaut_server == 1}
      {t 1=$argonaut_server.cn 2=$argonaut_server.ipHostNumber 3=$argonaut_server.argonautProtocol 4=$argonaut_server.argonautPort}There is an argonaut server running on %1 (%3://%2:%4){/t}<br/>
      {if $nb_argonaut_clients > 0}
        {t count=$nb_argonaut_clients 1=$nb_argonaut_clients plural="%1 systems are configured to run an argonaut client."}Only one system is configured to run an argonaut client.{/t}<br/>
      {else}
        {t}But no system is configured to run an argonaut client!{/t}<br/>
      {/if}
    {elseif $nb_argonaut_clients > 0}
      <img src="images/small_warning.png" style="vertical-align:middle;margin-right:3px;" alt="warning"/>{t count=$nb_argonaut_clients 1=$nb_argonaut_clients plural="%1 systems are configured to run an argonaut client, but there is no argonaut server configured!"}A system is configured to run an argonaut client, but there is no argonaut server configured!{/t}<br/>
    {/if}
  </div>
</div>

<div class="plugin_section">
  <span class="legend">
    {t}Computer name to use by unit{/t}
  </span>
  <div>
    <table style="border:1px solid #B0B0B0; width:100%; vertical-align:top; text-align:left;border-spacing:0;">
      {$output_next_computer_ids}
    </table>
  </div>
</div>
