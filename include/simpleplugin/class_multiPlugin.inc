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

/*! \brief   multiPlugin
    This class provides the functionality to have multiple plugins in a single tab
 */

class multiPlugin extends simplePlugin
{
  /* attribute list for save action */
  var $objectclasses  = array();
  var $ignore_account = TRUE;
  var $plugin         = array();

  var $tabClass = '';

  static function getAttributesInfo ()
  {
    return array();
  }

  function __construct ($dn = NULL, $object = NULL, $parent = NULL)
  {
    global $config;
    parent::__construct($dn, $object, $parent, FALSE);

    $plInfos = pluglist::pluginInfos(get_class($this));

    /* Load accounts */
    foreach ($config->data['TABS'][$this->tabClass] as $plug) {
      if (!plugin_available($plug['CLASS'])) {
        continue;
      }
      $name = $plug['CLASS'];
      $this->plugin[$name] = new $name($dn, $object, $parent);

      /* Acl base && category configuration,
          these settings will be overloaded in main.inc,
          if we are editing ourself */
      $this->plugin[$name]->set_acl_category($plInfos['plCategory'][0]);
      $this->plugin[$name]->set_acl_base($this->dn);
    }
  }

  function execute()
  {
    $display = "";

    /* Do we represent a valid account? */
    if ($this->parent === NULL) {
      $enabled = FALSE;
      foreach ($this->plugin as &$plug) {
        if ($plug->is_account) {
          $enabled = TRUE;
          break;
        }
      }
      unset($plug);
      if (!$enabled) {
        $display = '<img alt="" src="geticon.php?context=status&amp;icon=dialog-error&amp;size=16" align="middle"/>&nbsp;<b>'.
                    msgPool::noValidExtension()."</b>";
        $display .= back_to_main();
        return $display;
      }
    }

    /* Execude  objects */
    $is_first = TRUE;

    $readOnly = $this->readOnly();

    foreach ($this->plugin as &$plug) {
      $plug->read_only = $readOnly;
      if (!$is_first) {
        $display .= '<p class="seperator plugbottom">&nbsp;</p>';
      } else {
        $is_first = FALSE;
      }
      $display .= $plug->execute();
    }
    unset($plug);

    return $display;
  }


  /* Save data to object */
  function save_object()
  {
    foreach ($this->plugin as &$plug) {
      $plug->save_object();
    }
    unset($plug);
  }

  function check()
  {
    $message = parent::check();

    foreach ($this->plugin as &$plug) {
      if ($plug->is_account || $plug->ignore_account) {
        $tmp      = $plug->check();
        $message  = array_merge($message, $tmp);
      }
    }
    unset($plug);

    return $message;
  }

  function set_acl_category($cat)
  {
    parent::set_acl_category($cat);
    foreach ($this->plugin as &$plug) {
      $plug->set_acl_category($cat);
    }
    unset($plug);
  }

  function set_acl_base($base)
  {
    parent::set_acl_base($base);
    foreach ($this->plugin as &$plug) {
      $plug->set_acl_base($base);
    }
    unset($plug);
  }

  public function setNeedEditMode ($bool)
  {
    parent::setNeedEditMode($bool);
    foreach ($this->plugin as &$plug) {
      $plug->setNeedEditMode($bool);
    }
    unset($plug);
  }

  /* Save to LDAP */
  function save()
  {
    $errors = array();
    /* Save objects */
    foreach ($this->plugin as &$plug) {
      $plug->dn = $this->dn;
      if ($plug->is_account || $plug->ignore_account) {
        $result = $plug->save();
      } else {
        $result = $plug->remove_from_parent();
      }
      if (!empty($result)) {
        $errors = array_merge($errors, $result);
      }
    }
    unset($plug);
    return $errors;
  }

  function remove_from_parent()
  {
    $errors = array();
    /* Remove objects */
    foreach ($this->plugin as &$plug) {
      $plug->dn = $this->dn;
      $result = $plug->remove_from_parent();
      if (!empty($result)) {
        $errors = array_merge($errors, $result);
      }
    }
    unset($plug);
    return $errors;
  }

  function adapt_from_template($attrs, $skip = array())
  {
    /* Adapt objects */
    foreach ($this->plugin as &$plug) {
      $plug->dn = $this->dn;
      $plug->adapt_from_template($attrs, $skip);
    }
    unset($plug);
  }

  function resetCopyInfos()
  {
    $this->dn = 'new';
    foreach ($this->plugin as &$plug) {
      $plug->resetCopyInfos();
    }
    unset($plug);
  }
}

?>
