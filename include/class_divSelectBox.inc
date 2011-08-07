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

class divSelectBox {

	var $a_entries;
	var $summary;
	var $cols;

	// Members for page managment
  var $height=200;

	// Added php 4 constructor
	function divSelectBox(){
		$this->s_summary                = "";
		$this->a_entries                = array();
		$this->cols                     = 0;
	}

  function setHeight($h){
    $this->height=$h;
  }

	function AddEntry($a_entriedata) {
		$this->a_entries[] = $a_entriedata;
	}

  function DrawList(){
    $s_return  = "";
    $s_return .= "<div style=\"height:2px; overflow:hidden;background:#888888;width:100%;margin-right:0px;\"></div>\n";
    $s_return .= "<div style=\"padding-right:1px;padding-bottom:2px;height:".$this->height.";width:100%\">\n";
    $s_return .= "<div style=\"overflow: auto;width:100%;height:".($this->height)."px;\">\n";
    $s_return .= "<table ".
                    "summary='".$this->s_summary."' ".
                    "width='100%' ".
                    "cellspacing='0' ".
                    "style='overflow:scroll; ".
                    "height:".($this->height-2)."px;".
                    "padding-right:1px; ".
                    "padding-bottom:2px; ".
                    "border-left:solid 2px; ".
                    "border-top:solid 0px; ".
                    "border-color:#888888; ".
                    "'>\n";
    $s_return.=$this->_generatePage();
    $s_return.= "</table></div></div>";
    return ($s_return);
  }
	
	function _numentries(){
		$cnt = count($this->a_entries);
		return $cnt;
	}
	
	function SetSummary($msg){
		$this->s_summary = $msg;
	}

	function _generatePage(){
		
		$s_value 	= "";
		$s_key 		= "";
		$s_return 	= "";
		$i_alternate=0;
    $str = "";
	
    /* If divlist is empty, append a single white entry */
    if(count($this->a_entries)==0){
      $str.="<tr class='rowxp1'
                ><td class='list1nohighlight' style='height:100%; border-right:0px;'>&nbsp;</td></tr>";
      return($str);
    }

    $i = $this->_numEntries();
    foreach($this->a_entries as $s_key => $s_value){
      $i--;

      if($i_alternate!=0){ 
        $i_alternate=0; 
      } else {
        $i_alternate=1;
      }

      $s_return .= "\n<tr class='rowxp".($i_alternate)."'>";

      $cnt = 0; 					
    
      foreach($s_value as $s_key2 => $s_value2 ){				
        $this->cols = count($s_value) ;						
        $cnt++;					

        if(!isset($s_value2['class'])){
          $class = "list".$i_alternate; 
        } else {
          $class = $s_value2['class'];
        }

        if(!isset($s_value2['attach'])){
          $style = "";
        } else {
          $style = " ".$s_value2['attach']." "	;
        }

        $s_return .= "\n<td ".$style." class='".$class."'>";
        $s_return .= $s_value2['string'];
        $s_return .= "</td>";
      }
      $s_return.="\n</tr>";
    }
    $s_return.="\n<tr>";
    for($i = 0 ; $i < ($this->cols) ; $i ++){
      if($i >= ($this->cols-1)){
        $s_return .= "<td class='list1nohighlight' style='height:100%;border:0px;'><div style='font-size:1px;'>&nbsp;</div></td>";
      }else{
        $s_return .= "<td class='list1nohighlight' style='height:100%;'><div style='font-size:1px;'>&nbsp;</div></td>";
      }
      
    }
    $s_return.="</tr>";
    return $s_return;
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
