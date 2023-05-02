<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2015-2016  FusionDirectory

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
 * \file class_exceptions.inc
 * Source code for FusionDirectory exceptions
 */

/*! \class FusionDirectoryException
    \brief Parent class for all exceptions thrown in FusionDirectory
*/
class FusionDirectoryException extends Exception
{
  public function toArray (): array
  {
    return [
      'class'   => get_class($this),
      'message' => $this->getMessage(),
      'line'    => $this->getLine(),
      'file'    => $this->getFile(),
    ];
  }
}

/*! \class LDIFImportException
    \brief Exception class which can be thrown by LDAP if the LDIF format is broken
*/
class LDIFImportException extends FusionDirectoryException
{
}

/*! \class LDIFExportException
    \brief Exception class which can be thrown by LDAP class if LDIF export fails
*/
class LDIFExportException extends FusionDirectoryException
{
}

/*! \class LdapGeneralizedTimeBadFormatException
    \brief Exception class which can be thrown by LdapGeneralizedTime if the format does not match
*/
class LdapGeneralizedTimeBadFormatException extends FusionDirectoryException
{
}

/*! \class InvalidValueException
    \brief Exception class which can be thrown if an attribute is set to a value with a non-compatible type
*/
class InvalidValueException extends FusionDirectoryException
{
}

class NonExistingObjectTypeException extends FusionDirectoryException
{
  protected $type;

  public function __construct (string $type, int $code = 0, Throwable $previous = NULL)
  {
    $this->type = $type;

    parent::__construct(sprintf(_('Non-existing type "%s"!'), $this->type), $code, $previous);
  }

  public function toArray (): array
  {
    $array = parent::toArray();

    $array['type'] = $this->type;

    return $array;
  }
}

class NonExistingBranchException extends FusionDirectoryException
{
  protected $branch;

  public function __construct (string $branch, int $code = 0, Throwable $previous = NULL)
  {
    $this->branch = $branch;

    parent::__construct(sprintf(_('Non-existing branch "%s"!'), $this->branch), $code, $previous);
  }

  public function toArray (): array
  {
    $array = parent::toArray();

    $array['branch'] = $this->branch;

    return $array;
  }
}

class NonExistingLdapNodeException extends FusionDirectoryException
{
  protected $dn;

  public function __construct (string $dn, string $message = NULL, int $code = 0, Throwable $previous = NULL)
  {
    $this->dn = $dn;
    if ($message === NULL) {
      $message = sprintf(_('Non-existing dn "%s"!'), $this->dn);
    }

    parent::__construct($message, $code, $previous);
  }

  public function toArray (): array
  {
    $array = parent::toArray();

    $array['dn'] = $this->dn;

    return $array;
  }
}

class EmptyFilterException extends FusionDirectoryException
{
  public function __construct (string $message = "", int $code = 0, Throwable $previous = NULL)
  {
    if ($message === '') {
      $message = _('Filter is empty');
    }
    parent::__construct($message, $code, $previous);
  }
}

class NoManagementClassException extends FusionDirectoryException
{
}

class UnknownClassException extends FusionDirectoryException
{
  protected $class;

  public function __construct (string $class, int $code = 0, Throwable $previous = NULL)
  {
    $this->class = $class;

    parent::__construct(sprintf(_('Unknown class "%s"!'), $this->class), $code, $previous);
  }

  public function toArray (): array
  {
    $array = parent::toArray();

    $array['unknownclass'] = $this->class;

    return $array;
  }
}

class LDAPFailureException extends FusionDirectoryException
{
}

class LoginFailureException extends FusionDirectoryException
{
}

class LoginFailurePpolicyException extends LoginFailureException
{
}
