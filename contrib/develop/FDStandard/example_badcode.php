<?php

class bad_code {
  var $a = 1; // should complain about equals alignment
  var $bc = 2;

  var $a    = 1; // should not complain
  var $bcc  = 2;

  var $a= 1; // should complain about missing space before equal
  var $b= 2; // should complain about missing space before equal

  function bad_brace () { // should complain about the brace
    $this->a = 10;

    if ($this->a == 10) {
      $r = 1;
    } elseif ($this->a == 9) {
      $r = 2;
    } else {
      $r = 3;
    }

    if ($this->a == 10)
    { // should complain about the brace
      $r = 1;
    } // should complain about the brace
    elseif ($this->a == 9) {
      $r = 2;
    } // should complain about the brace
    else
    { // should complain about the brace
      $r = 3;
    }
  }

  function bad_indent ()
  {
    $b = 1; // should not complain

      $b = 2; // should complain about indentation

  $b = 3; // should complain about indentation

        $c = "4"; // should complain about indentation

    if (TRUE) {
    $b = 2; // should complain about indentation
    } else {
        $b = "5"; // should complain about indentation
      }// should complain about indentation
  }

  function bad_line_length ()
  {
    $s = "Something short"; // should not complain
    $s = "Something very long, this should warn about line length longer than 80";
    $s = "Something very very long, this should trigger an error about line length that is too big, because it is very long";
    $s = "Something not very long"; // should not complain, as it is the comment that goes too far.
  }

  function bad_spaces ($a,$b) // should complain
  {
    $a=1;   // should complain
    $a= 1;  // should complain

    $a =1;  // should complain
    $a = 1; // should not complain

    $a  = 1; // should not complain
    $ab = 1; // should not complain

    $this-> a = 1;  // should complain
    $this ->a = 1;  // should complain

    $this -> a = 1; // should complain

    $this->a = 1;   // should not complain

    if(TRUE) {        // should complain
      $a = 1;
    }elseif(FALSE) {  // should complain
      $a = 1;
    }else {           // should complain
      $a = 1;
    }

    if (FALSE){             // should complain
      $a = 1;
    } elseif($a == TRUE) {  // should complain
      $a = 1;
    } else{                 // should complain
      $a = 1;
    }

    if (TRUE) { // good one
      $a = 1;
    } elseif ($a == FALSE) {
      $a = 1;
    } else {
      $a = 1;
    }

    if (isset($a)) { // should not complain
      $a = 1;
    }

    some_function($a,$b); // should complain
    some_function($a, $b); // should not complain
    some_function($a,  $b); // should not complain
  }

  function bad_constant ()
  {
    $b = FALSE; // should not complain
    $b = False; // should complain
    $b = false; // should complain
    $b = TRUE;  // should not complain
    $b = True;  // should complain
    $b = true;  // should complain
    $b = NULL;  // should not complain
    $b = Null;  // should complain
    $b = null;  // should complain
  }

  function bad_comment ()
  {
    # should complain
  }

  function bad_return ()
  {
    return ($something); // should complain
  }

  function good_return ()
  {
    return ($something == "something"); // should not complain
  }

  function other_good_return ()
  {
    return $something; // should not complain
  }

  function bad_assignment ()
  {
    $a    = "aaaaa"; // should not complain for this assignment block
    $aa   = 23;
    $aaa  = 1;
    $b    = "a";
    $bb   = 0;

    $a  = "aaaaa"; // should complain for this assignment block
    $aa = 23;
    $aaa = 1;
    $b  = "a";
    $bb = 0;

    $a = 1; // should not complain
    for ($i = 0;$i < 10;$i++) { // should not complain
      $a = 1; // should not complain
    }
  }

}
