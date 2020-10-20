<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)

  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2020  FusionDirectory

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
class ACLEditionDialog implements FusionDirectoryDialog
{
  protected $attribute;

  protected $post_cancel = 'add_cancel';
  protected $post_finish = 'add_finish';

  protected $initialAclValue;
  protected $dialogState      = 'create';
  protected $aclObject        = '';
  protected $aclContents      = [];
  protected $savedAclContents = [];

  function __construct ($simplePlugin, $attribute, $acl = NULL)
  {
    $this->attribute        = $attribute;
    $this->initialAclValue  = $acl;

    /* New entry gets presets... */
    if ($acl === NULL) {
      $this->aclContents  = [];
    } else {
      $this->aclContents  = $acl;
    }
  }

  public function readPost ()
  {
    global $config;

    if (isset($_POST[$this->post_cancel])) {
      $this->handleCancel();
      return;
    } elseif (isset($_POST[$this->post_finish]) || isset($_GET[$this->post_finish])) {
      $this->handleFinish();
      return;
    }

    $new_acl = [];

    foreach ($_POST as $name => $post) {
      /* Actions... */
      if (preg_match('/^cat_edit_.*_x/', $name)) {
        $this->aclObject    = preg_replace('/^cat_edit_([^_]+)_.*$/', '\1', $name);
        $this->dialogState  = 'edit';
        foreach ($config->data['CATEGORIES'][$this->aclObject]['classes'] as $oc) {
          if (isset($this->aclContents[$oc])) {
            $this->savedAclContents[$oc] = $this->aclContents[$oc];
          }
          if (isset($this->aclContents[$this->aclObject.'/'.$oc])) {
            $this->savedAclContents[$this->aclObject.'/'.$oc] = $this->aclContents[$this->aclObject.'/'.$oc];
          }
        }
        break;
      }

      if (preg_match('/^cat_del_.*_x/', $name)) {
        $idx = preg_replace('/^cat_del_([^_]+)_.*$/', '\1', $name);
        foreach ($config->data['CATEGORIES'][$idx]['classes'] as $key) {
          unset($this->aclContents["$idx/$key"]);
        }
        break;
      }

      /* ACL checkbox saving... */
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
          $new_acl[$object] = [];
        }
        if (isset($new_acl[$object][$attribute])) {
          $new_acl[$object][$attribute] .= $value;
        } else {
          $new_acl[$object][$attribute] = $value;
        }
      }
    }

    /* Only be interested in new acl's, if we're in the right _POST place */
    if (isset($_POST['acl_dummy_0_0_0']) && ($this->aclObject != '') && is_array($config->data['CATEGORIES'][$this->aclObject])) {
      foreach ($config->data['CATEGORIES'][$this->aclObject]['classes'] as $oc) {
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

    if ($this->dialogState == 'edit') {
      /* Cancel edit acl? */
      if (isset($_POST['cancel_edit_acl'])) {
        $this->dialogState = 'create';
        foreach ($config->data['CATEGORIES'][$this->aclObject]['classes'] as $oc) {
          if (isset($this->savedAclContents[$oc])) {
            $this->aclContents[$oc] = $this->savedAclContents[$oc];
          }
          if (isset($this->savedAclContents[$this->aclObject.'/'.$oc])) {
            $this->aclContents[$this->aclObject.'/'.$oc] = $this->savedAclContents[$this->aclObject.'/'.$oc];
          }
        }
      }

      /* Save edit acl? */
      if (isset($_POST['submit_edit_acl'])) {
        $this->dialogState = 'create';
      }
    }
  }

  public function update (): bool
  {
    return ($this->dialogState !== 'closed');
  }

  protected function handleFinish ()
  {
    $this->attribute->addValue('', $this->aclContents);
    $this->dialogState = 'closed';
  }

  protected function handleCancel ()
  {
    if ($this->initialAclValue !== NULL) {
      $this->attribute->addValue('', $this->initialAclValue);
    }
    $this->dialogState = 'closed';
  }

  public function render (): string
  {
    global $config;

    /* Create templating instance */
    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');

    if ($this->dialogState == 'create') {
      /* Draw list */
      $aclList = new divSelectBox('aclList');
      $aclList->setHeight('auto');

      /* Add settings for all categories to the (permanent) list */
      foreach ($config->data['CATEGORIES'] as $section => $infos) {
        if (($section != 'all') && (count($infos['classes']) == 1)) {
          /* Hide empty categories */
          continue;
        }
        $summary = [];
        foreach ($infos['classes'] as $oc) {
          if (isset($this->aclContents[$oc]) && count($this->aclContents[$oc]) && isset($this->aclContents[$oc][0]) &&
              $this->aclContents[$oc][0] != '') {
            $summary[] = $oc;
            continue;
          }
          if (isset($this->aclContents["$section/$oc"]) && count($this->aclContents["$section/$oc"])) {
            $summary[] = $oc;
            continue;
          }
          if (isset($this->aclContents[$oc]) && !isset($this->aclContents[$oc][0]) && count($this->aclContents[$oc])) {
            $summary[] = $oc;
          }
        }

        /* Set summary... */
        if (empty($summary)) {
          $summary = '<i>'._('No ACL settings for this category').'</i>';
        } else {
          $summary = sprintf(_('ACL for these objects: %s'), join(', ', $summary));
        }

        $action = '<input class="center" type="image" src="geticon.php?context=actions&amp;icon=document-edit&amp;size=16"
                    alt="'._('Edit').'" name="cat_edit_'.$section.'"   title="'._('Edit category ACL').'"/>'.
                  '<input class="center" type="image" src="geticon.php?context=actions&amp;icon=edit-delete&amp;size=16"
                    alt="'._('Delete').'" name="cat_del_'.$section.'" title="'._('Reset category ACL').'"/>';

        $field1 = ['html' => $infos['description'], 'attach' => 'style="width:140px"'];
        $field2 = ['html' => $summary];
        $field3 = ['html' => $action, 'attach' => 'style="border-right:0px;width:40px"'];
        $aclList->addEntry([$field1, $field2, $field3]);
      }

      $smarty->assign('headline', _('List of available ACL categories'));
      $smarty->assign('aclEdition', $aclList->drawList());
    } elseif ($this->dialogState == 'edit') {
      /* Collect objects for selected category */
      $aclObjects = [];
      foreach ($config->data['CATEGORIES'][$this->aclObject]['classes'] as $idx => $class) {
        if ($idx == 0) {
          continue;
        }
        $pInfos = pluglist::pluginInfos($class);
        $key = $this->aclObject.'/'.$class;
        $aclObjects[$key] = $pInfos['plShortName'];
        /* Create sub acl if it does not exist */
        if (!isset($this->aclContents[$key])) {
          $this->aclContents[$key] = [];
        }
        if (!isset($this->aclContents[$key][0])) {
          $this->aclContents[$key][0] = '';
        }
      }
      if ($this->aclObject == 'all') {
        $aclObjects['all'] = _('All objects in current subtree');
        /* Create sub acl if it does not exist */
        $key = 'all';
        if (!isset($this->aclContents[$key])) {
          $this->aclContents[$key] = [];
        }
        if (!isset($this->aclContents[$key][0])) {
          $this->aclContents[$key][0] = '';
        }
      }
      $smarty->assign('headline', sprintf(_('Edit ACL for "%s"'), $config->data['CATEGORIES'][$this->aclObject]['description']));
      $smarty->assign('aclObjects', $aclObjects);
      $smarty->assign('aclContents', $this->aclContents);
      $tmp      = session::get('plist');
      $smarty->assign('plist', $tmp->info);
    }

    /* Show main page */
    $smarty->assign('dialogState', $this->dialogState);

    return $smarty->fetch(get_template_path('acleditiondialog.tpl', dirname(__FILE__)));
  }
}
