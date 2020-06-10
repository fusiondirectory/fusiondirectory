<?php
/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2003-2010  Cajus Pollmeier
  Copyright (C) 2011-2020  FusionDirectory

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
 * \file class_msgPool.inc
 * Source code for class msgPool
 */

define('LDAP_READ',   1);
define('LDAP_ADD',    2);
define('LDAP_MOD',    3);
define('LDAP_DEL',    4);
define('LDAP_SEARCH', 5);
define('LDAP_AUTH',   6);

/*!
 * \brief This class contains all the messages for the various actions
 */
class msgPool
{
  /*!
   * \brief Display that we have no permission to delete an object
   *
   * \param string $name Name of the object which will be deleted
   */
  public static function permDelete ($name = '')
  {
    if ($name == '') {
      return htmlescape(_('You have no permission to delete this object!'));
    }

    if (!is_array($name)) {
      return htmlescape(_('You have no permission to delete the object:'))."<br/><br/><i>$name</i>";
    }

    if (count($name) == 1) {
      return htmlescape(_('You have no permission to delete the object:')).'<br/>'.msgPool::buildList($name);
    }

    return htmlescape(_('You have no permission to delete these objects:')).'<br/>'.msgPool::buildList($name);
  }

  /*!
   * \brief Display that we have no permission to create an object
   *
   * \param string $name Name of the object which will be created
   */
  public static function permCreate ($name = '')
  {
    if ($name == '') {
      return htmlescape(_('You have no permission to create this object!'));
    }

    if (!is_array($name)) {
      return htmlescape(_('You have no permission to create the object:')).'<br/><br/><i>'.htmlescape($name).'</i>';
    }

    if (count($name) == 1) {
      return htmlescape(_('You have no permission to create the object:')).'<br/>'.msgPool::buildList($name);
    }

    return htmlescape(_('You have no permission to create these objects:')).'<br/>'.msgPool::buildList($name);
  }

  /*!
   * \brief Display that we have no permission to modify an object
   *
   * \param string $name Name of the object which cannot be modified (or array of objects names)
   * \param string $field Name of the field of the object which cannot be modified
   */
  public static function permModify ($name = '', $field = '')
  {
    if ($name == '') {
      return htmlescape(_('You have no permission to modify this object!'));
    }

    if (!is_array($name)) {
      if ($field != '') {
        return htmlescape(sprintf(_('You have no permission to modify the field "%s" of object "%s"'), $field, $name));
      } else {
        return sprintf(htmlescape(_('You have no permission to modify the object:%s')), '<br/><br/><i>'.htmlescape($name).'</i>');
      }
    }

    if (count($name) == 1) {
      return sprintf(htmlescape(_('You have no permission to modify the object:%s')), '<br/>'.msgPool::buildList($name));
    }

    return sprintf(htmlescape(_('You have no permission to modify these objects:%s')), '<br/>'.msgPool::buildList($name));
  }

  /*!
   * \brief Display that we have no permission to view an object
   *
   * \param string $name Name of the object which will be viewed
   */
  public static function permView ($name = '')
  {
    if ($name == '') {
      return htmlescape(_('You have no permission to view this object!'));
    }

    if (!is_array($name)) {
      return htmlescape(_('You have no permission to view the object:'))."<br/><br/><i>".htmlescape($name)."</i>";
    }

    if (count($name) == 1) {
      return htmlescape(_('You have no permission to view the object:')).'<br/>'.msgPool::buildList($name);
    }

    return htmlescape(_('You have no permission to view these objects:')).'<br/>'.msgPool::buildList($name);
  }

