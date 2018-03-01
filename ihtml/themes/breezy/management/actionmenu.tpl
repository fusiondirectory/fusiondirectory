{function menu level=2}
  <ul class="level{$level}">
  {foreach $data as $entry}
    <li id="actionmenu_{$entry.name|escape}" {if $entry.separator}style="border-top:1px solid #AAA"{/if}>
      {if isset($entry.actions)}
        <a href="#">
          <img src="{$entry.icon|escape}" alt="{$entry.name|escape}"/>&nbsp;{$entry.label|escape}
          &nbsp;<img src="images/forward-arrow.png" alt="forward arrow"/>
        </a>
        {menu data=$entry.actions level=$level+1}
      {else}
        <a href="#" onClick="document.getElementById('actionmenu').value='{$entry.name|escape}';document.getElementById('exec_act').click();">
          <img src="{$entry.icon|escape}" alt="{$entry.name|escape}">&nbsp;{$entry.label|escape}
        </a>
      {/if}
    </li>
  {/foreach}
  </ul>
{/function}

<div id="pulldown">
  <input type="hidden" name="act" id="actionmenu" value=""/>
  <div style="display:none"><input type="submit" name="exec_act" id="exec_act" value=""/></div>
  <ul class="level1" id="root">
    <li>
      <a href="#">{t}Actions{/t}&nbsp;<img class="center optional" src="images/down-arrow.png" alt="down arrow"/></a>
      {menu data=$actions}
    </li>
  </ul>
</div>
