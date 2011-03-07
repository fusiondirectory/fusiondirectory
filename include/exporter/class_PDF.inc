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

/* Avoid autoloading this class */

class PDF extends FPDF
{
  var $headline= "";

  function setHeadline($headline)
  {
    $this->headline= $headline;
  }


  function Header()
  {
      $this->SetFont('Helvetica','B',10);
      $this->Cell(0, 0, $this->headline, 0, 0, 'L');
      $this->Ln(5);
  }
  
  function Footer()
  {
      $this->SetY(-15);
      $this->SetFont('Helvetica','I',8);
      $this->Cell(0,10, _("Page")." ".$this->PageNo().'/{nb}',0,0,'C');
  }

}

?>
