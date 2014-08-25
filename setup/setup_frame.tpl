<body class='setup_body'>
{$php_errors}
<form action='setup.php' name='mainform' method='post' enctype='multipart/form-data'>
  {$msg_dialogs}
<div class='setup_dialog'>
  {include file={filePath file="setup_header.tpl"}}
  <div class='setup_menu'>
    <b>{t}FusionDirectory setup wizard{/t}</b>
  </div>

  <div style="padding:10px;background-color:white;">

    <div class='setup_navigation' id='menucell'>
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
</body>
</html>
