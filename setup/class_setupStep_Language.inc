<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007  Fabian Hickert
  Copyright (C) 2011-2013  FusionDirectory

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


class Step_Language extends setup_step
{
  var $lang_selected  = "";       /* Language selected from select box  */
  var $lang           = "en_EN";  /* Detected language */
  var $languages      = array();
  var $attributes     = array("lang","lang_selected");
  var $header_image   = 'geticon.php?context=applications&icon=config-language&size=48';

  function __construct()
  {
    $this->lang           = get_browser_language();
    $this->lang_selected  = "";
    $this->update_strings();
  }

  function update_strings()
  {
    $this->s_title      = _("Language setup");
    $this->s_title_long = _("Language setup");
    $this->s_info       = _("This step allows you to select your preferred language.");
  }

  function execute()
  {
    $this->languages = get_languages(TRUE);
    asort($this->languages);
    $this->languages = array_merge(array("" => _("Automatic")), $this->languages);
    $smarty = get_smarty();
    $smarty->assign("languages",      $this->languages);
    $smarty->assign("lang",           $this->lang);
    $smarty->assign("rtl",            language_is_rtl($this->lang));
    $smarty->assign("lang_selected",  $this->lang_selected);
    return $smarty->fetch(get_template_path("setup_language.tpl", TRUE, dirname(__FILE__)));
  }

  function save_object()
  {
    if ((isset($_POST['lang_selected'])) && isset($this->languages[$_POST['lang_selected']])) {
      $this->lang_selected  = $_POST['lang_selected'];
      $this->is_completed   = TRUE;

      if ($this->lang_selected != "") {
        session::set('lang', $this->lang_selected);
      } else {
        session::set('lang', $this->lang);
      }
    }
  }
}

?>
