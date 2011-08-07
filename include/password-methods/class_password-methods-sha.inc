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
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

class passwordMethodsha extends passwordMethod
{
  function passwordMethodsha($config)
  {
  }

  function is_available()
  {
    if (function_exists('sha1')){
      return(true);
    }elseif(function_exists('mhash')){
      return true;
    }else{
      return false;
    }
  }


  function generate_hash($password)
  {
    if (function_exists('sha1')) {
      $hash = "{SHA}" . base64_encode(pack("H*",sha1($password)));
    }elseif (function_exists('mhash')) {
      $hash = "{SHA}" . base64_encode(mHash(MHASH_SHA1, $password));
    }else{
      msg_dialog::display(_("Configuration error"), msgPool::missingext("mhash"), ERROR_DIALOG);
      return false;
    }

    return $hash; 
  }


  function get_hash_name()
  {
    return "sha";
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
