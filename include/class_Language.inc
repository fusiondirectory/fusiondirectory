<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2017-2018  FusionDirectory

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

/*!
 * \file class_Language.inc
 * Source code for the class Language
 */

/*!
 * \brief This class contains all the function needed to manage languages
 */
class Language
{
  /*!
   * \brief Initialize language configuration
   *
   * \param string $lang Language locale to use, defaults to self::detect()
   */
  public static function init ($lang = NULL)
  {
    global $BASE_DIR;

    if ($lang === NULL) {
      $lang = static::detect();
    }

    list ($language, , $char) = parse_gettext_lang($lang);
    putenv('LANGUAGE=');
    putenv("LANG=$lang");
    $langset = setlocale(LC_ALL, $lang, $language.'.'.$char);
    if ($langset === FALSE) {
      trigger_error('Setting locale to '.$lang.' failed');
    } elseif ($langset != $lang) {
      trigger_error('Setting locale to '.$lang.' failed, fell back to '.$langset);
    }
    $GLOBALS['t_language']            = $lang;
    $GLOBALS['t_gettext_message_dir'] = $BASE_DIR.'/locale/';
    static::setHeaders($lang, 'text/html');

    /* Set the text domain as 'fusiondirectory' */
    $domain = 'fusiondirectory';
    bindtextdomain($domain, LOCALE_DIR);
    textdomain($domain);
    if ($_SERVER['REQUEST_METHOD'] != 'POST') {
      @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $lang, 'Setting language to');
    }

    $ret = FALSE;

    /* Reset plist cache if language changed */
    if ((!session::is_set('lang')) || (session::get('lang') != $lang)) {
      $ret = TRUE;
      if (session::is_set('plist')) {
        if ($_SERVER['REQUEST_METHOD'] != 'POST') {
          @DEBUG(DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, session::get('lang'), 'Plist already loaded with language');
        }
        session::global_un_set('plist');
        session::set('lang', $lang);
        pluglist::load();
      }
    }

    session::set('lang', $lang);
    return $ret;
  }

  /*!
   * \brief Determine which language to show to the user
   *
   * Determines which language should be used to present fusiondirectory content
   * to the user. It does so by looking at several possibilites and returning
   * the first setting that can be found.
   *
   * -# Language configured by the user
   * -# Global configured language
   * -# Language as returned by al2gt (as configured in the browser)
   *
   * \return string gettext locale string
   */
  public static function detect ()
  {
    global $config;

    /* Try to use users primary language */
    $ui = get_userinfo();
    if (isset($ui) && ($ui !== NULL) && ($ui->language != '')) {
      return $ui->language.'.UTF-8';
    }

    /* Check for global language settings in configuration */
    if (isset($config) && ($config->get_cfg_value('language') != '')) {
      $lang = $config->get_cfg_value('language');
      if (!preg_match('/utf/i', $lang)) {
        $lang .= '.UTF-8';
      }
      return $lang;
    }

    /* Load supported languages */
    $languages = static::getList();

    /* Move supported languages to flat list */
    $langs = [];
    foreach (array_keys($languages) as $lang) {
      $langs[] = $lang.'.UTF-8';
    }

    /* Return gettext based string */
    return al2gt($langs);
  }

  /*!
   * \brief Get the language for the user connecting
   *
   * \param boolean $ownLanguage Should language names be stated in their own language as well
   */
  public static function getList ($ownLanguage = FALSE)
  {
    /* locales in english */
    $tmp_english = [
      'en_US' => 'English',
      'af_ZA' => 'Afrikaans',
      'ar_EG' => 'Arabic',
      'ca_ES' => 'Catalan',
      'cs_CZ' => 'Czech',
      'de_DE' => 'German',
      'el_GR' => 'Greek',
      'es_CO' => 'Colombian Spanish',
      'es_ES' => 'Spanish',
      'es_VE' => 'Venezuelan',
      'fa_IR' => 'Persian',
      'fi_FI' => 'Finnish',
      'fr_FR' => 'French',
      'hu_HU' => 'Hungarian',
      'id_ID' => 'Indonesian',
      'it_IT' => 'Italian',
      'ja_JP' => 'Japanese',
      'ko_KR' => 'Korean',
      'lv_LV' => 'Latvian',
      'nb_NO' => 'Norwegian Bokmål',
      'nl_NL' => 'Dutch',
      'pl_PL' => 'Polish',
      'pt_BR' => 'Brazilian',
      'pt_PT' => 'Portuguese',
      'ru_RU' => 'Russian',
      'sv_SE' => 'Swedish',
      'tr_TR' => 'Turkish',
      'vi_VN' => 'Vietnamese',
      'zh_TW' => 'Taiwanese',
      'zh_CN' => 'Chinese',
    ];

    $ret = [];
    if ($ownLanguage) {
      /* locales in their own language */
      $tmp_ownlang = [
        'en_US' => 'English',
        'ar_EG' => 'عربية',
        'af_ZA' => 'Afrikaans',
        'ca_ES' => 'Català',
        'cs_CZ' => 'Česky',
        'de_DE' => 'Deutsch',
        'el_GR' => 'ελληνικά',
        'es_CO' => 'Español Colombiano',
        'es_ES' => 'Español',
        'es_VE' => 'Castellano',
        'fa_IR' => 'پارسی',
        'fi_FI' => 'Suomi',
        'fr_FR' => 'Français',
        'hu_HU' => 'Magyar',
        'id_ID' => 'Bahasa Indonesia',
        'it_IT' => 'Italiano',
        'ja_JP' => '日本語',
        'ko_KR' => '한국어',
        'lv_LV' => 'Latviešu valoda',
        'nb_NO' => 'Norsk bokmål',
        'nl_NL' => 'Nederlands',
        'pl_PL' => 'Polski',
        'pt_BR' => 'Português (Brasil)',
        'pt_PT' => 'Português',
        'ru_RU' => 'русский язык',
        'sv_SE' => 'Svenska',
        'tr_TR' => 'Türkçe',
        'vi_VN' => 'Tiếng Việt',
        'zh_TW' => 'Taiwanese',
        'zh_CN' => '中文, 汉语, 漢語',
      ];

      foreach ($tmp_english as $key => $name) {
        $ret[$key] = _($name).' ('.$tmp_ownlang[$key].')';
      }
    } else {
      foreach ($tmp_english as $key => $name) {
        $ret[$key] = _($name);
      }
    }

    return $ret;
  }

  /*!
   * \brief Returns TRUE if $lang is a right to left language
   */
  public static function isRTL ($lang)
  {
    return preg_match('/^(fa_|ar_)/', $lang);
  }

  public static function setHeaders ($language, $mime)
  {
    list ($lang, $country, $char) = parse_gettext_lang($language);

    if (!headers_sent()) {
      header("Content-Language: $lang".(empty($country) ? '' : "-$country"));
      if (!empty($char) && preg_match('|^text/|', $mime)) {
        header("Content-Type: $mime; charset=$char");
      } else {
        header("Content-Type: $mime");
      }
    } else {
      trigger_error('Could not set language '.$lang.' header, headers already sent');
    }
  }
}
