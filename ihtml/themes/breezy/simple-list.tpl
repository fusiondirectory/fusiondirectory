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
                {if $BASE}<td>{$ROOT}&nbsp;</td><td>{$BACK}&nbsp;</td><td>{$HOME}&nbsp;</td>{/if}
                <td class="optional">{$RELOAD}&nbsp;</td>
                {if $BASE}<td>{$SEPARATOR}&nbsp;</td><td>{t}Base{/t} {$BASE}&nbsp;</td>{/if}
                {if $ACTIONS}<td>{$SEPARATOR}&nbsp;</td><td>{$ACTIONS}</td>{/if}
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