  /*!
   * \brief Display that we have no permission to move an object
   *
   * \param string $name Name of the object which will be moved
   */
  public static function permMove ($name = '')
  {
    if ($name == '') {
      return htmlescape(_('You have no permission to move this object!'));
    }

    if (!is_array($name)) {
      return htmlescape(_('You have no permission to move the object:'))."<br/><br/><i>".htmlescape($name)."</i>";
    }

    if (count($name) == 1) {
      return htmlescape(_('You have no permission to move the object:')).'<br/>'.msgPool::buildList($name);
    }

    return htmlescape(_('You have no permission to move these objects:')).'<br/>'.msgPool::buildList($name);
  }

  /*!
   * \brief Display field contains reserved keyword
   *
   * \param string $name The field which contains reserved keyword
   */
  public static function reserved ($name)
  {
    return htmlescape(sprintf(_('The field "%s" contains a reserved keyword!'), $name));
  }

  /*!
   * \brief Display that a command execution failed in this plugin
   *
   * \param string $type Command type
   *
   * \param string $command Command name
   *
   * \param string $plugin Name of the plugin
   */
  public static function cmdexecfailed ($type, $command = '', $plugin = '')
  {
    if ($command == '') {
      if ($plugin == '') {
        return htmlescape(sprintf(_('Cannot execute "%s" command!'), $type));
      } else {
        return htmlescape(sprintf(_('Cannot execute "%s" command for plugin %s!'), $type, $plugin));
      }
    } else {
      if ($plugin == '') {
        return htmlescape(sprintf(_('Cannot execute "%s" command (%s)!'), $type, $command));
      } else {
        return htmlescape(sprintf(_('Cannot execute "%s" command (%s) for plugin %s!'), $type, $command, $plugin));
      }
    }
  }

  /*!
   * \brief Display error about too larged value
   *
   * \param string $name Name of the value
   *
   * \param string $min The largest value
   */
  public static function toobig ($name, $min = '')
  {
    if ($min == '') {
      return htmlescape(sprintf(_('Value for "%s" is too large!'), $name));
    } else {
      return htmlescape(sprintf(_('"%s" must be smaller than %s!'), $name, $min));
    }
  }

  /*!
   * \brief Display error about too small value
   *
   * \param string $name Name of the value
   *
   * \param string $min The smallest value
   */
  public static function toosmall ($name, $min = '')
  {
    if ($min == '') {
      return htmlescape(sprintf(_('Value for "%s" is too small!'), $name));
    } else {
      return htmlescape(sprintf(_('"%s" must be %d or above!'), $name, $min));
    }
  }

  /*!
   * \brief Display a dependence between two objects
   *
   * \param string $name1 First object
   *
   * \param string $name2 Second object
   */
  public static function depends ($name1, $name2)
  {
    return htmlescape(sprintf(_('"%s" depends on "%s" - please provide both values!'), $name1, $name2));
  }

  /*!
   * \brief Display error about existing entry in the system
   *
   * \param string $name The attribute name
   * \param string $dn The existing entry dn
   */
  public static function duplicated ($name, $dn = NULL)
  {
    if ($dn == NULL) {
      return htmlescape(sprintf(_('There is already an entry with this "%s" attribute in the system!'), $name));
    } else {
      return htmlescape(sprintf(_('The entry "%s" already use this "%s" attribute!'), $dn, $name));
    }
  }

  /*!
   * \brief Display error about required field empty
   *
   * \param string $name Name of the field
   */
  public static function required ($name)
  {
    return htmlescape(sprintf(_('The required field "%s" is empty!'), $name));
  }

  /*!
   * \brief Return error about invalid value
   *
   * \param string $name The field name
   *
   * \param string $data The submited data
   *
   * \param string $example Example of a right submited data
   */
  public static function invalid (string $name, $data = '', string $example = ''): string
  {
    if (!is_string($data)) {
      $data = var_export($data, TRUE);
    }
    $error = htmlescape(sprintf(_('The field "%s" contains an invalid value.'), $name));
    $error .= '<br/><br/> "'.htmlescape($data).'"';

    /* Stylize example */
    if ($example !== '') {
      $error .= '<br/><br/><i>'.htmlescape(sprintf(_('Example: %s'), $example)).'</i> ';
    }

    return $error;
  }

