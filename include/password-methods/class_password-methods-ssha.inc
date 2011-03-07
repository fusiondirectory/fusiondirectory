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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class passwordMethodssha extends passwordMethod
{
  function passwordMethodssha($config)
  {
  }

  function is_available()
  {
    if(function_exists("sha1")){
      return true;
    }elseif(function_exists("mhash")){
      return true;
    }else{
      return false;
    }
  }

  function generate_hash($pwd)
  {
    if (function_exists("sha1")) {
      $salt= substr(pack("h*", md5(mt_rand())), 0, 8);
      $salt= substr(pack("H*", sha1($salt.$pwd)), 0, 4);
      $pwd= "{SSHA}".base64_encode(pack("H*", sha1($pwd.$salt)).$salt);
      return $pwd;       
    } elseif(function_exists("mhash")) {
      $salt=mhash_keygen_s2k(MHASH_SHA1,$pwd, substr(pack("h*",md5(mt_rand())),0,8),4);
      $pwd= "{SSHA}".base64_encode(mhash(MHASH_SHA1, $pwd.$salt).$salt);
    } else {
      msg_dialog::display(_("Configuration error"), msgPool::missingext("mhash"), ERROR_DIALOG);
      return(false);
    }
    return $pwd;
  }


  function get_hash_name()
  {
    return "ssha";
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
