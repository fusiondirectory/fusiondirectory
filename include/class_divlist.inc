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

/*!
 * \file class_divlist.inc
 * Source code for class divlist
 */ 
 
/*!
 * \brief This class contains all function to manage page
 */ 
class divlist {

  var $a_entries;
  var $a_header;
  var $b_displayPageNums;
  var $s_summary;
  var $cols;
  var $pageid;
  //FIXME: Ignoring the fact, that they were missing - height and width are numeric...
  var $s_height;
  var $s_width;
  
  // Members for page managment
  var $i_currentPage;
  var $i_entriesPerPage;

  var $force_height = false;
  var $list_footer = "";
  var $fix_list_for_plugins = FALSE;
  
  // Added php 4 constructor
  /*!
   * \brief Divlist constructor.
   * 		Initialized all the vars
   * 
   * \param string $pageid Identifier for the page
   */ 
  function divlist($pageid){

    $this->i_currentPage    = 0;
    $this->i_entriesPerPage = 10;
    $this->s_height         = "450";
    $this->s_width          = "600px";
    $this->s_summary        = "";
    $this->a_entries        = array();
    $this->a_header         = array();
    $this->b_displayPageNums= true;
    $this->cols             = 0;
    $this->pageid           = $pageid;
  }

  /*!
   * \brief Divlist destructor
   */ 
  function __destruct(){
  
  }

  /*!
   * \brief Set the plugin mode
   * 
   * \param boolean $val true
   */ 
  function SetPluginMode($val = TRUE)
  {
    $this->fix_list_for_plugins = $val;
  }

  /*!
   * \brief Set a footer
   * 
   * \param string $str The new footer
   */ 
  function SetFooter($str)
  {
    $this->list_footer = $str;
  }

  /*!
   * \brief Add an entry
   * 
   * \param $a_entriedata The entry to add
   */ 
  function AddEntry($a_entriedata) {
    $this->a_entries[] = $a_entriedata;
  }

  /*!
   * \brief Disable the page numbers
   */ 
  function DisablePageNums() {
    $this->b_displayPageNums =false;
  }

  /*!
   * \brief Set a header at the page
   * 
   * \param string $a_headerdata The header of the page
   */ 
  function SetHeader($a_headerdata) {
    $this->a_header[] = $a_headerdata ;
  }
  
  /*!
   * \brief Set the height of the page
   * 
   * \param integer $s_height The height
   */ 
  function SetHeight($s_height) {
    $this->s_height= $s_height;
    $this->force_height = true;
  }
 
  /*!
   * \brief Set the width of the page
   * 
   * \param integer $s_width The width
   */
  function SetWidth($s_width) {
    $this->s_width= $s_width;
    if(preg_match("/^[0-9]*$/",$this->s_width)){
      $this->s_width .="px";
    }
  }

  /*!
   * \brief Set the number of entries per page
   * 
   * \param int $i_num The number
   */ 
  function SetEntriesPerPage($i_num){
    $this->i_entriesPerPage=$i_num;
  }
  
  /*!
   * \brief This fonction draw the list of the page
   */ 
  function DrawList(){
    $s_return = "";
    if(!$this->force_height){
      $s_return.= "<table summary='".$this->s_summary."' style='width:".
                  $this->s_width.";height:".$this->s_height.
                  "px;' cellspacing='0' id='t_scrolltable'>";
    } else {
      $s_return.= "<table summary='".$this->s_summary."' style='width:".
                  $this->s_width.";height:".$this->s_height.
                  "px;' cellspacing='0' id='t_scrolltable_onlywidth'>";
    }

    $s_return.= $this->_generateHeader();
    $s_return.= $this->_generatePage();
    $s_return.= $this->_generateFooter(); 
    $s_return.= "</table>";
    
    return ($s_return);
  }
  
  /*!
   * \brief Calculate the number of the current page
   */ 
  function _numpages(){
    $cnt = count($this->a_entries);
    
    $pages  = (int) ($cnt / $this->i_entriesPerPage);
    if($cnt % $this->i_entriesPerPage){
      $pages++;
    }
    
    return $pages;
  }
  
