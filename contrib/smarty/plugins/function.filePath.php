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
  $dir = $smarty->template_dir;
  if (is_array($dir)) {
    $dir = $dir[0];
  }
  if (file_exists("$dir/themes/$theme/$filename")) {
    return "$dir/themes/$theme/$filename";
  }
  if (file_exists("themes/default/$filename")) {
    return "themes/default/$filename";
  }
  if (file_exists("$dir/themes/default/$filename")) {
    return "$dir/themes/default/$filename";
  }
  return $filename;
}
?>
