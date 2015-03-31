<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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
 * \file class_cvsExporter.inc
 * Source code for class cvsExporter
 */

/*!
 * \brief This class contains all the functions needed for cvs export
 */
class csvExporter
{
  var $result;

  /*!
   * \brief Create a cvsExporter
   *
   * \param array $headline
   *
   * \param array $header
   *
   * \param array $entries
   *
   * \param array $columns
   */
  function __construct($headline, $header, $entries, $columns = array())
  {
    // If no preset, render all columns
    if (!count($columns)) {
      foreach ($header as $index => $dummy) {
        $columns[] = $index;
      }
    }

    // Generate header
    $this->result = "#";
    foreach ($columns as $index) {
      if (isset($header[$index])) {
        $this->result .= trim(html_entity_decode($header[$index], ENT_QUOTES, 'UTF-8').";");
      } else {
        $this->result .= ";";
      }
    }
    $this->result = preg_replace('/;$/', '', $this->result)."\n";

    // Append entries
    foreach ($entries as $row) {
      foreach ($columns as $index) {
        if (isset($row["_sort$index"])) {
          $this->result .= trim(html_entity_decode($row["_sort$index"], ENT_QUOTES, 'UTF-8')).";";
        } else {
          $this->result .= ";";
        }
      }
      $this->result = preg_replace('/;$/', '', $this->result)."\n";
    }
  }

  /*!
   * \brief Get the result
   */
  function query()
  {
     return $this->result;
  }


  /*!
   * \brief Get Informations
   */
  static function getInfo()
  {
    return array("exportCSV" => array( "label" => _("CSV"), "image" => "geticon.php?context=mimetypes&icon=text-csv&size=16", "class" => "csvExporter", "mime" => "text/x-csv", "filename" => "export.csv" ));
  }

}

?>
