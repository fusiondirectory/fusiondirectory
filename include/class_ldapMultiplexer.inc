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

/*!
 * \file class_ldapMultiplexer.inc
 * Source code for class ldapMultiplexer
 */

 /**
  * This class contains all function to manage ldap multiplexer
  * @method void set_size_limit ($size)
  * @method void cd ($dir)
  * @method boolean|resource modify_batch (array $changes)
  * @method string get_error ($details = TRUE)
  * @method array get_objectclasses ($force_reload = FALSE)
  * @method string|array fetch (bool $cleanUpNumericIndices = FALSE)
  * @method string|resource search ($filter, $attrs = [], $scope = 'subtree', array $controls = NULL)
  * @method string|resource cat ($dn, $attrs = ["*"], $filter = "(objectclass=*)")
  * @method int count ()
  * @method bool success ()
  * @method true|0|"" rmdir_recursive (string $deletedn)
  * @method void create_missing_trees (string $target, bool $ignoreReferralBases = TRUE)
  * @method mixed add ($attrs)
  */
class ldapMultiplexer
{

  /* Internal stuff */
  protected $object;

  /* Result resource */
  protected $sr;

  /*!
   * \brief LADP multiplexer constructor
   *
   * \param $object Object LDAP
   */
  public function __construct (&$object)
  {
    /* Store object */
    $this->object = $object;

    /* Set result resource */
    $this->sr = $this->object->getSearchResource();
  }

  /*!
   * \brief Call a ldap method with his parameters
   *
   * \param string $methodName The name of the method
   *
   * \param $parameters Parameters for the method
   */
  public function __call ($methodName, $parameters)
  {
    /* Add resource pointer if the mentioned methods are used */
    if (in_array($methodName, ['search','ls','cat','fetch','clearResult','resetResult','count','getDN','rmdir_recursive','create_missing_trees','import_single_entry','import_complete_ldif','parse_result'])) {
      array_unshift($parameters, $this->sr);
    }

    return call_user_func_array([$this->object, $methodName], $parameters);
  }

  /*
   * \brief Get a member name from a ldap object
   *
   * \param string $memberName
   */
  public function __get ($memberName)
  {
    return $this->object->$memberName;
  }
}
