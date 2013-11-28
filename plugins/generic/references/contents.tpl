<div id="{$sectionId}" class="plugin_section{$sectionClasses}">
  <span class="legend">
    {$section}
  </span>
  <div>
    {if $attributes.refs}
      <ul>
        {foreach from=$attributes.refs key=key item=object}
          <li style="list-style-type:none;">
            {$object.link}
            <ul>
              {foreach from=$object.tabs item=tab}
                <li style="list-style-type:none;">{$tab.link}
                  <ul>
                    {foreach from=$tab.fields key=ofield item=field}
                      <li style="list-style-type:disc;">
                        {t escape=no 1=$ofield 2=$field.tab 3=$field.field}<strong>%1</strong> references our field <strong>%3</strong> from tab <strong>%2</strong>{/t}
                      </li>
                    {/foreach}
                  </ul>
                </li>
              {/foreach}
            </ul>
          </li>
        {/foreach}
      </ul>
    {else}
      <b>{t}This object has no relationship to other objects.{/t}</b>
    {/if}
  </div>
</div>
