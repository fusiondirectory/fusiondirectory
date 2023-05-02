<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2013-2020  FusionDirectory

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
class templateDialog implements FusionDirectoryDialog
{
  protected $management;
  protected $type;
  protected $template = NULL;
  protected $templates;
  protected $target = NULL;
  protected $closed = FALSE;

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

  function readPost ()
  {
    if (isset($_POST[$this->post_cancel])) {
      $this->handleCancel();
      return;
    }

    if (($this->target === NULL) &&
        is_object($this->template) &&
        (isset($_POST[$this->post_finish]) || isset($_GET[$this->post_finish]))
        ) {
      $this->template->readPost();
      $this->template->update();
      $this->handleFinish();
      return;
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
      $this->template->readPost();
      $this->template->update();
      if ($this->target !== NULL) {
        $this->management->openTabObject($this->template->apply($this->target));
        $this->management->handleTemplateApply();
        return;
      } else {
        if (empty($this->template->getNeeded())) {
          $this->handleFinish();
          return;
        }
      }
    }
  }

  public function update (): bool
  {
    return !$this->closed;
  }

  function setNextTarget ($target)
  {
    $this->target = $target;
    $this->template->reset();
  }

  public function render (): string
  {
    $smarty = get_smarty();
    if (is_object($this->template)) {
      $templateOutput = $this->template->render();
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

  protected function handleFinish ()
  {
    $this->management->closeDialogs();
    $this->management->openTabObject($this->template->apply());
  }

  protected function handleCancel ()
  {
    $this->management->removeLocks();
    $this->closed = TRUE;
  }
}
