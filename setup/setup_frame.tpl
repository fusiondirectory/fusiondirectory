<body class='setup_body'>
{$php_errors}
<form action='setup.php' name='mainform' method='post' enctype='multipart/form-data'>
  {$msg_dialogs}
<input id='focus' name='focus' type='image' src='images/empty.png' style='width:0px; height:0px;' >
<div class='setup_dialog'>
  {include file={filePath file="setup_header.tpl"}}
  <div class='setup_menu'>
    <b>{t}FusionDirectory setup wizard{/t}</b>
  </div>

  <div style="padding:10px;background-color:white;">

    <div class='setup_navigation'>
      <div style='padding:4px'>
        <img src='images/setup/fai.png' class='center' alt='{t}Installation{/t}'>&nbsp;&nbsp;<font style="font-size:24px;">{t}Setup{/t}</font>
      </div>
      {$navigation}
    </div>

    <div class='setup_contents'>
      <div style='border-bottom:1px dashed #AAAAAA'>
        {$errors}
        {$header}
      </div>
      <div style='padding-left:12px;'>
        {$contents}
        {$bottom}
        <br>
      </div>
    </div>
    <div style="clear:both">  </div>
  </div>
</div>


<input type='hidden' name='setup_goto_step' value=''>
</form>

<script type='javascript'>
  document.getElementById('focus').focus();
</script>
</body>
</html>
