<h2>{t}Processing the requested operation{/t}</h2>
{$message}
<br>
<br>
<div style="margin:3px;background-color:white; border:1px solid #A0A0A0">
<iframe src="{$src}" style="width:100%;height:450px;background-color:#FFFFFF;" name="status">
	<p>{t}Your browser doesn't support iframes, please use this link to perform the requested operation.{/t}
	<br>
	<a href="{$src}">{$src}</a>
	</p>
</iframe>
</div>

<!--
<p class="seperator">&nbsp;</p>
<div style="text-align:right;">
	<p>
		<input type="submit" name="back" value="{msgPool type=backButton}">
	</p>
</div>
-->
