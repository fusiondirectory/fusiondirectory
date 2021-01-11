<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
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

/*!
 * \file class_templateHandling.inc
 * Source code for the class templateHandling
 */

/*! \brief this class stores static methods used to parse templates LDAP data
 */
class templateHandling
{
  /*! \brief Fetch a template from LDAP and returns its attributes and dependencies information */
  public static function fetch ($dn)
  {
    global $config;

    $ldap = $config->get_ldap_link();
    $ldap->cat($dn);
    $attrs    = $ldap->fetch(TRUE);
    $attrs    = static::fieldsFromLDAP($attrs);
    list($depends, $errors) = static::attributesDependencies($attrs);
    msg_dialog::displayChecks($errors);
    $attrs    = static::sortAttributes($attrs, $depends);
    return [$attrs, $depends];
  }

  /*! \brief Translate template attrs into $attrs as if taken from LDAP */
  public static function fieldsFromLDAP (array $template_attrs)
  {
    $attrs = [];
    if (isset($template_attrs['fdTemplateField'])) {
      unset($template_attrs['fdTemplateField']['count']);
      sort($template_attrs['fdTemplateField']);
      foreach ($template_attrs['fdTemplateField'] as $field) {
        if (!preg_match('/^([^:]+):(.*)$/s', $field, $m)) {
          throw new FusionDirectoryException('Template field does not match format');
        }
        if (isset($attrs[$m[1]])) {
          $attrs[$m[1]][] = $m[2];
          $attrs[$m[1]]['count']++;
        } else {
          $attrs[$m[1]]           = [$m[2]];
          $attrs[$m[1]]['count']  = 1;
        }
      }
    }
    return $attrs;
  }

  /*! \brief Translate $attrs into template attrs */
  public static function fieldsToLDAP (array $template_attrs, array $attrs)
  {
    /* First a bit of cleanup */
    unset($template_attrs['dn']);
    unset($template_attrs['fdTemplateField']['count']);
    unset($template_attrs['objectClass']['count']);
    unset($template_attrs['cn']['count']);
    if (isset($template_attrs['count'])) {
      for ($i = 0; $i < $template_attrs['count']; ++$i) {
        /* Remove numeric keys */
        unset($template_attrs[$i]);
      }
    }
    unset($template_attrs['count']);

    /* Remove all concerned values */
    foreach ($template_attrs['fdTemplateField'] as $key => $value) {
      preg_match('/^([^:]+):(.*)$/s', $value, $m);
      if (isset($attrs[$m[1]])) {
        unset($template_attrs['fdTemplateField'][$key]);
      }
    }
    /* Then insert non-empty values */
    foreach ($attrs as $key => $value) {
      if (is_array($value)) {
        foreach ($value as $v) {
          if ($value == "") {
            continue;
          }
          $template_attrs['fdTemplateField'][] = $key.':'.$v;
        }
      } else {
        if ($value == "") {
          continue;
        }
        $template_attrs['fdTemplateField'][] = $key.':'.$value;
      }
    }
    sort($template_attrs['fdTemplateField']);
    return $template_attrs;
  }

  /*! \brief Check template fields
   *
   * Returns errors if there are recursive dependencies.
   * Might check more things later
   */
  public static function checkFields ($attrs)
  {
    list(, $errors) = static::attributesDependencies($attrs);
    return $errors;
  }

  /*! \brief Parse a mask (without surrounding %) using $attrs attributes and apply modifiers
   *
   * \return iterable an array or iterable object of possible results
   */
  public static function parseMask (string $mask, array $attrs)
  {
    if ($mask == '|') {
      return ['%'];
    }
    $modifiers = '';
    if (preg_match('/^([^|]+)\|/', $mask, $m)) {
      $modifiers = $m[1];
      $mask = substr($mask, strlen($m[0]));
    }
    $result = [];
    if (isset($attrs[$mask])) {
      $result = $attrs[$mask];
      if (is_array($result)) {
        unset($result['count']);
        if (empty($result)) {
          /* No value and empty value is the same in LDAP so we have to treat them the same */
          $result = [''];
        }
      } else {
        $result = [$result];
      }
    } elseif (($mask != '') && !preg_match('/c/', $modifiers)) {
      throw new FusionDirectoryException(sprintf(_('"%s" was not found in attributes'), $mask));
    }
    $len    = strlen($modifiers);
    for ($i = 0; $i < $len; ++$i) {
      $args     = [];
      $modifier = $modifiers[$i];
      if (preg_match('/^\[([^\]]+)\].*$/', substr($modifiers, $i + 1), $m)) {
        /* get modifier args */
        $args = explode(',', $m[1]);
        $i += strlen($m[1]) + 2;
      }
      $result = static::applyModifier($modifier, $args, $result);
    }
    return $result;
  }

