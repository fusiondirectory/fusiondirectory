<div id="{$sectionId}"  class="plugin_section">
  <span class="legend">
    {$section}
  </span>
  <div>
    <img src="{$attributes.groups_stats.img}" alt="group icon"/>
    {t count=$attributes.groups_stats.nb 1=$attributes.groups_stats.nb plural="There are %1 groups:"}There is 1 group:{/t}
    <ul>
      {foreach from=$attributes.groups_stats.groups item=g}
        <li style="list-style-image:url({$g.img})">
        {if $g.nb > 0}
          {t count=$g.nb 1=$g.name 2=$g.nb plural="%2 are %1 groups"}One of them is a %1 group{/t}
        {else}
          {t 1=$g.name}There is no %1 group{/t}
        {/if}
        </li>
      {/foreach}
    </ul>
  </div>
</div>
