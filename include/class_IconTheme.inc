<?php

class IconThemeDir
{
  private $Size;
  private $MinSize;
  private $MaxSize;
  private $Type      = 'Threshold';
  private $Threshold = 2;

  function __construct($infos)
  {
    $this->Size     = $infos['Size'];
    $this->MinSize  = $infos['Size'];
    $this->MaxSize  = $infos['Size'];
    foreach (array('Type', 'MaxSize', 'MinSize', 'Threshold') as $key) {
      if (isset($infos[$key])) {
        $this->$key = $infos[$key];
      }
    }
    /* Thanks to this Threshold and Scaled are the same */
    if ($this->Type == 'Threshold') {
      $this->MinSize = $this->Size - $this->Threshold;
      $this->MaxSize = $this->Size + $this->Threshold;
    }
  }

  function MatchesSize($size)
  {
    switch ($this->Type) {
      case 'Fixed':
        return ($this->Size == $size);
      case 'Threshold':
      case 'Scalable':
        return (($this->MinSize <= $size) && ($size <= $this->MaxSize));
    }
  }

  function SizeDistance($size)
  {
    switch ($this->Type) {
      case 'Fixed':
        return abs($this->Size - $size);
      case 'Threshold':
      case 'Scalable':
        if ($size < $this->MinSize) {
          return $this->MinSize - $size;
        }
        if ($size > $this->MaxSize) {
          return $size - $this->MaxSize;
        }
        return 0;
    }
  }
}

class IconTheme
{
  private $subdirs = array();
  private $path;
  private $parent;

  function __construct($folder, $default_parent = 'default')
  {
    $this->path = $folder;
    $datas  = parse_ini_file($folder.'/index.theme', TRUE, INI_SCANNER_RAW);
    if ($datas === FALSE) {
      throw new Exception('Error while parsing theme file');
    }
    $dirs   = preg_split('/,/', $datas['Icon Theme']['Directories']);
    foreach ($dirs as $name) {
      $this->subdirs[strtolower($datas[$name]['Context'])][$name] = new IconThemeDir($datas[$name]);
    }

    if (isset($datas['Inherits'])) {
      $this->parent = $datas['Inherits'];
    } else {
      $this->parent = $default_parent;
    }
  }

  function FindIcon($context, $icon, $size)
  {
    $context = strtolower($context);
    return $this->FindIconHelper($context, $icon, $size);
  }

  function FindIconHelper($context, $icon, $size)
  {
    $filename = $this->LookupIcon($context, $icon, $size);
    if ($filename != NULL) {
      return $filename;
    }

    if ($this->parent !== NULL) {
      if (isset(self::$themes[$this->parent])) {
        $parent = self::$themes[$this->parent];
      } else {
        $parent = self::$themes['default'];
      }
      return $parent->FindIconHelper($context, $icon, $size);
    }

    return NULL;
  }

  function LookupIcon($context, $iconname, $size)
  {
    foreach ($this->subdirs[$context] as $path => &$subdir) {
      if ($subdir->MatchesSize($size)) {
        foreach (array('png', 'svg', 'xpm') as $extension) {
          $filename = $this->path.'/'.$path.'/'.$iconname.'.'.$extension;
          if (file_exists($filename)) {
            return $filename;
          }
        }
      }
    }
    unset($subdir);
    $minimal_size = PHP_INT_MAX;
    foreach ($this->subdirs[$context] as $path => &$subdir) {
      if (($sizedistance = $subdir->SizeDistance($size)) < $minimal_size) {
        foreach (array('png', 'svg', 'xpm') as $extension) {
          $filename = $this->path.'/'.$path.'/'.$iconname.'.'.$extension;
          if (file_exists($filename)) {
            $closest_filename = $filename;
            $minimal_size     = $sizedistance;
          }
        }
      }
    }
    unset($subdir);
    if (isset($closest_filename)) {
      return $closest_filename;
    }
    return NULL;
  }

  static public $themes;
  static public function loadThemes($path)
  {
    if ($dir = opendir("$path")) {
      while (($file = readdir($dir)) !== FALSE) {
        if (file_exists("$path/$file/index.theme") && !preg_match("/^\./", $file)) {
          try {
            self::$themes[$file] = new IconTheme("$path/$file");
          } catch (Exception $e) {
          }
        }
      }
    }
  }
}

?>