  /*!
   * \brief Calculate the number of entries
   * 
   * \return the number of entries
   */ 
  function _numentries(){
    return count($this->a_entries);
  }

  /*!
   * \brief Generate the footer of the page
   */ 
  function _generateFooter()
  {
    $s_return = "";
    if(!empty($this->list_footer)){

      // Using scrolltable?
      if($this->i_entriesPerPage == 0) {
        if(!$this->force_height) {
          $s_return.= "\n<tr><td class='scrollhead'><table summary='' style='width:100%' cellspacing='0' id='t_scrollfoot'>";
        } else {
          $s_return.= "\n<tr><td class='scrollhead'><table summary='' style='width:100%' cellspacing='0'                      id='t_scrollfoot_onlywidth'>";
        }
      }
      $s_return .= "<tr><td class='listfooter' style='border-bottom:0px;'>".$this->list_footer."</td>";
          // Attach a 13px-wide column (used as scrollbar space in body-table),
    // but do this only if we are really using scrolltables.
    if($this->i_entriesPerPage == 0) {
      $s_return.= "\n<td class='listfooter' style='width:13px;border-right:0px;'>&nbsp;</td>";
      $s_return.= "\n</table></td>";
    }

    $s_return.= "\n</tr>";

    }
    return($s_return);
  }
  
  /*!
   * \brief Generate the page's header
   */ 
  function _generateHeader(){
    $s_return = "";
    $s_value  = "";
    $s_key    = "";
    
    // Using scrolltable?
    if($this->i_entriesPerPage == 0) {
      if(!$this->force_height) {
        $s_return.= "\n<tr><td class='scrollhead'><table summary='' style='width:".$this->s_width.";' cellspacing='0' id='t_scrollhead'>";
      } else {
        $s_return.= "\n<tr><td class='scrollhead'><table summary='' style='width:".$this->s_width.";' cellspacing='0' id='t_scrollhead_onlywidth'>";
      }
    }

    $s_return.= "\n<tr>";

    foreach($this->a_header[0] as $s_key => $s_value ){
      if(!isset($s_value['attach'])){
        $s_value['attach'] = "";
      }

      $s_return.= "\n<td class='listheader' ".$s_value['attach'].">".$s_value['string']."</td>";

      // Increment column counter
      $this->cols++;

    }
    
    // Attach a 13px-wide column (used as scrollbar space in body-table),
    // but do this only if we are really using scrolltables.
    if($this->i_entriesPerPage == 0) {
      $s_return.= "\n<td class='listheader' style='width:13px;border-right:0px;'>&nbsp;</td>";
      $s_return.= "\n</table></td>";
    }

    $s_return.= "\n</tr>";
    return $s_return;
  }
  
  /*!
   * \brief Set the structure of table
   * 
   * \param string $msg The summary
   */ 
  function SetSummary($msg){
    $this->s_summary = $msg;
  }

