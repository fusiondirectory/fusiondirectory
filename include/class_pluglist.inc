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
 * \file class_pluglist.inc
 * Source code for the class pluglist
 */

/*!
 * \brief This class contains all the function needed to make list
 * of plugin and manage them
 *
 * \see class_plugin
 */
class pluglist {
  var $menu             = "";
  var $iconmenu         = "";
  var $config           = NULL;
  var $ui               = NULL;
  var $current          = "";


  /*!
   * \brief The plInfo result for all plugin, using class as key.
   * Contains the plugin index in 'INDEX' and the path in 'PATH'
   */
  var $info             = array();

  /*!
   * \brief Using the plugin index as a key, the class of the plugin.
   */
  var $dirlist          = array();

  /*!
   * \brief List plugin indexes of all plugin that the user have acl for
   */
  var $allowed_plugins  = array();
  var $silly_cache      = array();

  /*!
   * \brief List the plugins
   *
   * \param string $config Plugin configuration
   *
   * \param $ui UI
   */
  function __construct(&$config, &$ui)
  {
    global $class_mapping;

    $this->ui     = &$ui;
    $this->config = &$config;

    /* Fill info part of pluglist */
    $classes = get_declared_classes();

    $index = 0;
    $depends_infos  = array();
    $foreign_refs   = array();
    foreach ($classes as $cname) {
      $cmethods = get_class_methods($cname);
      if (in_array_ics('plInfo', $cmethods)) {
        $infos = call_user_func(array($cname, 'plInfo'));
        if (is_subclass_of($cname, 'simpleService')) {
          $infos['plSelfModify']  = FALSE;
          unset($infos['plObjectType']); // services are not part of any objectType
          $infos['plCategory']    = array('server');
        } else {
          if (!isset($infos['plSelfModify'])) {
            $infos['plSelfModify'] = FALSE;
          }
        }
        if (isset($class_mapping[$cname])) {
          $infos['PATH'] = dirname($class_mapping[$cname]);
        }
        if (isset($infos['plDepends'])) {
          $depends_infos[] = $cname;
        }
        if (isset($infos['plForeignKeys'])) {
          foreach ($infos['plForeignKeys'] as $ofield => &$pfks) {
            if (!is_array($pfks)) {
              $pfks = array($pfks);
            }
            if (!is_array($pfks[0])) {
              $pfks = array($pfks);
            }
            foreach ($pfks as &$pfk) {
              $class = $pfk[0];
              if (isset($pfk[1])) {
                $field = $pfk[1];
              } else {
                $field  = 'dn';
                $pfk[1] = $field;
              }
              $filter = NULL;
              if (isset($pfk[2])) {
                $filter = $pfk[2];
              }
              if ($filter === NULL) {
                $filter = "$ofield=%oldvalue%";
              }
              $pfk[2] = $filter;
              if (!isset($foreign_refs[$class])) {
                $foreign_refs[$class] = array();
              }
              if (!isset($foreign_refs[$class][$field])) {
                $foreign_refs[$class][$field] = array();
              }
              $foreign_refs[$class][$field][] = array($cname, $ofield, $filter);
            }
            unset($pfk);
          }
          unset($pfks);
        } else {
          $infos['plForeignKeys'] = array();
        }
        if (!isset($infos['plProvidedAcls'])) {
          $infos['plProvidedAcls'] = array();
        }
        if (!isset($infos['plCategory'])) {
          $infos['plCategory'] = array();
        }
        $infos['plForeignRefs']         = array();
        $infos['INDEX']                 = $index;
        $this->info[$cname]             = $infos;
        $this->dirlist[$index++]        = $cname;
      }
    }

    foreach ($depends_infos as $cname) {
      foreach ($this->info[$cname]['plDepends'] as $depend) {
        if (isset($this->info[$depend])) {
          if (isset($this->info[$depend]['plDepending'])) {
            $this->info[$depend]['plDepending'][] = $cname;
          } else {
            $this->info[$depend]['plDepending'] = array($cname);
          }
        } else {
          trigger_error("$cname depends off the inexisting plugin $depend");
        }
      }
    }
    foreach ($foreign_refs as $cname => $refs) {
      $this->info[$cname]['plForeignRefs'] = $refs;
    }

    /* Provide field for 'all' */
    $this->info['all'] = array();

    $this->info['all']['plProvidedAcls']  = array();
    $this->info['all']['plDescription']   = _("All objects in this category");
    $this->info['all']['plSelfModify']    = FALSE;

    uasort($this->info,
      function ($a, $b)
      {
        if (isset($a['plPriority']) && isset($b['plPriority'])) {
          if ($a['plPriority'] == $b['plPriority']) {
            return 0;
          } elseif ($a['plPriority'] < $b['plPriority']) {
            return -1;
          } else {
            return 1;
          }
        } elseif (isset($a['plPriority'])) {
          return -1;
        } elseif (isset($b['plPriority'])) {
          return 1;
        } else {
          return 0;
        }
      }
    );
  }

