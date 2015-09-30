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

class ObjectsAttribute extends GenericDialogAttribute
{
  //TODO : insert in black list used workstations
  protected $dialogClass  = 'ObjectSelectDialog';
  protected $types        = array();


  function getFilterBlackList ()
  {
    return array(
      'dn' => array_merge($this->getValue(), $this->plugin->used_workstations)
    );
  }

  protected function fillDisplayValueFrom ($i, $attrs)
  {
    if ($attrs) {
      foreach (ogroup::$objectTypes as $code => $objectType) {
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
          unset(ogroup::$objectTypes[$code]);
        }
      }
      if (!isset($this->displays[$i])) {
        trigger_error('Unkown type for "'.$attrs['dn'].'"');
        $this->displays[$i] = sprintf(_("Unknown type : %s"), LDAP::fix($attrs['dn']));
        $this->types[$i]    = 'I';
      }
    } else {
      $this->displays[$i] = sprintf(_("Non existing dn: %s"), LDAP::fix($this->value[$i]));
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
      try {
        $infos = array();
        if ($this->types[$key] !== 'I') {
          $infos = objects::infos(ogroup::$objectTypes[$this->types[$key]]);
        }
      } catch (NonExistingObjectTypeException $e) {
        die('No type for '.$key.' '.$value);
      }
      if (isset($infos['icon'])) {
        $img = $infos['icon'];
      } else {
        $img = 'images/empty.png';
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
    $this->getDisplayValues(); // refresh types and displays
    return '['.join(array_unique($this->types)).']';
  }

  protected function removeValue ($row)
  {
    parent::removeValue($row);
    unset($this->types[$row]);
  }
}

class ObjectSelectDialog extends GenericSelectDialog
{
  protected $dialogClass = 'objectSelect';
}

class ogroup extends simplePlugin
{
  var $mainTab        = TRUE;
  var $objectclasses  = array('groupOfNames', 'gosaGroupOfNames');

  var $used_workstations = array();

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

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Object group'),
      'plDescription' => _('Object group information'),
      'plSelfModify'  => FALSE,
      'plPriority'    => 1,
      'plObjectType'  => array('ogroup' => array(
        'name'        => _('Group'),
        'description' => _('Group'),
        'ou'          => get_ou('ogroupRDN'),
        'filter'      => 'objectClass=groupOfNames',
        'icon'        => 'geticon.php?context=types&icon=resource-group&size=16',
        'tabClass'    => 'ogrouptabs',
      )),
      'plForeignKeys'  => array(
        'member' => array_values(
          array_map(
            function ($a)
            {
              return array($a);
            },
            self::$objectTypes
          )
        )
      ),

      'plProvidedAcls'  => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'  => _('Properties'),
        'attrs' => array(
          new BaseSelectorAttribute(get_ou('ogroupRDN')),
          new UidAttribute(
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
            'member', TRUE,
            array(), 'dn'
          )
        )
      ),
      'system_trust' => array(
        'name'  => _('System trust'),
        'icon'  => 'geticon.php?context=status&icon=locked&size=16',
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
        - Check() will complain if such a system is assigned to this object group.*/
    $base = $this->config->current['BASE'];
    $res  = get_list("(|(objectClass=gotoWorkstation)(objectClass=gotoTerminal))", "none",
                      $base, array("dn"), GL_NO_ACL_CHECK | GL_SUBSEARCH);
    $ws_dns = array();
    foreach ($res as $data) {
      $ws_dns[] = $data['dn'];
    }
    $res = get_list("(&(member=*)(objectClass=gosaGroupOfNames)(|(gosaGroupObjects=[W])(gosaGroupObjects=[T])))", "none",
                    $base, array("dn","member"), GL_NO_ACL_CHECK | GL_SUBSEARCH);
    $this->used_workstations = array();
    foreach ($res as $og) {
      if ($og['dn'] == $this->dn) {
        continue;
      }
      $this->used_workstations = array_merge($this->used_workstations, array_intersect($ws_dns, $og['member']));
    }

    $this->reload();
  }

  function compute_dn()
  {
    return 'cn='.$this->attributesAccess['cn']->getValue().','.get_ou('ogroupRDN').$this->base;
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

  function check()
  {
    $message = parent::check();
    $this->reload();
    if (preg_match('/W/', $this->gosaGroupObjects) && preg_match('/T/', $this->gosaGroupObjects)) {
      $message[] = _('Putting both workstations and terminals in the same group is not allowed');
    }

    return $message;
  }

  function getGroupObjectTypes()
  {
    $this->reload();
    return $this->gosaGroupObjects;

  }
}
?>
