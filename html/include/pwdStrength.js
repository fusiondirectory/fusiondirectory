/*************************************************************
Created: 20060120
Author:  Steve Moitozo <god at zilla dot us>
Description: This is a quick and dirty password quality meter 
     written in JavaScript
License: MIT License (see below)
=================================
Revision Author: Dick Ervasti (dick dot ervasti at quty dot com)
Revision Description: Exchanged text based prompts for a graphic thermometer
=================================
Revision Author: Jay Bigam jayb <o> tearupyourlawn <o> com
Revision Date: Feb. 26, 2007
Revision Description: Changed D. Ervasti's table based "thermometer" to CSS.
Revision Notes: - Verified to work in FF2, IE7 and Safari2
                - Modified messages to reflect Minimum strength requirement
                - Added formSubmit button disabled until minimum requirement met
=================================
Modified: 20061111 - Steve Moitozo corrected regex for letters and numbers
                     Thanks to Zack Smith -- zacksmithdesign.com
                     and put MIT License back in
Modified: 20100201 - Cajus Pollmeier stripped parts unnessesary for GOsa and
                     moved to prototype. Stripped comments.
---------------------------------------------------------------
Copyright (c) 2006 Steve Moitozo <god at zilla dot us>

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall
be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.
---------------------------------------------------------------
 
************************************************************ */
function testPasswordCss(passwd)
{
    var intScore   = 0
    
    // PASSWORD LENGTH
    if (passwd.length==0 || !passwd.length)                         // length 0
    {
      intScore = -1
    }
    else if (passwd.length>0 && passwd.length<5) // length between 1 and 4
    {
      intScore = (intScore+3)
    }
    else if (passwd.length>4 && passwd.length<8) // length between 5 and 7
    {
      intScore = (intScore+6)
    }
    else if (passwd.length>7 && passwd.length<12)// length between 8 and 15
    {
      intScore = (intScore+12)
    }
    else if (passwd.length>11)                    // length 16 or more
    {
      intScore = (intScore+18)
    }
    
    
    // LETTERS (Not exactly implemented as dictacted above because of my limited understanding of Regex)
    if (passwd.match(/[a-z]/))                              // [verified] at least one lower case letter
    {
      intScore = (intScore+1)
    }
    
    if (passwd.match(/[A-Z]/))                              // [verified] at least one upper case letter
    {
      intScore = (intScore+5)
    }
    
    // NUMBERS
    if (passwd.match(/\d+/))                                 // [verified] at least one number
    {
      intScore = (intScore+5)
    }
    
    if (passwd.match(/(.*[0-9].*[0-9].*[0-9])/))             // [verified] at least three numbers
    {
      intScore = (intScore+5)
    }
    
    
    // SPECIAL CHAR
    if (passwd.match(/.[!,@,#,$,%,^,&,*,?,_,~]/))            // [verified] at least one special character
    {
      intScore = (intScore+5)
    }
    
                                 // [verified] at least two special characters
    if (passwd.match(/(.*[!,@,#,$,%,^,&,*,?,_,~].*[!,@,#,$,%,^,&,*,?,_,~])/))
    {
      intScore = (intScore+5)
    }
  
    
    // COMBOS
    if (passwd.match(/([a-z].*[A-Z])|([A-Z].*[a-z])/))        // [verified] both upper and lower case
    {
      intScore = (intScore+2)
    }

          if (passwd.match(/([a-zA-Z])/) && passwd.match(/([0-9])/)) // [verified] both letters and numbers
          {
                  intScore = (intScore+2)
          }

    // [verified] letters, numbers, and special characters
    if (passwd.match(/([a-zA-Z0-9].*[!,@,#,$,%,^,&,*,?,_,~])|([!,@,#,$,%,^,&,*,?,_,~].*[a-zA-Z0-9])/))
    {
      intScore = (intScore+2)
    }

  
    if(intScore == -1)
    {
       $('meterEmpty').setStyle({width: '100%'});
         $('meterFull').setStyle({width: '0%'});
    }
    else if(intScore > -1 && intScore < 16)
    {
       $('meterEmpty').setStyle({width: '100%'});
         $('meterFull').setStyle({width: '0%'});
    }
    else if (intScore > 15 && intScore < 25)
    {
       $('meterEmpty').setStyle({width: '100%'});
         $('meterFull').setStyle({width: '25%'});
    }
    else if (intScore > 24 && intScore < 35)
    {
       $('meterEmpty').setStyle({width: '100%'});
         $('meterFull').setStyle({width: '50%'});
    }
    else if (intScore > 34 && intScore < 45)
    {
       $('meterEmpty').setStyle({width: '100%'});
         $('meterFull').setStyle({width: '75%'});
    }
    else
    {
       $('meterEmpty').setStyle({width: '100%'});
         $('meterFull').setStyle({width: '100%'});
    }
}
