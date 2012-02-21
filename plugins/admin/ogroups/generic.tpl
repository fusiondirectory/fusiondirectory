<table summary="" style="width:100%;">
 <tr>
  <td style="width:50%; vertical-align:top">
   <input type="hidden" name="ogroupedit" value="1">
   <table summary="">
    <tr>
     <td><LABEL for="cn">{t}Group name{/t}</LABEL>{$must}</td>
     <td>
{render acl=$cnACL}
       <input type='text' name="cn" id="cn" size=25 maxlength=60 value="{$cn}" title="{t}Name of the group{/t}">
{/render}
     </td>
    </tr>
    <tr>
     <td><LABEL for="description">{t}Description{/t}</LABEL></td>
     <td>
{render acl=$descriptionACL}
  <input type='text' id="description" name="description" size=40 maxlength=80 value="{$description}" title="{t}Descriptive text for this group{/t}">
{/render}
     </td>
    </tr>
    <tr>
     <td colspan=2>&nbsp;</td>
    </tr>
    <tr>
     <td><LABEL for="base">{t}Base{/t}</LABEL>{$must}</td>
     <td>
{render acl=$baseACL}
       {$base}
{/render}
     </td>
    </tr>
   </table>

  <p class="seperator">&nbsp;</p>
  <h2><img alt="" class="center" align="middle" src="images/lists/locked.png" /> {t}System trust{/t}</h2>
    {t}Trust mode{/t}&nbsp;
    {render acl=$trustmodeACL}
        <select name="trustmode" id="trustmode" size=1
            onChange="changeSelectState('trustmode', 'wslist');
                      changeSelectState('trustmode', 'add_ws');
                      changeSelectState('trustmode', 'del_ws');">
          {html_options options=$trustmodes selected=$trustmode}
        </select><br>
    {/render}
    {render acl=$trustmodeACL}
       <select style="width:100%" id="wslist" name="workstation_list[]" size=8 multiple {$trusthide}>
        {html_options values=$workstations output=$workstations}
        {if $emptyArrAccess}
            <option disabled>&nbsp;</option>
        {/if}
       </select>
    {/render}
       <br>
    {render acl=$trustmodeACL}
       <input type="submit" id="add_ws" value="{msgPool type=addButton}" name="add_ws" {$trusthide}>&nbsp;
    {/render}
    {render acl=$trustmodeACL}
       <input type="submit" id="del_ws" value="{msgPool type=delButton}" name="delete_ws" {$trusthide}>
    {/render}


  </td>
  <td style="border-left:1px solid #A0A0A0; padding-left:10px;">
   <b><LABEL for="members">{t}Member objects{/t}</LABEL></b>&nbsp;({$combinedObjects})
   <br>
{render acl=$memberACL}
   <select style="width:100%; height:450px;" id="members" name="members[]" size="15" multiple>
    {$members}
   </select>
{/render}
   <br>
{render acl=$memberACL}
   <input type=submit value="{msgPool type=addButton}" name="edit_membership">&nbsp;
{/render}
{render acl=$memberACL}
   <input type=submit value="{msgPool type=delButton}" name="delete_membership">
{/render}
  </td>
 </tr>
</table>

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
  focus_field('cn');
  -->
</script>
