<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
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

class department extends plugin
{
  /* department attributes */
  var $ou           = "";
  var $description  = "";
  var $base         = "";
  var $st           = "";
  var $l            = "";

  var $postalAddress            = "";
  var $businessCategory         = "";
  var $telephoneNumber          = "";
  var $facsimileTelephoneNumber = "";
  var $view_logged              = FALSE;

  var $type       = "organizationalUnit";
  var $namingAttr = "ou";

  /* Headpage attributes */
  var $last_dep_sorting = "invalid";
  var $departments      = array();
  var $must_be_tagged   = FALSE;

  /* attribute list for save action */
  var $attributes = array("ou", "description", "businessCategory", "st", "l", "postalAddress",
      "telephoneNumber", "facsimileTelephoneNumber", "manager");

  /* Do not append the structural object classes here, they are added dynamically in the constructor */
  var $objectclasses  = array("top", "gosaDepartment");
  var $structuralOC   = array("organizationalUnit");

  var $orig_base  = "";
  var $orig_ou    = "";
  var $baseSelector;

  var $manager_enabled  = FALSE;
  var $manager_name     = "";
  var $manager          = "";

  function department (&$config, $dn)
  {
    /* Add the default structural object class if this is a new entry
     */
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    if ($dn == "" || $dn == "new" || !$ldap->dn_exists($dn)) {
      $this->objectclasses = array_merge($this->structuralOC, $this->objectclasses);
    } else {
      $ldap->cat($dn, array("structuralObjectClass"));
      $attrs = $ldap->fetch();
      if (isset($attrs['structuralObjectClass']['count'])) {
        for ($i = 0; $i < $attrs['structuralObjectClass']['count']; $i++) {
          $this->objectclasses[] = $attrs['structuralObjectClass'][$i];
        }
      } else {
        /* Could not detect structural object class for this object, fall back to the default
         */
        $this->objectclasses = array_merge($this->structuralOC, $this->objectclasses);
      }
    }
    $this->objectclasses = array_unique($this->objectclasses);

    plugin::plugin($config, $dn);
    $this->is_account = TRUE;
    $this->ui = get_userinfo();
    $this->dn = $dn;
    $this->orig_dn = $dn;

    /* Save current naming attribute
     */
    $nA             = $this->namingAttr;
    $orig_nA        = "orig_".$nA;
    $this->$orig_nA = $this->$nA;

    $this->config = $config;

    /* Set base */
    if ($this->dn == "new") {
      $ui = get_userinfo();
      if (session::is_set('CurrentMainBase')) {
        $this->base = session::get('CurrentMainBase');
      } else {
        $this->base = dn2base($ui->dn);
      }
    } else {
      $this->base = preg_replace ("/^[^,]+,/", "", $this->dn);
    }

    $this->orig_base = $this->base;

    /* Instanciate base selector */
    $this->baseSelector = new baseSelector($this->get_allowed_bases(), $this->base);
    $this->baseSelector->setSubmitButton(FALSE);
    $this->baseSelector->setHeight(300);
    $this->baseSelector->update(TRUE);

    // If the 'manager' attribute is present in gosaDepartment allow to manage it.
    $ldap = $this->config->get_ldap_link();
    $ocs  = $ldap->get_objectclasses();
    if (isset($ocs['gosaDepartment']['MAY']) && in_array('manager', $ocs['gosaDepartment']['MAY'])) {
      $this->manager_enabled = TRUE;

      // Detect the managers name
      $this->manager_name = "";
      $ldap = $this->config->get_ldap_link();
      if (!empty($this->manager)) {
        $ldap->cat($this->manager, array('cn'));
        if ($ldap->count()) {
          $attrs = $ldap->fetch();
          $this->manager_name = $attrs['cn'][0];
        } else {
          $this->manager_name = "("._("Unknown")."!): ".$this->manager;
        }
      }
    }
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* Log view */
    if ($this->is_account && !$this->view_logged) {
      $this->view_logged = TRUE;
      new log("view", "department/".get_class($this), $this->dn);
    }

    /* Reload departments */
    $this->config->get_departments($this->dn);
    $this->config->make_idepartments();
    $smarty = get_smarty();
    $smarty->assign("usePrototype", "true");

    // Clear manager attribute if requested
    if (preg_match("/ removeManager/i", " ".implode(array_keys($_POST), ' ')." ")) {
      $this->manager      = "";
      $this->manager_name = "";
    }

    // Allow to manager manager attribute
    if ($this->manager_enabled) {

      // Allow to select a new inetOrgPersion:manager
      if (preg_match("/ editManager/i", " ".implode(array_keys($_POST), ' ')." ")) {
        $this->dialog = new singleUserSelect($this->config, get_userinfo());
      }
      if ($this->dialog && count($this->dialog->detectPostActions())) {
        $users = $this->dialog->detectPostActions();
        if (isset($users['targets']) && count($users['targets'])) {

          $headpage = $this->dialog->getHeadpage();
          $dn = $users['targets'][0];
          $attrs = $headpage->getEntry($dn);
          $this->manager = $dn;
          $this->manager_name = $attrs['cn'][0];
          $this->dialog = NULL;
        }
      }
      if (isset($_POST['add_users_cancel'])) {
        $this->dialog = NULL;
      }
      if ($this->dialog) {
        return $this->dialog->execute();
      }
    }
    $smarty->assign("manager", $this->manager);
    $smarty->assign("manager_name", $this->manager_name);
    $smarty->assign("manager_enabled", $this->manager_enabled);


    $tmp = $this->plInfo();
    foreach ($tmp['plProvidedAcls'] as $name => $translation) {
      $smarty->assign($name."ACL", $this->getacl($name));
    }

    /* Hide base selector, if this object represents the base itself
     */
    $smarty->assign("is_root_dse", FALSE);
    if ($this->dn == $this->config->current['BASE']) {
      $smarty->assign("is_root_dse", TRUE);
      $nA = $this->namingAttr."ACL";
      $smarty->assign($nA, $this->getacl($this->namingAttr, TRUE));
    }

    /* Hide all departments, that are subtrees of this department */
    $bases = $this->get_allowed_bases();
    if (($this->dn == "new") || ($this->dn == "")) {
      $tmp = $bases;
    } else {
      $tmp  = array();
      foreach ($bases as $dn => $base) {
        /* Only attach departments which are not a subtree of this one */
        if (!preg_match("/".preg_quote($this->dn)."/", $dn)) {
          $tmp[$dn] = $base;
        }
      }
    }
    $this->baseSelector->setBases($tmp);

    foreach ($this->attributes as $val) {
      $smarty->assign("$val", htmlentities($this->$val, ENT_COMPAT, 'UTF-8'));
    }
    $smarty->assign("base", $this->baseSelector->render());

    $smarty->assign("dep_type", $this->type);

    $dep_types = departmentManagement::get_support_departments();
    $tpl = "";
    foreach ($dep_types as $key => $data) {
      if ($data['OC'] == $this->type) {
        $tpl = $data['TPL'];
        break;
      }
    }
    if ($tpl == "") {
      trigger_error("No template specified for container type '".$this->type."', please update departmentManagement::get_support_departments().");
      $tpl = "generic.tpl";
    }
    return $smarty->fetch (get_template_path($tpl, TRUE));
  }

