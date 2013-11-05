<table summary="" class="listing_container">
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
                <td>{$ROOT}&nbsp;</td><td>{$BACK}&nbsp;</td><td>{$HOME}&nbsp;</td><td class="optional">{$RELOAD}&nbsp;</td><td>{$SEPARATOR}&nbsp;</td>
                {if $BASE}<td>{t}Base{/t} {$BASE}&nbsp;</td><td>{$SEPARATOR}&nbsp;</td>{/if}
                <td><img src="images/rocket.png" alt="" class="center optional"/></td><td>{$ACTIONS}</td>
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
