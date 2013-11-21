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

/* ACL categories list */
class ACLDialog extends GenericDialog
{
  protected $initialAclValue;
  protected $post_cancel = 'cancel_new_acl';
  protected $post_finish = 'submit_new_acl';

  function __construct($simplePlugin, &$attribute, $acl = NULL)
  {
    $this->attribute        = $attribute;
    $this->dialog           = new acl_createedit($simplePlugin->config, $acl);
    $this->initialAclValue  = $acl;
  }

  function handle_finish ()
  {
    $this->attribute->addValue($this->dialog->getAclEntry());
    return FALSE;
  }

  function handle_cancel ()
  {
    if ($this->initialAclValue !== NULL) {
      $this->attribute->addValue($this->initialAclValue);
    }
    return FALSE;
  }
}

/* complicated stuff */
class ACLsAttribute extends OrderedArrayAttribute
{
  protected $order = TRUE;
  protected $dialogClass = 'ACLDialog';

  function __construct ($label, $description, $ldapName, $required = FALSE, $defaultValue = array(), $acl = "")
  {
    Attribute::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->edit_enabled = TRUE;
  }

  function getAcl ()
  {
    return Attribute::getAcl();
  }

  function setAcl ($acl)
  {
    return Attribute::setAcl($acl);
  }

  function check ()
  {
    return Attribute::check();
  }

  function renderAttributeInput ($template = FALSE)
  {
  }

  function renderAttribute(&$attributes, $readOnly)
  {
    Attribute::renderAttribute($attributes, $readOnly);
  }

  function setParent(&$plugin)
  {
    Attribute::setParent($plugin);
  }

  function loadPostValue ()
  {
    if ($this->isVisible()) {
      parent::loadPostValue();
      parent::applyPostValue();
      if (isset($_POST['add'.$this->getHtmlId()])) {
        $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this));
      }
    }
  }

  protected function handleAddAndEditValue ()
  {
  }

  protected function handleEdit($key)
  {
    $this->editingValue = $this->value[$key];
    $this->delPostValue($key);
    $this->plugin->openDialog(new $this->dialogClass($this->plugin, $this, $this->editingValue));
  }

  function applyPostValue ()
  {
  }

  protected function getAttributeArrayValue($value)
  {
    /* Convert text value to displayable array value */

    /* Summarize ACL */
    $summary = "";
    foreach ($value as $name => $object) {
      if (count($object)) {
        $summary .= "$name, ";
      }
    }
    $summary = sprintf(_("Contains settings for these objects: %s"), preg_replace('/, $/', '', $summary));
    return array($summary);
  }

  function addValue($value)
  {
    $this->value[] = $value;
    $this->reIndexValues();
  }

  function readValue($value)
  {
    $acl = explode(':', $value, 2);
    return array($acl[0] => acl::extractACL($acl[1]));
  }

  function writeValue($key, $value)
  {
    $acl = '';
    foreach ($value as $object => $contents) {
      /* Only save, if we've some contents in there... */
      if (count($contents)) {
        $acl .= $object.';';
        foreach ($contents as $attr => $permission) {
          /* First entry? Its the one for global settings... */
          if ($attr == '0') {
            $acl .= $permission;
          } else {
            $acl .= '#'.$attr.';'.$permission;
          }
        }
        $acl .= ',';
      }
    }
    return $key.':'.preg_replace('/,$/', '', $acl);
  }
}