  function clear_fields()
  {
    $this->dn   = "";
    $this->base = "";

    foreach ($this->attributes as $val) {
      $this->$val = "";
    }
  }

  function remove_from_parent()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd ($this->dn);
    $ldap->rmdir_recursive($this->dn);
    new log("remove", "department/".get_class($this), $this->dn, array_keys($this->attrs), $ldap->get_error());
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
    }

    /* Optionally execute a command after we're done */
    $this->handle_post_events('remove');
  }

  function must_be_tagged()
  {
    return $this->must_be_tagged;
  }

  /* Save data to object */
  function save_object()
  {
    if (isset($_POST['dep_generic_posted'])) {

      $nA     = $this->namingAttr;
      $old_nA = $this->$nA;

      /* Create a base backup and reset the
         base directly after calling plugin::save_object();
         Base will be set seperatly a few lines below */
      $base_tmp = $this->base;
      plugin::save_object();
      $this->base = $base_tmp;

      /* Refresh base */
      if ($this->acl_is_moveable($this->base)) {
        if (!$this->baseSelector->update()) {
          msg_dialog::display(_("Error"), msgPool::permMove(), ERROR_DIALOG);
        }
        if ($this->base != $this->baseSelector->getBase()) {
          $this->base         = $this->baseSelector->getBase();
          $this->is_modified  = TRUE;
        }
      }

      /* If this is the root directory service entry then avoid
         changing the naming attribute of this entry.
       */
      if ($this->dn == $this->config->current['BASE']) {
        $this->$nA = $old_nA;
      }
    }
  }


  /* Check values */
  function check()
  {
    /* Call common method to give check the hook */
    $message = plugin::check();

    /* Check for presence of this department */
    $ldap = $this->config->get_ldap_link();
    $ldap->ls ("(&(ou=".$this->ou.")(objectClass=organizationalUnit))", $this->base, array('dn'));
    if ($this->orig_dn == "new" && $ldap->count()) {
      $message[] = msgPool::duplicated(_("Name"));
    } elseif ($this->orig_dn != $this->dn && $ldap->count()) {
      $message[] = msgPool::duplicated(_("Name"));
    }

    /* All required fields are set? */
    if ($this->ou == "") {
      $message[] = msgPool::required(_("Name"));
    }
    if ($this->description == "") {
      $message[] = msgPool::required(_("Description"));
    }

    if (tests::is_department_name_reserved($this->ou, $this->base)) {
      $message[] = msgPool::reserved(_("Name"));
    }

    if (preg_match ('/[#+:=>\\\\\/]/', $this->ou)) {
      $message[] = msgPool::invalid(_("Name"), $this->ou, "/[^#+:=>\\\\\/]/");
    }
    if (!tests::is_phone_nr($this->telephoneNumber)) {
      $message[] = msgPool::invalid(_("Phone"), $this->telephoneNumber, "/[\/0-9 ()+*-]/");
    }
    if (!tests::is_phone_nr($this->facsimileTelephoneNumber)) {
      $message[] = msgPool::invalid(_("Fax"), $this->facsimileTelephoneNumber, "/[\/0-9 ()+*-]/");
    }

    // Check if a wrong base was supplied
    if (!$this->baseSelector->checkLastBaseUpdate()) {
      $message[] = msgPool::check_base();;
    }

    /* Check if we are allowed to create or move this object
     */
    if ($this->orig_dn == "new" && !$this->acl_is_createable($this->base)) {
      $message[] = msgPool::permCreate();
    } elseif ($this->orig_dn != "new" && $this->base != $this->orig_base && !$this->acl_is_moveable($this->base)) {
      $message[] = msgPool::permMove();
    }

    return $message;
  }


  /* Save to LDAP */
  function save()
  {
    $ldap = $this->config->get_ldap_link();

    /* Ensure that ou is saved too, it is required by objectClass gosaDepartment
     */
    $nA       = $this->namingAttr;
    $this->ou = $this->$nA;

    plugin::save();

    /* Write back to ldap */
    $ldap->cat($this->dn, array('dn'));
    $ldap->cd($this->dn);

    if ($ldap->count()) {
      $this->cleanup();
      $ldap->modify ($this->attrs);
      new log("modify", "department/".get_class($this), $this->dn, array_keys($this->attrs), $ldap->get_error());
      $this->handle_post_events('modify');
    } else {
      $ldap->add($this->attrs);
      $this->handle_post_events('add');
      new log("create", "department/".get_class($this), $this->dn, array_keys($this->attrs), $ldap->get_error());
    }
    if (!$ldap->success()) {
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }

    return FALSE;
  }

  /* Move/Rename complete trees */
  function recursive_move($src_dn, $dst_dn, $force = FALSE)
  {
    /* Print header to have styles included */
    $smarty = get_smarty();

    echo '  <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2//EN">
      <html>
      <head>
      <title></title>
      <style type="text/css">@import url("themes/default/style.css");</style>
      <script language="javascript" src="include/focus.js" type="text/javascript"></script>
      </head>
      <body style="background: none; margin:4px;" id="body" >
      ';
    echo "<h3>".sprintf(_("Moving '%s' to '%s'"), "<i>".LDAP::fix($src_dn)."</i>", "<i>".LDAP::fix($dst_dn)."</i>")."</h3>";


    /* Check if the destination entry exists */
    $ldap = $this->config->get_ldap_link();

    /* Check if destination exists - abort */
    $ldap->cat($dst_dn, array('dn'));
    if ($ldap->fetch()) {
      trigger_error("Recursive_move ".LDAP::fix($dst_dn)." already exists.",
          E_USER_WARNING);
      echo sprintf("Recursive_move: '%s' already exists", LDAP::fix($dst_dn))."<br>";
      return FALSE;
    }

    /* Perform a search for all objects to be moved */
    $objects = array();
    $ldap->cd($src_dn);
    $ldap->search("(objectClass=*)", array("dn"));
    while ($attrs = $ldap->fetch()) {
      $dn           = $attrs['dn'];
      $objects[$dn] = strlen($dn);
    }

    /* Sort objects by indent level */
    asort($objects);
    reset($objects);

    /* Copy objects from small to big indent levels by replacing src_dn by dst_dn */
    foreach ($objects as $object => $len) {


      $src = str_replace("\\", "\\\\", $object);
      $dst = preg_replace("/".str_replace("\\", "\\\\", $src_dn)."$/", "$dst_dn", $object);
      $dst = str_replace($src_dn, $dst_dn, $object);

      echo "<b>"._("Object").":</b> ".LDAP::fix($src)."<br>";

      $this->update_acls($object, $dst, TRUE);

      if (!$this->copy($src, $dst)) {
        echo "<font color='#FF0000'><br>".sprintf(_("FAILED to copy %s, aborting operation"), LDAP::fix($src))."</font>";
        return FALSE;
      }
      echo "<script language=\"javascript\" type=\"text/javascript\">scrollDown2();</script>";
      flush();
    }

    /* Remove src_dn */
    $ldap->cd($src_dn);
    $ldap->recursive_remove();
    $this->orig_dn    = $this->dn = $dst_dn;
    $this->orig_base  = $this->base;
    $this->entryCSN   = getEntryCSN($this->dn);

    echo '<p class="seperator">&nbsp;</p>';

    echo "<div style='width:100%;text-align:right;'><form name='form' method='post' action='?plug=".$_GET['plug']."' target='_parent'>
      <br><input type='submit' name='back' value='"._("Continue")."'>
      </form></div>";

    echo "<script language=\"javascript\" type=\"text/javascript\">scrollDown2();</script>";
    echo "</body></html>";

    return TRUE;
  }


  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return array(
      "plShortName"   => _("Generic"),
      "plDescription" => _("Departments"),
      "plSelfModify"  => FALSE,
      "plCategory"    => array("department" => array("objectClass" => "gosaDepartment", "description" => _("Departments"))),
      "plObjectType"  => array("department" => array("name" => _("Department"), "filter" => "objectClass=gosaDepartment")),

      "plProvidedAcls" => array(
        "ou"                => _("Department name"),
        "description"       => _("Description"),
        "businessCategory"  => _("Category"),
        "base"              => _("Base"),

        "st"                        => _("State"),
        "l"                         => _("Location"),
        "postalAddress"             => _("Address"),
        "telephoneNumber"           => _("Telephone"),
        "facsimileTelephoneNumber"  => _("Fax"),
        "manager"                   => _("Manager"),
      )
    );
  }
}

?>
