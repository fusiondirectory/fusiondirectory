<table class="listing-container">
  <tbody>
    <tr>
      <td class="list">
        <div class="contentboxh">
          <p class="contentboxh">&nbsp;{$HEADLINE}&nbsp;{$SIZELIMIT}</p>
        </div>

        <div class="contentboxb" style="background:white;">
          <table>
            <tbody>
              <tr>
                {foreach from=$NAVIGATION item="action"}
                  <td class="{$action.class}">
                  {if $action.enabled}
                    <input type="image" src="{$action.icon|escape}"
                      name="{$action.id}" title="{$action.desc}" alt="{$action.name}"/>
                  {else}
                    <img src="{$action.icon|escape}&amp;disabled=1" alt="{$action.name}"/>
                  {/if}
                  &nbsp;</td>
                {/foreach}
                {if $BASE}<td><img src="images/lists/seperator.png" alt="-" height="16" width="1" class="center"/>&nbsp;</td><td>{t}Base{/t} {$BASE}&nbsp;</td>{/if}
                {if $ACTIONS}<td><img src="images/lists/seperator.png" alt="-" height="16" width="1" class="center"/>&nbsp;</td><td>{$ACTIONS}</td>{/if}
              </tr>
            </tbody>
          </table>
        </div>

        <div style="margin-top:4px;">
          {$LIST}
        </div>
      </td>
      <td class="filter">
        {$FILTER}
      </td>
    </tr>
  </tbody>
</table>

<input type="hidden" name="ignore"/>
