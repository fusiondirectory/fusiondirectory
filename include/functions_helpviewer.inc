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

/* Simple class to parse the xml help file */
class parseXml
{
  var $parser;
  var $filename;
  var $entries;

  function parseXml($file)
  {
    $this->parser   = xml_parser_create();
    $this->filename = $file;
    xml_set_object($this->parser, $this);
    xml_set_element_handler($this->parser, "tag_open", "tag_close");
    return($this->entries);
  }
  function parse()
  {
    $this->entries = array();
    $fh= fopen($this->filename, "r");
    $xmldata= fread($fh, 100000);
    fclose($fh);
    if(!xml_parse($this->parser, chop($xmldata))){
        print(sprintf(_("XML error in guide.xml: %s at line %d"),
            xml_error_string(xml_get_error_code($this->parser)),
            xml_get_current_line_number($this->parser)));
        exit;
    }
    return($this->entries);
  }
  function tag_open($parser,$tag,$attrs)
  {
    @$this->entries[$attrs['NAME']]=$attrs;
  }
  function tag_close(){; }

}


/* This function genereates the Index */
function genIndex()
{
  global $helpobject;
  $str = "";
  $helpobject = session::global_get('helpobject');
  $ui= get_userinfo();
  $test = new pluglist(session::global_get('config'), $ui);
  $current_hl = "";
  foreach($helpobject['helpconf'] as $id => $attrs){
    $path = $test -> get_path($id);
    $exists = true;
    $helpdir = "../doc/core/".$helpobject['lang']."/html/".preg_replace("/^.*\//i","",$path)."/";
    if(!is_dir($helpdir)){
      $exists = false;
    }
    $print_hl = false;
    if($current_hl != $attrs['HEADLINE']){
      $current_hl = $attrs['HEADLINE'];
      $str .= "<h1>"._($current_hl)."</h1>";
    }
    $name = $attrs['NAME'];
    $file = "index.html";
    //$path = $plug;
    if($exists){
      $str .= "<p style='padding-left:20px;'><a href='?plug=".$id."'><b>"._($name)."</b></a></p>";
    }else{
      $str .= "<p style='padding-left:20px;'><b>"._($name)."</b> ("._("No help available for this plugin.").")</p>";
    }
  }
  return (utf8_decode($str));
}


/* Some replacements */
$backwardlink  = "<a href=\"?pg=%s\"  class=\"maintitlebar\">
                  <img src='images/back.png' align=\"middle\" alt=\""._("previous")."\" border=\"0\">
                 </a>";

$forwardlink   = "<a href=\"?pg=%s\"  class=\"maintitlebar\">
                  <img src='images/forward.png' align=\"middle\" alt=\""._("next")."\" border=\"0\">
                 </a>";

$pre_mark                     = "<span style=\"background-color: #FFFC35;\">" ;                       // Sign words with this
$suf_mark                     = "</span>";                          //  and this


/* Define which tags musst be delete, header, navigation, banner */
$replacements=array();
$replacements['from']=array("@<!DOC.*<BODY >@si",
    "/border=\".*\"/i",
    "'<code.*code>'",
//    "/alt=\".*\"/i",
    "/<HR>/",
    "@<ADDRESS[^>]*?>.*?ADDRESS>@si",
    "@<\/BODY[^>]*?>.*?HTML>@si",
    "'<TABLE.*>'",
    "/src.*icons/i",
    "/src=\"/i",
    "/<H1 ALIGN=\"CENTER\">/",
 /* picture replacements */
 //  "",
    );

$replacements['to']=array("",
    " border=\"0\" ",
    "",
  //  "",
    "",
    "",
    "",
    "<table border=1 cellspacing=0 bgcolor=\"#E0E0E0\" width=\"95%\" align=\"center\" cellpadding=\"3\" summary=\"\">",
    "src=\"",
    "src=\"images/",
    "<H1>",
 /* picture replacements */
//    "",
  );


/* Reads all files in specified directory with contents an some inforations about the file */
/* Read all files with contents*/
/*                 |Folder="/var/ww...",
                   |        |Fileprefix="node"
                   |        |       |Filesuffix=".html"
                   |        |       |       |WithoutContent=false(This means : read content)
                   |        |       |       |          |Singlepage=false(Means read all, if w want to read single, specify its filename)"*/
