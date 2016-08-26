<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012-2016  FusionDirectory

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

/*! \brief This class allow to handle easily an Base selector attribute
 *
 */
class BaseSelectorAttribute extends Attribute
{
  private $baseSelector = NULL;
  private $orig_dn      = NULL;
  private $ou           = NULL;

  /*! \brief The constructor of BaseSelectorAttribute
   *
   *  \param string $ou The ou your objects are into. It will be used in order to detect the base they are in.
   */
  function __construct ($ou, $label = NULL, $desc = NULL)
  {
    if ($label === NULL) {
      $label = _('Base');
    }
    if ($desc === NULL) {
      $desc = _('Object base');
    }
    parent::__construct($label, $desc, 'base', FALSE, '', 'base');
    $this->setInLdap(FALSE);
    $this->ou = $ou;
  }

  function setManagedAttributes ($dontcare)
  {
    trigger_error('method setManagedAttributes is not supported for BaseSelectorAttribute');
  }

  function setParent (&$plugin)
  {
    parent::setParent($plugin);
    if (is_object($this->plugin)) {
      /* Do base conversation */
      if ($this->plugin->is_template) {
        $this->ou = 'ou=templates,'.$this->ou;
      }
      if ($this->plugin->dn == "new") {
        $ui = get_userinfo();
        $this->setValue(dn2base(session::global_is_set("CurrentMainBase")?"cn=dummy,".session::global_get("CurrentMainBase"):$ui->dn));
      } else {
        $this->setValue(dn2base($this->plugin->dn, $this->ou));
      }
      $this->orig_dn = $this->plugin->dn;
      /* Instanciate base selector */
      $this->initialValue = $this->value;
      $this->baseSelector = new baseSelector($this->plugin->get_allowed_bases(), $this->value);
      $this->baseSelector->setSubmitButton(FALSE);
      $this->baseSelector->setHeight(300);
      $this->baseSelector->update(TRUE);
    }
  }

  function loadPostValue ()
  {
  }

  function applyPostValue ()
  {
    if (!$this->disabled && $this->isVisible()) {
      /* Refresh base */
      if  ($this->plugin->acl_is_moveable($this->value) || ($this->plugin->dn == 'new')) {
        if (!$this->baseSelector->update()) {
          if ($this->plugin->dn == 'new') {
            msg_dialog::display(_('Error'), msgPool::permCreate(), ERROR_DIALOG);
          } else {
            msg_dialog::display(_('Error'), msgPool::permMove($this->plugin->dn), ERROR_DIALOG);
          }
        }
        if ($this->value != $this->baseSelector->getBase()) {
          $this->setValue($this->baseSelector->getBase());
        }
      }
    }
  }

  function check ()
  {
    $error = parent::check();
    if (!empty($error)) {
      return $error;
    } else {
      /* Check if we are allowed to create/move this user */
      if (($this->orig_dn == 'new') && !$this->plugin->acl_is_createable($this->value)) {
        return msgPool::permCreate();
      } elseif (
        ($this->orig_dn != 'new') &&
        ($this->plugin->dn != $this->orig_dn) &&
        !$this->plugin->acl_is_moveable($this->value)) {
        return msgPool::permMove($this->plugin->dn);
      }
      // Check if a wrong base was supplied
      if (!$this->baseSelector->checkLastBaseUpdate()) {
        return msgPool::check_base();
      }
    }
  }

  function setValue ($value)
  {
    parent::setValue($value);
    if (is_object($this->plugin)) {
      /* Set the new acl base */
      if ($this->plugin->dn == 'new') {
        $this->plugin->set_acl_base($this->value);
        $this->plugin->parent->set_acl_base();
      }

      if (($this->baseSelector !== NULL) && ($this->baseSelector->getBase() !== $this->value)) {
        $this->baseSelector->setBase($this->value);
      }
    }
  }

  function getValue ()
  {
    return $this->value;
  }

  function renderFormInput ()
  {
    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    if ($this->disabled) {
      $display = $this->renderInputField(
        'text', '',
        array(
          'value' => '{literal}'.htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8').'{/literal}'
        )
      );
    } else {
      $display = '{literal}'.$this->baseSelector->render().'{/literal}';
    }
    return $this->renderAcl($display);
  }

  function getHtmlId()
  {
    if (isset($this->baseSelector)) {
      return $this->baseSelector->getInputHtmlId();
    } else {
      return '';
    }
  }

  function serializeAttribute(&$attributes, $form = TRUE)
  {
    if (!$form || $this->visible) {
      parent::serializeAttribute($attributes, $form);

      $attributes[$this->getLdapName()]['choices'] = $this->baseSelector->getBases();
    }
  }
}
