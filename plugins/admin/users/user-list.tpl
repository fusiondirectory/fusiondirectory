<table summary="" class="listing_container">
  <tbody>
    <tr>
      <td class="list">
        <div class="contentboxh">
          <p class="contentboxh">&nbsp;{$HEADLINE}&nbsp;{$SIZELIMIT}</p>
        </div>

        <div class="contentboxb">
         <div style='background:white;padding:3px'>
          <table><tr>
            <td>{$ROOT}&nbsp;</td><td>{$BACK}&nbsp;</td><td>{$HOME}&nbsp;</td><td>{$RELOAD}&nbsp;</td><td>{$SEPARATOR}&nbsp;</td><td>{t}Base{/t} {$BASE}&nbsp;</td><td>{$SEPARATOR}&nbsp;</td><td><img src='images/rocket.png' alt='' class='center'></td><td> {$ACTIONS}</td>
          </tr></table>
         </div>
        </div>

        <div style='margin-top:4px;'>
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