function readfiles($basedir,$prefix,$suffix,$onlyIndex,$singlepage=false)
{
  global $replacements;

  $str    = array();  // Temporary variable
  $cnt    = 0;        // Array index creation
  $file   = "";       // Contains Filename

  $dir = opendir($basedir);

  $str['global']['start']       = $cnt;     // collect basic informations - Startpage
  $str['global']['basedir']     = $basedir; // collect basic informations - Basedirectory

  /* Startime for Benchmark */ 
  $start =   (time()+microtime());

  /* if singlepage == false -> Get all pages, */
  if(!$singlepage) {

    /* While theres is an unreaded file in our resource */
    while (($file = readdir($dir)) !== false) {
      
      if((is_dir($basedir."/".$file))&&($file != "..")&&($file[0]!=".")){
        $str[$file]=(readfiles($basedir."/".$file."/",$prefix,$suffix,$onlyIndex,false));
      }

      /* Filter all files which arn't intressting */
      if((strstr($file,$suffix))&&($file!=".")&&($file!="..")&&(strstr($file,$prefix))){

        /* Collect informations */
        $str[$file]=array();
        $str[$file]['name']   = $file;
        $str[$file]['size']   = filesize($basedir.$file);

        /* Readfile conent too ? */
        if(!$onlyIndex){
          $str[$file]['content']  = remove_unwanted_tags(linkwrapper(getcontents($basedir.$file),""),$replacements);
          $str[$file]['headline'] = getheader_from_content($str[$file]['content']);
        }

        /* Include file status, for debugging, not used in script yet */
        $str[$file]['stat']   = stat($basedir.$file);
        $cnt++;
      }
    }

    /* Only get on file*/
  }else{
    /* Pages read = 1 */       
    $cnt = 1;

    /* Prepare result*/
    $file                 = $singlepage;
    $str[$file]           = array();
    $str[$file]['name']   = $file;
    $str[$file]['size']   = filesize($basedir.$file);

    /* If onlyIndex == true skip reading content */
    if(!$onlyIndex){
      $str[$file]['content']  = remove_unwanted_tags(linkwrapper(getcontents($basedir.$file),""),$replacements);
      $str[$file]['headline'] = getheader_from_content($str[$file]['content']);
    }

    /* Include file status, for debugging, not used in script yet */
    $str[$file]['stat']   = stat($basedir.$file);
  }

  /* Sort to  right order */
  asort($str);

  /* Endtime for Benchmark*/
  $end = (time()+microtime());
  $str['global']['cmptime'] = $end-$start;

  /* Number of pages readed */
  $str['global']['numpages']= $cnt;
  closedir($dir);
  return($str);
}


/* Read filecontent */
function getcontents($file)
{
  $str = "" ;   // Temporary variable for file contents 
  $tmp = "" ;   // Temporary varibale for partitial file contents

  /* open file and read*/
  $fp = fopen($file,"r");
  if($fp) {
    while($tmp = fread($fp,512))
    {
      $str.=  $tmp;
    }
  }else{
    return(false);
  }
  return($str);
}


/*Remove tags */
function remove_unwanted_tags($str,$replacements)
{
  $str=preg_replace($replacements['from'],$replacements['to'],$str);
  return($str);
}


/*Converts the all links to specified path, is needed to get simple navigation */
function linkwrapper($str,$link)
{
  $str = preg_replace("/HREF=\"http/i","target=\"_blank\" href=\"http",$str);
  $str = str_replace("HREF=\"","href=\"".$link."?pg=",$str);  
  return($str);
}


/* Search content */
function search($arr,$word)
{
  global $minwordlength,$allowed_chars_in_searchword;
  /* Prepare Vars */ 
  $result                     =array(); // Search result, filename, + hits + hits per word + matches 
  $words                      =array(); // Temporary searchword handling
  $useablewords               =array(); // Temporary searchword handling
  $tryword                    = "";     // Temporary searchword handling
  $result['global']['maxhit'] = 0;
  session::un_set('lastresults');
  session::un_set('parsed_search_keyword');
  session::set('parsed_search_keyword',"");

  error_reporting(E_ALL | E_STRICT);

  /* prepare searchwords */
  $word   = trim($word);

  /* Filter all unusable chars */
  $word   = preg_replace($allowed_chars_in_searchword,"",$word);
  $words  = explode(" ",str_replace("+"," ",$word));

  /* Check all wordlengths */
  foreach($words as $tryword){
    $tryword = trim($tryword);

    /* Filter words smaler than 3 chars */
    if(strlen($tryword)>=$minwordlength) {
      session::set('parsed_search_keyword', session::get('parsed_search_keyword').$tryword." ");
      $useablewords[]=$tryword;
    }
  }

  /* Use words to search the content */
  foreach($arr as $keys=>$vals)
  {
    foreach($vals as $key=>$val){
      /* overallhits counts hits per page */
      $overallhits=0;

      /* Search all words */
      foreach($useablewords as $word)
      {
        /* Skip key global, it contains no file data - it is a summary info*/
        if($key!="global")
        {

          /* Get all hits for the word in $matches*/
          preg_match_all("/".$word."/i",$arr[$keys][$key]['content'], $matches,PREG_OFFSET_CAPTURE);

          /* Filter in Tag results*/
          if(count($matches[0])){
            foreach($matches[0] as $num=>$hit){
              if(isset($arr[$keys][$key]['content']) && (is_in_tag($arr[$keys][$key]['content'],$hit[1]))){
                unset($matches[0][$num]);    
              }    
            }
          }

          /* Count matches */
          $overallhits=$overallhits + count($matches[0]);    

          /* Save collected data */
          $result[$keys][$key]['hits'][$word]    = count($matches[0]); 
          $result[$keys][$key]['hits']['overall']= $overallhits;  

          /* Save max hits for page */
          if($overallhits > $result['global']['maxhit']){
            $result['global']['maxhit']=$overallhits;  
          }

          /* Add results for word to return value*/
          $result[$keys][$key]['match'][$word]=array();
          $result[$keys][$key]['match'][$word]=$matches[0];
        }
      }
    }
  }

  /* Save result in Session, so we can mark words later, or go back to search, without searching again*/
  session::set('lastresults',$result);
  return($result);
}