class aclRole extends simplePlugin
{
  var $objectclasses  = array('top','gosaRole');
  var $mainTab        = TRUE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Role'),
      'plDescription' => _('Access control roles'),
      'plObjectType'  => array('aclRole' => array(
        'aclCategory' => 'acl',
        'name'        => _('ACL role'),
        'filter'      => 'objectClass=gosaRole',
        'ou'          => get_ou('aclRoleRDN')
      )),

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo()
  {
    return array(
      'properties' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new BaseSelectorAttribute(get_ou('aclRoleRDN')),
          new HostNameAttribute(
            _('Name'), _('A name for this role'),
            'cn', TRUE
          ),
          new TextAreaAttribute(
            _('Description'), _('Short description of this role'),
            'description', FALSE
          ),
        )
      ),
      'acls' => array(
        'name'  => _('ACLs'),
        'class' => array('fullwidth'),
        'attrs' => array(
          new ACLsAttribute(
            '', _('ACLs which are part of this group'),
            'gosaAclTemplate', FALSE
          ),
        )
      ),
    );
  }

  function compute_dn()
  {
    return 'cn='.$this->cn.",".get_ou('aclRoleRDN').$this->base;
  }

  static function typeName($type)
  {
    $types = self::getTypeNames();
    return $types[$type];
  }

  static function getTypeNames()
  {
    static $types = NULL;
    if ($types === NULL) {
      $types = array(
        "reset" => _("Reset ACL"),
        "one"   => _("One level"),
        "base"  => _("Current object"),
        "sub"   => _("Complete subtree"),
        "psub"  => _("Complete subtree (permanent)")
      );
      asort($types);
    }
    return $types;
  }
}

class acl_createedit extends acl
{
  /* Helpers */
  var $dialogState      = "create";
  var $aclType          = "";
  var $aclObject        = "";
  var $aclContents      = array();
  var $aclTypes         = array();
  var $recipients       = array();
  var $savedAclContents = array();

  static function plInfo()
  {
  }

