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

class ObjectsAttribute extends GenericDialogAttribute
{
  //TODO : insert in black list used workstations
  static $objectTypes = array(
    "U" => "user",
    "G" => "group",
    "A" => "application",
    "S" => "server",
    "W" => "workstation",
    "O" => "winstation",
    "T" => "terminal",
    "F" => "phone",
    "P" => "printer"
  );

  protected $dialogClass  = 'ObjectSelectDialog';
  protected $types        = array();

  protected function fillDisplayValueFrom ($i, $attrs)
  {
    if ($attrs) {
      foreach (self::$objectTypes as $code => $objectType) {
        try {
          if (objects::isOfType($attrs, $objectType)) {
            $infos = objects::infos($objectType);
            $this->types[$i]    = $code;
            $this->displays[$i] = $attrs[$infos['mainAttr']][0];
            if (isset($attrs['description'][0])) {
              $this->displays[$i] .= ' ['.$attrs['description'][0].']';
            } elseif (isset($attrs['uid'][0])) {
              $this->displays[$i] .= ' ['.$attrs['uid'][0].']';
            }
            break;
          }
        } catch (NonExistingObjectTypeException $e) {
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $e->getMessage(), "Objecttype");
          unset(self::$objectTypes[$code]);
        }
      }
      if (!isset($this->displays[$i])) {
        trigger_error('Unkown type for "'.$attrs['dn'].'"');
        $this->displays[$i] = sprintf(_("Unknown type : %s"), LDAP::fix($attrs['dn']));
        $this->types[$i]    = 'I';
      }
    } else {
      $this->displays[$i] = sprintf(_("Non existing dn: %s"), LDAP::fix($attrs['dn']));
      $this->types[$i]    = 'I';
    }
  }

  function renderOnlyFormInput()
  {
    $id       = $this->getHtmlId();
    $display  = '<select multiple="multiple"  name="row'.$id.'[]" id="row'.$id.'"'.
                ($this->disabled? ' disabled="disabled"':'').
                ' >'."\n";
    foreach ($this->getDisplayValues() as $key => $value) {
      $infos = array();
      if ($this->types[$key] !== 'I') {
        $infos = objects::infos(self::$objectTypes[$this->types[$key]]);
      }
      if (isset($infos['icon'])) {
        $img = $infos['icon'];
      } else {
        $img = 'images/false.png';
      }
      $display .= '<option value="'.$key.'" class="select"'.
                  ' style="background-image:url(\''.get_template_path($img).'\');"'.
                  '>'.$value."</option>\n";
    }
    $display .= '</select><br/>'."\n";
    return $display;
  }

  function ldapAttributesToGet()
  {
    return array(
      'objectClass',
      'cn',
      'sn',
      'ou',
      'uid',
      'givenName',
      'description',
    );
  }

  function listObjectTypes()
  {
    return '['.join(array_unique($this->types)).']';
  }
}

class ObjectSelectDialog extends GenericSelectDialog
{
  protected $dialogClass = 'objectSelect';
  protected $post_cancel = 'objectSelect_cancel';
  protected $post_finish = 'objectSelect_save';
}

