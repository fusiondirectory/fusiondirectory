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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class ldapMultiplexer {

  /* Internal stuff */
  protected $object;

  /* Result resource */
  protected $sr;
 
  public function __construct(&$object) {
    /* Store object */
    $this->object= $object;

    /* Set result resource */
    $this->sr= $this->object->getSearchResource();
  }
 
  public function __call($methodName, $parameters) {
    /* Add resource pointer if the mentioned methods are used */
    if (preg_match('/^(search|ls|cat|fetch|clearResult|resetResult|count|getDN|recursive_remove|rmdir_recursive|gen_xls|create_missing_trees|import_single_entry|import_complete_ldif)$/', $methodName)){
      array_unshift($parameters, $this->sr);
    }

    $class= new ReflectionClass($this->object);
    $method= $class->getMethod($methodName);

    return $method->invokeArgs($this->object, $parameters);
  }


  public function __get($memberName) {
    return $this->object->$memberName;
  }

}

?>
