{if !$acl_readable}

	<h2>{msgPool type=permView}</h2>

{else}
	{if $dialogState eq 'head'}
	<h1>{t}Assigned ACL for current entry{/t}</h1>
	{$aclList}


	{if $acl_createable}
	<input type="submit" name="new_acl" value="{t}New ACL{/t}" >
	{/if}
	{/if}

	{if $dialogState eq 'create'}
	<h1>{t}ACL type{/t}
		{if !$acl_writeable}
			<select size="1" name="dummy_t" title="{t}Select an acl type{/t}" disabled>
				{html_options options=$aclTypes selected=$aclType}
				<option disabled>&nbsp;</option>
			</select>&nbsp;
		{else} 
			<select size="1" name="aclType" title="{t}Select an acl type{/t}" onChange="document.mainform.submit()">
				{html_options options=$aclTypes selected=$aclType}
				<option disabled>&nbsp;</option>
			</select>&nbsp;
			{if $javascript eq 'false'}
				<input type="submit" value="{t}Apply{/t}" name="refresh">
			{/if}
		{/if}
	</h1>

	<p class="seperator">&nbsp;</p>
	<div style='padding:3px;'>
	{t}Additional filter options{/t}&nbsp; 
		{if !$acl_writeable}
		<input type='text' value='{$aclFilter}' disabled name='dummy_f' style='width:600px;'>
		{else}
		<input type='text' value='{$aclFilter}' name='aclFilter' style='width:600px;'>
		{/if}
	</div>

	<table style="width:100%">
	 <tr>
	  <td style="width:48%; vertical-align:top;">
	   {t}Use members from{/t}
	   <select name="target" onChange="document.mainform.submit()">
			{html_options options=$targets selected=$target}
			<option disabled>&nbsp;</option>
	   </select>
	   {if $javascript eq 'false'}<input type="submit" value="{t}Apply{/t}" name="refresh">{/if}<br><br>
	  </td>
	  <td>
	  </td>
	  <td>
	  </td>
	 </tr>
	 <tr>
	  <td style="width:48%">
	   {t}Available members{/t}<br>
		{if !$acl_writeable}
	   <select style="width:100%;height:180px;" disabled name="dummy_s[]" size="20" multiple title="{t}List message possible targets{/t}">
				{html_options options=$sources}
				<option disabled>&nbsp;</option>
	   </select>
		{else}
	   <select style="width:100%;height:180px;" name="source[]" size="20" multiple title="{t}List message possible targets{/t}">
				{html_options options=$sources}
				<option disabled>&nbsp;</option>
	   </select>
		{/if}
	  </td>
	  <td style="vertical-align:center; text-align:center">
		{if $acl_writeable}
	   <input type="submit" value="&gt;" name="add">
	   <br><br>
	   <input type="submit" value="&lt;" name="del">
		{/if}
	  </td>
	  <td style="width:48%; vertical-align:top;">
	   {t}Members{/t}<br>
		{if !$acl_writeable}
	   <select style="width:100%;height:180px;" disabled name="dummy_r[]" size="20" multiple title="{t}List message recipients{/t}">
				{html_options options=$recipients}
				<option disabled>&nbsp;</option>
	   </select>

		{else}
	   <select style="width:100%;height:180px;" name="recipient[]" size="20" multiple title="{t}List message recipients{/t}">
				{html_options options=$recipients}
				<option disabled>&nbsp;</option>
	   </select>
		{/if}
	  </td>
	 </tr>
	</table>

	{if $aclType ne 'reset'}
	{if $aclType ne 'role'}
	{if $aclType ne 'base'}
	<p class="seperator">&nbsp;</p>

	<h1>{t}List of available ACL categories{/t}</h1>
	{$aclList}
	{/if}
	{/if}
	{/if}

	{if $aclType eq 'base'}
	<p class="seperator">&nbsp;</p>
	<h1>{t}ACL for this object{/t}</h1>
	{$aclSelector}
	{/if}

	{if $aclType eq 'role'}
	<p class="seperator">&nbsp;</p>
	<h1>{t}Available roles{/t}</h1>
	{$roleSelector}
	{/if}

	<p class="seperator">&nbsp;</p>
	<div style='text-align:right;margin-top:5px'>
		{if $acl_writeable}
		<input type="submit" name="submit_new_acl" value="{t}Apply{/t}">
		&nbsp;
		{/if}
		<input type="submit" name="cancel_new_acl" value="{t}Cancel{/t}">
	</div>
	{/if}

	{if $dialogState eq 'edit'}

	<h1>{$headline}</h1>

	{$aclSelector}

	<p class="seperator">&nbsp;</p>
	<div style='text-align:right;margin-top:5px'>
		<input type="submit" name="submit_edit_acl" value="{t}Apply{/t}">
		&nbsp;
		<input type="submit" name="cancel_edit_acl" value="{t}Cancel{/t}">
	</div>
	{/if}
{/if}