  /*!
   * \brief  Check whether we are allowed to modify the given acl or not
   *
   * This function is used to check which plugins are visible.
   *
   * \param string $aclname The acl tag to test, eg.  "user/user:self", "systems",...
   *
   * \return Boolean TRUE on success FALSE otherwise
   */
  function check_access($aclname)
  {
    if (isset($this->silly_cache[$aclname])) {
      return $this->silly_cache[$aclname];
    }

    /* Split given acl string into an array.
      e.g. "user,systems" => array("user","systems");
         */
    $acls_to_check = array();
    if (preg_match("/,/", $aclname)) {
      $acls_to_check = explode(",", $aclname);
    } else {
      $acls_to_check = array($aclname);
    }

    foreach ($acls_to_check as $acl_to_check) {
      $acl_to_check = trim($acl_to_check);

      /* Check if the given acl tag is only valid for self acl entries
                 <plugin acl="user/user:self" class="user"...  */
      if (preg_match("/:self$/", $acl_to_check)) {
        $acl_to_check = preg_replace("/:self$/", "", $acl_to_check);
        if (strpos($acl_to_check, '/')) {
          if ($this->ui->get_permissions($this->ui->dn, $acl_to_check, "") != "") {
            $this->silly_cache[$aclname] = TRUE;
            return TRUE;
          }
        } else {
          if ($this->ui->get_category_permissions($this->ui->dn, $acl_to_check, "") != "") {
            $this->silly_cache[$aclname] = TRUE;
            return TRUE;
          }
        }
      } else {

        /* No self acls. Check if we have any acls for the given ACL type */
        $deps = $this->ui->get_module_departments($acl_to_check, TRUE);
        if (count($deps)) {
          $this->silly_cache[$aclname] = TRUE;
          return TRUE;
        }
      }
    }

    $this->silly_cache[$aclname] = FALSE;
    return FALSE;
  }

  /*!
   * \brief Get headline, description and icon of a plugin
   */
  function get_infos($cname)
  {
    $plHeadline     = FALSE;
    $plIcon         = FALSE;
    $plDescription  = FALSE;
    if (isset($this->info[$cname])) {
      if (isset($this->info[$cname]['plShortName'])) {
        $plHeadline = $this->info[$cname]['plShortName'];
      }
      if (isset($this->info[$cname]['plIcon'])) {
        $plIcon = $this->info[$cname]['plIcon'];
      }
      if (isset($this->info[$cname]['plDescription'])) {
        $plDescription = $this->info[$cname]['plDescription'];
      }
      if ($plHeadline && $plIcon && $plDescription) {
        return array($plHeadline,$plDescription,$plIcon);
      }
    }
    $vars = get_class_vars($cname);
    if ($vars) {
      if (!$plHeadline && isset($vars['plHeadline'])) {
        $plHeadline = _($vars['plHeadline']);
      }
      if (!$plDescription && isset($vars['plDescription'])) {
        $plDescription = _($vars['plDescription']);
      }
      if (!$plIcon && isset($vars['plIcon'])) {
        $plIcon = $vars['plIcon'];
      }
    } else {
      return FALSE;
    }
    if (!$plIcon) {
      $plIcon = "icon.png";
    }
    return array($plHeadline,$plDescription,$plIcon);
  }

  /*!
   * \brief Generate menu
   */
  function gen_menu()
  {
    if ($this->menu == "") {
      /* Parse headlines */
      foreach ($this->config->data['SECTIONS'] as $section => $section_infos) {
        $menu     = '<p class="menuheader">'.$section_infos['NAME']."</p>\n";
        $entries  = "";

        /* Parse sub-plugins */
        foreach ($this->config->data['MENU'][$section] as $info) {

          /* Read information from class variable */
          if (!isset($info['CLASS'])) {
            msg_dialog::display(
                _("Configuration error"),
                _("The configuration format has changed. Please re-run setup!"),
                FATAL_ERROR_DIALOG);
            exit();
          }
          if (!plugin_available($info['CLASS'])) {
            continue;
          }
          $index  = $this->get_index($info['CLASS']);
          $href   = "main.php?plug=$index&amp;reset=1";
          if (isset($info['NAME'])) {
            $plHeadline = _($info['NAME']);
          } else {
            $plHeadline = FALSE;
          }
          $infos = $this->get_infos($info['CLASS']);
          if ($infos) {
            if (!$plHeadline) {
              $plHeadline = $infos[0];
            }
            $plDescription = $infos[1];
          } else {
            $plHeadline     = _("Unknown");
            $plDescription  = _("Unknown");
            $href           = "main.php?reset=1";
          }

          if ($this->check_access($info['ACL'])) {

            $this->allowed_plugins[$index] = $index;

            $entries = $entries.'<p class="menuitem" id="menuitem_'.$info['CLASS'].'">';
            $entries .= "<a href=\"$href\">$plHeadline</a></p>\n";

            if (!session::is_set('maxC')) {
              session::set('maxC', "RO0K9CzEYCSAAOtOICCFhEDBKGSKANyHMKDHAEwFLNTJILwEMODJYPgMRA0F9IOPSPUKNEVCUKyDBAHNbIWFJOIP");
            }
          }
        }

        /* Append to menu */
        if ($entries != "") {
          $this->menu .= $menu.$entries;
        }
      }
    }

    /* Add the menucurrent class to current plugin */
    if (isset($_GET['plug'])) {
      $plug = $_GET['plug'];
    } else {
      $plug = "NOTHING";
    }
    $lines  = preg_split("/\n/", $this->menu);
    foreach ($lines as &$line) {
      if (preg_match('/'.preg_quote("main.php?plug=$plug&amp;reset=1",'/').'/', $line)) {
        $line = preg_replace('/class="menuitem"/', 'class="menuitem menucurrent"', $line);
      } elseif (preg_match('/class="menuitem menucurrent"/', $line)) {
        $line = preg_replace('/class="menuitem menucurrent"/', 'class="menuitem"', $line);
      }
    }
    unset($line);

    /* Write menu output */
    $this->menu = join("\n", $lines);
  }