  /*! \brief Return attrs needed before applying template
   *
   * \return array An array of attributes which are needed by the template
   */
  public static function neededAttrs (array &$attrs, array $flatdepends)
  {
    $needed = [];
    foreach ($flatdepends as $attr => $depends) {
      if ((isset($depends[0])) && ($depends[0] == 'askme')) {
        $needed[] = $attr;
        unset($flatdepends[$attr]);
        unset($attrs[$attr]);
      }
    }
    $dependencies = array_unique(array_merge(...array_values($flatdepends)));
    foreach ($dependencies as $attr) {
      if (empty($flatdepends[$attr])) {
        $needed[] = $attr;
      }
    }
    return array_unique($needed);
  }

  /*! \brief Parse template masks in an array
   *
   * \param array $attrs        The attributes in LDAP format
   * \param array $specialAttrs Some additional fake attributes which can be used in masks
   * \param string $target      Dn of an object to ignore in the unicity check
   *
   * \return array An array with the final values of attributes
   */
  public static function parseArray (array $attrs, array $specialAttrs, $target = NULL)
  {
    foreach ($attrs as $name => &$attr) {
      if (is_array($attr)) {
        foreach ($attr as $key => &$string) {
          if (!is_numeric($key)) {
            continue;
          }
          $string = static::parseString($string, array_merge($attrs, $specialAttrs), NULL, $name, $target);
        }
        unset($string);
      }
    }
    unset($attr);
    return $attrs;
  }

  /*! \brief Parse template masks in a single string
   *
   * \param string $string          The mask
   * \param array $attrs            The attributes in LDAP format
   * \param callable $escapeMethod  Method to call to escape mask result
   * \param string $unique          Name of the LDAP attribute to check unicity on, if any
   * \param string $target          Dn of an object to ignore in the unicity check
   *
   * \return string the string with patterns replaced by their values
   */
  public static function parseString (string $string, array $attrs, $escapeMethod = NULL, string $unique = NULL, string $target = NULL): string
  {
    global $config;

    if (preg_match('/^%%/', $string)) {
      /* Special case: %% at beginning of string means do not touch it. Used by binary attributes. */
      return preg_replace('/^%%/', '', $string);
    }

    $offset = 0;
    $vars   = [];
    while (preg_match('/%([^%]+)%/', $string, $m, PREG_OFFSET_CAPTURE, $offset)) {
      $replace  = static::parseMask($m[1][0], $attrs);
      $vars[]   = [$m[0][1], strlen($m[0][0]), $replace];
      $offset   = $m[0][1] + strlen($m[0][0]);
    }

    $generator = static::iteratePossibleValues($string, $vars, $escapeMethod);

    $string = $generator->current();

    if (($unique !== NULL) && !empty($vars)) {
      $ldap = $config->get_ldap_link();
      $ldap->cd($config->current['BASE']);
      /* Return the first found unique value */
      foreach ($generator as $value) {
        if (class_available('archivedObject')) {
          $filter = archivedObject::buildUniqueSearchFilter($unique, $value);
          $ldap->search($filter, ['dn']);
          if ($ldap->count() > 0) {
            continue;
          }
        }
        $filter = '('.ldap_escape_f($unique).'='.ldap_escape_f($value).')';
        $ldap->search($filter, ['dn']);
        if ($ldap->count() == 0) {
          return $value;
        }
        if (($target !== NULL) && ($ldap->count() == 1)) {
          $attrs = $ldap->fetch();
          if ($attrs['dn'] == $target) {
            return $value;
          }
        }
      }
    }

    return $string;
  }


