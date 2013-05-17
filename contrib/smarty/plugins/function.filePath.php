<?php

function smarty_function_filePath($params, &$smarty)
{
  $config = session::global_get('config');

  /* Set theme */
  if (isset ($config)) {
    $theme = $config->get_cfg_value("theme", "default");
  } else {
    $theme = "default";
  }

  $filename = $params['file'];

  if (file_exists("themes/$theme/$filename")) {
    return "themes/$theme/$filename";
  }
  if (file_exists($smarty->template_dir."/themes/$theme/$filename")) {
    return $smarty->template_dir."/themes/$theme/$filename";
  }
  if (file_exists("themes/default/$filename")) {
    return "themes/default/$filename";
  }
  if (file_exists($smarty->template_dir."/themes/default/$filename")) {
    return $smarty->template_dir."/themes/default/$filename";
  }
  return $filename;
}
?>