  /*!
   * \brief Generate de page
   */ 
  function _generatePage(){
    
    $s_value  = "";
    $s_key    = "";
    $s_return   = "";
   
    if($this->i_entriesPerPage == 0) {
      // We need to construct a "body"-table that is width-of-scrollbar thinner than the "head"-table.
      if(!$this->force_height || $this->fix_list_for_plugins) {
        $s_return .= "\n<tr><td colspan='".$this->cols."' class='scrollbody'><div style='width:".$this->s_width.";height:".($this->s_height-20)."px;' id='d_scrollbody' class='scrollbody'>";
        $s_return .= "<table summary='' style='height:100%;width:581px;' cellspacing='0' id='t_scrollbody'>";
      } else {
        $s_return .= "\n<tr><td colspan='".$this->cols."' class='scrollbody'><div style='width:".$this->s_width.";height:".($this->s_height-20)."px;' id='d_scrollbody_onlywidth' class='scrollbody'>";
        $s_return .= "<table summary='' style='height:100%;width:581px;' cellspacing='0' id='t_scrollbody_onlywidth'>";
      }
    }
 
    $i_alternate=0;
    if(isset($_GET['start'])){
      session::set('start'.$this->pageid,$_GET['start']);
      $start = $_GET['start'];
    } else {
      if(session::is_set('start'.$this->pageid)){
        $start = session::get('start'.$this->pageid);
      } else {
        $start=0;
      }
    }

    /* If divlist is empty, append a single white entry */
    if(count($this->a_entries)==0){
      $str = $s_return."<tr>";
      $str.="<td class='list1nohighlight' colspan='".$this->cols."' style='height:100%;border-right:0px;width:100%;'>&nbsp;</td>";
      $str.="</tr>";
      if($this->i_entriesPerPage == 0) {
        $str.="</table></div></td></tr>";
      }
      return($str);
    } else {
    
      //FIXME: Ahrg. I'd like to read this code without wasting my time.
      if ($this->i_entriesPerPage > 0) {
        while($start > $this->_numentries()){
          $start = $start - $this->i_entriesPerPage;
        } 
    
        $stop  = $start + $this->i_entriesPerPage;
    
        $appendempty = ($this->_numentries() -$start);
    
        for($i = $start ; $i < $stop;$i++){
          
          if(isset($this->a_entries[$i])){
                  
            $s_value = $this->a_entries[$i];
            
            // Alternation of background color    
            if($i_alternate) $i_alternate=0; else $i_alternate=1;
            
          $s_return .= "\n<tr class='rowxp".$i_alternate."'>";
            
            $cnt = 0;           
              
            foreach($s_value as $s_key2 => $s_value2 ){       
    
              $cnt++;         
    
              if(!isset($s_value2['class'])){
                $class = "list".$i_alternate; 
              }else{
                $class = $s_value2['class'];
              }
    
              if(!isset($s_value2['attach'])){
                $style = "";
              }else{
                $style = " ".$s_value2['attach']." "  ;
              }
    
              $s_return .= "\n<td ".$style." class='".$class."'>";
              $s_return .= $s_value2['string'];
              $s_return .= "\n</td>";
            }
            if($cnt == 0 ){
              $s_return.="\n<td>&nbsp;</td>";
            }
  
            $s_return .= "\n</tr>";
          } 
        }
    
        if(!(($stop)<$this->_numentries())){
          $nums = $stop - $this->_numentries();// - $stop;
          for($i = 0 ; $i < $nums ; $i ++ ){
            $s_return.="<tr>";
            $cnt=0;
            for($a = 0, $l= count($this->a_header[0]); $a < $l ; $a ++ ) {
              if($a ==(count($this->a_header[0])-1)){
                $s_return.="\n<td class='list1' style='border:0px;height:26px;'>&nbsp;</td>";
              } else {
                $s_return.="\n<td class='list1' style='height:26px;'>&nbsp;</td>";
              }
            }
            $s_return.="\n</tr>";
          } 
        }
    
        if($this->b_displayPageNums){
          $s_return .= "<tr><td colspan='".$this->cols."' align='center'>".range_selector($this->_numentries(),$start,$this->i_entriesPerPage)."</td></tr>";
        }
      } else {
        // $this->i_entriesPerPage <= 0
        // We should display all entries on one page
  
        $i = $this->_numEntries();
        foreach($this->a_entries as $s_key => $s_value){
          $i--;
  
          if($i_alternate!=0){ 
            $i_alternate=0; 
          } else {
            $i_alternate=1;
          }
        
          $s_return .= "\n<tr class='rowxp".$i_alternate."'>";
            
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
              $style = " ".$s_value2['attach']." "  ;
            }
            
            $s_return .= "\n<td ".$style." class='".$class."'>";
            $s_return .= $s_value2['string'];
            $s_return .= "</td>";
          }
          $s_return .= "\n</tr>";
        }
      }
      //print row to fill empty space
        $s_return.="\n<tr>";
        for ($i= 1; $i <= $this->cols; $i++){
          if ($i == $this->cols){
           $s_return.= "<td class='list1nohighlight' style='height:100%;border-right:0px;'>&nbsp;</td>";
          } else {
           $s_return.= "<td class='list1nohighlight' style='height:100%;'>&nbsp;</td>";
          }
        }
        $s_return.="\n</tr>";
  
      if($this->i_entriesPerPage == 0) {
        $s_return .= "\n</table></div></td>";
      }
      $s_return .= "</tr>";
      return $s_return;
    }
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
