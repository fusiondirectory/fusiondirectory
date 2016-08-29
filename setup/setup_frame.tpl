<!-- Headline -->
<body class="setup">
  {$php_errors}
  <form action="setup.php" name="mainform" id="mainform" method="post" enctype="multipart/form-data">
    <div class="setup_header">
      <div id="header-left">
        <img id="fd_logo" class="optional" src="geticon.php?context=applications&amp;icon=fusiondirectory&amp;size=48" alt="FusionDirectory"/>
        <a class="plugtop">
          <img src="{$headline_image|escape}" alt="{t}{$headline}{/t}"/>{t}{$headline}{/t}
        </a>
      </div>
      <div id="header-right">
        <div class="version">
          {$version}
        </div>
      </div>
    </div>

    <table class="framework">
      <tbody>
        <tr>
          <!-- Menu -->
          <td id="menucell" class="setup-navigation">
            {$navigation}
            <br/>
          </td>

          <!-- Plugin window -->
          <td id="maincell">
            {$msg_dialogs}
            <div class="plugin-window">
              {$contents}
            </div>
          </td>
        </tr>
      </tbody>
    </table>

    {$errors}
    {$focus}
    <input type="hidden" name="setup_goto_step" value=""/>
  </form>

</body>
</html>
