<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding=4 border=0>
 <!-- Headline container -->
 <tr>
   <td colspan=2>
     <h2><img alt="" class="center" align="middle" src="plugins/users/images/iconMini.png"> {t}Personal picture{/t}</h2>
   </td>
 </tr>
 <!-- Base container -->
 <tr>
 <!-- Image container -->
  <td>
   <table>
    <tr>
     <td style='width:150px; height:200px; background-color:gray; vertical-align: middle; text-align: center;'>
      <img  src="getbin.php?rand={$rand}" alt='' style='max-width:150px; max-height: 200px; vertical-align: middle;' >
     </td>
    </tr>
   </table>
  </td>
  </tr>
  <tr>
   <!-- Name, ... -->
   <td style="vertical-align:bottom; width:100%;">
     <input type="hidden" name="MAX_FILE_SIZE" value="2000000">
     <input id="picture_file" name="picture_file" type="file" size="20" maxlength="255" accept="image/*.jpg">
     &nbsp;
     <input type=submit name="picture_remove" value="{t}Remove picture{/t}">
   </td>
 </tr>
</table>
<br>
<p class="plugbottom">
  <input type=submit name="picture_edit_finish" value="{msgPool type=saveButton}">
  &nbsp;
  <input type=submit name="picture_edit_cancel" value="{msgPool type=cancelButton}">
</p>

