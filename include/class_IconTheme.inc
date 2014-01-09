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

  function __construct($folder, $default_parent)
  {
    $this->path = $folder;
    $datas  = @parse_ini_file($folder.'/index.theme', TRUE, INI_SCANNER_RAW);
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
    if (isset(self::$fallbacks[$context.'/'.$icon])) {
      foreach (self::$fallbacks[$context.'/'.$icon] as $fallback) {
        $filename = $this->LookupIcon($fallback[0], $fallback[1], $size);
        if ($filename != NULL) {
          return $filename;
        }
      }
    }

    if ($this->parent !== NULL) {
      $parent = $this->findTheme($this->parent);
      if ($parent === NULL) {
        $parent = $this->findTheme(self::$default_theme);
      }
      return $parent->FindIconHelper($context, $icon, $size);
    }

    return NULL;
  }

  function LookupIcon($context, $iconname, $size)
  {
    foreach ($this->subdirs[$context] as $path => &$subdir) {
      if ($subdir->MatchesSize($size)) {
        foreach (self::$extensions as $extension) {
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
        foreach (self::$extensions as $extension) {
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

  static public $default_theme  = 'default';
  static public $extensions     = array('png', 'xpm');

  /* We store themes in the session. To do otherwise, override these methods. */
  static public $session_var    = 'IconThemes';
  static public function loadThemes($path)
  {
    $themes = array();
    if ($dir = opendir("$path")) {
      while (($file = readdir($dir)) !== FALSE) {
        if (file_exists("$path/$file/index.theme") && !preg_match("/^\./", $file)) {
          try {
            if ($file == self::$default_theme) {
              $themes[$file] = new IconTheme("$path/$file",  NULL);
            } else {
              $themes[$file] = new IconTheme("$path/$file", self::$default_theme);
            }
          } catch (Exception $e) {
          }
        }
      }
    }
    $_SESSION[self::$session_var] = $themes;
  }
  static public function findThemeIcon($theme, $context, $icon, $size)
  {
    if (isset($_SESSION[self::$session_var][$theme])) {
      return $_SESSION[self::$session_var][$theme]->FindIcon($context, $icon, $size);
    }
    return $_SESSION[self::$session_var][self::$default_theme]->FindIcon($context, $icon, $size);
  }
  public function findTheme($theme)
  {
    if (isset($_SESSION[self::$session_var][$theme])) {
      $ret = &$_SESSION[self::$session_var][$theme];
      return $ret;
    }
    return NULL;
  }

  /* Fallback system */
  static public $fallbacks = array(
    'types/user-group'              => array(
      array('applications','system-users')
    ),
    'types/resource-group'          => array(
      array('actions','resource-group')
    ),
    'types/user'                    => array(
      array('places','user-identity'),
      array('status','avatar-default'),
    ),
    'applications/user-info'        => array(
      array('actions','user-properties'),
      array('types','contact'),
      array('mimetypes','x-office-contact'),
    ),
    'types/contact'                 => array(
      array('mimetypes','x-office-contact'),
    ),
    'applications/office-calendar'  => array(
      array('mimetypes','x-office-calendar'),
    ),
    'actions/application-exit'      => array(
      array('actions','system-log-out'),
    ),
    'mimetypes/text-csv'            => array(
      array('mimetypes','x-office-spreadsheet'),
      array('mimetypes','text-x-generic'),
    ),
    'mimetypes/application-pdf'     => array(
      array('mimetypes','x-office-document'),
    ),
    'actions/document-export'      => array(
      array('actions','document-send'),
    ),
    'actions/download'      => array(
      array('actions','document-save'),
    ),
    'actions/document-restore'      => array(
      array('actions','document-import'),
      array('actions','document-open'),
    ),
    'actions/document-edit'      => array(
      array('actions','edit'),
      array('applications','text-editor'),
      array('applications','accessories-text-editor'),
      array('actions','document-open'),
    ),
    'categories/settings'      => array(
      array('categories','gnome-settings'),
      array('categories','preferences-other'),
      array('categories','preferences-system'),
    ),
  );
}

?>
