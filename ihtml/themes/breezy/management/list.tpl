<input type="hidden" value="{$PID}" name="PID"/>

<div style="width:100%;">
<table class="listingTable management">
  <thead><tr>
    {if $multiSelect}
      <th class="checkbox">
        <input type="checkbox" id="select_all" name="select_all" title="{t}Select all{/t}" onClick='toggle_all_("listing_selected_[0-9]*$","select_all");' />
      </th>
    {/if}
    {foreach from=$headers key=index item=header}
      {if $header.sortable}
        <th {$header.props}><a href="?plug={$PLUG}&amp;PID={$PID}&amp;act=SORT_{$index}">{$header.label|escape}
        {if isset($header.sortdirection)}
          &nbsp;<img title="{if $header.sortdirection}{t}Up{/t}{else}{t}Down{/t}{/if}" src="geticon.php?context=actions&amp;size=16&amp;icon=view-sort-{if $header.sortdirection}descending{else}ascending{/if}" alt="{if $header.sortdirection}{t}Sort up{/t}{else}{t}Sort down{/t}{/if}"/>
        {/if}
        </a></th>
      {else}
        <th {$header.props}>{$header.label|escape}</th>
      {/if}
    {/foreach}
  </tr></thead>

  <tbody>
    {if (count($rows) == 0)}
      <tr><td colspan="{$columnCount}" class="filler">&nbsp;</td></tr>
    {/if}

    {foreach from=$rows item=row}
      <tr class="{' '|implode:$row.classes}">
      {if $multiSelect}
        <td class="checkbox">
          <input type="checkbox" id="listing_selected_{$row.index}" name="listing_selected_{$row.index}"/>
        </td>
      {/if}
      {foreach from=$row.cells item=cell}
        <td {$cell.props}>{$cell.render}</td>
      {/foreach}
      </tr>
    {/foreach}
  </tbody>
</table>
</div>

{if ($showFooter)}
  <div class="nlistFooter">

  {foreach from=$objectCounts item=type}
    <img src="{$type.icon|escape}" title="{$type.name|escape}" alt="{$type.name|escape}"/>&nbsp;{$type.count}&nbsp;&nbsp;&nbsp;&nbsp;
  {/foreach}

  </div>
{/if}
