<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2018  FusionDirectory

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
 * \file class_divSelectBox.inc
 * Source code for class divSelectBox
 */

/*!
 * \brief This class contains all the functions to
 * manage select box
 */
class divSelectBox
{
  protected $headers = FALSE;
  protected $a_entries;
  protected $s_summary;
  protected $cols;

  protected $id;

  protected $height = '200px';

  /*!
   * \brief Default divSelectBox constructor
   */
  function __construct ($id)
  {
    $this->s_summary  = '';
    $this->a_entries  = [];
    $this->cols       = 0;
    $this->id         = $id;
  }

  /*!
   * \brief Set new height value
   *
   * \param integer $h Height
   */
  function setHeight ($h)
  {
    if (is_numeric($h)) {
      $this->height = $h.'px';
    } else {
      $this->height = $h;
    }
  }

  /*!
   * \brief Add an entry
   *
   * \param array $a_entriedata
   */
  function addEntry ($a_entriedata)
  {
    $this->a_entries[] = $a_entriedata;
  }

  /*!
   * \brief Set column headers
   *
   * \param array $headers
   */
  function setHeaders ($headers)
  {
    $this->headers = $headers;
  }

  /*!
   * \brief Draw the list
   */
  function drawList ()
  {
    $s_return = '';
    $s_return .= '<div style="border:1px solid rgb(170,170,170);padding-right:1px;height:'.$this->height.';width:100%">'."\n";
    $s_return .= '<div style="overflow:auto; width:100%; height:100%;">'."\n";
    $s_return .= '<table '.
                    'class="listingTable" '.
                    'id="'.$this->id.'" '.
                    'style="overflow:scroll; '.
                      'height:98%; '.
                      'width:100%; '.
                      'border:none; '.
                    '"'.
                  ">\n";
    $s_return .= $this->generatePage();
    $s_return .= '</table></div></div>';
    if ($this->headers !== FALSE) {
      $s_return .=
        '<script>
          var sorter'.$this->id.' = tsorter.create(\''.$this->id.'\');
        </script>';
    }
    return $s_return;
  }

  /*!
   * \brief Set summary
   *
   * \param string $msg
   */
  function setSummary ($msg)
  {
    $this->s_summary = $msg;
  }

  /*!
   * \brief Generate the page
   */
  protected function generatePage ()
  {
    $display = '';
    if ($this->headers !== FALSE) {
      $display .= '<thead><tr>';
      foreach ($this->headers as $header) {
        if ($header === '') {
          $header = '&nbsp;';
        }
        $display .= '<th>'.$header.'</th>';
      }
      $display .= '</tr></thead>'."\n";
    }
    return $display.'<tbody>'.$this->generateBody().'</tbody>';
  }

  /*!
   * \brief Generate the body
   */
  protected function generateBody ()
  {
    /* If divselectbox is empty, append a single white entry */
    if (count($this->a_entries) == 0) {
      $str = '';
      if ($this->headers !== FALSE) {
        $this->cols = count($this->headers);
        $str .= '<tr>';
        for ($i = 0; $i < ($this->cols); $i++) {
          if ($i >= ($this->cols - 1)) {
            $str .= '<td style="height:100%;border:0px;">&nbsp;</td>';
          } else {
            $str .= '<td style="height:100%;">&nbsp;</td>';
          }
        }
        $str .= '</tr>';
      } else {
        $str .= '<tr><td style="height:100%; border-right:0px;">&nbsp;</td></tr>';
      }
      return $str;
    }

    $s_return = '';
    $i        = count($this->a_entries);
    foreach ($this->a_entries as $s_value) {
      $i--;

      $s_return .= "\n<tr>";

      $cnt = 0;

      $this->cols = count($s_value);
      foreach ($s_value as $s_value2) {
        $cnt++;

        if (!isset($s_value2['class'])) {
          $class = "";
        } else {
          $class = "class='".$s_value2['class']."'";
        }

        if (!isset($s_value2['attach'])) {
          $style = "";
        } else {
          $style = " ".$s_value2['attach']." ";
        }

        $s_return .= "\n<td".$style.$class.">";
        if (isset($s_value2['string'])) {
          if ($s_value2['string'] === '') {
            $s_return .= '&nbsp;';
          } else {
            $s_return .= htmlescape($s_value2['string']);
          }
        } else {
          $s_return .= $s_value2['html'];
        }
        $s_return .= '</td>';
      }
      $s_return .= "\n</tr>";
    }
    $s_return .= "\n<tr>";
    for ($i = 0; $i < ($this->cols); $i++) {
      if ($i >= ($this->cols - 1)) {
        $s_return .= '<td style="height:100%;border:0px;"><div style="font-size:1px;">&nbsp;</div></td>';
      } else {
        $s_return .= '<td style="height:100%;"><div style="font-size:1px;">&nbsp;</div></td>';
      }
    }
    $s_return .= '</tr>';
    return $s_return;
  }
}
