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
class FusionDirectoryException extends Exception {}

/*! \class LDIFImportException
    \brief Exception class which can be thrown by LDAP if the LDIF format is broken
*/
class LDIFImportException extends FusionDirectoryException {}

/*! \class LdapGeneralizedTimeBadFormatException
    \brief Exception class which can be thrown by LdapGeneralizedTime if the format does not match
*/
class LdapGeneralizedTimeBadFormatException extends FusionDirectoryException {}

class NonExistingObjectTypeException extends FusionDirectoryException {}
class NonExistingBranchException extends FusionDirectoryException {}
class NonExistingLdapNodeException extends FusionDirectoryException {}
class EmptyFilterException extends FusionDirectoryException {}
class NoManagementClassException extends FusionDirectoryException {}
class LDAPFailureException extends FusionDirectoryException {}
