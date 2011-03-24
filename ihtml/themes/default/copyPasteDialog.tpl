<h2>{t}Copy & paste wizard{/t}</h2>

<b>{$message}</b>
<br>
<br>
{if $Complete == false}
	{t}Some values need to be unique in the complete directory while some combinations make no sense. FusionDirectory shows the relevant attributes. Please maintain the values below to fulfill the policies.{/t}
	<br>
{t}Remember that some properties like taken snapshots will not be copied!{/t}&nbsp;
{t}Or if you copy or cut an entry within FusionDirectory and delete the source object, you may get errors while pasting this object again!{/t}

	<p class='seperator'>&nbsp;</p>
	<br>
	{$AttributesToFix}
	{if $SubDialog == false}
	<br>

	<div style='text-align:right;width:100%;'>
		<input type='submit' name='PerformCopyPaste' value='{t}Save{/t}'>&nbsp;
	{if $type == "modified"}
		<input type='submit' name='abort_current_cut-copy_operation' value='{t}Cancel{/t}'>
	{/if}
	
		<input type='submit' name='abort_all_cut-copy_operations' value='{t}Cancel all{/t}'>
	</div>
	{/if}
{else}
	<p class='seperator'>&nbsp;</p>
	<br>
	<h2>{t}Operation complete{/t}</h2>
	<div style='text-align:right;width:100%;'>
		<input type='submit' name='Back' value='{t}Finish{/t}'>&nbsp;
	</div>
{/if}
