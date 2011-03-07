<body style='background-color:#F1F1F1;background-image:none;'>
{$php_errors}

<form action='helpviewer.php' name='mainform' method='post' enctype='multipart/form-data'>
<div style='background-color:#CDF6BD;width:100%;border-bottom:1px solid #909090;'>
  <table summary="" style="width:99%;">
   <tr>
    <td style="width:33%;">
     <b>{t}GOsa help viewer{/t}</b>
	</td>
    <td style="width:33%;text-align:center">
		{$backward}
		&nbsp;&nbsp;
		<a href="?pg={$index}"  class="maintitlebar">
			{t}Index{/t}
		</a>
		&nbsp;&nbsp;
		{$forward}
	</td>
	<td style="text-align:right;">
	 <input type='text' name="search_string" size="15" value="{$search_string}" maxlength="50">&nbsp;<input type="submit" name="search" value="{t}Search{/t}">
	</td>
   </tr>
  </table>
</div>
<div style="height:100%;width:98%;padding:5px;margin:0px;background-color:#F1F1F1;">
{$help_contents}
</div>

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
<!-- // First input field on page
  focus_field('search_string');
-->
</script>

</form>
</body>
</html>