  /*! \brief Generator that yields possible template mask values
   *
   * \param string $rule            The mask
   * \param array $variables        The possible values for each mask with its position and length
   * \param callable $escapeMethod  Method to call to escape mask result
   */
  protected static function iteratePossibleValues (string $rule, array $variables, $escapeMethod = NULL)
  {
    if (!count($variables)) {
      yield $rule;
      return;
    }

    /* Start from the end to avoid messing the positions, and to avoid ids at the end if not needed (common usecase) */
    list($pos, $length, $val) = array_pop($variables);

    /* $val may be an iterator or an array */
    foreach ($val as $possibility) {
      if ($escapeMethod !== NULL) {
        $possibility = $escapeMethod($possibility);
      }
      $nrule = mb_substr_replace($rule, $possibility, $pos, $length);
      foreach (static::iteratePossibleValues($nrule, $variables, $escapeMethod) as $result) {
        yield $result;
      }
    }
  }

  /*! \brief Parse template masks in a single string and list the fields it needs
   *
   * \return array An array with the names of the fields used in the string pattern
   */
  public static function listFields ($string)
  {
    $fields = [];
    $offset = 0;
    while (preg_match('/%([^%]+)%/', $string, $m, PREG_OFFSET_CAPTURE, $offset)) {
      $mask   = $m[1][0];
      $offset = $m[0][1] + strlen($m[0][0]);
      if ($mask == '|') {
        continue;
      }
      if (preg_match('/^([^|]+)\|/', $mask, $m)) {
        $mask = substr($mask, strlen($m[0]));
      }
      $fields[] = $mask;
    }
    return $fields;
  }

  private static function modifierRemoveAccents ($str)
  {
    $str = htmlentities($str, ENT_NOQUOTES, 'UTF-8');

    $str = preg_replace('#&([A-za-z])(?:acute|cedil|circ|grave|orn|ring|slash|th|tilde|uml);#', '\1', $str);
    // handle ligatures
    $str = preg_replace('#&([A-za-z]{2})(?:lig);#', '\1', $str);
    // delete unhandled characters
    return [preg_replace('#&[^;]+;#', '', $str)];
  }

  private static function modifierTranslit (array $args, $str)
  {
    $localesaved = setlocale(LC_CTYPE, 0);
    $ret = [];
    foreach ($args as $arg) {
      setlocale(LC_CTYPE, [$arg,"$arg.UTF8"]);
      $ret[] = iconv('UTF8', 'ASCII//TRANSLIT', $str);
    }
    setlocale(LC_CTYPE, $localesaved);
    return array_unique($ret);
  }

  private static function modifierPregReplace (array $args, $str)
  {
    $pattern = '/\s/';
    $replace = '';
    if (count($args) >= 1) {
      $pattern = $args[0];
      if (count($args) >= 2) {
        $replace = $args[1];
      }
    }

    return [preg_replace($pattern.'u', $replace, $str)];
  }

  private static function modifierSubString (array $args, $str)
  {
    if (count($args) < 1) {
      trigger_error("Missing 's' substr modifier parameter");
    }
    if (count($args) < 2) {
      array_unshift($args, 0);
    }
    if (preg_match('/^(\d+)-(\d+)$/', $args[1], $m)) {
      $res = [];
      for ($i = $m[1];$i <= $m[2]; ++$i) {
        $res[] = mb_substr($str, $args[0], $i);
      }
      return array_unique($res);
    } else {
      return [mb_substr($str, $args[0], $args[1])];
    }
  }

  private static function modifierRandomString (array $args)
  {
    $length = 8;
    $chars  = 'b';
    if (count($args) >= 2) {
      $length = random_int($args[0], $args[1]);
      if (count($args) >= 3) {
        $chars = $args[2];
      }
    } elseif (count($args) >= 1) {
      $length = $args[0];
    }
    $res = '';
    for ($i = 0; $i < $length; ++$i) {
      switch ($chars) {
        case 'd':
          /* digits */
          $res .= (string)random_int(0, 9);
          break;
        case 'l':
          /* letters */
          $nb = random_int(65, 116);
          if ($nb > 90) {
            /* lowercase */
            $nb += 6;
          }
          $res .= chr($nb);
          break;
        case 'b':
          /* both */
        default:
          $nb = random_int(65, 126);
          if ($nb > 116) {
            /* digit */
            $nb = (string)($nb - 117);
          } else {
            if ($nb > 90) {
              /* lowercase */
              $nb += 6;
            }
            $nb = chr($nb);
          }
          $res .= $nb;
          break;
      }
    }

    return [$res];
  }