  /*!
   * \brief Display about missing PHP extension
   *
   * \param string $name The name of the extension
   */
  public static function missingext ($name)
  {
    return htmlescape(sprintf(_('Missing %s PHP extension!'), $name));
  }

  /*!
   * \brief Text for a cancel button
   */
  public static function cancelButton ()
  {
    return htmlescape(_('Cancel'));
  }

  /*!
   * \brief Text for a ok button
   */
  public static function okButton ()
  {
    return htmlescape(_('Ok'));
  }

  /*!
   * \brief Text for an apply button
   */
  public static function applyButton ()
  {
    return htmlescape(_('Apply'));
  }

  /*!
   * \brief Text for a save button
   */
  public static function saveButton ()
  {
    return htmlescape(_('Save'));
  }

  /*!
   * \brief Text for an add button
   *
   * \param string $what Text for the button
   */
  public static function addButton ($what = '')
  {
    return htmlescape(($what == '') ? sprintf(_('Add')) : sprintf(_('Add %s'), $what));
  }

  /*!
   * \brief Text for an delete button
   *
   * \param string $what Text for the button
   */
  public static function delButton ($what = '')
  {
    return htmlescape(($what == '') ? sprintf(_('Delete')) : sprintf(_('Delete %s'), $what));
  }

  /*!
   * \brief Text for an edit button
   *
   * \param string $what Text for the button
   */
  public static function editButton ($what = '')
  {
    return htmlescape(($what == '') ? sprintf(_('Edit...')) : sprintf(_('Edit %s...'), $what));
  }

  /*!
   * \brief Text for a back button
   */
  public static function backButton ()
  {
    return htmlescape(_('Back'));
  }

  /*!
   * \brief a list from an array
   *
   * \param array $data Array with the elements of the list
   */
  public static function buildList ($data)
  {
    $objects = '<ul>';
    foreach ($data as $key => $value) {
      if (is_numeric($key)) {
        $objects .= "<li>\n<i>".htmlescape($value)."</i></li>";
      } else {
        $objects .= "<li>\n".htmlescape($value)." (<i>".htmlescape($key)."</i>)</li>";
      }
    }
    $objects .= '</ul>';
    return $objects;
  }

  /*!
   * \brief Display error about invalid extension from account
   *
   * \param string $name Name of the extension
   */
  public static function noValidExtension ($name)
  {
    return htmlescape(sprintf(_('This account has no valid %s extensions!'), $name));
  }

  /*!
   * \brief List the features settings enabled
   *
   * \param string $name Name of the setting
   *
   * \param array $depends Contains all the settings enabled
   */
  public static function featuresEnabled ($name, $depends = '')
  {
    if (($depends == '') || (is_array($depends) && (count($depends) == 0))) {
      return htmlescape(sprintf(_('This account has %s settings enabled. You can disable them by clicking below.'), $name));
    } else {
      if (is_array($depends)) {
        $depends = implode(' / ', $depends);
      }
      return htmlescape(sprintf(_('This account has %s settings enabled. To disable them, you\'ll need to remove the %s settings first!'), $name, $depends));
    }
  }

  /*!
   * \brief List the features settings disabled
   *
   * \param string $name Name of the tab
   *
   * \param array $depends Tabs this tab depends upon
   *
   * \param array $conflicts Tabs this tab conflicts with
   */
  public static function featuresDisabled ($name, array $depends = [], array $conflicts = [])
  {
    if (empty($depends) && empty($conflicts)) {
      return htmlescape(sprintf(_('This account has %s settings disabled. You can enable them by clicking below.'), $name));
    } elseif (!empty($depends)) {
      return htmlescape(sprintf(_('This account has %s settings disabled. To enable them, you\'ll need to add the %s settings first!'), $name, implode(' / ', $depends)));
    } else {
      return htmlescape(sprintf(_('This account has %s settings disabled. To enable them, you\'ll need to disable the %s settings first!'), $name, implode(' / ', $conflicts)));
    }
  }

