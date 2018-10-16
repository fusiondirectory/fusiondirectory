<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2013-2018  FusionDirectory

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
 * \file class_ldapFilter.inc
 * Source code for class ldapFilter and ldapFilterLeaf
 */

 /*!
  * \brief This class allows to parse and execute on a array an LDAP filter
  * Example:
  * $filter = ldapFilter::parse("(&(objectClass=testClass)(testField=value))");
  * $array = array('objectClass' => array('testClass'), 'testField' => array ('value1', 'value'));
  * if ($filter($array)) {
  *   // do something
  * }
  */
class ldapFilter
{
  static $operators = array('!', '&', '|');

  protected $operator;
  protected $subparts;

  function __construct($operator, $subparts)
  {
    $this->operator = $operator;
    $this->subparts = $subparts;
  }

  function __toString()
  {
    return '('.$this->operator.join($this->subparts).')';
  }

  function __invoke($array)
  {
    $stopValue = FALSE;
    switch ($this->operator) {
      case '!':
        return !$this->subparts[0]($array);
      case '|':
        $stopValue = TRUE;
      case '&':
        foreach ($this->subparts as $subpart) {
          if ($subpart($array) == $stopValue) {
            return $stopValue;
          }
        }
        return !$stopValue;
      default:
        die('Unknown operator');
    }
  }

  function getOperator()
  {
    return $this->operator;
  }

  function getSubparts()
  {
    return $this->subparts;
  }

  function listUsedAttributes(&$result = array())
  {
    foreach ($this->subparts as $subpart) {
      $subpart->listUsedAttributes($result);
    }
    return $result;
  }

  static function parse($filter)
  {
    // Remove starting and ending parenthesis
    $filter = preg_replace(array('/^\\s*\\(/', '/\\)\\s*$/'), '', $filter);

    if (in_array($filter[0], ldapFilter::$operators)) {
      $subfilters = array();
      /* We need an ending parenthesis in order to catch last subpart correctly */
      $filter .= ')';
      $offset = 0;
      $level  = 0;
      $open;
      while (preg_match('/[^\\\\](\\(|\\))/', $filter, $m, PREG_OFFSET_CAPTURE, $offset)) {
        $offset = $m[0][1] + 1;
        if ($m[1][0] == '(') {
          if ($level == 0) {
            $open = $m[1][1];
          }
          $level++;
        } elseif ($m[1][0] == ')') {
          $level--;
          if ($level == 0) {
            $subfilters[] = ldapFilter::parse(substr($filter, $open + 1, $m[0][1] - $open));
          }
        }
      }
      if (in_array($filter[0], array('&', '|')) && (count($subfilters) == 1)) {
        /* Avoid empty levels */
        return $subfilters[0];
      } else {
        return new ldapFilter($filter[0], $subfilters);
      }
    } elseif (preg_match('/^([^\\(\\)\\=\\>\\<]+)('.join('|', ldapFilterLeaf::$operators).')([^\\(\\)]+)$/', $filter, $m)) {
      return new ldapFilterLeaf($m[1], $m[2], $m[3]);
    } else {
      throw new FusionDirectoryException('Failed to parse '.$filter);
    }
  }
}

/*!
 * \brief Leaf of an LDAP filter, for instance (objectClass=*)
 */
class ldapFilterLeaf extends ldapFilter
{
  static $operators = array('=','~=','>=','<=');

  protected $pattern;
  protected $dnFilter = FALSE;

  function __construct($left, $operator, $right)
  {
    if (@strrpos($left, ':dn:', -4) !== FALSE) {
      $this->dnFilter = TRUE;
      $left = substr($left, 0, -4);
    }
    parent::__construct($operator, array($left, $right));
    if (($this->operator == '=') || ($this->operator == '~=')) {
      $prefix = '';
      $suffix = '';
      if (preg_match('/^\\*/', $this->subparts[1])) {
        $prefix = '.*';
      }
      if (preg_match('/\\*$/', $this->subparts[1])) {
        $suffix = '.*';
      }
      $search = preg_replace(array('/^\\*/','/\\*$/'), '', $this->subparts[1]);
      if ($this->dnFilter) {
        $this->pattern = '/'.$left.'='.$prefix.preg_quote($search, '/').$suffix.',/';
      } elseif ($this->subparts[1] == '*') {
        $this->pattern = '/^.*$/';
      } else {
        $this->pattern = '/^'.$prefix.preg_quote($search, '/').$suffix.'$/';
      }
    }
  }

  function isDnFilter()
  {
    return $this->dnFilter;
  }

  function __toString()
  {
    return '('.$this->subparts[0].($this->dnFilter ? ':dn:' : '').$this->operator.$this->subparts[1].')';
  }

  function __invoke($array)
  {
    if ($this->dnFilter) {
      switch ($this->operator) {
        case '~=':
          trigger_error('Filter apply might not work as expected');
        case '=':
          return (isset($array['dn']) && preg_match($this->pattern, $array['dn']));
        default:
          die('Unsupported dn operator: '.$this->operator);
      }
    }
    if (isset($array[$this->subparts[0]])) {
      $values = $array[$this->subparts[0]];
      if (!is_array($values)) {
        $values = array($values);
      }
      foreach ($values as $value) {
        switch ($this->operator) {
          case '~=':
            trigger_error('Filter apply might not work as expected');
          case '=':
            if (preg_match($this->pattern, $value)) {
              return TRUE;
            }
            break;
          case '<=':
            if ($value <= $this->subparts[1]) {
              return TRUE;
            }
            break;
          case '>=':
            if ($value >= $this->subparts[1]) {
              return TRUE;
            }
            break;
          default:
            die('Unknown operator: '.$this->operator);
        }
      }
    }
    return FALSE;
  }

  function listUsedAttributes(&$result = array())
  {
    if ($this->dnFilter) {
      $result['dn'] = 'dn';
    } else {
      $result[$this->subparts[0]] = $this->subparts[0];
    }
    return $result;
  }
}

function fdTemplateFilter($filter)
{
  if ($filter instanceof ldapFilterLeaf) {
    if ($filter->isDnFilter()) {
      return $filter;
    } elseif ($filter->getOperator() == '=') {
      $subparts = $filter->getSubparts();
      return new ldapFilterLeaf('fdTemplateField', '=', $subparts[0].':'.$subparts[1]);
    } else {
      trigger_error('Not able to adapt this filter for templates');
    }
  } else {
    $subparts = $filter->getSubparts();
    foreach ($subparts as &$subpart) {
      $subpart = fdTemplateFilter($subpart);
    }
    unset($subpart);
    return new ldapFilter($filter->getOperator(), $subparts);
  }
  return $filter;
}
