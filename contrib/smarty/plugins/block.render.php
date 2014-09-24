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
function smarty_block_render($params, $text, &$smarty)
{
	/* Skip closing tag </render> */
	if(empty($text)) {
		return("");
	}

	/* Get acl parameter */
	$acl = "";
	if (isset($params['acl'])) {
		$acl = $params['acl'];
	}

	/* Debug output */
	if (session::is_set('DEBUGLEVEL') && session::get('DEBUGLEVEL') & DEBUG_ACL ){
		echo "<div style='color:blue;font-size:2;'>&nbsp;".$acl."</div>";
	}



	/* Parameter : checkbox, checked
     *  If the parameter 'checkbox' is given, we create a html checkbox in front
     *   of the current object.
     *	The parameter 'checked' specifies whether the box is checked or not.
     *  The checkbox disables or enables the current object.
     */
	if(isset($params['checkbox']) && $params['checkbox']){

		/* Detect name and id of the current object */
		$use_text = preg_replace("/\n/"," ",$text);
		$name = preg_replace('/^.* name[ ]*=[ ]*("|\')([^\"\' ]*).*$/i',"\\2",$use_text);

		/* Detect id */
		if(preg_match("/ id=(\"|')[^\"']*(\"|')/i",$text)){
			$id = preg_replace('/^.* id[ ]*=[ ]*("|\')([^\"\' ]*).*$/i',"\\2",$use_text);
		}else{
			$id = "";
		}

		/* Is the box checked? */
		isset($params['checked'])&&$params['checked'] ? $check = " checked " : $check = "";

		/* If name isset, we have a html input field */
		if(!empty($name)){

			/* Print checkbox */
			echo "<input type='checkbox' name='use_".$name."' ".$check."
					onClick=\"changeState('".$name."');\" class='center'>";

			/* Disable current object, if checkbox isn't checked */
			if($check == ""){
				$text = preg_replace("/name=/i"," disabled name=",$text);
			}

			/* Add id to current entry, if it is missing */
			if($id == ""){
				$text = preg_replace("/name=/i"," id=\"".$name."\" name=",$text);
			}
		}
	}


	/* Read / Write*/
	if(preg_match("/w/i",$acl)){
		return ($text);
	}

	$text = preg_replace ("/\n/","GOSA_LINE_BREAK",$text);

	/* Disable objects, but keep those active that have mode=read_active */
	if(!(isset($params['mode']) && ($params['mode']=='read_active') && preg_match("/(r|w)/",$acl))){

		/* Disable options && greyout divlists */
		$from 	= array("/class=['\"]list1nohighlight['\"]/i",
				"/class=['\"]list0['\"]/i",
				"/class=['\"]list1['\"]/i",
				"/class=['\"]sortableListItem[^'\"]*['\"]/i");
		$to 	= array("class='list1nohighlightdisabled'",
				"class='list1nohighlightdisabled'",
				"class='list1nohighlightdisabled'",
				"class='sortableListItemDisabled'");

		if(!preg_match('/ disabled(="disabled")?( |\/?>)/', $text)){
			$from [] = "/name=/i" ;
			$to   [] = "disabled name=";
		}

		$text 	= preg_replace($from,$to,$text);

		/* Replace picture if object is disabled */
		if(isset($params['disable_picture'])){
			$syn = "/src=['\"][^\"']*['\"]/i";
			$new = "src=\"".$params['disable_picture']."\"";
			$text = preg_replace($syn,$new,$text);
		}
	}

	/* Read only */
	if(preg_match("/r/i",$acl)){
		return(preg_replace("/GOSA_LINE_BREAK/","\n",$text));
	}

	/* No acls */
	if(preg_match("/type['\"= ].*submit/",$text)){
		$text = preg_replace("/submit/","button",$text);
	}else{
		$text = preg_replace("/value=['\"][^\"']*['\"]/","",$text);
	}

	/* Remove select options */
	$from 	= array("#<option.*<\/option>#i",
			"/(<textarea.*>).*(<\/textarea>)/i",
			"/^(.*<input.*)checked(.*>.*)$/i");

	$to 	= array(" ",
			"\\1\\2",
			"\\1 \\2");
	$text 	= preg_replace($from,$to,$text);
	$text = preg_replace("/GOSA_LINE_BREAK/","\n",$text);

	return $text;
}

?>
