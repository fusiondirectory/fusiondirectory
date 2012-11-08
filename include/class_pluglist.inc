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
  var $index            = 0;
  var $menu             = "";
  var $iconmenu         = "";
  var $menuparts        = array();
  var $config           = NULL;
  var $dirlist          = array();
  var $ui               = NULL;
  var $current          = "";
  var $info             = array();
  var $allowed_plugins  = array();
  var $silly_cache      = array();

  /*!
   * \brief List the plugins
   *
   * \param string $config Plugin configuration
   *
   * \param $ui UI
   */
  function pluglist(&$config, &$ui)
  {
    $this->ui     = &$ui;
    $this->config = &$config;

    /* Create dirlist for all plugins */
    $this->dirlist = $this->get_plugins ($this->dirlist, $this->config->data['MENU']);

    /* Fill info part of pluglist */
    $classes = get_declared_classes();

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
        $this->info[$cname] = $infos;
      }
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
   * \brief Get the plugins of a list
   *
   * \param array $list
   *
   * \param string $config Plugin configuration
   */
  function get_plugins($list, &$config)
  {
    global $class_mapping;

    if (!isset($config['CLASS'])) {
      if (is_array($config)) {
        foreach ($config as $val) {
          $list = $this->get_plugins($list, $val);
        }
      }
    } else {
      if (is_array($config) && isset($class_mapping[$config['CLASS']])) {
        $list[$this->index++] = dirname($class_mapping[$config['CLASS']]);
      } else {
        $list[$this->index++] = "";
      }
    }

    return $list;
  }


  /*!
   * \brief  Check whether we are allowed to modify the given acl or not
   *
   * This function is used to check which plugins are visible.
   *
   * \param string $aclname The acl tag to test, eg.  "users/user:self", "systems",...
   *
   * \return Boolean TRUE on success FALSE otherwise
   */
  function check_access($aclname)
  {
    if (isset($this->silly_cache[$aclname])) {
      return $this->silly_cache[$aclname];
    }

    /* Split given acl string into an array.
      e.g. "user,systems" => array("users","systems");
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
                 <plugin acl="users/user:self" class="user"...  */
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
      $plIcon = "plugin.png";
    }
    return array($plHeadline,$plDescription,$plIcon);
  }

  /*!
   * \brief Generate menu
   */
  function gen_menu()
  {
    if ($this->menu == "") {
      $first  = TRUE;
      $cfg    = $this->config->data['MENU'];

      /* Parse headlines */
      foreach ($cfg as $headline => $plug) {
        if ($first) {
          $style = "";
          $first = FALSE;
        } else {
          $style = "style='border-top:1px solid #AAA; margin-top:0.8em;'";
        }
        $menu = "<p class=\"menuheader\" $style>"._($headline)."</p>\n";
        $entries = "";
        $this->menuparts[_($headline)] = array();

        /* Parse sub-plugins */
        foreach ($plug as $info) {

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
          $href   = "main.php?plug=$index&reset=1";
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

            $entries = $entries."<p class=\"menuitem\" ".
              "onClick='return question(\""._("You are currently editing a database entry. Do you want to dismiss the changes?")."\", \"$href\");'>";
            if (session::global_get('js')) {
              $entries .= "$plHeadline</p>\n";
            } else {
              $entries .= "<a class=\"menuitem\" href=\"$href\">$plHeadline</a></p>\n";
            }

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

    /* Write menu output */
    return $this->menu;
  }

  function gen_current()
  {
    /* Do we have a current value? */
    if ($this->current == "") {
      $tmp = array_keys($this->menuparts);
      $this->current = $tmp[0];
    }

    /* Fill current array */
    $result = "<table width=\"100%\" summary=\"\">";
    $count  = 0;
    foreach ($this->menuparts[$this->current] as $entry) {
      if ($count == 2) {
        $result .= "</tr>";
        $count  = 0;
      }
      if ($count == 0) {
        $result .= "<tr>";
      }
      $result .= "<td>$entry</td>";
      $count++;
    }

    /* Add missing cell? */
    if ($count == 1) {
      $result .= "<td>&nbsp;</td>";
    }

    $result .= "</table>";
    return $result;
  }

  /*!
   * \brief Show the menu icon
   */
  function show_iconmenu()
  {
    global $class_mapping;
    if ($this->iconmenu == "") {
      $cfg = $this->config->data['MENU'];

      if (isset($this->config->current['ICONSIZE'])) {
        list($x, $y) = explode("x", $this->config->get_cfg_value("iconsize"));
        $isize = "width=\"$x\" height=\"$y\"";
      } else {
        $isize = "";
      }

      /* Parse headlines */
      foreach ($cfg as $headline => $plug) {
        $col = 1;
        $menu = "<h1 class=\"menuheader\">".
          _($headline)."</h1>\n<table summary=\"\" style=\"width:100%;".
          "font-size: 14px;\" cellpadding=7 border=0>\n<tr>\n";
        $entries = "";

        foreach ($plug as $info) {

          if (!plugin_available($info['CLASS'])) {
            continue;
          }

          /* Read information from class variable */
          $infos = $this->get_infos($info['CLASS']);
          if ($infos) {
            list ($plHeadline, $plDescription, $plIcon) = $infos;
            $index  = $this->get_index($info['CLASS']);
            $href   = "main.php?plug=".$index."&amp;reset=1";
          } else {
            /* Check if class is available. If the class doesn't exists display error symbol
              to avoid that a user clicks on a non existing plugin  */
            $plHeadline = $plDescription = _("Unknown");
            $info['ICON'] = "error.png";
            $href = "main.php?reset=1";
          }

          if ($this->check_access($info['ACL'])) {
            /* Load icon */
            if (isset($info['ICON'])) {
              $image = get_template_path('images/'.$info['ICON']);
            } else {
              if (!preg_match("/\//", $plIcon)) {
                $image = get_template_path("plugins/".preg_replace('%^.*/([^/]+)/[^/]+$%', '\1', $class_mapping[$info['CLASS']])."/images/$plIcon");
              } else {
                $image = $plIcon;
              }
            }
            if ($col > 5) {
              $entries  = $entries."</tr><tr>";
              $col      = 1;
            }
            $entries .= "<td class=\"iconmenu\" style=\"width:20%;\" onClick='location.href=\"".$href."\"'>";
            if (session::global_get('js')) {
              $entries .= "<img $isize border=0 align=middle src=\"$image\" alt=\"*\">&nbsp;$plHeadline";
            } else {
              $entries .= "<a class=\"iconmenu\" href=\"".$href."\">".
                "<img $isize border=0 align=middle src=\"$image\" alt=\"*\">&nbsp;$plHeadline</a>";
            }
            $entries .= "</td>\n";
            $col++;
          }
        }

        /* Append to menu */
        if ($entries != "") {
          $this->iconmenu .= $menu.$entries;

          /* Fill up remaining columns */
          if ($col != 1) {
            $col--;
            while ($col % 5) {
              $this->iconmenu = $this->iconmenu."<td style=\"width:20%\">&nbsp;</td>\n";
              $col++;
            }
          }

          /* close table */
          $this->iconmenu = $this->iconmenu."</tr>\n</table>\n";
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
    return "../".$this->dirlist[$index];
  }

  /*
   * \brief Search for plugin index (id), identify entry by path and class
   *
   * \param string $class The name of the class
   */
  function get_index($class)
  {
    /* Search for plugin index (id), identify entry by path && class */
    $data = $this->config->data['MENU'];
    foreach ($data as $section => $plugins) {
      foreach ($plugins as $key => $plugin) {
        if ($plugin['CLASS'] == $class) {
          return $key;
        }
      }
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

}
?>
