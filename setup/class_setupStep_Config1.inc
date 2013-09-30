<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2007 Fabian Hickert
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

class Step_Config1 extends setup_step
{

  var $peopleou   = "ou=people";
  var $groupou    = "ou=groups";
  var $peopledn   = "uid";
  var $uidbase    = 1100;
  var $strict     = TRUE;

  var $cngen_active = FALSE;
  var $cngen        = "%sn% %givenName%";

  var $header_image       = "images/setup/server.png";
  var $account_expiration = FALSE;
  var $base_hook          = "";
  var $base_hook_active   = FALSE;

  var $encryption = "crypt/md5";
  var $theme      = "default";

  var $include_personal_title = FALSE;

  var $pwd_rules  = array(
    "pwminlen"                => 6,
    "pwminlen_active"         => FALSE,
    "pwdiffer"                => 5,
    "pwdiffer_active"         => FALSE,
    "externalpwdhook"         => "",
    "externalpwdhook_active"  => FALSE
  );

  var $id_settings = array(
    "idgen"         => "%laS|sn%-%laSs[2-4]|givenName%",
    "idgen_active"  => FALSE,
    "minid"         => "100",
    "minid_active"  => FALSE
  );

  var $crypt_methods  = array();

  var $attributes = array("peopleou","groupou","peopledn","uidbase","encryption","theme",
                          "base_hook","base_hook_active","account_expiration","strict","include_personal_title",
                          "cngen_active","cngen");

  function __construct()
  {
    $this->update_strings();

    $tmp = passwordMethod::get_available_methods();
    foreach ($tmp['name'] as $name) {
      $this->crypt_methods[$name] = $name;
    }
  }


  function update_strings()
  {
    $this->s_title      = _("FusionDirectory settings 1/3");
    $this->s_title_long = _("FusionDirectory settings 1/3");
    $this->s_info       = _("FusionDirectory generic settings");
  }


  function execute()
  {
    global $BASE_DIR;
    $smarty = get_smarty();
    $smarty->assign("peopledns",      array("uid","cn"));
    $smarty->assign("id_settings",    reverse_html_entities($this->id_settings));
    $smarty->assign("crypt_methods",  reverse_html_entities($this->crypt_methods));
    $smarty->assign("themes",         reverse_html_entities($this->get_themes()));
    $smarty->assign("pwd_rules",      reverse_html_entities($this->pwd_rules));
    $smarty->assign("bool", array(FALSE => _("No"), TRUE => _("Yes")));
    foreach ($this->attributes as $attr) {
      $smarty->assign($attr, htmlentities($this->$attr, ENT_QUOTES, "UTF-8"));
    }

    /* !!! Attention strict is inverse. If you change this,
     *  don't forget to change save_object too
     */
    $smarty->assign("strict", !$this->strict);
    return $smarty->fetch("$BASE_DIR/setup/setup_config1.tpl");
  }


  function get_themes()
  {
    $dir = opendir( "../ihtml/themes/");
    $themes = array();
    while (($file = readdir($dir)) !== FALSE) {
      if (is_dir("../ihtml/themes/".$file) && !preg_match("/^\./", $file)) {
        $themes[$file] = $file;
      }
    }
    return $themes;
  }

  function check()
  {
    $message = array();

    if (isset($this->id_settings['minid_active']) && !is_numeric($this->id_settings['minid'])) {
      $message[] = sprintf(_("The specified value for '%s' must be a numeric value"), _("GID / UID min id"));
    }

    if (preg_match("/,$/", $this->peopleou)) {
      $message[] = sprintf(_("Don't add a trailing comma to '%s'."), _("People storage ou"));
    }

    if (preg_match("/,$/", $this->groupou)) {
      $message[] = sprintf(_("Don't add a trailing comma to '%s'."), _("Group storage ou"));
    }

    if (!is_numeric($this->uidbase)) {
      $message[] = _("Uid base must be numeric");
    }

    if (($this->pwd_rules['pwminlen_active']) && !is_numeric($this->pwd_rules['pwminlen'])) {
      $message[] = _("The given password minimum length is not numeric.");
    }
    if (($this->pwd_rules['pwdiffer_active']) && !is_numeric($this->pwd_rules['pwdiffer'])) {
      $message[] = _("The given password differ value is not numeric.");
    }
    return $message;
  }

  function save_object()
  {
    if (isset($_POST['step5_posted'])) {

      /* Get attributes */
      foreach ($this->attributes as $attr) {
        if (isset($_POST[$attr])) {
          $this->$attr = validate(get_post($attr));
        }
      }

      /* !!! Attention strict is inverse. If you change this,
       *  don't forget to change the smarty assignment too
       */
      if (isset($_POST['strict'])) {
        if ($_POST['strict']) {
          $this->strict = FALSE;
        } else {
          $this->strict = TRUE;
        }
      }

      if (isset($_POST['minid_active'])) {
        $this->id_settings['minid_active'] = TRUE;
        if (isset($_POST['minid'])) {
          $this->id_settings['minid'] = get_post('minid');
        }
      } else {
        $this->id_settings['minid_active'] = FALSE;
      }

      /* Generic settings */
      if (isset($_POST['idgen_active'])) {
        $this->id_settings['idgen_active'] = TRUE;
        if (isset($_POST['idgen'])) {
          $this->id_settings['idgen'] = get_post('idgen');
        }
      } else {
        $this->id_settings['idgen_active'] = FALSE;
      }

      /* Get password settings */
      if (isset($_POST['pwdiffer_active'])) {
        $this->pwd_rules['pwdiffer_active'] = TRUE;
        if (isset($_POST['pwdiffer'])) {
          $this->pwd_rules['pwdiffer'] = get_post('pwdiffer');
        }
      } else {
        $this->pwd_rules['pwdiffer_active'] = FALSE;
      }

      /* Get password minimum length posts */
      if (isset($_POST['pwminlen_active'])) {
        $this->pwd_rules['pwminlen_active'] = TRUE;
        if (isset($_POST['pwminlen'])) {
          $this->pwd_rules['pwminlen'] = get_post('pwminlen');
        }
      } else {
        $this->pwd_rules['pwminlen_active'] = FALSE;
      }

      /* External pwd settings */
      if (isset($_POST['externalpwdhook_active'])) {
        $this->pwd_rules['externalpwdhook_active'] = TRUE;
        if (isset($_POST['externalpwdhook'])) {
          $this->pwd_rules['externalpwdhook'] = get_post('externalpwdhook');
        }
      } else {
        $this->pwd_rules['externalpwdhook_active'] = FALSE;
      }

      /* base hook settings */
      $this->pwd_rules['base_hook_active'] = FALSE;

      if (isset($_POST['cngen_active'])) {
        $this->cngen_active = TRUE;
        if (isset($_POST['cngen'])) {
          $this->cngen = get_post('cngen');
        }
      } else {
        $this->cngen_active = FALSE;
      }
    }

    $tmp = $this->check();
    if (count($tmp) == 0) {
      $this->is_completed = TRUE;
    } else {
      $this->is_completed = FALSE;
    }
  } // if template posted


  /* Attributes that are interesting for configuration generation */
  function get_attributes()
  {
    $tmp = setup_step::get_attributes();
    foreach (array("pwd_rules","id_settings") as $attr) {
      $tmp[$attr] = $this->$attr;
    }
    return $tmp;
  }
}

?>
