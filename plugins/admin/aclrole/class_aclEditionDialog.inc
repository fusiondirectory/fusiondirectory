<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2015  FusionDirectory

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
class ACLEditionDialog extends GenericDialog
{
  protected $initialAclValue;
  protected $dialogState      = 'create';
  protected $aclObject        = '';
  protected $aclContents      = array();
  protected $savedAclContents = array();

  function __construct($simplePlugin, &$attribute, $acl = NULL)
  {
    $this->attribute        = $attribute;
    $this->initialAclValue  = $acl;

    /* New entry gets presets... */
    if ($acl === NULL) {
      $this->aclContents  = array();
    } else {
      $this->aclContents  = $acl;
    }
  }

  function handle_finish ()
  {
    $this->attribute->addValue($this->aclContents);
    return FALSE;
  }

  function handle_cancel ()
  {
    if ($this->initialAclValue !== NULL) {
      $this->attribute->addValue($this->initialAclValue);
    }
    return FALSE;
  }

  function save_object()
  {
    global $config;

    $new_acl = array();

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

  function dialog_execute ()
  {
    global $config;

    $this->save_object();

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
        $summary = array();
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

        $field1 = array('html' => $infos['description'], 'attach' => 'style="width:140px"');
        $field2 = array('html' => $summary);
        $field3 = array('html' => $action, 'attach' => 'style="border-right:0px;width:40px"');
        $aclList->AddEntry(array($field1, $field2, $field3));
      }

      $smarty->assign('headline', _('List of available ACL categories'));
      $smarty->assign('aclEdition', $aclList->DrawList());
    } elseif ($this->dialogState == 'edit') {
      /* Collect objects for selected category */
      $aclObjects = array();
      foreach ($config->data['CATEGORIES'][$this->aclObject]['classes'] as $idx => $class) {
        if ($idx == 0) {
          continue;
        }
        $pInfos = pluglist::pluginInfos($class);
        $aclObjects[$this->aclObject.'/'.$class] = $pInfos['plShortName'];
      }
      if ($this->aclObject == 'all') {
        $aclObjects['all'] = _('All objects in current subtree');
      }
      $smarty->assign('headline', sprintf(_('Edit ACL for "%s"'), $config->data['CATEGORIES'][$this->aclObject]['description']));
      $smarty->assign('aclEdition', $this->buildAclSelector($aclObjects));
    }

    /* Show main page */
    $smarty->assign('dialogState', $this->dialogState);

