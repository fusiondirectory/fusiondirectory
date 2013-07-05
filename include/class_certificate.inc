<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2013  FusionDirectory

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
 * \file class_certificate.inc
 * Source code for class certificate
 */

/* certificates */
define("PEM", "pem");
define("DER", "der");

/*!
 * \brief This class contains all the function needed to import certificates
 */
class certificate
{
  /* vars */
  var $data;
  var $type;
  var $error;

  /*
   * \brief Certificate constructor (initialize all vars)
   */
  function certificate()
  {
    $this->data   = "";
    $this->type   = FALSE;
    $this->error  = "";
    $this->info   = array();
  }

  /*
   * \brief Reads specified Certfile/string and convert it to PEM
   *
   * \param string $data
   *
   * \param boolean $type FALSE
   */
  function import($data, $type = FALSE)
  {
    /* if is file read from file, else use string as it is*/
    if (is_file($data)) {
      $fp   = fopen($data, "r+");
      $str  = "";

      if (!$fp) {
        $this->certificate();
        $this->error = msgPool::cannotReadFile($data);
        return FALSE;
      } else {
        /* Reading data*/
        while (!feof($fp)) {
          $str .= fgets($fp, 1024);
        }
      }
      /* Filename given, so we use the data from the file */
      $this->data = $str;
    } else {
      /* Cert as String, use this string */
      $this->data = $data;
    }

    /* Data can't be empty */
    if ($data = "") {
      $this->certificate();
      $this->error = _("Certificate is empty!");
      return FALSE;
    }

    /* Prefer specified certtype*/
    if ($type) {
      $this->type = $type;
    } else {
      /* Detect certtype, cause there is none specified */

      /* PEM always starts with ----BEGIN CERTIFICATE-----*/
      if (strstr($this->data, "CERTIFICATE"))  {
        $this->type = PEM;
      } else {
        /* We test DER now, on fail abort */
        $this->type = DER;
      }
    }

    /* Convert to PEM to give $this->info the ability to read the cert */
    if ($this->type == DER) {
      $this->derTOpem();
    }

    /* If cert is loaded correctly and is PEM now, we could read some data out of it */
    if (count($this->info()) <= 1) {
      $this->certificate();
      $this->error = _("Cannot load certificate - only PEM/DER is supported!");
      /* Reset*/
      return FALSE;
    }

    $this->info(FALSE);

    /* Loaded a readable cert */
    return TRUE;
  }

  /*
   * \brief Get all data of a certificate
   *
   * \param boolean $ret true
   *
   * \return Array with all containing data
   */
  function info($ret = TRUE)
  {
    if ($this->type != PEM) {
      $this->error = _("Cannot extract information for non PEM certificates!");
      return FALSE;
    } else {
      /* return an array with all given information */
      $this->info = openssl_x509_parse($this->data);

      if ($ret) {
        return $this->info;
      }
    }
  }

  /* Return Functions */
  function getvalidto_date()
  {
    return $this->_genericGet1('validTo_time_t');
  }

  function getvalidfrom_date()
  {
    return $this->_genericGet1('validFrom_time_t');
  }

  /*!
   * \brief Get the name of the certificate
   *
   * \return String with the name, but return FALSE if not found
   */
  function getname()
  {
    return $this->_genericGet1('name');
  }

  /*!
   * \brief Get the CN of the certificate
   *
   * \return String with the CN, but return FALSE if not found
   */
  function getCN()
  {
    return $this->_genericGet2('CN');
  }

  function getO()
  {
    return $this->_genericGet2('O');
  }

  function getOU()
  {
    return $this->_genericGet2('OU');
  }
  /*!
   * \brief Get the serial number of the certificate
   *
   * \return Integer number, but return FALSE if not found
   */
  function getSerialNumber()
  {
    return $this->_genericGet1('serialNumber');
  }

  protected function _genericGet1($key)
  {
    if (isset($this->info[$key])) {
      return $this->info[$key];
    } else {
      return FALSE;
    }
  }

  protected function _genericGet2($key)
  {
    if (isset($this->info['subject'][$key])) {
      return $this->info['subject'][$key];
    } else {
      return FALSE;
    }
  }

  /*
   * \brief Check if the certificate is valid
   *    Is valide if the length of array > 1
   *    and type is true
   */
  function isvalid()
  {
    return (($this->type != FALSE) && (count($this->info) > 1));
  }


  /*
   * \brief Export Certificate to specified file, with specified method
   *
   * \param boolean $type
   *
   * \param string $filename Initialized at 'temp'
   */
  function export($type, $filename = "temp")
  {
    /* Check if valid cert is loaded*/
    if ($this->type != FALSE) {
      /* Check if we must convert the cert */
      if ($this->type != $type) {
        $strConv = $this->type."TO".$type;
        $this->$strConv();
      }

      /* open file for writing */
      $fp = fopen($filename, "w+");

      if (!$fp) {
        $this->error = msgPool::cannotWriteFile($filename);
        return FALSE;
      } else {
        fwrite($fp, $this->data, strlen($this->data));
      }
      return TRUE;
    } else {
      $this->error = _("No valid certificate loaded!");
      return FALSE;
    }
    return FALSE;
  }


  /*
   * \brief Convert der to pem Certificate
   */
  function derTOpem()
  {
    /* if type is DER start convert */
    if ($this->type == DER) {
      /* converting */
      $this->type = PEM;

      $str = base64_encode($this->data);
      $len = strlen($str);

      $end = "";

      while ($len > 0 ) {
        $len  = $len - 64;
        $str1 = substr($str, 0, 64)."\n";
        $str  = substr($str, 64, $len);
        $end  .= $str1;
      }

      $strend = "-----BEGIN CERTIFICATE-----\n".$end;
      $strend .= "-----END CERTIFICATE-----";

      $this->data     = $strend;
      return TRUE;
    }
    return FALSE;
  }

  /*
   * Convert pem to der Certificate
   */
  function pemTOder()
  {
    if ($this->type == PEM) {
      $this->type = DER;

      $str = $this->data;

      $str = str_replace("-----BEGIN CERTIFICATE-----", "", $str);
      $str = str_replace("-----END CERTIFICATE-----", "", $str);

      $str = base64_decode($str);

      $this->data = $str;
      return TRUE;
    }
    return FALSE;
  }
}

?>
