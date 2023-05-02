<fieldset><legend>{$NAME|escape}</legend>
  <label for="nt_{$attribute}" title="{t 1=$NAME}Minimum date for %1{/t}">
    {t}Newer than{/t}
    <input type="date" id="nt_{$attribute}" name="nt_{$attribute}" value="{$nt_value}"/>
  </label>
  <label for="ot_{$attribute}" title="{t 1=$NAME}Maximum date for %1{/t}">
    {t}Older than{/t}
    <input type="date" id="ot_{$attribute}" name="ot_{$attribute}" value="{$ot_value}"/>
  </label>
</fieldset>
