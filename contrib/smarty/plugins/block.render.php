<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

function smarty_block_render($params, $text, &$smarty)
{
  /* Skip closing tag </render> */
  if (empty($text)) {
    return '';
  }

  /* Get acl parameter */
  $acl = '';
  if (isset($params['acl'])) {
    $acl = $params['acl'];
  }

  /* Debug output */
  if (session::is_set('DEBUGLEVEL') && session::get('DEBUGLEVEL') & DEBUG_ACL ) {
    echo '<div style="color:blue;font-size:2;">&nbsp;'.$acl.'</div>';
  }

  /* Read / Write*/
  if (preg_match('/w/i', $acl)) {
    return $text;
  }

  $text = preg_replace ("/\n/", 'GOSA_LINE_BREAK', $text);

  /* Disable objects, but keep those active that have mode=read_active */
  if (!(isset($params['mode']) && ($params['mode'] == 'read_active') && preg_match('/(r|w)/', $acl))) {
    if (!preg_match('/ disabled(="disabled")?( |\/?>)/', $text)) {
      $text = preg_replace('/name=/i', 'disabled="disabled" name=', $text);
    }
  }

  /* Read only */
  if (preg_match('/r/i', $acl)) {
    return preg_replace('/GOSA_LINE_BREAK/', "\n", $text);
  }

  /* No acls */
  if (preg_match('/type[\'"= ].*submit/', $text)) {
    $text = preg_replace('/submit/', 'button', $text);
  } else {
    $text = preg_replace('/value=[\'"][^"\']*[\'"]/', '', $text);
  }

  /* Remove select options */
  $from = array(
    "#<option.*<\/option>#i",
    "/(<textarea.*>).*(<\/textarea>)/i",
    "/^(.*<input.*)checked(.*>.*)$/i"
  );

  $to = array(
    " ",
    "\\1\\2",
    "\\1 \\2"
  );

  $text = preg_replace($from, $to, $text);
  $text = preg_replace('/GOSA_LINE_BREAK/', "\n", $text);

  return $text;
}
?>
