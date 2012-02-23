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

/*! \brief This class contains all the functions needed for cvs export
 *
 */
class csvExporter
{
  var $result;

  function csvExporter($headline, $header, $entries, $columns= array()) {
    // If no preset, render all columns
    if (!count($columns)) {
      foreach ($header as $index => $dummy) {
        $columns[]= $index;
      }
    }
    
    // Generate header
    $this->result= "#";
    foreach ($columns as $index) {
      if (isset($header[$index])){
        $this->result.= trim($header[$index]).";";
      } else {
        $this->result.= ";";
      }
    }
    $this->result= preg_replace('/;$/', '', $this->result)."\n";

    // Append entries
    foreach ($entries as $row) {
      foreach ($columns as $index) {
        if (isset($row["_sort$index"])){
          $this->result.= trim($row["_sort$index"]).";";
        } else {
          $this->result.= ";";
        }
      }
      $this->result= preg_replace('/;$/', '', $this->result)."\n";
    }
  }


  function query()
  {
     return $this->result;
  }


  static function getInfo()
  {
    return array("exportCVS" => array( "label" => _("CSV"), "image" => "images/lists/csv.png", "class"=> "csvExporter", "mime" => "text/x-csv", "filename" => "export.csv" ));
  }

}

?>
