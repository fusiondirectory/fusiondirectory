<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
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

/*!
 * \file class_management.inc
 * Source code for the class management
 */

/*!
 * \brief This class contains all the function needed to manage various
 * functions like new,save, edit, apply, cut/copy/paste , snapshot etc...
 */
class management
{
  /*!
   * \brief  Generates the plugin header which is displayed whenever a tab object is opened.
   */
  protected function getHeader()
  {
    if ($this->skipHeader) {
      return '';
    }

    $plInfos  = pluglist::pluginInfos(get_class($this));
    $plTitle  = $plInfos['plTitle'];
    $plIcon   = $plInfos['plIcon'];

    if (!preg_match('/^geticon/', $plIcon)) {
      $plIcon = get_template_path($plIcon);
    }
    return print_header($plIcon, $plTitle, get_object_info());
  }

  /*!
   * \brief  Generates the footer which is used whenever a tab object is displayed.
   */
  protected function _getTabFooter()
  {
    // Do not display tab footer for non tab objects
    if (!($this->tabObject instanceOf simpleTabs)) {
      return '';
    }

    // Check if there is a dialog opened - We don't need any buttons in this case.
    if ($this->tabObject->dialogOpened()) {
      return '';
    }

    // Skip footer if requested;
    if ($this->skipFooter) {
      return '';
    }

    // In case an of locked entry, we may have opened a read-only tab.
    $str = '';
    if ($this->tabObject->readOnly()) {
      $str .= '<p class="plugbottom">'."\n".
        '<input type="submit" name="edit_cancel" value="'.msgPool::cancelButton().'">'."\n".
        '</p>';
      return $str;
    } else {
      // Display ok, (apply) and cancel buttons
      $str .= '<p class="plugbottom">'."\n";
      $str .= '<input type="submit" name="edit_finish" style="width:80px" value="'.msgPool::okButton().'"/>'."\n";
      $str .= "&nbsp;\n";
      if ($this->displayApplyBtn) {
        $str .= '<input type="submit" name="edit_apply" value="'.msgPool::applyButton().'"/>'."\n";
        $str .= "&nbsp;\n";
      }
      $str .= '<input type="submit" name="edit_cancel" value="'.msgPool::cancelButton().'"/>'."\n";
      $str .= '</p>';
    }
    return $str;
  }

  /*!
   *  \brief  Save object modifications and keep dialogs opened
   */
  protected function applyChanges()
  {
    if ($this->tabObject instanceOf simpleTabs) {
      $this->tabObject->save_object();
      $msgs = $this->tabObject->save();
      if (count($msgs)) {
        msg_dialog::displayChecks($msgs);
      } else {
        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__, $this->dns, 'Modifications applied!');
        $this->tabObject->re_init();
      }
    }
  }

  /*!
   * \brief  This method closes dialogs
   *          and cleans up the cached object info and the ui.
   */
  public function closeDialogs()
  {
    $this->last_dn  = $this->dn;
    $this->dn       = "";
    $this->last_dns = $this->dns;
    $this->dns      = array();

    $this->last_tabObject     = $this->tabObject;
    $this->tabObject          = NULL;
    $this->last_dialogObject  = $this->dialogObject;
    $this->dialogObject       = NULL;

    $this->skipFooter   = FALSE;
    set_object_info();
  }

  /*!
   *  \brief  Every click in the list user interface sends an event
   *          here can we connect those events to a method.
   *          eg. see management::registerEvent('new','createUser')
   *          When the action/event new is send, the method 'createUser'
   *          will be called.
   */
  function registerAction($action, $target)
  {
    $this->actions[$action] = $target;
  }

  /*!
   * \brief  Removes ldap object locks created by this class.
   *         Whenever an object is edited, we create locks to avoid
   *         concurrent modifications.
   *         This locks will automatically removed here.
   */
  function remove_lock()
  {
    if (!empty($this->dn) && $this->dn != "new") {
      del_lock($this->dn);
    }
    if (count($this->dns)) {
      del_lock($this->dns);
    }
  }

  function is_modal_dialog()
  {
    return (is_object($this->tabObject) || is_object($this->dialogObject));
  }

  function openTabObject($object, $base)
  {
    $this->tabObject = $object;
    $this->tabObject->set_acl_base($base);
    $this->tabObject->parent = &$this;
  }
}
?>
