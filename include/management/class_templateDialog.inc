<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2019  FusionDirectory

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
 * \brief Template dialog handling
 */
class templateDialog
{
  protected $management;
  protected $type;
  protected $template = NULL;
  protected $templates;
  protected $target = NULL;

  protected $tabObject;

  protected $post_finish = 'template_continue';
  protected $post_cancel = 'template_cancel';

  function __construct ($management, $type, $dn = NULL, $target = NULL)
  {
    $this->management       = $management;
    $this->type             = $type;
    $this->templates        = objects::getTemplates($this->type);
    if ($dn !== NULL) {
      if (isset($this->templates[$dn])) {
        $this->template = new template($this->type, $dn);
      } else {
        trigger_error('Unknown template "'.$dn.'"');
      }
    }
    $this->target = $target;
  }

  function save_object ()
  {
    if (isset($_POST[$this->post_cancel])) {
      return $this->handle_cancel();
    }

    if (($this->target === NULL) &&
        (isset($_POST[$this->post_finish]) || isset($_GET[$this->post_finish])) &&
        is_object($this->template)) {
      $this->template->save_object();
      return $this->handle_finish();
    }

    if (
      isset($_POST['template']) &&
      isset($this->templates[$_POST['template']])
      ) {
      if (is_object($this->template)) {
        trigger_error('redefining template object');
      }
      $this->template = new template($this->type, $_POST['template']);
      /* This method can loop if there are several targets */
      unset($_POST['template']);
    }
    if (is_object($this->template)) {
      if ($this->target !== NULL) {
        $this->management->openTabObject($this->template->apply($this->target));
        $this->management->handleTemplateApply();
        return FALSE;
      } else {
        $this->template->save_object();
      }
    }

    return TRUE;
  }

  function setNextTarget ($target)
  {
    $this->target = $target;
    $this->template->reset();
  }

  function execute ()
  {
    $smarty = get_smarty();
    if (is_object($this->template)) {
      $templateOutput = $this->template->execute();
      if ($this->template->dialogOpened()) {
        return $templateOutput;
      } else {
        $smarty->assign('template_dialog', $templateOutput);
      }
    } else {
      $smarty->assign('templates', $this->templates);
    }
    $display = $smarty->fetch(get_template_path('template.tpl'));
    return $display;
  }

  function handle_finish ()
  {
    $this->management->closeDialogs();
    $this->management->openTabObject($this->template->apply());
    return FALSE;
  }

  function handle_cancel ()
  {
    $this->management->remove_lock();
    $this->management->closeDialogs();
    return FALSE;
  }
}
