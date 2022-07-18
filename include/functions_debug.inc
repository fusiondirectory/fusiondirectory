<?php

/*----------------------------------------------------------------------
* Title.........: Debug Lib
* Version.......: 0.5.4
* Author........: Thomas Schüßler <tulpe@atomar.de>
* Filename......: debuglib.php(s)
* Last changed..: 16. July 2003
* License.......: Free to use. Postcardware ;)
*
*-----------------------------------------------------------------------
*
* Functions in this library:
*
* print_a( array array [,int mode] )
*   prints arrays in a readable, understandable form.
*   if mode is defined the function returns the output instead of
*   printing it to the browser
*
* Happy debugging and feel free to email me your comments.
*
* History: (starting with version 0.5.3 at 2003-02-24)
*
*   - added tooltips to the td's showing the type of keys and values (thanks Itomic)
* 2003-07-16
*   - pre() function now trims trailing tabs
----------------------------------------------------------------------*/

/*!
 * \file functions_debug.inc
 * Source code for print_a class
 * and helper function
 */

/*!
 * \brief print_a class and helper function
 * prints out an array in a more readable way
 * than print_r()
 *
 * based on the print_a() function from
 * Stephan Pirson (Saibot)
 */
class printAClass {

  // this can be changed to FALSE if you don't like the fancy string formatting
  var $look_for_leading_tabs = TRUE;

  var $output;
  var $iterations;
  var $key_bg_color   = '1E32C8';
  var $value_bg_color = 'DDDDEE';
  var $fontsize       = '8pt';
  var $keyalign       = 'center';
  var $fontfamily     = 'Verdana';
  var $export_flag;
  var $show_object_vars;
  var $export_dumper_path = 'http://tools.www.mdc.xmc.de/print_a_dumper/print_a_dumper.php';
  /* i'm still working on the dumper! don't use it now
  * put the next line into the print_a_dumper.php file (optional)
  * print htmlspecialchars( stripslashes ( $_POST['array'] ) ); */
  var $export_hash;

  /*!
   * \brief printAClass constructor
   */
  function __construct ()
  {
    $this->export_hash = uniqid('');
  }


  /*! recursive function!
   * if print_a() was called with a fourth parameter (1 or 2)
   * and you click on the table a window opens with only the output of print_a() in it
   * 1 = serialized array
   * 2 = normal print_a() display
   *
   * put the following code on the page defined with $export_dumper_path;
   * --->%---- snip --->%----
   * if($_GET['mode'] == 1) {
   *  print htmlspecialchars( stripslashes ( $_POST['array'] ) );
   * } elseif($_GET['mode'] == 2) {
   *  print_a(unserialize( stripslashes($_POST['array'])) );
   * }
   * ---%<---- snip ---%<----
   *
   *
   * \param array $array
   *
   * \param boolean $iteration false
   *
   * \param boolean $key_bg_color false
   */
  function print_a ($array, $iteration = FALSE, $key_bg_color = FALSE)
  {
    if (!$key_bg_color) {
      $key_bg_color = $this->key_bg_color;
    }

    if (!$iteration && isset($this->export_flag)) {
      $this->output .= '<form id="pa_form_'.$this->export_hash.'" action="'.$this->export_dumper_path.'?mode='.$this->export_flag.'" method="post" target="_blank"><input name="array" type="hidden" value="'.htmlspecialchars( serialize( $array ) ).'"></form>';
    }

    // lighten up the background color for the key td's =)
    if ($iteration) {
      for ($i = 0; $i < 6; $i += 2) {
        $c = substr( $key_bg_color, $i, 2 );
        $c = hexdec( $c );
        $c += 15;
        if ($c > 255) {
          $c = 255;
        }
        if (!isset($tmp_key_bg_color)) {
          $tmp_key_bg_color = '';
        }
        $tmp_key_bg_color .= sprintf( "%02X", $c );
      }
      $key_bg_color = $tmp_key_bg_color;
    }

    // build a single table ... may be nested
    $this->output .= '<table style="border:none;" '.( !$iteration && $this->export_flag ? 'onClick="document.getElementById(\'pa_form_'.$this->export_hash.'\').submit();" )' : '' ).'>';
    foreach ($array as $key => $value) {

      $value_style  = 'color:black;';
      $key_style    = 'color:white;';

      $type = gettype($value);

      // change the color and format of the value
      switch ($type) {
        case 'array':
          break;

        case 'object':
          $key_style = 'color:#FF9B2F;';
          break;

        case 'integer':
          $value_style = 'color:green;';
          break;

        case 'double':
          $value_style = 'color:red;';
          break;

        case 'bool':
          $value_style = 'color:blue;';
          break;

        case 'resource':
          $value_style = 'color:darkblue;';
          break;

        case 'string':
          if ($this->look_for_leading_tabs && preg_match('/^\t/m', $value)) {
            $search       = ['/\t/', "/\n/"];
            $replace      = ['&nbsp;&nbsp;&nbsp;','<br />'];
            $value        = preg_replace( $search, $replace, htmlspecialchars($value));
            $value_style  = 'color:black;border:1px gray dotted;';
          } else {
            $value_style  = 'color:black;';
            $value        = nl2br(htmlspecialchars($value));
          }
          break;
      }

      $this->output .= '<tr>';
      $this->output .= '<td nowrap align="'.$this->keyalign.'" style="background-color:#'.$key_bg_color.';'.$key_style.';font:bold '.$this->fontsize.' '.$this->fontfamily.';" title="'.gettype( $key ).'['.$type.']">';
      $this->output .= $key;
      $this->output .= '</td>';
      $this->output .= '<td nowrap="nowrap" style="background-color:#'.$this->value_bg_color.';font: '.$this->fontsize.' '.$this->fontfamily.'; color:black;">';

      // value output
      if ($type == 'array') {
        if (count($value)) {
          $this->print_a( $value, TRUE, $key_bg_color );
        } else {
          $this->output .= '<div style="color:blue;">Array (empty)</div>';
        }
      } elseif ($type == 'object') {
        if ($this->show_object_vars) {
          $this->print_a( get_object_vars( $value ), TRUE, $key_bg_color );
        } else {
          $this->output .= '<div style="'.$value_style.'">OBJECT - '.get_class($value).'</div>';
        }
      } else {
        $this->output .= '<div style="'.$value_style.'" title="'.$type.'">'.$value.'</div>';
      }

      $this->output .= '</td>';
      $this->output .= '</tr>';
    }
    $this->output .= '</table>';
  }
}

/*
 * \brief helper function.. calls print_a () inside the printAClass
 *
 * \param array $array
 *
 * \param boolean $return_mode false
 *
 * \param boolean $show_object_vars false
 *
 * \param boolean $export_flag false
 */
function print_a ($array, $return_mode = FALSE, $show_object_vars = FALSE, $export_flag = FALSE )
{
  $e = error_reporting(0);
  if (is_array($array) || is_object($array)) {
    $pa = new printAClass;
    if ($show_object_vars) {
      $pa->show_object_vars = TRUE;
    }
    if ($export_flag) {
      $pa->export_flag = $export_flag;
    }

    $pa->print_a($array);

    $output = &$pa->output;
  } else {
    $output = '<span style="color:red;font-size:small;">print_a( '.gettype( $array ).' )</span>';
  }

  error_reporting ($e);
  if ($return_mode) {
    return $output;
  } else {
    print $output;
    return TRUE;
  }
}