  /*!
   * \brief Display Add features button
   *
   * \param string $name Name of the feature
   */
  public static function addFeaturesButton ($name)
  {
    return htmlescape(sprintf(_('Add %s settings'), $name));
  }

  /*!
   * \brief Display Remove features button
   *
   * \param string $name Name of the feature
   */

  public static function removeFeaturesButton ($name)
  {
    return htmlescape(sprintf(_('Remove %s settings'), $name));
  }

  /*!
   * \brief Display : Click the "Edit" button below to change information in this dialog
   */
  public static function clickEditToChange ()
  {
    return htmlescape(_('Click the "Edit" button on the bottom right corner of the page to edit the fields'));
  }

  /*!
   * \brief Build an array with the months
   */
  public static function months ()
  {
    return [_('January'), _('February'), _('March'), _('April'),
        _('May'), _('June'), _('July'), _('August'), _('September'),
        _('October'), _('November'), _('December')];
  }

  /*!
   * \brief Build an array with the days of a week.
   *    Start by Sunday
   */
  public static function weekdays ()
  {
    return [ _('Sunday'), _('Monday'), _('Tuesday'), _('Wednesday'), _('Thursday'), _('Friday'), _('Saturday')];
  }

  /*!
   * \brief Display LDAP error
   *
   * \param string $error Error to display
   *
   * \param string $dn the DN
   *
   * \param integer $type
   *
   * \param string $plugin
   */
  public static function ldaperror ($error, $dn = '', $type = 0, $plugin = '')
  {
    /* Assign headline depending on type */
    $typemap = [1 => _('read operation'), _('add operation'), _('modify operation'),
        _('delete operation'), _('search operation'), _('authentication')];

    if (isset($typemap[$type])) {
      $msg = htmlescape(sprintf(_('LDAP %s failed!'), $typemap[$type]));
    } else {
      $msg = htmlescape(_('LDAP operation failed!'));
    }

    /* Fill DN information */
    if ($dn != '') {
      $msg .= '<br/><br/><i>'.htmlescape(_('Object')).':</i> '.htmlescape($dn);
    }

    $msg .= '<br/><br/><i>'.htmlescape(_('Error')).':</i> '.htmlescape($error);

    return $msg;
  }

  /*!
   * \brief Display error about an incorrect upload
   *
   * \param string $reason The reason of the upload failed
   */
  public static function incorrectUpload ($reason = '')
  {
    if ($reason == '') {
      return htmlescape(_('Upload failed!'));
    }

    return sprintf(htmlescape(_('Upload failed: %s')), '<br/><br/><i>'.htmlescape($reason).'</i>');
  }

  /*!
   * \brief Display error about communication failure with the infrastructure service
   *
   * \param string $error The error of the communication failure
   */
  public static function siError ($error = '')
  {
    if ($error == '') {
      return htmlescape(_('Communication failure with the infrastructure service!'));
    }
    return sprintf(htmlescape(_('Communication failure with the infrastructure service: %s')), '<br/><br/>'.htmlescape(sprintf(_('Error: %s'), $error)));
  }

  /*!
   * \brief Display checking for a support
   *
   * \param string $what Name of the support
   */
  public static function checkingFor ($what)
  {
    return htmlescape(sprintf(_('Checking for %s support'), $what));
  }

  /*!
   * \brief Display install or activate a PHP module
   *
   * \param string $what Name of the module
   */
  public static function installPhpModule ($what)
  {
    return htmlescape(sprintf(_('Install and activate the %s PHP module.'), $what));
  }

  /*!
   * \brief Display error when checking the base
   */
  public static function check_base ()
  {
    return htmlescape(_('The supplied base is not valid and has been reset to the previous value!'));
  }
}
