<body>

  {$php_errors}

{* FusionDirectory login - smarty template *}

<div id="window-container">

<div id="window-div">
<form action="index.php" method="post" id="loginform" name="loginform">

{$msg_dialogs}
  <div id="window-titlebar">
    <img id="fd-logo" src="geticon.php?context=applications&amp;icon=fusiondirectory&amp;size=48" alt="FusionDirectory logo"/>
    <p>
      {t}Two factor authentication{/t}
    </p>
  </div>
  <div id="window-content">
    {foreach from=$methodOutputs key=method item=methodOutput}
      <div class="secondfactormethod" id="{$method|escape}">
        {$methodOutput}
      </div>
    {/foreach}
  </div>
  <div id="window-footer" class="plugbottom">
    <div>
      <!-- Display error message on demand -->
      {$message}
    </div>
    <div>
    </div>
  </div>

</form>
</div>

</div>

{include file={filePath file="copynotice.tpl"}}

<script type="text/javascript">
<!--
  next_msg_dialog();
-->
</script>
</body>
</html>
