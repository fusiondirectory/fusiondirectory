<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>

<head>
  <title>{if isset($title)}{$title}{else}FusionDirectory{/if}</title>

  <meta name="generator" content="my hands">
  <meta name="description" content="FusionDirectory - Login">
  <meta name="author" lang="en" content="FusionDirectory Project">
  <meta name="viewport" content="width=device-width">

  <meta http-equiv="Expires" content="Mon, 26 Jul 1997 05:00:00 GMT">
  <meta http-equiv="Last-Modified" content="{$date} GMT">
  <meta http-equiv="Cache-Control" content="no-cache">
  <meta http-equiv="Pragma" content="no-cache">
  <meta http-equiv="Cache-Control" content="post-check=0, pre-check=0">
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">

  {include file={filePath file="css.tpl"}}
  <link rel="stylesheet" type="text/css" href="{filePath file="printer.css"}" media="print">

  <link rel="shortcut icon" href="favicon.ico">

  <script language="javascript" src="include/prototype.js" type="text/javascript"></script>
  <script language="javascript" src="include/fusiondirectory.js" type="text/javascript"></script>
{if $usePrototype == 'true'}
  <script language="javascript" src="include/scriptaculous.js" type="text/javascript"></script>
  <script language="javascript" src="include/builder.js" type="text/javascript"></script>
  <script language="javascript" src="include/effects.js" type="text/javascript"></script>
  <script language="javascript" src="include/dragdrop.js" type="text/javascript"></script>
  <script language="javascript" src="include/controls.js" type="text/javascript"></script>
  <script language="javascript" src="include/pulldown.js" type="text/javascript"></script>
  <script language="javascript" src="include/datepicker.js" type="text/javascript"></script>
{/if}
</head>

