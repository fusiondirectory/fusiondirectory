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

/* definitions */

/* certificates */
define("PEM","pem");
define("DER","der");

class certificate 
{
  /* vars */
  var $data;
  var $type;
  var $error;

  /* Initialize all vars*/ 
  function certificate()
  {
    $this->data= "";
    $this->type= false;
    $this->error="";  
    $this->info = array();
  } 

  /* Reads specified Certfile/string and convert it to PEM*/
  function import($data,$type=false)
  {
    /* if is file read from file, else use string as it is*/
    if(is_file($data))  {
      $fp = fopen($data,"r+");
      $str = "";

      if(!$fp){
        $this->certificate();
        $this->error= msgPool::cannotReadFile($data);
        return(false);
      }else{
        /* Reading data*/
        while(!feof($fp)){
          $str.=fgets($fp,1024);
        }
      }
      /* Filename given, so we use the data from the file */
      $this->data = $str;
    } else {
      /* Cert as String, use this string */
      $this->data = $data;
    }

    /* Data can't be empty */
    if($data = ""){
      $this->certificate();
      $this->error = _("Certificate is empty!"); 
      return(false);
    }

    /* Prefer specified certtype*/
    if($type) {
      $this->type = $type;  
    }else{
      /* Detect certtype, cause there is none specified */      

      /* PEM allways starts with ----BEGIN CERTIFICATE-----*/
      if(strstr($this->data,"CERTIFICATE"))  {
        $this->type=PEM;
      } else {
        /* We test DER now, on fail abort */
        $this->type=DER;
      }
    }

    /* Convert to PEM to give $this->info the ability to read the cert */  
    if($this->type == DER )    {
      $this->derTOpem();
    }

    /* If cert is loaded correctly and is PEM now, we could read some data out of it */
    if(count($this->info()) <=1)  {
      $this->certificate();
      $this->error = _("Cannot load certificate - only PEM/DER is supported!");
      /* Reset*/
      return(false);
    }

    $this->info(false);

    /* Loaded a readable cert */
    return(true);
  }

  /* Returns Array with all containing data */
  function info($ret = true)
  {
    if($this->type != PEM){
      $this->error = _("Cannot extract information for non PEM certificates!");
      return(false);
    } else {
      /* return an array with all given information */
      $this->info=openssl_x509_parse($this->data);

      if($ret)
        return($this->info);
    }
  }

  /* Return Functions */
  function getvalidto_date()
  {
    if(isset($this->info['validTo_time_t'])){
      return($this->info['validTo_time_t']);
    }else{
      return(false);
    }
  } 

  function getvalidfrom_date()
  {
    if(isset($this->info['validFrom_time_t'])){
      return($this->info['validFrom_time_t']);
    }else{
      return(false);
    }

  }

  function getname()
  {
    if(isset($this->info['name'])){
      return($this->info['name']);
    }else{
      return(false);
    }
  }

  function getCN()
  {
    if(isset($this->info['subject']['CN'])){
      return($this->info['subject']['CN']);
    }else{
      return(false);
    }
  }

  function getO()
  {
    if(isset($this->info['subject']['O'])){
      return($this->info['subject']['O']);
    }else{
      return(false);
    }
  }

  function getOU()
  {
    if(isset($this->info['subject']['OU'])){
      return($this->info['subject']['OU']);
    }else{
      return(false);
    }
  }

  function getSerialNumber()
  {
    if(isset($this->info['serialNumber'])){
      return($this->info['serialNumber']);
    }else{
      return(false);
    }
  }

  function isvalid()
  {
    return (($this->type != false)&&(count($this->info)>1));
  }
  

  /* Export Certificate to specified file, with specified method*/
  function export($type,$filename="temp")
  {
    /* Check if valid cert is loaded*/
    if($this->type!=false){
      /* Check if we must convert the cert */
      if($this->type!= $type){
        $strConv = $this->type."TO".$type;
        $this->$strConv();
      }    

      /* open file for writing */
      $fp = fopen($filename,"w+");

      if(!$fp){
        $this->error= msgPool::cannotWriteFile($filename);
        return(false);
      }else{
        fwrite($fp,$this->data,strlen($this->data));        
      }
      return(true);
    }else{
      $this->error= _("No valid certificate loaded!");
      return(false);    
    }
    return(false);
  } 


  /* Convert der to pem Certificate */
  function derTOpem()
  {
    /* if type is DER start convert */
    if($this->type == DER)    {
      /* converting */
      $this->type= PEM; 
      $str = base64_encode($this->data);
      $len = strlen($str);

      $end = "";

      while($len > 0 )      {
        $len = $len - 64;
        $str1 = substr($str,0,64)."\n";
        $str  = substr($str,64,$len);
        $end.= $str1;
      }

      $strend = "-----BEGIN CERTIFICATE-----\n".$end;
      $strend .= "-----END CERTIFICATE-----";

      $this->data     = $strend;
      return(true); 
    }
    return(false);
  }

  /*Convert pem to der Certificate */
  function pemTOder()
  {
    if($this->type == PEM)    {
      $this->type= DER;

      $str = $this->data;

      $str = str_replace("-----BEGIN CERTIFICATE-----","",$str);
      $str = str_replace("-----END CERTIFICATE-----","",$str);

      $str = base64_decode($str);

      $this->data = $str;
      return(true);
    }
    return(false);
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
