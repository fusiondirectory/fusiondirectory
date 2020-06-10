<input
  class="base-selector"
  type="text"
  name="{$htmlid}"
  id="{$htmlid}"
  onkeydown="$('bs_{$pid}').hide()"
  onfocus="$('bs_{$pid}').hide()"
  onmouseover="Element.clonePosition($('bs_{$pid}'), '{$htmlid}', {literal}{{/literal}setHeight: false, setWidth: false, offsetTop:(Element.getHeight('{$htmlid}')){literal}}{/literal});$('bs_{$pid}').show();"
  onmouseout="rtimer= Element.hide.delay(0.25, 'bs_{$pid}')"
  value="{$currentValue|escape}"/>

<div id="autocomplete_{$pid}" class="autocomplete"></div>
<script type="text/javascript">
  new Ajax.Autocompleter('{$htmlid}', 'autocomplete_{$pid}', 'autocomplete.php?type=base', { minChars: 3, frequency: 0.5 });
  {if $submitButton}
    $('{$htmlid}').observe(
      'keypress',
      function(event) {
        if(event.keyCode == Event.KEY_RETURN) {
          $('submit_base_{$pid}').click();
        }
      }
    );
  {/if}
</script>

<div
  class="treeList"
  style="display:none;max-height:{$height}px"
  id="bs_{$pid}"
  onmouseover="window.clearTimeout(rtimer);"
  onmouseout="rtimer= Element.hide.delay(0.25, 'bs_{$pid}')">
  <a
    class="treeList{if $selected}Selected{/if}"
    onclick="$('bs_rebase_{$pid}').value='{$rootBase|base64_encode}';  $('submit_tree_base_{$pid}').click();"
    >/&nbsp;[{t}Root{/t}]</a>

  {function tree}
    <ul class="{$class}">
    {foreach from=$tree key="dn" item="entry"}
      <li>
        <a
          {if $entry.selected}class="treeListSelected"{/if}
          {if $entry.link}onclick="$('bs_rebase_{$pid}').value='{$dn|base64_encode}';$('submit_tree_base_{$pid}').click();"{/if}
          title="{$dn|escape}">
          <img class="center"
            src="{$entry.img|escape}"
            alt=""/>&nbsp;{$entry.name|escape}{if $entry.description}&nbsp;<span class="informal">[{$entry.description|escape}]</span>{/if}
        </a>
        {tree tree=$entry.tree class=""}
      </li>
    {/foreach}
    </ul>
  {/function}

  {tree tree=$tree class="treeList"}
</div>

{if $submitButton}
  &nbsp;<input class="center" type="image" src="geticon.php?context=actions&amp;icon=submit&amp;size=16" title="{t}Submit{/t}" name="submit_base_{$pid}" id="submit_base_{$pid}" alt="{t}Submit{/t}"/>
{/if}
<input type="submit" formnovalidate="formnovalidate" style="display:none" name="submit_tree_base_{$pid}" id="submit_tree_base_{$pid}"/>
<input type="hidden" name="bs_rebase_{$pid}" id="bs_rebase_{$pid}"/>
<input type="hidden" name="BPID" id="BPID" value="{$pid}"/>
