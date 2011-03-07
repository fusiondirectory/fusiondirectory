<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding=4>
<tr>
  <td style="vertical-align:top;">
   <div class="contentboxh" style="height:20px;">
    <p class="contentboxh" style="font-size:12px">
     <b>{t}Select systems to add{/t} {$hint}</b><br>
    </p>
   </div>
   <div class="contentboxb">
    <p class="contentboxb" style="border-top:1px solid #B0B0B0;background-color:#F8F8F8">
     <select style="width:600px; margin-top:4px; height:450px;" name="wslist[]" size="15" multiple>
	{html_options values=$wslist output=$wslist}
     </select>
    </p>
   </div>
  </td>
  <td style="vertical-align:top;">
   <div class="contentboxh" style="height:20px;">
    <p class="contentboxh" style="font-size:12px"><img src="{$launchimage}" align="right" alt="[F]"><b>{t}Filters{/t}</b></p>
   </div>
   <div class="contentboxb" style="border-top:1px solid #B0B0B0;background-color:#F8F8F8">
     <table summary="" style="width:100%;">
      {$alphabet}
     </table>
    <table summary="" width="100%" style="border-top:1px solid #B0B0B0;background-color:#F8F8F8"><tr><td width="50%"><img alt="" src="{$tree_image}" align=middle>&nbsp;{t}Display systems of department{/t}</td>
    <td><select name="depselect" size=1 onChange="mainform.submit()" title="{t}Choose the department the search will be based on{/t}">
      {html_options options=$deplist selected=$depselect}
    </select></td></tr></table>
    <table summary="" width="100%" style="border-top:1px solid #B0B0B0;background-color:#F8F8F8"><tr><td width="50%"><img alt="" src="{$search_image}" align=middle>&nbsp;{t}Display systems matching{/t}</td>
    <td><input type='text' name='regex' maxlength='20' value='{$regex}' title='{t}Regular expression for matching addresses{/t}' onChange="mainform.submit()"></td></tr></table>
   {$apply}
   </div>
  </td>
</tr>
</table>

<p class="plugbottom">
  <input type=submit name="add_ws_finish" value="{msgPool type=addButton}">
  &nbsp;
  <input type=submit name="add_ws_cancel" value="{msgPool type=cancelButton}">
</p>
