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

class groupManagement extends simpleManagement
{
  protected $objectTypes  = array('ogroup', 'role', 'group');

  protected $autoFilter = FALSE;

  public static $skipTemplates = TRUE;

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
    $this->filterXMLPath  = get_template_path('group-filter.xml', TRUE, dirname(__FILE__));
    $this->listXMLPath    = get_template_path('group-list.xml', TRUE, dirname(__FILE__));

    // Check if we are able to communicate with Argonaut server
    if (class_available('supportDaemon') && class_available('argonautAction')) {
      $o = new supportDaemon();
      $this->siActive = $o->is_available();
    }

    parent::__construct();

    $this->headpage->registerElementFilter('filterProperties', 'groupManagement::filterProperties');
  }

  function configureFilter ()
  {
    parent::configureFilter();

    if (!class_available('posixGroup')) {
      /* If posix plugin is not installed, ignore posix filter */
      $this->filter->elements['PRIMARY']['unset']  = '';
      $this->filter->elements['PRIMARY']['set']    = '';
    }
    if (!class_available('mailAccount') || !class_available('posixGroup')) {
      $this->filter->elements['MAIL']['unset']  = '';
      $this->filter->elements['MAIL']['set']    = '';
    }
    if (!class_available('sambaAccount') || !class_available('posixGroup')) {
      $this->filter->elements['SAMBA']['unset']  = '';
      $this->filter->elements['SAMBA']['set']    = '';
    }
    if (static::$skipTemplates) {
      $this->filter->elements['FILTERTEMPLATE']['unset']  = '';
      $this->filter->elements['FILTERTEMPLATE']['set']    = '';
    }
  }

  function configureHeadpage ()
  {
    parent::configureHeadpage();

    // Register Daemon Events
    if ($this->siActive) {
      $this->headpage->xmlData['actionmenu']['action'][4]['action'] = array();
      $this->headpage->xmlData['actionmenu']['action'][5]['action'] = array();
      $events = argonautEventTypes::get_event_types();
      foreach ($events as $name => $data) {
        $this->registerAction('T_'.$name, 'handleEvent');
        $this->headpage->xmlData['actionmenu']['action'][4]['action'][] = array(
        'name' => 'T_'.$name,
        'type' => 'entry',
        'image' => $data['img'],
        'label' => $data['name'],
        );
        $this->registerAction('S_'.$name, 'handleEvent');
        $this->headpage->xmlData['actionmenu']['action'][5]['action'][] = array(
        'name' => 'S_'.$name,
        'type' => 'entry',
        'image' => $data['img'],
        'label' => $data['name'],
        );
      }
    } else {
      unset($this->headpage->xmlData['actionmenu']['action'][3]);
      unset($this->headpage->xmlData['actionmenu']['action'][4]);
      unset($this->headpage->xmlData['actionmenu']['action'][5]);
      $this->headpage->xmlData['actionmenu']['action'] = array_values($this->headpage->xmlData['actionmenu']['action']);
    }
    $this->registerAction('saveEvent', 'saveEventDialog');
  }

  /*! \brief    Handle Argonaut events
   *            All schedules and triggered events are handled here.
   */
  function handleEvent($action, array $target)
  {
    global $config;

    if (!$this->siActive) {
      return;
    }

    // Detect whether this event is scheduled or triggered.
    $triggered = TRUE;
    if (preg_match('/^S_/', $action)) {
      $triggered = FALSE;
    }

    // Detect triggered or scheduled actions
    $headpage = $this->getHeadpage();
    $event = preg_replace('/^[TS]_/', '', $action);

    // Now send FAI/Argonaut events here.
    $mac = array();

    // Collect target mac addresses
    $ldap = $config->get_ldap_link();
    foreach ($target as $dn) {
      $obj = $headpage->getEntry($dn);
      if (isset($obj['member']['count'])) {
        for ($i = 0; $i < $obj['member']['count']; $i++) {
          $ldap->cat($obj['member'][$i]);
          if ($attrs = $ldap->fetch()) {
            if (isset($attrs['macAddress'][0])) {
              $mac[] = $attrs['macAddress'][0];
            } else {
              msg_dialog::display(_('Action canceled'), sprintf(_('System %s has no mac address defined, cannot trigger action'), $obj['member'][$i]), ERROR_DIALOG);
            }
          } else {
            msg_dialog::display(_('Action canceled'), sprintf(_('Could not find system %s, cannot trigger action'), $obj['member'][$i]), ERROR_DIALOG);
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
      $this->dialogObject = new argonautAction($event, $mac);

      if ($triggered) {
        $this->dialogObject->scheduled = FALSE;
        $res = $o_queue->append($this->dialogObject);
        if ($o_queue->is_error()) {
          msg_dialog::display(_('Infrastructure service'), msgPool::siError($o_queue->get_error()), ERROR_DIALOG);
        } else {
          msg_dialog::display(_('Action triggered'), sprintf(_('Action called without error (result was "%s")'), $res), INFO_DIALOG);
        }
        $this->closeDialogs();
      } else {
        $this->dialogObject->scheduled = TRUE;
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

  static function filterProperties($row, $dn, $objectType, $attrs, $gosaGroupObjects = NULL)
  {
    global $config;
    static $grouptabs = array();

    // Load information if needed
    $result = '&nbsp;';
    if (!empty($attrs)) {
      if (strtolower($objectType) == 'ogroup') {
        if (empty($gosaGroupObjects)) {
          return $result;
        } else {
          return static::filterGroupObjects($row, $gosaGroupObjects);
        }
      }
      if (strtolower($objectType) == 'role') {
        return '<input class="center" type="image" src="geticon.php?context=types&amp;icon=role&amp;size=16" '.
                 'alt="'._('Role').'" title="'._('Edit role properties').'" '.
                 'name="listing_edit_'.$row.'" style="padding:1px"/>';
      }
      if (empty($grouptabs)) {
        foreach ($config->data['TABS']['GROUPTABS'] as $plug) {
          if ($plug['CLASS'] == 'posixGroup') {
            continue;
          }
          if (class_available($plug['CLASS'])) {
            $name = $plug['CLASS'];

            $grouptabs[$name] = new $name($dn);
          }
        }
      }
      $result = '<input class="center" type="image" src="geticon.php?context=types&amp;icon=user-group&amp;size=16" '.
                       'alt="'._('Posix').'" title="'._('Edit posix properties').'" '.
                       'name="listing_edit_tab_group_'.$row.'" style="padding:1px"/>';
      foreach ($grouptabs as $class => $grouptab) {
        if ($grouptab->is_this_account($attrs)) {
          $infos = pluglist::pluginInfos($class);
          if (isset($infos['plSmallIcon'])) {
            $result .= '<input class="center" type="image" src="'.htmlentities($infos['plSmallIcon'], ENT_COMPAT, 'UTF-8').'" '.
                       'alt="'.$infos['plTitle'].'" title="'.$infos['plTitle'].'" '.
                       'name="listing_edit_tab_'.$class.'_'.$row.'" style="padding:1px"/>';
          } else {
            @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $infos['plShortName']." ($class)", 'No icon for');
          }
        } else {
          $result .= '<img src="images/empty.png" alt=" " class="center optional '.$class.'" style="padding:1px"/>';
        }
      }
    }

    return $result;
  }

  static function filterGroupObjects($row, $gosaGroupObjects)
  {
    $types  = preg_replace('/[^a-z]/i', '', $gosaGroupObjects[0]);
    $result = '';
    for ($i = 0; $i < strlen($types); $i++) {
      if ($types[$i] == 'I') {
        continue;
      }
      $info   = objects::infos(ogroup::$objectTypes[$types[$i]]);
      $result .= '<img class="center" src="'.htmlentities($info['icon'], ENT_COMPAT, 'UTF-8').'" '.
        'alt="'.$info['name'].'" title="'.$info['name'].'" style="padding:1px"/>';
    }
    if ($result == '') {
      return '&nbsp;';
    }
    return $result;
  }

  function renderList()
  {
    $smarty = get_smarty();
    $filters = array(
      array('id'  => 'USER',        'label'  => _('Show user groups')),
      array('id'  => 'ROLE',        'label'  => _('Show organizational roles')),
      array('id'  => 'APPLICATION', 'label'  => _('Show application groups')),
      array('id'  => 'UNKNOWN',     'label'  => _('Show unidentified groups')),
    );
    if (class_available('posixGroup')) {
      $filters[] = array('id'  => 'PRIMARY',  'label'  => _('Show primary groups'));
      if (class_available('mailAccount')) {
        $filters[] = array('id'  => 'MAIL',  'label'  => _('Show mail groups'));
      }
      if (class_available('sambaAccount')) {
        $filters[] = array('id'  => 'SAMBA', 'label'  => _('Show samba groups'));
      }
    }
    if (class_available('simpleSecurityObject')) {
      $filters[] = array('id'  => 'SIMPLESECURITYOBJECT',  'label'  => _('Show DSA entries'));
    }
    if (class_available('systemManagement')) {
      $filters = array_merge(
        $filters,
        array(
          array('id'  => 'SERVER',      'label'  => _('Show server groups')),
          array('id'  => 'WORKSTATION', 'label'  => _('Show workstation groups')),
          array('id'  => 'WINDOWS',     'label'  => _('Show windows groups')),
          array('id'  => 'TERMINAL',    'label'  => _('Show terminal groups')),
          array('id'  => 'PRINTER',     'label'  => _('Show printer groups')),
          array('id'  => 'PHONE',       'label'  => _('Show phone groups')),
        )
      );
    }
    if (!static::$skipTemplates) {
      $filters[] = array(
        'id'    => 'FILTERTEMPLATE',
        'label' => sprintf(_('Show %s'), _('Template'))
      );
    }
    $smarty->assign('objectFilters', $filters);
    $this->headpage->update();
    $display = $this->headpage->render();
    return $this->getHeader().$display;
  }
}
?>
