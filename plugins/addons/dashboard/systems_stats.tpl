<div id="{$sectionId}" class="plugin_section">
  <span class="legend">
    {$section}
  </span>
  <div>
    <ul>
    {foreach from=$attributes.stats.systems item=stat}
    <li style="list-style-image:url({$stat.img})">
      {$stat.nb} {$stat.name}
    </li>
    {/foreach}
    </ul>
    {if $attributes.stats.argonaut.nb_servers > 1}
      <img src="images/small_warning.png" style="vertical-align:middle;margin-right:3px;" alt="warning"/>{t}There are several argonaut servers! (this is not yet supported){/t}
    {elseif $attributes.stats.argonaut.nb_servers == 1}
      {t escape=none 1=$attributes.stats.argonaut.server.link 2=$attributes.stats.argonaut.server.ipHostNumber 3=$attributes.stats.argonaut.server.argonautProtocol 4=$attributes.stats.argonaut.server.argonautPort}There is an argonaut server running on %1 (%3://%2:%4){/t}<br/>
      {if $attributes.stats.argonaut.nb_clients > 0}
        {t count=$attributes.stats.argonaut.nb_clients 1=$attributes.stats.argonaut.nb_clients plural="%1 systems are configured to run an argonaut client."}Only one system is configured to run an argonaut client.{/t}<br/>
      {else}
        {t}But no system is configured to run an argonaut client!{/t}<br/>
      {/if}
    {elseif $attributes.stats.argonaut.nb_clients > 0}
      <img src="images/small_warning.png" style="vertical-align:middle;margin-right:3px;" alt="warning"/>{t count=$attributes.stats.argonaut.nb_clients 1=$attributes.stats.argonaut.nb_clients plural="%1 systems are configured to run an argonaut client, but there is no argonaut server configured!"}A system is configured to run an argonaut client, but there is no argonaut server configured!{/t}<br/>
    {/if}
  </div>
</div>