/* Detect 10 Best result entries, sort and call createResultEntry to create HTML output for  complete list */
function searchlist($arr,$res,$maxresults)
{
  $global = $res['global'];
  $topten = array();        // To detect 10 best solutions
  $ret    = "";             // return value
  unset($res['global']);

  /* Detect 10 best Sites */
  foreach($res as $key=>$resa)
  foreach($resa as $keya=>$val){
    /* Skip results with no hits */
    if($val['hits']['overall']>0){
      $topten[$key."/".$keya] = $val['hits']['overall'];
    }
  }

  /* Sort by hit position in content, to easier mark words */
  asort($topten);
  $topten = array_reverse($topten);
  $topten = (array_slice($topten,0,$maxresults));


  /* We have a result, an array with all content, an array with hits and position and we have the 10 best hits */
  /* Foreach */  
  foreach($topten as $key => $hits)  {

    $ks = explode("/",$key);
    $k1 = $ks[0];
    $k2 = $ks[1];
    
    $ret.= createResultEntry($arr[$k1][$k2],$res[$k1][$k2],$key,$global['maxhit']);    
  }

  /* appending footer message for resultlist */
  $ret.= "<br>
              ".sprintf(_("%s results for your search with the keyword %s"), 
                            "<b>".count($topten)."</b>", 
                            "<b>".session::get('parsed_search_keyword')."</b>");
  $ret.="<br>
        <br>";
  return($ret);
}


/* This function marks a string with the given search result for this string*/
function markup_page($arr,$res)
{
  global $pre_mark,$suf_mark;
  
  $ret    = "";             // return value
  $repl   = array();
  $posadd = 0;

  foreach($res['match'] as $word => $matches)   {
    foreach($matches as $matchnr=>$match)   {
      $repl[$match[1]]=$match[0];
    }
  }

  ksort($repl);
  
  foreach($repl as $position=>$word)  {
    $pos1 = strlen($arr);
    $arr= markword($arr,($position+$posadd),$word,$pre_mark,$suf_mark);
    $pos2 = strlen($arr);
    $posadd =$posadd + ($pos2 - $pos1);
  }
  return($arr); 
}


/* This function marks a single word with the specified prefix and suffix */
function markword($string,$position,$word,$prefix,$suffix)
{
  $wordlength   = strlen($word);
  $wholelength  = strlen($string); 

  $first = substr($string,0,$position);
  $last  = substr($string,($position+$wordlength),$wholelength);  

  return($first.$prefix.$word.$suffix.$last);
} 

/* Creates HTML output for a single search result entry */
function createResultEntry($entry,$res,$name,$max)
{
  $percentage = (int)(($res['hits']['overall'] / $max) * 100) ;
  $color  = dechex($percentage+150);
  $color2 = dechex(150 - $percentage);

  $entry['content'] = preg_replace("\"".$entry['headline']."\"","",$entry['content'],1);

  if(strlen($color)==1) $color = "0".$color;



  /* the object tag is needed for W3c */
  $str =  "<a href=\"?pg=".$name."&amp;mark=1\" title=\"".$percentage."% ".$entry['headline']."\">
          <object>
          <table summary=\"\"  width=\"98%\" align=\"center\">
            <tr>
              <td height=15>
                <b>".utf8_encode($entry['headline'])."</b> -".utf8_encode(substr(strip_tags($entry['content']),0,120))."...
              </td>
              <td width=50 valign=\"top\">".progressbar($percentage,50,8)."</td>
             </tr>
             <tr>
              <td colspan=2>
                <b>
                  ".(sprintf(_("%s%% hit rate in file %s"),$percentage,$name))."
                </b>
              </td>
            </tr>
          </table>
          </object></a>
          ";
  $str.=  "<hr size=\"1\">";

  return($str);
}


/*Simple function to detect if we prepare to change a tag or visible text */
function is_in_tag($string,$pos)
{
  $pos1 = $pos2 = 0;
  if(preg_match("/</",$string)){
    $pos1 = strpos($string,"<",$pos);
  }
  if(preg_match("/>/",$string)){
    $pos2 = strpos($string,">",$pos);
  }
  if ($pos1 > $pos2)  {
    return(true);
  }else{
    return(false);
  }
}

/*Returns frist line of readable text, it should be the headline */
function getheader_from_content($str)
{
  $str = strip_tags($str);
  $pos = 0;
  $arr = preg_split("/\n/",$str);
  foreach($arr as $possibleheadline){
    if(strlen($possibleheadline)>=3){
      return $possibleheadline; 
    }
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