  function __construct (&$config, $acl = NULL)
  {
    /* Include config object */
    plugin::__construct($config, $config->current['BASE']);

    $this->loadAclEntry($acl);
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    $new_acl    = array();

    foreach ($_POST as $name => $post) {
      /* Actions... */
      if (preg_match('/^cat_edit_.*_x/', $name)) {
        $this->aclObject    = preg_replace('/^cat_edit_([^_]+)_.*$/', '\1', $name);
        $this->dialogState  = 'edit';
        foreach ($this->config->data['CATEGORIES'][$this->aclObject]['classes'] as $oc) {
          if (isset($this->aclContents[$oc])) {
            $this->savedAclContents[$oc] = $this->aclContents[$oc];
          }
        }
        continue;
      }

      if (!$this->acl_is_writeable("gosaAclEntry")) {
        continue;
      }

      if (preg_match('/^cat_del_.*_x/', $name)) {
        $idx = preg_replace('/^cat_del_([^_]+)_.*$/', '\1', $name);
        foreach ($this->config->data['CATEGORIES'][$idx]['classes'] as $key) {
          unset($this->aclContents["$idx/$key"]);
        }
        continue;
      }

      /* ACL saving... */
      if (preg_match('/^acl_([^_]+)_(.*)_([^_yx])$/', $name, $matches)) {
        $object     = $matches[1];
        $attribute  = $matches[2];
        $value      = $matches[3];

        /* Skip for detection entry */
        if ($object == 'dummy') {
          continue;
        }

        /* Ordinary ACL */
        if (!isset($new_acl[$object])) {
          $new_acl[$object] = array();
        }
        if (isset($new_acl[$object][$attribute])) {
          $new_acl[$object][$attribute] .= $value;
        } else {
          $new_acl[$object][$attribute] = $value;
        }
      }
    }

    /* Only be interested in new acl's, if we're in the right _POST place */
    if (isset($_POST['acl_dummy_0_0_0']) && ($this->aclObject != "") && is_array($this->config->data['CATEGORIES'][$this->aclObject])) {
      foreach ($this->config->data['CATEGORIES'][$this->aclObject]['classes'] as $oc) {
        unset($this->aclContents[$oc]);
        unset($this->aclContents[$this->aclObject.'/'.$oc]);
        if (isset($new_acl[$oc])) {
          $this->aclContents[$oc] = $new_acl[$oc];
        }
        if (isset($new_acl[$this->aclObject.'/'.$oc])) {
          $this->aclContents[$this->aclObject.'/'.$oc] = $new_acl[$this->aclObject.'/'.$oc];
        }
      }
    }

    /* Cancel edit acl? */
    if (isset($_POST['cancel_edit_acl'])) {
      $this->dialogState = 'create';
      foreach ($this->config->data['CATEGORIES'][$this->aclObject]['classes'] as $oc) {
        if (isset($this->savedAclContents[$oc])) {
          $this->aclContents[$oc] = $this->savedAclContents[$oc];
        }
      }
    }

    if ($this->acl_is_writeable("gosaAclEntry")) {
      /* Save edit acl? */
      if (isset($_POST['submit_edit_acl'])) {
        $this->dialogState = 'create';
      }

      /* Save common values */
      foreach (array("aclObject") as $key) {
        if (isset($_POST[$key])) {
          $this->$key = validate($_POST[$key]);
        }
      }
    }

    /* Create templating instance */
    $smarty = get_smarty();
    $smarty->assign("usePrototype", "true");

    $smarty->assign("gosaAclEntryACL", $this->getacl('gosaAclEntry'));

    if ($this->dialogState == 'create') {
      /* Draw list */
      $aclList = new divSelectBox("aclList");
      $aclList->SetHeight(450);

      /* Add settings for all categories to the (permanent) list */
      foreach ($this->config->data['CATEGORIES'] as $section => $infos) {
        $summary = "";
        foreach ($infos['classes'] as $oc) {
          if (isset($this->aclContents[$oc]) && count($this->aclContents[$oc]) && isset($this->aclContents[$oc][0]) &&
              $this->aclContents[$oc][0] != "") {

            $summary .= "$oc, ";
            continue;
          }
          if (isset($this->aclContents["$section/$oc"]) && count($this->aclContents["$section/$oc"])) {
            $summary .= "$oc, ";
            continue;
          }
          if (isset($this->aclContents[$oc]) && !isset($this->aclContents[$oc][0]) && count($this->aclContents[$oc])) {
            $summary .= "$oc, ";
          }
        }

        /* Set summary... */
        if ($summary == "") {
          $summary = '<i>'._("No ACL settings for this category").'</i>';
        } else {
          $summary = sprintf(_("ACL for these objects: %s"), preg_replace('/, $/', '', $summary));
        }

        $action = "";
        if ($this->acl_is_readable("gosaAclEntry")) {
          $action .= "<input class='center' type='image' src='images/lists/edit.png'
            alt='"._("Edit")."' name='cat_edit_$section'   title='"._("Edit category ACL")."'>";
        }
        if ($this->acl_is_writeable("gosaAclEntry")) {
          $action .= "<input class='center' type='image' src='images/lists/trash.png'
            alt='"._("Delete")."' name='cat_del_$section' title='"._("Reset category ACL")."'>";
        }

        $field1 = array("html" => $infos['description'], "attach" => "style='width:140px'");
        $field2 = array("html" => $summary);
        $field3 = array("html" => $action, "attach" => "style='border-right:0px;width:40px'");
        $aclList->AddEntry(array($field1, $field2, $field3));
      }

      $smarty->assign("aclList", $aclList->DrawList());
    } elseif ($this->dialogState == 'edit') {
      $smarty->assign('headline', sprintf(_("Edit ACL for '%s'"), $this->config->data['CATEGORIES'][$this->aclObject]['description']));

      /* Collect objects for selected category */
      foreach ($this->config->data['CATEGORIES'][$this->aclObject]['classes'] as $idx => $class) {
        if ($idx == 0) {
          continue;
        }
        $pInfos = pluglist::pluginInfos($class);
        $aclObjects[$this->aclObject.'/'.$class] = $pInfos['plShortName'];
      }
      if ($this->aclObject == 'all') {
        $aclObjects['all'] = _("All objects in current subtree");
      }
      $smarty->assign('aclSelector', $this->buildAclSelector($aclObjects));
    }

    /* Show main page */
    $smarty->assign("dialogState", $this->dialogState);

    return $smarty->fetch(get_template_path('acl_role.tpl', dirname(__FILE__)));
  }

  function loadAclEntry($acl = NULL)
  {
    /* New entry gets presets... */
    if ($acl === NULL) {
      $this->aclContents  = array();
    } else {
      $this->aclContents  = $acl;
    }
  }

  function getAclEntry()
  {
    return $this->aclContents;
  }

  function save_object()
  {
    plugin::save_object();
  }
}
?>
