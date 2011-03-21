<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011  FusionDirectory

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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

session_cache_limiter("private");

/* Check for parameter completenes */
if (!isset($_GET['x']) || !isset($_GET['y']) || !isset($_GET['p'])){
  die ("Missing parameters!");
}
if (!is_numeric($_GET['x']) || !is_numeric($_GET['y'])){
  die ("Parameters must be numeric!");
}

$p= (int)($_GET['p']);
$x= (int)($_GET['x']);
$y= (int)($_GET['y']);

/* Check percentage */
if ($p < 0){
  $p= 0;
} elseif ($p > 100){
  $p= 100;
}
$p= intval ($p);

/* Check dimensions */
if ($x < 3 || $x > 1000){
  $x= 180;
}
if ($y < 3 || $y > 700){
  $y= 20;
}

if(!function_exists("imagecreate")){
  syslog(LOG_ERR, "FusionDirectory is missing the gd library, please install php5-gd to be able to see progress images.");
  echo "Please install the php5-gd library, FusionDirectory can't create images without it.";
  exit();
}else{

  $x_matches= FALSE;
  $y_matches= FALSE;
  foreach (array(7,6,5,4,3,2,1,0) as $font){
    $fx= ImageFontWidth($font) * strlen("$p%");
    $fy= ImageFontHeight($font);

    /* Look if font size matches image size */
    if ($fx < ($x-2)){
      $x_matches= TRUE;
    }
    if ($fy < ($y-2)){
      $y_matches= TRUE;
    }
    if ($x_matches && $y_matches){
      break;
    }
  }

  /* Draw image in GD image stream */
  $im = imagecreate ($x, $y)
    or die ("Cannot Initialize new GD image stream");

  /* Set colors */
  $bg_color= imagecolorallocate ($im, 255, 255, 255);
  $br_color= imagecolorallocate ($im, 0,0,0);
  $fi_color= imagecolorallocate ($im, 0,0,180);
  $tx_color= imagecolorallocate ($im, 240, 10, 90);

  /* Draw progress bar */
  imagerectangle ($im, 0, 0, $x-1, $y-1, $br_color);
  imagefilledrectangle ($im, 1, 1, (($x - 2) * $p / 100),
      $y - 2, $fi_color);

  /* Is font to big for progress bar? */
  if ($font != 0){
    imagestring ($im, $font, ($x - $fx) / 2, ($y - $fy) / 2, "$p%", $tx_color);
  }

  /* Finally draw the image and remove context */
  header ("Content-type: image/png");
  imagepng ($im);
  imagedestroy ($im);
}
?>