    return $smarty->fetch(get_template_path('acleditiondialog.tpl', dirname(__FILE__)));
  }

  /*!
   * \brief Create a checkbox
   *
   * \param String $name Name of the acl checkbox
   * \param String $text Label for the checkbox
   * \param boolean $state
   *
   * \return String containing checkbox
   */
  function mkchkbx($name, $text, $state = FALSE)
  {
    $tname = preg_replace('/[^a-z0-9]/i', '_', $name);
    return  '<input id="acl_'.$tname.'" type="checkbox" name="acl_'.$name.'"'.($state ? ' checked="checked"' : '').'/>'."\n".
            '<label for="acl_'.$tname.'">'.$text.'</label>'."\n";
  }

  /*!
   * \brief Make a read and write box
   *
   * \param String $name Name of the box
   * \param String $state
   *
   * \return String containing checkbox
   */
  function mkrwbx($name, $state = '')
  {
    $rstate = (preg_match('/r/', $state) ? ' checked="checked"' : '');
    $wstate = (preg_match('/w/', $state) ? ' checked="checked"' : '');
    $tname  = preg_replace('/[^a-z0-9]/i', '_', $name);

    return  '<input id="acl_'.$tname.'_r" type="checkbox" name="acl_'.$name.'_r"'.$rstate.'/>'."\n".
            '<label for="acl_'.$tname.'_r">'._('read').'</label>'."\n".
            '<input id="acl_'.$tname.'_w" type="checkbox" name="acl_'.$name.'_w"'.$wstate.'/>'."\n".
            '<label for="acl_'.$tname.'_w">'._('write').'</label>'."\n";
  }

  /*!
   * \brief Build an acl selector form
   *
   * \param Array $list
   *
   * \return the acl selector form
   */
  function buildAclSelector($list)
  {
    global $config;

    $display  = '<input type="hidden" name="acl_dummy_0_0_0" value="1"/>';
    $cols     = 3;
    $tmp      = session::global_get('plist');
    $plist    = $tmp->info;
    asort($plist);

    /* Add select all/none buttons */
    $style = 'style="width:100px;"';
    $display .= '<input '.$style.' type="button" name="toggle_all_create" onClick="acl_toggle_all(\'_0_c$\');" value="Toggle C"/>';
    $display .= '<input '.$style.' type="button" name="toggle_all_move"   onClick="acl_toggle_all(\'_0_m$\');" value="Toggle M"/>';
    $display .= '<input '.$style.' type="button" name="toggle_all_remove" onClick="acl_toggle_all(\'_0_d$\');" value="Toggle D"/> - ';
    $display .= '<input '.$style.' type="button" name="toggle_all_read"   onClick="acl_toggle_all(\'_0_r$\');" value="Toggle R"/>';
    $display .= '<input '.$style.' type="button" name="toggle_all_write"  onClick="acl_toggle_all(\'_0_w$\');" value="Toggle W"/> - ';

    $display .= '<input '.$style.' type="button" name="toggle_all_sub_read"  onClick="acl_toggle_all(\'[^0]_r$\');" value="R+"/>';
    $display .= '<input '.$style.' type="button" name="toggle_all_sub_write"  onClick="acl_toggle_all(\'[^0]_w$\');" value="W+"/>';

    $display .= '<br/>';

    $style    = 'style="width:50px;"';
    $display  .= '<input '.$style.' type="button" name="set_true_all_create"  onClick="acl_set_all(\'_0_c$\',true);"  value="C+"/>';
    $display  .= '<input '.$style.' type="button" name="set_false_all_create" onClick="acl_set_all(\'_0_c$\',false);" value="C-"/>';
    $display  .= '<input '.$style.' type="button" name="set_true_all_move"    onClick="acl_set_all(\'_0_m$\',true);"  value="M+"/>';
    $display  .= '<input '.$style.' type="button" name="set_false_all_move"   onClick="acl_set_all(\'_0_m$\',false);" value="M-"/>';
    $display  .= '<input '.$style.' type="button" name="set_true_all_remove"  onClick="acl_set_all(\'_0_d$\',true);"  value="D+"/>';
    $display  .= '<input '.$style.' type="button" name="set_false_all_remove" onClick="acl_set_all(\'_0_d$\',false);" value="D-"/> - ';
    $display  .= '<input '.$style.' type="button" name="set_true_all_read"    onClick="acl_set_all(\'_0_r$\',true);"  value="R+"/>';
    $display  .= '<input '.$style.' type="button" name="set_false_all_read"   onClick="acl_set_all(\'_0_r$\',false);" value="R-"/>';
    $display  .= '<input '.$style.' type="button" name="set_true_all_write"   onClick="acl_set_all(\'_0_w$\',true);"  value="W+"/>';
    $display  .= '<input '.$style.' type="button" name="set_false_all_write"  onClick="acl_set_all(\'_0_w$\',false);" value="W-"/> - ';

    $display .= '<input '.$style.' type="button" name="set_true_all_read"   onClick="acl_set_all(\'[^0]_r$\',true);"  value="R+"/>';
    $display .= '<input '.$style.' type="button" name="set_false_all_read"  onClick="acl_set_all(\'[^0]_r$\',false);" value="R-"/>';
    $display .= '<input '.$style.' type="button" name="set_true_all_write"  onClick="acl_set_all(\'[^0]_w$\',true);"  value="W+"/>';
    $display .= '<input '.$style.' type="button" name="set_false_all_write" onClick="acl_set_all(\'[^0]_w$\',false);" value="W-"/>';

    $list = acl::sort_by_priority($list);
    foreach ($list as $key => $name) {
      /* Create sub acl if it does not exist */
      if (!isset($this->aclContents[$key])) {
        $this->aclContents[$key] = array();
      }
      if (!isset($this->aclContents[$key][0])) {
        $this->aclContents[$key][0] = '';
      }

      $currentAcl = $this->aclContents[$key];

      /* Get the overall plugin acls */
      $overall_acl = '';
      if (isset($currentAcl[0])) {
        $overall_acl = $currentAcl[0];
      }

      // Detect configured plugins
      $expand = ((count($currentAcl) > 1) || ($currentAcl[0] != ''));

      /* Object header */
      $tname = preg_replace('/[^a-z0-9]/i', '_', $key);

      if ($expand) {
        $back_color = '#C8C8FF';
      } else {
        $back_color = '#C8C8C8';
      }

      $display .= '<table style="width:100%;border:1px solid #A0A0A0;border-spacing:0;border-collapse:collapse;">'."\n".
                  '  <tr>'."\n".
                  '    <td style="background-color:'.$back_color.';height:1.8em;" colspan='.($cols - 1).'>'.
                        '<b>'._('Object').': '.$name.'</b>'.
                      '</td>'."\n".
                  '    <td align="right" style="background-color:'.$back_color.';height:1.8em;">'."\n".
                  '    <input id="show'.$tname.'" type="button" onclick="$(\''.$tname.'\').toggle();" value="'._('Show/hide advanced settings').'"/></td>'."\n".
                  '  </tr>'."\n";

      /* Generate options */
      $spc      = '&nbsp;&nbsp;';
      $options  =   $this->mkchkbx($key.'_0_c', _('Create objects'),  preg_match('/c/', $overall_acl)).$spc;
      $options  .=  $this->mkchkbx($key.'_0_m', _('Move objects'),    preg_match('/m/', $overall_acl)).$spc;
      $options  .=  $this->mkchkbx($key.'_0_d', _('Remove objects'),  preg_match('/d/', $overall_acl)).$spc;
      if ($plist[preg_replace('%^.*/%', '', $key)]['plSelfModify']) {
        $options .= $this->mkchkbx($key.'_0_s', _('Grant permission to owner'), preg_match('/s/', $overall_acl)).$spc;
      }

      /* Global options */
      $more_options =   $this->mkchkbx($key.'_0_r',  _('read'), preg_match('/r/', $overall_acl)).$spc;
      $more_options .=  $this->mkchkbx($key.'_0_w', _('write'), preg_match('/w/', $overall_acl));

      $display .= '  <tr>'."\n".
                  '    <td style="background-color:#E0E0E0" colspan="'.($cols - 1).'">'.$options.'</td>'."\n".
                  '    <td style="background-color:#D4D4D4">&nbsp;'._('Complete object').': '.$more_options.'</td>'."\n".
                  '  </tr>'."\n";

      /* Walk through the list of attributes */
      $cnt    = 1;
      $splist = $plist[preg_replace('%^.*/%', '', $key)]['plProvidedAcls'];
      $display .= '  <tr id="tr_'.$tname.'" style="vertical-align:top;height:0px;">'."\n".
                  '    <td colspan="'.$cols.'">'."\n".
                  '      <div id="'.$tname.'" style="overflow:hidden; display:none;vertical-align:top;width:100%;">'."\n".
                  '        <table style="width:100%;border-collapse: collapse;" border="1">'."\n";

      foreach ($splist as $attr => $dsc) {
        /* Skip pl* attributes, they are internal... */
        if (preg_match('/^pl[A-Z]+.*$/', $attr)) {
          continue;
        }

        /* Open table row */
        if ($cnt == 1) {
          $display .= '  <tr>'."\n";
        }

        /* Close table row */
        if ($cnt == $cols) {
          $cnt  = 1;
          $end  = '  </tr>'."\n";
        } else {
          $cnt++;
          $end  = '';
        }

        /* Collect list of attributes */
        $state = '';
        if (isset($currentAcl[$attr])) {
          $state = $currentAcl[$attr];
        }
        $display .= '    <td style="border:1px solid #A0A0A0;width:'.(int)(100 / $cols).'%">'."\n".
                    '      <b>'.$dsc.'</b> ('.$attr.')<br/>'.$this->mkrwbx($key.'_'.$attr, $state).'</td>'.$end."\n";
      }

      /* Fill missing td's if needed */
      if ((--$cnt != $cols) && ($cnt != 0)) {
        $display .= str_repeat('    <td style="border:1px solid #A0A0A0;width:'.(int)(100 / $cols).'%">&nbsp;</td>'."\n", $cols - $cnt);
      }

      $display .= '        </table>'."\n".
                  '      </div>'."\n".
                  '    </td>'."\n".
                  '  </tr>'."\n";

      $display .= '</table><br/>'."\n";
    }

    return $display;
  }
}
