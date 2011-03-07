<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007  Fabian Hickert
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

class Step_License extends setup_step
{
  var $license_found  = false;
  var $License        = "";
  var $header_image   = "images/setup/license.png";
  var $accepted       = FALSE;
  var $attributes     = array("accepted");
  

  function Step_License()
  {
    $this->update_strings();
  }

  
  function load_license()
  {
    if(!file_exists("../setup/license.txt")){
      $this->License = "<font color='red'>License not found. Setup stops here.</font>";
      $this->license_found = false;
    }else{
      $fp = fopen("../setup/license.txt","r");
      $this->License = "";
      while (!feof($fp)){
        $this->License .= fread($fp, 255);
      }
      $this->license_found = true;
    }
  }


  function update_strings()
  {
    $this->s_title      = _("License");
    $this->s_title_long = _("License");
    $this->s_info       = _("Terms and conditions for usage");
  }

 
  function execute()
  {
    $this->load_license();

    $smarty = get_smarty();
    $smarty -> assign("License",nl2br($this->License));
    $smarty -> assign("license_found",$this->license_found);
    $smarty -> assign("accepted",$this->accepted);

    return($smarty -> fetch (get_template_path("../setup/setup_license.tpl")));
  }

  
  function save_object()
  {
    if(isset($_POST['step_license'])){
      if(isset($_POST['accepted'])){
        $this->accepted = TRUE;
      }else{
        $this->accepted = FALSE;
      }
    } 
  
    if($this->license_found && $this->accepted){
      $this->is_completed = true;
    }else{
      $this->is_completed = false;
    }
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
