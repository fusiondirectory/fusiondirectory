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
function smarty_function_msgPool($params, &$smarty)
{
  if(class_available("msgPool") && isset($params['type'])){
    $parameter = array();
    foreach($params as $para => $value){
      if(!preg_match("/^type$/i",$para)){
        $parameter[$para] = $value;
      }
    }
    if(is_callable("msgPool::".$params['type'])){
      echo call_user_func_array(array("msgPool",$params['type']), $parameter);
    }else{
      trigger_error("Unknown msgPool function ".$params['type']);
    }
  }else{
    trigger_error("Unknown class msgPool.");
  }
}
?>