  private static function modifierDate (array $args)
  {
    if (count($args) < 1) {
      $args[] = 'now';
    }
    if (count($args) < 2) {
      $args[] = 'Y-m-d';
    }
    $dateObject = new DateTime($args[0], new DateTimeZone('UTC'));
    if ($args[1] == 'epoch') {
      /* Special handling for shadowExpire: days since epoch */
      return [floor($dateObject->format('U') / 86400)];
    }
    return [$dateObject->format($args[1])];
  }

  /*
    First parameter is whether the number should always be there or only in case of duplicates (1 or 0, defaults to 0).
    Second parameter is starting number, defaults to 1.
    Third parameter is step, defaults to 1.
  */
  private static function modifierNumber (array $args)
  {
    if (count($args) < 1) {
      $args[] = FALSE;
    }
    if (count($args) < 2) {
      $args[] = 1;
    }
    if (count($args) < 3) {
      $args[] = 1;
    }
    $numberGenerator = function ($mandatory, $start, $step)
    {
      if (!$mandatory) {
        yield '';
      }
      $i = $start;
      while (TRUE) {
        yield $i;
        $i += $step;
      }
    };

    return $numberGenerator($args[0], $args[1], $args[2]);
  }

  /*
    Modifier parameters:
    * id
    * starting number, defaults to 1.
    * step, defaults to 1.
  */
  private static function modifierIncremental (array $args): array
  {
    global $config;

    if (count($args) < 1) {
      throw new FusionDirectoryException(_('Missing id parameter for incremental modifier'));
    }
    if (count($args) < 2) {
      $args[] = 1;
    }
    if (count($args) < 3) {
      $args[] = 1;
    }
    $configDn = CONFIGRDN.$config->current['BASE'];
    Lock::addOrFail($configDn);
    $tabObject = objects::open($configDn, 'configuration');
    $json = $tabObject->getBaseObject()->fdIncrementalModifierStates;
    if (empty($json)) {
      $modifierStates = [];
    } else {
      $modifierStates = json_decode($json, TRUE);
    }
    if (isset($modifierStates[$args[0]])) {
      $value = $modifierStates[$args[0]]['value'] + $args[2];
    } else {
      $value = $args[1];
    }
    $modifierStates[$args[0]] = [
      'value' => $value,
      'date'  => date('Y-m-d'),
    ];
    $tabObject->getBaseObject()->fdIncrementalModifierStates = json_encode($modifierStates);
    $errors = $tabObject->save();
    Lock::deleteByObject($configDn);
    if (!empty($errors)) {
      throw $errors[0];
    }

    return [$value];
  }

  private static function modifierTitleCase ($str)
  {
    return [mb_convert_case($str, MB_CASE_TITLE, 'UTF-8')];
  }

  /*! \brief Apply a modifier
   *
   * \param string $m the modifier
   * \param array $args the parameters
   * \param mixed $str the string or array to apply the modifier on
   *
   * \return iterable an array or iterable object of possible values
   * */
  protected static function applyModifier (string $m, array $args, $str)
  {
    mb_internal_encoding('UTF-8');
    mb_regex_encoding('UTF-8');
    if (is_array($str) && (!is_numeric($m)) && (strtolower($m) == $m)) {
      /* $str is an array and $m is lowercase, so it's a string modifier */
      if (count($str) == 0) {
        $str = '';
      } else {
        $str = reset($str);
      }
    }
    switch ($m) {
      case 'F':
        // First
        $result = [reset($str)];
        break;
      case 'L':
        // Last
        $result = [end($str)];
        break;
      case 'J':
        // Join
        if (isset($args[0])) {
          $result = [join($args[0], $str)];
        } else {
          $result = [join($str)];
        }
        break;
      case 'C':
        // Count
        $result = [count($str)];
        break;
      case 'M':
        // Match
        if (count($args) < 1) {
          trigger_error('Missing "M" match modifier parameter');
          $args[] = '/.*/';
        }
        $result = array_filter(
          $str,
          function ($s) use ($args)
          {
            return preg_match($args[0], $s);
          }
        );
        break;
      case '4':
        // IPv4
        $result = array_filter($str, 'tests::is_ipv4');
        break;
      case '6':
        // IPv6
        $result = array_filter($str, 'tests::is_ipv6');
        break;
      case 'c':
        // comment
        $result = [''];
        break;
      case 'b':
        // base64
        if (isset($args[0]) && ($args[0] == 'd')) {
          $result = [base64_decode($str)];
        }
        $result = [base64_encode($str)];
        break;
      case 'u':
        // uppercase
        $result = [mb_strtoupper($str, 'UTF-8')];
        break;
      case 'l':
        // lowercase
        $result = [mb_strtolower($str, 'UTF-8')];
        break;
      case 'a':
        // remove accent
        $result = static::modifierRemoveAccents($str);
        break;
      case 't':
        // translit
        $result = static::modifierTranslit($args, $str);
        break;
      case 'p':
        // spaces
        $result = static::modifierPregReplace($args, $str);
        break;
      case 's':
        // substring
        $result = static::modifierSubString($args, $str);
        break;
      case 'r':
        // random string
        $result = static::modifierRandomString($args);
        break;
      case 'd':
        // date
        $result = static::modifierDate($args);
        break;
      case 'n':
        // number
        $result = static::modifierNumber($args);
        break;
      case 'i':
        // title case
        $result = static::modifierTitleCase($str);
        break;
      case 'e':
        // incremental number
        $result = static::modifierIncremental($args);
        break;
      default:
        trigger_error("Unkown modifier '$m'");
        $result = [$str];
        break;
    }
    return $result;
  }