class ogroup extends simplePlugin
{
  var $mainTab        = TRUE;
  var $objectclasses  = array('top', 'gosaGroupOfNames');

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Generic'),
      'plDescription' => _('Object group generic'),
      'plSelfModify'  => FALSE,
      'plPriority'    => 1,
      'plObjectType'  => array('ogroup' => array(
        'description' => _('Object groups'),
        'name'        => _('Object group'),
        'filter'      => 'objectClass=gosaGroupOfNames'
      )),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'icon'  => 'images/rightarrow.png',
        'attrs' => array(
          new BaseSelectorAttribute(get_ou('ogroupRDN')),
          new HostNameAttribute(
            _('Name'), _('Name of this group'),
            'cn', TRUE
          ),
          new TextAreaAttribute(
            _('Description'), _('Short description of this group'),
            'description', FALSE
          ),
          new HiddenAttribute('gosaGroupObjects')
        )
      ),
      'members' => array(
        'name'  => _('Member objects'),
        'attrs' => array(
          new ObjectsAttribute(
            '', _('Objects member of this group'),
            'member', FALSE,
            array(), 'dn'
          )
        )
      ),
      'system_trust' => array(
        'name'  => _('System trust'),
        'icon'  => 'images/lists/locked.png',
        'attrs' => array(
          new SelectAttribute(
            _('Trust mode'), _('Type of authorization for those hosts'),
            'trustMode', FALSE,
            array('', 'fullaccess', 'byhost'),
            '',
            array(_('disabled'), _('full access'), _('allow access to these hosts'))
          ),
          new SystemsAttribute(
            '', _('Only allow this group to connect to this list of hosts'),
            'host', FALSE
          )
        )
      )
    );
  }

  function __construct (&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->attributesAccess['trustMode']->setInLdap(FALSE);
    $this->attributesAccess['trustMode']->setManagedAttributes(
      array(
        'multiplevalues' => array('notbyhost' => array('','fullaccess')),
        'erase' => array(
          'notbyhost' => array('host')
        )
      )
    );
    if ((count($this->host) == 1) && ($this->host[0] == '*')) {
      $this->trustMode = 'fullaccess';
    } elseif (count($this->host) > 0) {
      $this->trustMode = 'byhost';
    }

    /* Detect all workstations, which are already assigned to an object group
        - Those objects will be hidden in the add object dialog.
        - Check() will complain if such a system is assigned to this object group.
    $base = $this->config->current['BASE'];
    $res  = get_list("(|(objectClass=gotoWorkstation)(objectClass=gotoTerminal))", "none",
                      $base, array("dn"), GL_NO_ACL_CHECK | GL_SUBSEARCH);
    $ws_dns = array();
    foreach ($res as $data) {
      $ws_dns[] = $data['dn'];
    }
    $res = get_list("(&(member=*)(objectClass=gosaGroupOfNames))", "none",
                    $base, array("dn","member","gosaGroupObjects"), GL_NO_ACL_CHECK | GL_SUBSEARCH);
    $this->used_workstations = array();
    foreach ($res as $og) {
      if ($og['dn'] == $this->dn) {
        continue;
      }
      $test = array_intersect($ws_dns, $og['member']);
      if (($og['gosaGroupObjects'][0] == "[W]" || $og['gosaGroupObjects'][0] == "[T]") && count($test)) {
        $this->used_workstations = array_merge($this->used_workstations, $test);
      }
    }
     */

    $this->reload();
  }

  function compute_dn()
  {
    return 'cn='.$this->attributesAccess['cn']->getValue().','.get_ou('ogroupRDN').$this->base;
  }

  function save_object()
  {
    parent::save_object();
    $this->reload();
  }

  function prepare_save()
  {
    $this->reload();
    parent::prepare_save();

    if ($this->trustMode == 'fullaccess') {
      $this->attrs['host'] = array('*');
    }

    /* Trust accounts */
    if (($this->trustMode != "") && !in_array('hostObject', $this->attrs['objectClass'])) {
      $this->attrs['objectClass'][] = 'hostObject';
    } elseif (($this->trustMode == "") && (($key = array_search('hostObject', $this->attrs['objectClass'])) !== FALSE)) {
      unset($this->attrs['objectClass'][$key]);
    }
  }

  function reload()
  {
    $this->gosaGroupObjects = $this->attributesAccess['member']->listObjectTypes();
  }
}

class ogroup_old extends plugin
{
  function AddDelMembership()
  {
          $skipped = FALSE;

          $test = array_unique(array_intersect($this->used_workstations, $objects_dns));
          if(count($test)) {
            $str = "";
            foreach ($test as $dn) {
              $str .= "<li>".$dn."</li>";
            }
            msg_dialog::display(_("Error"), sprintf(_("These systems are already configured by other object groups and cannot be added:")."<br><ul>%s</ul>", $str), ERROR_DIALOG);
          }

          foreach($objects as $object){

              $tmp = "";
              foreach($this->memberList as $obj){
                  $tmp .= $obj['type'];
              }

              $type  = $this->getObjectType($object);
              $name= $this->getObjectName($object);
              $dn = $object['dn'];

              /* Fill array */
              if (isset($object["description"][0])){
                  $object= array("text" => "$name [".$object["description"][0]."]", "type" => "$type");
              } elseif (isset($object["uid"][0])) {
                  $object= array("text" => "$name [".$object["uid"][0]."]", "type" => "$type");
              } else {
                  $object= array("text" => "$name", "type" => "$type");
              }

              if(preg_match("/T/",$tmp) && $type == "W"){
                  $skipped =TRUE;
              }elseif(preg_match("/W/",$tmp) && $type == "T"){
                  $skipped =TRUE;
              }elseif(in_array($dn, $this->used_workstations)) {
                  continue;
              }else{

                  $this->memberList["$dn"]= $object;
                  $this->member["$dn"]= $dn;
                  uasort ($this->memberList, 'sort_list');
                  reset ($this->memberList);
              }
          }
          if($skipped){
              msg_dialog::display(_("Information"), _("You cannot combine terminals and workstations in one object group!"), INFO_DIALOG);
          }
  }

  function convert_list($input)
  {
    $temp= "";
    $conv= array(
        "Y" => "plugins/users/images/select_template.png",
        "U" => "plugins/users/images/iconMini.png",
        "G" => "plugins/groups/images/iconMini.png",
        "A" => "plugins/ogroups/images/application.png",
        "D" => "plugins/departments/images/department.png",
        "S" => "plugins/ogroups/images/server.png",
        "W" => "plugins/ogroups/images/workstation.png",
        "O" => "plugins/ogroups/images/winstation.png",
        "T" => "plugins/ogroups/images/terminal.png",
        "F" => "plugins/ogroups/images/phone.png",
        "P" => "plugins/ogroups/images/printer.png",
        "I" => "images/false.png");

    foreach ($input as $key => $value){
      /* Generate output */
      $temp.= "<option title='".addslashes( $key)."' value=\"$key\" class=\"select\" style=\"background-image:url('".get_template_path($conv[$value['type']])."');\">".$value['text']."</option>\n";
    }

    return ($temp);
  }

  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    /* To many different object types? */
    if (strlen($this->gosaGroupObjects) > 4){
      $message[]= _("You can combine two different object types at maximum, only!");
    }

    return ($message);
  }
}
?>
