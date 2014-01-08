<?php

function smarty_function_iconPath($params, &$smarty)
{
  $config = session::global_get('config');

  /* Set theme */
  $theme = IconTheme::$default_theme;
  if (isset ($config)) {
    $theme = $config->get_cfg_value("theme");
  }

  return IconTheme::findThemeIcon($theme, $params['context'], $params['icon'], $params['size']);
}
?>
