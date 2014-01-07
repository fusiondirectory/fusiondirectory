<?php

function smarty_function_iconPath($params, &$smarty)
{
  $config = session::global_get('config');

  /* Set theme */
  $theme = "default";
  if (isset ($config)) {
    $theme = $config->get_cfg_value("theme", "default");
    if (!isset(IconTheme::$themes[$theme])) {
      $theme = 'default';
    }
  }

  return IconTheme::$themes[$theme]->FindIcon($params['context'], $params['icon'], $params['size']);
}
?>
