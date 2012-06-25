<?php
/*
 * accept-to-gettext.inc -- convert information in 'Accept-*' headers to
 * gettext language identifiers.
 * Copyright (c) 2003, Wouter Verhelst <wouter@debian.org>
 * Copyright (c) 2012, FusionDirectory project <contact@fusiondirectory.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Usage:
 *
 *  $locale=al2gt(<array of supported languages/charsets in gettext syntax>,
 *                <MIME type of document>);
 *  setlocale('LC_ALL', $locale); // or 'LC_MESSAGES', or whatever...
 *
 * Example:
 *
 *  $langs=array('nl_BE.ISO-8859-15','nl_BE.UTF-8','en_US.UTF-8','en_GB.UTF-8');
 *  $locale=al2gt($langs, 'text/html');
 *  setlocale('LC_ALL', $locale);
 *
 * Note that this will send out header information (to be
 * RFC2616-compliant), so it must be called before anything is sent to
 * the user.
 *
 * Assumptions made:
 * * Charset encodings are written the same way as the Accept-Charset
 *   HTTP header specifies them (RFC2616), except that they're parsed
 *   case-insensitive.
 * * Country codes and language codes are the same in both gettext and
 *   the Accept-Language syntax (except for the case differences, which
 *   are dealt with easily). If not, some input may be ignored.
 * * The provided gettext-strings are fully qualified; i.e., no "en_US";
 *   always "en_US.ISO-8859-15" or "en_US.UTF-8", or whichever has been
 *   used. "en.ISO-8859-15" is OK, though.
 * * The language is more important than the charset; i.e., if the
 *   following is given:
 *
 *   Accept-Language: nl-be, nl;q=0.8, en-us;q=0.5, en;q=0.3
 *   Accept-Charset: ISO-8859-15, utf-8;q=0.5
 *
 *   And the supplied parameter contains (amongst others) nl_BE.UTF-8
 *   and nl.ISO-8859-15, then nl_BE.UTF-8 will be picked.
 *
 * $Log: accept-to-gettext.inc,v $
 * Revision 1.1.1.1  2003/11/19 19:31:15  wouter
 * * moved to new CVS repo after death of the old
 * * Fixed code to apply a default to both Accept-Charset and
 *   Accept-Language if none of those headers are supplied; patch from
 *   Dominic Chambers <dominic@encasa.com>
 *
 * Revision 1.2  2003/08/14 10:23:59  wouter
 * Removed little error in Content-Type header syntaxis.
 *
 * Revision 2012/06/07 Côme BERNIGAUD
 * Rewrote a lot of things, use functions and PHP features in order to be more efficient and more readable
 */

/*!
 * \brief Parse a string like nl-be, nl;q=0.8, en-us;q=0.5, en;q=0.3
 * into an array like array('nl-be'=>1,'nl'=>0.8,'en-us'=>0.5,'en'=>0.3)
 */
function parse_scores ($str)
{
  $scores = array();
  $parts = preg_split('/,/', $str);
  foreach ($parts as $part) {
    $part = trim(strtolower($part));
    if (preg_match("/(.*);q=(.*)/", $part, $matches)) {
      $scores[$matches[1]] = $matches[2];
    } else {
      $scores[$part] = 1;
    }
  }
  return $scores;
}

/*!
 * \brief Compute the maximum value in scores array, between keys from testvals array.
 * example: max_scores(array('a'=>1,'b'=>2,'c'=>3,'d'=>4,'e'=>5),array('a','d','f')) returns 4
 * if no key from testvals is found, returns 0
 */
function max_scores ($scores, $testvals)
{
  $values = array_intersect_key($scores, array_flip($testvals));
  if (empty($values)) {
    return 0;
  } else {
    return max($values);
  }
}

/*!
 * \brief Parse a lang in gettext format and returns lang, country and charset.
 * example: en_US.utf-8 will return ('en','us','utf-8')
 */
function parse_gettext_lang ($str)
{
  if (preg_match("/^([^_]*)(_([^_]*))?\.(.*)$/", $str, $m)) {
    return array(strtolower($m[1]), strtolower($m[3]), strtolower($m[4]));
  } else {
    return FALSE;
  }
}

function al2gt($gettextlangs, $mime)
{
  /* Check if ACCEPT_LANGUAGE isset */
  if (empty($_SERVER["HTTP_ACCEPT_LANGUAGE"])) {
    /* default to "everything is acceptable", as RFC2616 specifies */
    $acceptLang = '*';
  } else {
    $acceptLang = $_SERVER["HTTP_ACCEPT_LANGUAGE"];
  }
  if (empty($_SERVER["HTTP_ACCEPT_CHARSET"])) {
    /* default to "everything is acceptable", as RFC2616 specifies */
    $acceptChar = 'ISO-8859-1,*;q=0.5';
  } else {
    $acceptChar = $_SERVER["HTTP_ACCEPT_CHARSET"];
  }

  $lang_scores = parse_scores($acceptLang);
  $char_scores = parse_scores($acceptChar);

  /* RFC2616: ``If no "*" is present in an Accept-Charset field, then
   * all character sets not explicitly mentioned get a quality value of
   * 0, except for ISO-8859-1, which gets a quality value of 1 if not
   * explicitly mentioned.'' */
  if (!isset($char_scores['ISO-8859-1']) && !isset($char_scores['*'])) {
    $char_scores['ISO-8859-1'] = 1;
  }

  /* Loop through the available languages/encodings, and pick the one
   * with the highest score, excluding the ones with a charset the user
   * did not include. */
  $max_lang     = NULL; // The lang with the maximum score found
  $max_l_score  = 0;    // The lang score of this lang
  $max_c_score  = 0;    // The char score of this lang
  foreach ($gettextlangs as $gtlang) {
    if (!($infos = parse_gettext_lang($gtlang))) {
      continue;
    }
    list ($lang, $country, $char) = $infos;

    $lang_score = max_scores($lang_scores, array($lang,$lang.'-'.$country,'*'));
    $char_score = max_scores($char_scores, array($char,'*'));
    if ($char_score == 0) {
      continue; // exclude charsets the user refuses
    }

    if   (($lang_score > $max_l_score)
      || (($lang_score == $max_l_score) && ($char_score > $max_c_score))) { // if lang scores are equals we compare char scores
      $max_l_score  = $lang_score;
      $max_c_score  = $char_score;
      $max_lang     = $gtlang;
    }
  }

  if ($max_lang === NULL) {
    return NULL;
  }

  /* We must re-parse the gettext-string now, since we may have found it
   * through a "*" qualifier.*/
  list ($lang, $country, $char) = parse_gettext_lang($max_lang);
  if (!headers_sent()) {
    header("Content-Language: $lang".(empty($country)?"":"-$country"));
    if (!empty($char)) {
      header("Content-Type: $mime; charset=$char");
    }
  }
  return $max_lang;
}
?>
