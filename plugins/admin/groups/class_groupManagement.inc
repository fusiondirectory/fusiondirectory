<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003  Cajus Pollmeier
  Copyright (C) 2011-2018  FusionDirectory

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

class groupManagement extends management
{
  public static $skipTemplates = FALSE;

  public static $columns = array(
    array('ObjectTypeColumn',   array()),
    array('LinkColumn',         array('attribute' => 'cn',          'label' => 'Name')),
    array('LinkColumn',         array('attribute' => 'description', 'label' => 'Description')),
    array('GroupContentColumn', array('label' => 'Members')),
    array('PropertiesColumn',   array('label' => 'Properties')),
    array('ActionsColumn',      array('label' => 'Actions')),
  );

  protected $siActive = FALSE;

  static function plInfo()
  {
    return array(
      'plShortName'   => _('Groups and roles'),
      'plTitle'       => _('Manage groups and roles'),
      'plDescription' => _('Allows you to manage object groups, POSIX groups and roles'),
      'plIcon'        => 'geticon.php?context=types&icon=resource-group&size=48',
      'plSection'     => 'accounts',
      'plManages'     => array('ogroup', 'group', 'role'),
      'plPriority'    => 20,

      'plProvidedAcls' => array()
    );
  }

  function __construct()
  {
    // Check if we are able to communicate with Argonaut server
    if (class_available('supportDaemon') && class_available('argonautAction')) {
      $o = new supportDaemon();
      $this->siActive = $o->is_available();
    }

    parent::__construct();

    // TODO enable sort by properties and member types
    $this->filter->addElement(new GroupContentFilterElement($this->filter));
  }

  protected function configureActions()
  {
    parent::configureActions();

    if ($this->siActive) {
      $triggerActions   = array();
      $scheduleActions  = array();
      $events = argonautEventTypes::get_event_types();
      foreach ($events as $name => $infos) {
        $triggerActions[] = new Action(
          'trigger_'.$name, $infos['name'], $infos['img'],
          '*', 'handleEvent'
        );
        $scheduleActions[] = new Action(
          'schedule_'.$name, $infos['name'], $infos['img'],
          '*', 'handleEvent'
        );
      }
      $this->registerAction(
        new SubMenuAction(
          'trigger', _('Trigger action'), 'geticon.php?context=types&icon=action&size=16',
          $triggerActions
        )
      );
      $this->registerAction(
        new SubMenuAction(
          'schedule', _('Schedule action'), 'geticon.php?context=actions&icon=task-schedule&size=16',
          $scheduleActions
        )
      );
      $this->actions['trigger']->setSeparator(TRUE);
      $this->registerAction(new HiddenAction('saveEvent', 'saveEventDialog'));
    }
  }

  /*! \brief    Handle Argonaut events
   *            All schedules and triggered events are handled here.
   */
  function handleEvent($action)
  {
    global $config;

    if (!$this->siActive) {
      return;
    }

    // Detect whether this event is scheduled or triggered.
    $triggered  = ($action['action'] == 'trigger');
    $event      = $action['subaction'];

    // Now send FAI/Argonaut events here.
    $mac = array();

    // Collect target mac addresses
    $ldap = $config->get_ldap_link();
    foreach ($action['targets'] as $dn) {
      $obj = $this->listing->getEntry($dn);
      if (isset($obj['member'])) {
        foreach ($obj['member'] as $member) {
          $ldap->cat($member);
          if ($attrs = $ldap->fetch()) {
            if (isset($attrs['macAddress'][0])) {
              $mac[] = $attrs['macAddress'][0];
            } else {
              msg_dialog::display(_('Action canceled'), sprintf(_('System %s has no mac address defined, cannot trigger action'), $member), ERROR_DIALOG);
            }
          } else {
            msg_dialog::display(_('Action canceled'), sprintf(_('Could not find system %s, cannot trigger action'), $member), ERROR_DIALOG);
          }
        }
      }
    }

    if ((count($mac) == 0) && $triggered) {
      return;
    }

    $o_queue  = new supportDaemon();

    /* Skip installation or update trigerred events,
     *  if this entry is currently processing.
     */
    if ($triggered && in_array($event, array('reinstall','update'))) {
      foreach ($mac as $key => $mac_address) {
        if ($o_queue->is_currently_installing($mac_address)) {
          msg_dialog::display(_('Action canceled'), sprintf(_('System %s is currently installing'), $dn), ERROR_DIALOG);
          unset($mac[$key]);
          logging::log('security', 'systems/'.get_class($this), '', array(), 'Skip adding "argonautAction::'.$event.'" for mac "'.$mac_address.'", there is already a job in progress.');
        }
      }
    }

    if ((count($mac) == 0) && $triggered) {
      return;
    }

    // Prepare event to be added
    $events   = argonautEventTypes::get_event_types();
    if (isset($events[$event])) {
      $this->dialogObject = new argonautAction($event, $mac, !$triggered);

      if ($triggered) {
        $res = $o_queue->append($this->dialogObject);
        if ($o_queue->is_error()) {
          msg_dialog::display(_('Infrastructure service'), msgPool::siError($o_queue->get_error()), ERROR_DIALOG);
        } else {
          if (is_array($res) && count($res) > 1) {
            msg_dialog::display(_('Action triggered'), sprintf(_('Action called without error (results were "%s")'), implode(', ', $res)), INFO_DIALOG);
          } else {
            if (is_array($res)) {
              $res = $res[0];
            }
            msg_dialog::display(_('Action triggered'), sprintf(_('Action called without error (result was "%s")'), $res), INFO_DIALOG);
          }
        }
        $this->closeDialogs();
      }
    }
  }

  /*! \brief  Save event dialogs.
   *          And append the new Argonaut event.
   */
  function saveEventDialog()
  {
    $this->dialogObject->save_object();
    $msgs = $this->dialogObject->check();
    if (count($msgs)) {
      msg_dialog::displayChecks($msgs);
      return;
    }
    if ($this->siActive) {
      $o_queue = new supportDaemon();
      $o_queue->append($this->dialogObject);
      if ($o_queue->is_error()) {
        msg_dialog::display(_('Infrastructure service'), msgPool::siError($o_queue->get_error()), ERROR_DIALOG);
      }
      $this->closeDialogs();
    }
  }

  /*! \brief  Detects actions/events send by the ui
   *           and the corresponding targets.
   */
  function detectPostActions()
  {
    $action = parent::detectPostActions();
    if (isset($_POST['save_event_dialog'])) {
      $action['action'] = 'saveEvent';
    } elseif (isset($_POST['abort_event_dialog'])) {
      $action['action'] = 'cancel';
    }
    return $action;
  }
}
?>
