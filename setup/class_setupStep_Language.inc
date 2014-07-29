<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007  Fabian Hickert
  Copyright (C) 2011-2015  FusionDirectory

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

class Step_Language extends setupStep
{
  var $header_image = 'geticon.php?context=applications&icon=config-language&size=48';

  static function getAttributesInfo()
  {
    return array(
      'lang' => array(
        'name'      => _('Preferred language'),
        'class'     => array('fullwidth', 'setup_language'),
        'attrs'     => array(
          new DisplayAttribute(
            '', '',
            'msg', FALSE
          ),
          new SelectAttribute(
            '', '',
            'lang_selected', FALSE
          ),
          new HiddenAttribute(
            'lang', FALSE,
            'en_EN'
          )
        )
      )
    );
  }

  function __construct()
  {
    parent::__construct();
    $this->lang = get_browser_language();
    $this->attributesAccess['lang_selected']->setSize(20);
  }

  function update_strings()
  {
    $this->s_title      = _("Language setup");
    $this->s_title_long = _("Language setup");
    $this->s_info       = _("This step allows you to select your preferred language.");
    $this->msg          = _('At this point, you can select the site wide default language. Choosing "automatic" will use the language requested by the browser. This setting can be overriden per user.');
  }

  function execute()
  {
    $languages = get_languages(TRUE);
    asort($languages);
    $languages = array_merge(array("" => _("Automatic")), $languages);
    $this->attributesAccess['lang_selected']->setChoices(array_keys($languages), array_values($languages));
    return parent::execute();
  }

  function save_object()
  {
    parent::save_object();
    $this->is_completed   = TRUE;

    if ($this->lang_selected != "") {
      session::set('lang', $this->lang_selected);
    } else {
      session::set('lang', $this->lang);
    }
  }
}

?>
