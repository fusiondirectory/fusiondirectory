<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2011-2018  FusionDirectory

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

class ThemeFileParsingException extends Exception
{
}

/*!
 * \brief Icon theme directory
 */
class IconThemeDir
{
  /* Nominal (unscaled) size of the icons in this directory.
   * Required. */
  private $Size;

  /* Specifies the minimum (unscaled) size that the icons in this directory can be scaled to.
   * Defaults to the value of Size if not present. */
  private $MinSize;

  /* Specifies the maximum (unscaled) size that the icons in this directory can be scaled to.
   * Defaults to the value of Size if not present. */
  private $MaxSize;

  /* The type of icon sizes for the icons in this directory.
   * Valid types are Fixed, Scalable and Threshold.
   * The type decides what other keys in the section are used.
   * If not specified, the default is Threshold. */
  private $Type      = 'Threshold';

  /* The icons in this directory can be used if the size differ at most this much from the desired (unscaled) size.
   * Defaults to 2 if not present. */
  private $Threshold = 2;

  public function __construct ($infos)
  {
    $this->Size     = $infos['Size'];
    $this->MinSize  = $infos['Size'];
    $this->MaxSize  = $infos['Size'];
    foreach (['Type', 'MaxSize', 'MinSize', 'Threshold'] as $key) {
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

  public function MatchesSize ($size)
  {
    switch($this->Type) {
      case 'Fixed':
        return ($this->Size == $size);
      default:
      case 'Threshold':
      case 'Scalable':
        return (($this->MinSize <= $size) && ($size <= $this->MaxSize));
    }
  }

  public function SizeDistance ($size)
  {
    switch($this->Type) {
      case 'Fixed':
        return abs($this->Size - $size);
      default:
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

/*!
 * \brief Icon theme
 */
class IconTheme
{
  private $subdirs = [];
  private $path;
  private $parent;

  public function __construct ($folder, $default_parent)
  {
    $this->path = $folder;
    $datas  = @parse_ini_file($folder.'/index.theme', TRUE, INI_SCANNER_RAW);
    if ($datas === FALSE) {
      throw new ThemeFileParsingException('Error while parsing theme file');
    }
    if (isset($datas['Icon Theme']['Directories']) && !empty($datas['Icon Theme']['Directories'])) {
      $dirs   = preg_split('/,/', $datas['Icon Theme']['Directories']);
      foreach ($dirs as $name) {
        $this->subdirs[strtolower($datas[$name]['Context'])][$name] = new IconThemeDir($datas[$name]);
      }
    }

    if (isset($datas['Icon Theme']['Inherits'])) {
      $this->parent = $datas['Icon Theme']['Inherits'];
    } else {
      $this->parent = $default_parent;
    }
  }

  public function FindIcon ($context, $icon, $size)
  {
    $context = strtolower($context);
    if (strpos($icon, '/') !== FALSE) {
      return NULL;
    }
    return $this->FindIconHelper($context, $icon, $size);
  }

  protected function FindIconHelper ($context, $icon, $size)
  {
    $filename = $this->LookupIcon($context, $icon, $size);
    if ($filename != NULL) {
      return $filename;
    }
    if (isset(static::$fallbacks[$context.'/'.$icon])) {
      foreach (static::$fallbacks[$context.'/'.$icon] as $fallback) {
        $filename = $this->LookupIcon($fallback[0], $fallback[1], $size);
        if ($filename != NULL) {
          return $filename;
        }
      }
    }

    if ($this->parent !== NULL) {
      $parent = $this->findTheme($this->parent);
      if ($parent === NULL) {
        $parent = $this->findTheme(static::$default_theme);
      }
      return $parent->FindIconHelper($context, $icon, $size);
    }

    return NULL;
  }

  protected function LookupIcon ($context, $iconname, $size)
  {
    if (!isset($this->subdirs[$context])) {
      return NULL;
    }
    foreach ($this->subdirs[$context] as $path => &$subdir) {
      if ($subdir->MatchesSize($size)) {
        foreach (static::$extensions as $extension) {
          $filename = $this->path.'/'.$path.'/'.$iconname.'.'.$extension;
          if (file_exists($filename)) {
            return $filename;
          }
        }
      }
    }
    unset($subdir);
    if (static::$find_closest) {
      $minimal_size = PHP_INT_MAX;
      foreach ($this->subdirs[$context] as $path => &$subdir) {
        if (($sizedistance = $subdir->SizeDistance($size)) < $minimal_size) {
          foreach (static::$extensions as $extension) {
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
    }
    return NULL;
  }

  static public $default_theme  = 'breezy';
  static public $extensions     = ['png', 'xpm', 'svg'];
  static public $find_closest   = FALSE;

  /* We store themes in the session. To do otherwise, override these methods. */
  static public $session_var    = 'IconThemes';
  static public function loadThemes ($path)
  {
    $themes = [];
    if ($dir = opendir("$path")) {
      while (($file = readdir($dir)) !== FALSE) {
        if (file_exists("$path/$file/index.theme") && !preg_match("/^\./", $file)) {
          try {
            if ($file == static::$default_theme) {
              $themes[$file] = new IconTheme("$path/$file", NULL);
            } else {
              $themes[$file] = new IconTheme("$path/$file", static::$default_theme);
            }
          } catch (ThemeFileParsingException $e) {
            continue;
          }
        }
      }
    }
    $_SESSION[static::$session_var] = $themes;
  }

  static public function findThemeIcon ($theme, $context, $icon, $size)
  {
    if (!isset($_SESSION[static::$session_var])) {
      die('Error: no theme found in session');
    }
    if (isset($_SESSION[static::$session_var][$theme])) {
      return $_SESSION[static::$session_var][$theme]->FindIcon($context, $icon, $size);
    }
    return $_SESSION[static::$session_var][static::$default_theme]->FindIcon($context, $icon, $size);
  }
  public function findTheme ($theme)
  {
    if (isset($_SESSION[static::$session_var][$theme])) {
      $ret = &$_SESSION[static::$session_var][$theme];
      return $ret;
    }
    return NULL;
  }

  /* Fallback system */
  static public $fallbacks = [
    'types/user-group'              => [
      ['applications','system-users']
    ],
    'types/resource-group'          => [
      ['actions','resource-group']
    ],
    'types/user'                    => [
      ['places','user-identity'],
      ['status','avatar-default'],
    ],
    'types/contact'                 => [
      ['mimetypes','x-office-contact'],
    ],
    'types/certificate'             => [
      ['mimetypes','stock_certificate'],
      ['mimetypes','application-certificate'],
      ['actions','view-certificate'],
    ],
    'applications/user-info'        => [
      ['actions','user-properties'],
      ['types','contact'],
      ['mimetypes','x-office-contact'],
      ['types','user'],
      ['places','user-identity'],
      ['status','avatar-default'],
    ],
    'applications/office-calendar'  => [
      ['mimetypes','x-office-calendar'],
    ],
    'applications/os-linux'         => [
      ['applications','linux'],
    ],
    'applications/os-windows'       => [
      ['applications','windows'],
    ],
    'applications/samba'            => [
      ['applications','os-windows'],
      ['applications','windows'],
    ],
    'applications/config-language'  => [
      ['applications','locale'],
      ['applications','preferences-desktop-locale'],
    ],
    'mimetypes/text-csv'            => [
      ['mimetypes','x-office-spreadsheet'],
      ['mimetypes','text-x-generic'],
    ],
    'mimetypes/application-pdf'     => [
      ['mimetypes','x-office-document'],
    ],
    'actions/application-exit'      => [
      ['actions','system-log-out'],
    ],
    'actions/document-export'       => [
      ['actions','document-send'],
    ],
    'actions/download'              => [
      ['actions','document-save'],
    ],
    'actions/document-restore'      => [
      ['actions','document-import'],
      ['actions','document-open'],
    ],
    'actions/document-edit'         => [
      ['actions','edit'],
      ['applications','text-editor'],
      ['applications','accessories-text-editor'],
      ['actions','document-open'],
    ],
    'actions/snapshot'              => [
      ['actions','document-save'],
    ],
    'actions/system-reboot'         => [
      ['actions','view-refresh'],
    ],
    'actions/system-update'         => [
      ['applications','system-software-update'],
    ],
    'actions/system-reinstall'      => [
      ['applications','system-installer'],
    ],
    'actions/task-start'            => [
      ['actions','media-playback-start'],
    ],
    'actions/task-stop'             => [
      ['actions','media-playback-stop'],
    ],
    'actions/task-schedule'         => [
      ['actions','chronometer'],
      ['actions','smallclock'],
    ],
    'actions/up'                    => [
      ['actions','go-up'],
      ['actions','arrow-up'],
    ],
    'actions/upload'                => [
      ['actions','document-import'],
      ['actions','up'],
    ],
    'actions/down'                  => [
      ['actions','go-down'],
      ['actions','arrow-down'],
    ],
    'actions/previous'              => [
      ['actions','go-previous'],
      ['actions','arrow-left'],
    ],
    'actions/next'                  => [
      ['actions','go-next'],
      ['actions','arrow-right'],
    ],
    'actions/submit'                => [
      ['actions','go-jump'],
    ],
    'categories/settings'           => [
      ['categories','gnome-settings'],
      ['categories','preferences-other'],
      ['categories','preferences-system'],
    ],
    'categories/checks'             => [
      ['actions','view-task'],
      ['actions','view-calendar-tasks'],
      ['actions','checkbox'],
      ['status','task-complete'],
    ],
    'devices/server'                => [
      ['places','server'],
      ['places','network-server'],
    ],
    'devices/media-cdrom'           => [
      ['devices','media-optical'],
    ],
    'devices/terminal'              => [
      ['applications','utilities-terminal'],
    ],
    'devices/computer-windows'      => [
      ['applications','os-windows'],
      ['applications','windows'],
    ],
    'devices/template'              => [
      ['actions','document-new'],
    ],
    'status/object-locked'          => [
      ['status','changes-prevent'],
    ],
    'status/object-unlocked'        => [
      ['status','changes-allow'],
    ],
    'status/task-waiting'        => [
      ['actions','task-schedule'],
    ],
    'places/folder-network'        => [
      ['places','folder-remote'],
    ],
  ];
}

?>
