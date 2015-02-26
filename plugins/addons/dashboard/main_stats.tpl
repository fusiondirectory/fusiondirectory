<div class="plugin_section">
  <span class="legend">
    {t}Statistics{/t}
  </span>
  <div>
    <ul>
    {foreach from=$attributes.stats item=stat}
    <li style="list-style-image:url({$stat.img})">
      {$stat.nb} {$stat.name}
    </li>
    {/foreach}
    </ul>
  </div>
</div>
