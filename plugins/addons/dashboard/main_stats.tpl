<div class="plugin_section">
  <span class="legend">
    {t}Statistics{/t}
  </span>
  <div>
    <ul style="list-style-type:none;line-height:2em;">
    {foreach from=$attributes.stats item=stat}
    <li>
{if isset($stat.href)}
      <a href="{$stat.href}"><img style="vertical-align:middle;" src="{$stat.img}" alt=""/>&nbsp;{$stat.name}&nbsp;: {$stat.nb}</a>
{else}
      <img style="vertical-align:middle;" src="{$stat.img}" alt=""/>&nbsp;{$stat.name}&nbsp;: {$stat.nb}
{/if}
    </li>
    {/foreach}
    </ul>
  </div>
</div>