  /*! \brief Flattens dependencies (if a depends of b which depends of c then a depends of c) */
  protected static function flatDepends (&$cache, &$errors, $depends, $key, array $forbidden = [])
  {
    if (isset($cache[$key])) {
      return $cache[$key];
    }

    $forbidden[] = $key;

    $array =
      array_map(
        function ($a) use (&$cache, &$errors, $depends, $forbidden, $key)
        {
          if (in_array($a, $forbidden)) {
            $errors[] = sprintf(
              _('Recursive dependency in the template fields: "%1$s" cannot depend on "%2$s" as "%2$s" already depends on "%1$s"'),
              $key,
              $a
            );
            return [];
          }
          $deps = static::flatDepends($cache, $errors, $depends, $a, $forbidden);
          if (($askmeKey = array_search('askme', $deps)) !== FALSE) {
            /* Do not flat special askme dependency */
            unset($deps[$askmeKey]);
          }
          return $deps;
        },
        $depends[$key]
      );
    $array[]      = $depends[$key];
    $cache[$key]  = array_unique(array_merge_recursive(...$array));
    return $cache[$key];
  }

  /*! \brief Computes dependencies between attributes: which attributes must be filled in order to compute each attribute value */
  protected static function attributesDependencies (array $attrs)
  {
    /* Compute dependencies of each attr */
    $depends = [];
    foreach ($attrs as $key => $values) {
      $depends[$key] = [];
      if (!is_array($values)) {
        $values = [$values];
      }
      unset($values['count']);
      foreach ($values as $value) {
        $offset = 0;
        while (preg_match('/%([^%\|]+\|)?([^%]+)%/', $value, $m, PREG_OFFSET_CAPTURE, $offset)) {
          $offset = $m[0][1] + strlen($m[0][0]);
          $depends[$key][] = $m[2][0];
          if (!isset($attrs[$m[2][0]])) {
            /* Dependency which has no value might be missing */
            $attrs[$m[2][0]]    = [];
            $depends[$m[2][0]]  = [];
          }
        }
      }
    }
    /* Flattens dependencies */
    $flatdepends  = [];
    $errors       = [];
    foreach ($depends as $key => $value) {
      static::flatDepends($flatdepends, $errors, $depends, $key);
    }
    return [$flatdepends, $errors];
  }

  /*! \brief Sort attrs depending of dependencies */
  protected static function sortAttributes (array $attrs, array $flatdepends)
  {
    uksort($attrs, function ($k1, $k2) use ($flatdepends)
    {
      if (in_array($k1, $flatdepends[$k2])) {
        return -1;
      } elseif (in_array($k2, $flatdepends[$k1])) {
        return 1;
      } else {
        /* When no direct dependency, we sort by number of dependencies */
        $c1 = count($flatdepends[$k1]);
        $c2 = count($flatdepends[$k2]);
        if ($c1 == $c2) {
          return 0;
        }
        return (($c1 < $c2) ? -1 : 1);
      }
    });
    return $attrs;
  }
}