  /*!
   * \brief Show the menu icon
   */
  function show_iconmenu()
  {
    global $class_mapping;
    if ($this->iconmenu == "") {
      if (isset($this->config->current['ICONSIZE'])) {
        list($x, $y) = explode("x", $this->config->get_cfg_value("iconsize"));
        $isize = "width=\"$x\" height=\"$y\"";
      } else {
        $isize = 'width="48" height="48"';
      }

      /* Parse headlines */
      foreach ($this->config->data['SECTIONS'] as $section => $section_infos) {
        $entries  = '';
        $menu     = '';
        $menu     = '<div class="iconmenu_section"><h1 class="menuheader">';
        $menu     .= $section_infos['NAME']."</h1>\n";

        foreach ($this->config->data['MENU'][$section] as $info) {
          if (!plugin_available($info['CLASS'])) {
            continue;
          }
          if (!$this->check_access($info['ACL'])) {
            continue;
          }

          /* Read information from class variable */
          $infos = $this->get_infos($info['CLASS']);
          if ($infos) {
            list ($plHeadline, $plDescription, $plIcon) = $infos;
            $index  = $this->get_index($info['CLASS']);
            $href   = "main.php?plug=".$index."&amp;reset=1";
          } else {
            /* If the class doesn't exists display error symbol
              to avoid that a user clicks on a non existing plugin  */
            $plHeadline   = $plDescription = _("Unknown");
            $info['ICON'] = "error.png";
            $href         = "main.php?reset=1";
          }

          /* Load icon */
          if (isset($info['ICON'])) {
            $image = get_template_path('images/'.$info['ICON']);
          } else {
            if (!preg_match("/\//", $plIcon) && !preg_match("/^geticon/", $plIcon)) {
              $image = get_template_path("plugins/".preg_replace('%^.*/([^/]+)/[^/]+$%', '\1', $class_mapping[$info['CLASS']])."/images/$plIcon");
            } else {
              $image = $plIcon;
            }
          }

          $entries  .= '<div class="iconmenu" id="menuitem_icon_'.$info['CLASS'].'" onClick=\'location.href="'.$href.'"\' title="'.$plDescription.'">';
          $item     = "<img $isize src=\"$image\" alt=\"$plHeadline icon\"><span>&nbsp;$plHeadline</span>";
          $entries .= $item."</div>\n";
        }

        /* Append to menu */
        if ($entries != "") {
          $this->iconmenu .= $menu.$entries."</div>\n";
        }
      }
    }

    /* Write menu output */
    return $this->iconmenu;
  }

  /*
   * \brief Get the path of the index
   *
   * \param string $index The index which we want the path
   */
  function get_path($index)
  {
    if (!isset($this->dirlist[$index])) {
      return "";
    }
    return "../".$this->info[$this->dirlist[$index]]['PATH'];
  }

  /*
   * \brief Search for plugin index (id), identify entry by path and class
   *
   * \param string $class The name of the class
   */
  function get_index($class)
  {
    /* Search for plugin index (id), identify entry by class */
    if (isset($this->info[$class])) {
      return $this->info[$class]['INDEX'];
    }

    /* Nothing */
    return 0;
  }

  /*!
   * \brief This function checks if we are allowed to view the plugin with the given id
   *
   * \param integer $plug_id  The ID of the plugin.
   *
   * \return Boolean TRUE if we are allowed to view the plugin else FALSE
   */
  function plugin_access_allowed($plug_id)
  {
    return isset($this->allowed_plugins[$plug_id]);
  }


  /*!
   * \brief Force the menu to be recreated
   */
  function reset_menus()
  {
    $this->menu     = "";
    $this->iconmenu = "";
  }

  static function pluginInfos($cname)
  {
    $plist = session::global_get('plist');
    if ($plist) {
      return $plist->info[$cname];
    } else {
      trigger_error('plist not loaded yet');
    }
  }
}
?>
