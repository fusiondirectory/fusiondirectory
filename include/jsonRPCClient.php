<?php
/*
          COPYRIGHT

Copyright 2007 Sergio Vaccaro <sergio@inservibile.org>

This file is part of JSON-RPC PHP.

JSON-RPC PHP is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

JSON-RPC PHP is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with JSON-RPC PHP; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*!
 * \file jsonRPCClient.php
 * Source code for class jsonRPCClient
 */

/*!
 * \brief The object of this class are generic jsonRPC 1.0 clients
 * http://json-rpc.org/wiki/specification
 *
 * \author sergio <jsonrpcphp@inservibile.org>
 */
class jsonRPCClient {

  /*!
   * \brief Debug state
   *
   * \var boolean $debug
   */
  private $debug;

  /*!
   * \brief The server URL
   *
   * !var string $url
   */
  private $url;

  /*!
   * \brief The request id
   *
   * \var integer $id
   */
  private $id;

  /*!
   * \brief If true, notifications are performed instead of requests
   *
   * \var boolean $notification
   */
  private $notification = false;

  /*!
   * \brief If true, HTTPS is used instead of HTTP
   *
   * \var boolean $use_ssl
   */
  private $use_ssl = false;

  /*!
   * \brief Path to the .crt file of the trusted CA
   *
   * \var string $cacertfile
   */
  private $cacertfile = "";

  /*!
   * \brief Takes the connection parameters
   *
   * \param string $url
   *
   * \param boolean $debug false
   */
  public function __construct($url, $cacertfile = "", $debug = false) {
    // server URL
    $this->url = $url;
    // proxy
    empty($proxy) ? $this->proxy = '' : $this->proxy = $proxy;
    // debug state
    $this->debug = $debug;
    // message id
    $this->id = 1;
    $this->use_https  = (preg_match("@^https://@i",$this->url)?TRUE:FALSE);
    $this->cacertfile   = $cacertfile;
  }

  /*!
   * \brief Sets the notification state of the object.
   *        In this state, notifications are performed, instead of requests.
   *
   * \param boolean $notification
   */
  public function setRPCNotification($notification) {
    empty($notification) ?
              $this->notification = false
              :
              $this->notification = true;
  }

  /*!
   * \brief Performs a jsonRCP request and gets the results as an array
   *
   * \param string $method
   *
   * \param array $params
   *
   * \return array
   */
  public function __call($method,$params) {

    $debug="";

    // check
    if (!is_scalar($method)) {
      throw new Exception('Method name has no scalar value');
    }

    // check
    if (is_array($params)) {
      // no keys
      $params = array_values($params);
    } else {
      throw new Exception('Params must be given as array');
    }

    // sets notification or request task
    if ($this->notification) {
      $currentId = NULL;
    } else {
      $currentId = $this->id;
    }

    // prepares the request
    $request = array(
            'method' => $method,
            'params' => $params,
            'id' => $currentId
            );
    $request = json_encode($request);
    $this->debug && $debug.='***** Request *****'."\n".$request."\n".'***** End Of request *****'."\n\n";


    if (!$this->use_https) {
      // performs the HTTP POST
      $opts = array ('http' => array (
                'method'  => 'POST',
                'header'  => 'Content-type: application/json',
                'content' => $request
                ));

      $context  = stream_context_create($opts);
      if ($fp = @fopen($this->url, 'r', false, $context)) {
        $response = '';
        while($row = fgets($fp)) {
          $response.= trim($row)."\n";
        }
        $this->debug && $debug.='***** Server response *****'."\n".$response.'***** End of server response *****'."\n";
        $response = json_decode($response,true);
      } else {
        throw new Exception('Unable to connect to '.$this->url);
      }
    } else {
      // performs the HTTPS POST
      $res = $this->send_post_ssl_curl($this->url,$request,$this->cacertfile);
      $response = json_decode($res['DATA'],true);
    }

    // debug output
    if ($this->debug) {
      echo nl2br($debug);
    }

    // final checks and return
    if (!$this->notification) {
      // check
      if ($response['id'] != $currentId) {
        throw new Exception('Incorrect response id (request id: '.$currentId.', response id: '.$response['id'].')');
      }
      if (!is_null($response['error'])) {
        throw new Exception('Request error: '.$response['error']);
      }

      return $response['result'];

    } else {
      return true;
    }
  }

  public function send_post_ssl_curl ($url,$request,$cacertfile)
  {
    // create a new curl resource
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, $url);

    curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, true);

    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, true);
    curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2);
    curl_setopt($ch, CURLOPT_CAINFO, $cacertfile);

    curl_setopt($ch, CURLOPT_POST, true);
    curl_setopt($ch, CURLOPT_POSTFIELDS, $request);
    curl_setopt($ch, CURLOPT_HTTPHEADER, array('Content-type: application/json'));
    curl_setopt($ch, CURLOPT_HEADER, false); //FALSE to exclude the header from the output (otherwise it screws up json_decode)
    curl_setopt($ch, CURLOPT_NOBODY, false); //FALSE because we want the body too
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true); // get the response as a string from curl_exec(), rather than echoing it
    curl_setopt($ch, CURLOPT_FRESH_CONNECT, true); // don't use a cached version of the url
    //curl_setopt($ch, CURLOPT_TIMEOUT, 4);
    $returnData = curl_exec($ch);
    if (curl_errno($ch)) {
      throw new Exception('Unable to connect to '.$url.' : '.curl_error($ch));
    }
    $httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    curl_close($ch);

    return array('CODE'=>$httpcode,'DATA'=>$returnData);
  }
}
?>
