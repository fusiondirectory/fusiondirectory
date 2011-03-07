<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding=0 border=0>
 <tr>
  <td style="width:50%; vertical-align:top;">

<!-- must_change_password  -->
{render acl=$mustchangepasswordACL checkbox=$multiple_support checked=$use_mustchangepassword}
   <input type="checkbox" class="center" name="mustchangepassword" value="1" {$mustchangepassword}>
{/render}
   {t}User must change password on first login{/t}
   <br>

<!-- shadowMin -->
{render acl=$shadowMinACL checkbox=$multiple_support checked=$use_activate_shadowMin}
   <input type="checkbox" class="center" name="activate_shadowMin" value="1" {$activate_shadowMin}>
{/render}
{render acl=$shadowMinACL}
   {$shadowmins}
{/render}
   <br>

<!-- activate_shadowMax -->
{render acl=$shadowMaxACL checkbox=$multiple_support checked=$use_activate_shadowMax}
   <input type="checkbox" class="center" name="activate_shadowMax" value="1" {$activate_shadowMax}>
{/render}
{render acl=$shadowMaxACL}
   {$shadowmaxs}
{/render}
   <br>

<!-- activate_shadowExpire -->
{render acl=$shadowExpireACL checkbox=$multiple_support checked=$use_activate_shadowExpire}
   <table border="0" cellpadding="0" cellspacing="0"><tr><td><input type="checkbox" class="center" name="activate_shadowExpire" value="1" {$activate_shadowExpire}>
{/render}
   {t}Password expires on{/t}&nbsp;
   {render acl=$shadowExpireACL}
   </td><td style='width:130px'>
   <input type="text" id="shadowExpire" name="shadowExpire" class="date" style='width:100px' value="{$shadowExpire}">
     {if $shadowExpireACL|regex_replace:"/[cdmr]/":"" == "w"}
     <script type="text/javascript">
     {literal}
       var datepicker  = new DatePicker({ relative : 'shadowExpire', language : '{/literal}{$lang}{literal}',
                             keepFieldEmpty : true, enableCloseEffect : false, enableShowEffect : false});
     {/literal}
     </script>
     {/if}
     </td></tr></table>
   {/render}

<!-- shadowInactive -->
{render acl=$shadowInactiveACL checkbox=$multiple_support checked=$use_activate_shadowInactive}
   <input type="checkbox" class="center" name="activate_shadowInactive" value="1" {$activate_shadowInactive}>
{/render}
{render acl=$shadowInactiveACL}
   {$shadowinactives}
{/render}
   <br>

<!-- activate_shadowWarning -->
{render acl=$shadowWarningACL checkbox=$multiple_support checked=$use_activate_shadowWarning}
   <input type="checkbox" class="center" name="activate_shadowWarning" value="1" {$activate_shadowWarning}>
{/render}
{render acl=$shadowWarningACL}
   {$shadowwarnings}
{/render}
  </td>
 </tr>
</table>
