<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2016  FusionDirectory

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

class group extends simplePlugin
{
  var $mainTab          = TRUE;
  var $objectclasses    = array('posixGroup');

  static function plInfo()
  {
    if (class_available('mixedGroup')) {
      return array();
    }
    return array(
      'plShortName'   => _('Group'),
      'plDescription' => _('POSIX group information'),
      'plSelfModify'  => FALSE,
      'plObjectType'  => array('group' => array(
        'name'        => _('POSIX group'),
        'description' => _('POSIX user group'),
        'ou'          => get_ou('groupRDN'),
        'filter'      => 'objectClass=posixGroup',
        'icon'        => 'geticon.php?context=types&icon=user-group&size=16',
        'mainAttr'    => 'cn',
      )),
      'plForeignKeys'  => array(
        'memberUid' => array('user','uid')
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
          new BaseSelectorAttribute(get_ou('groupRDN')),
          new StringAttribute(
            _('Name'), _('Name of this group'),
            'cn', TRUE,
            '', '', (strict_uid_mode() ? '/^[a-z0-9_-]+$/' : '/^[a-z0-9_.-]+$/')
          ),
          new TextAreaAttribute(
            _('Description'), _('Short description of this group'),
            'description', FALSE
          ),
          new BooleanAttribute(
            _('Force GID'), _('Force GID value for this group'),
            'force_id', FALSE
          ),
          new IntAttribute(
            _('GID'), _('GID value for this group'),
            'gidNumber', FALSE,
            0, FALSE, ''
          )
        )
      ),
      'members' => array(
        'name'  => _('Group members'),
        'attrs' => array(
          new UsersAttribute(
            '', _('Group members'),
            'memberUid', FALSE,
            array(), 'uid'
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

    $this->attributesAccess['gidNumber']->setUnique(TRUE);
    $this->attributesAccess['force_id']->setInLdap(FALSE);
    $this->attributesAccess['force_id']->setManagedAttributes(
      array(
        'disable' => array (
          FALSE => array (
            'gidNumber',
          )
        )
      )
    );
  }

  function check()
  {
    $message = parent::check();

    /* Check ID's if they are forced by user */
    if ($this->force_id) {
      if ($this->gidNumber < $this->config->get_cfg_value("minId")) {
        $message[] = msgPool::toosmall(_("GID"), $this->config->get_cfg_value("minId"));
      }
    }

    return $message;
  }

  function resetCopyInfos()
  {
    parent::resetCopyInfos();

    $this->force_id = FALSE;
    $this->attributesAccess['gidNumber']->setInitialValue("");
    $this->gidNumber = "";
  }

  function prepare_save()
  {
    if (!$this->force_id) {
      /* Handle gidNumber */
      if ($this->attributesAccess['gidNumber']->getInitialValue() != "") {
        $this->gidNumber = $this->attributesAccess['gidNumber']->getInitialValue();
      } else {
        /* Calculate new id's. We need to place a lock before calling get_next_id
           to get real unique values.
         */
        $wait = 10;
        while (get_lock("gidnumber") != "") {
          sleep (1);

          /* Oups - timed out */
          if ($wait-- == 0) {
            msg_dialog::display(_("Warning"), _("Timeout while waiting for lock. Ignoring lock!"), WARNING_DIALOG);
            break;
          }
        }
        add_lock ("gidnumber", "gosa");
        $this->gidNumber = get_next_id("gidNumber", $this->dn);
      }
    }

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

  protected function post_save()
  {
    del_lock("gidnumber");
    parent::post_save();
  }

  function compute_dn()
  {
    return 'cn='.$this->attributesAccess['cn']->getValue().','.get_groups_ou().$this->base;
  }

  function addUser($dn, $uid)
  {
    $this->attributesAccess['memberUid']->addValue($dn, array('uid' => array($uid), 'cn' => array($uid)));
  }

  function removeUser($uid)
  {
    $this->attributesAccess['memberUid']->searchAndRemove($uid);
  }
}

?>
