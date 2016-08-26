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

/*! \brief This class allow to handle easily an File LDAP attribute
 *
 */
class FileAttribute extends Attribute
{
  function loadPostValue()
  {
    $this->postValue = $this->value;
    if (!empty($_FILES[$this->getHtmlId()]['name']) && $this->isVisible()) {
      if ($_FILES[$this->getHtmlId()]['size'] <= 0) {
        msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file is empty")), ERROR_DIALOG);
      } elseif (!file_exists($_FILES[$this->getHtmlId()]['tmp_name'])) {
        // Is there a tmp file, which we can use ?
        msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file not found")), ERROR_DIALOG);
      } elseif (!$handle = @fopen($_FILES[$this->getHtmlId()]['tmp_name'], "r")) {
        // Can we open the tmp file, for reading
        msg_dialog::display(_("Error"), sprintf(_("Cannot read uploaded file: %s"), _("file not readable")), ERROR_DIALOG);
      } else {
        // Everything just fine :)

        // Reading content
        $this->readFile($handle);
      }
      $_FILES[$this->getHtmlId()]['name'] = ""; // so that we only handle the file once
    }
  }

  /*! \brief This function read the file from the given handle and then closes it
   *
   *  \param filehandle $handle The handle on the opened uploaded file
   */
  function readFile($handle)
  {
    $this->postValue = fread($handle, 1024);
    while (!feof($handle)) {
      $this->postValue .= fread($handle, 1024);
    }
    @fclose($handle);
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display = $this->renderInputField('file', $id);
    return $this->renderAcl($display);
  }

  function displayValue($value)
  {
    return sprintf(_('%s (%d bytes)'), $this->getLabel(), mb_strlen($value, '8bit'));
  }
}

/* FileAttribue with download button */
class FileDownloadAttribute extends FileAttribute
{
  protected $extension;
  protected $upload;

  function __construct ($label, $description, $ldapName, $required = FALSE, $extension = '', $upload = FALSE, $defaultValue = "", $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->extension  = $extension;
    $this->upload     = $upload;
  }

  function computeFilename()
  {
    return $this->getLdapName().$this->extension;
  }

  function loadPostValue ()
  {
    if ($this->isVisible()) {
      $this->postValue = $this->value;
      foreach (array_keys($_POST) as $name) {
        if (preg_match('/^download'.$this->getHtmlId().'/', $name)) {
          session::set('binary', $this->value);
          session::set('binarytype', 'octet-stream');
          session::set('binaryfile', $this->computeFilename());
          header('location: getbin.php');
          exit();
        }
      }
      if ($this->upload && isset($_POST['upload'.$this->getHtmlId()])) {
        parent::loadPostValue();
      }
    }
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display = '';
    if ($this->upload) {
      $display  .= $this->renderInputField('file', $id);
      $display  .= $this->renderInputField('submit', 'upload'.$id, array('value' => _('Upload')));
    }
    $display  .= $this->renderInputField(
      'image', 'download'.$id,
      array(
        'title' => _('Download'),
        'alt'   => _('Download'),
        'class' => 'center',
        'src'   => 'geticon.php?context=actions&amp;icon=document-save&amp;size=16',
      )
    );
    return $this->renderAcl($display);
  }

  public function htmlIds()
  {
    $id   = $this->getHtmlId();
    $ids  = array('download'.$id);
    if ($this->upload) {
      $ids[] =  $id;
      $ids[] =  'upload'.$id;
    }
    return $ids;
  }

  function renderAttribute(&$attributes, $readOnly)
  {
    if ($this->upload == FALSE) {
      parent::renderAttribute($attributes, FALSE);
    } else {
      parent::renderAttribute($attributes, $readOnly);
    }
  }
}

class FileTextAreaAttribute extends FileDownloadAttribute
{
  function __construct ($label, $description, $ldapName, $required = FALSE, $extension = '.txt', $upload = TRUE, $defaultValue = "", $acl = "")
  {
    parent::__construct(
      $label, $description, $ldapName, $required,
      $extension, $upload, $defaultValue, $acl
    );
  }
  /*! \brief Update this attributes postValue depending of the $_POST values
   */
  function loadPostValue ()
  {
    if ($this->isVisible()) {
      $this->postValue = $this->value;
      foreach (array_keys($_POST) as $name) {
        if (preg_match('/^download'.$this->getHtmlId().'/', $name)) {
          session::set('binary', $this->value);
          session::set('binarytype', 'octet-stream');
          session::set('binaryfile', $this->computeFilename());
          header('location: getbin.php');
          exit();
        }
      }
      if ($this->upload) {
        if (isset($_POST['upload'.$this->getHtmlId()])) {
          parent::loadPostValue();
        } else {
          $id = $this->getHtmlId().'_text';
          if (isset($_POST[$id])) {
            $this->setPostValue($_POST[$id]);
          }
        }
      }
    }
  }

  function renderFormInput ()
  {
    $id = $this->getHtmlId();
    $display  = '<textarea name="'.$id.'_text" id="'.$id.'_text"'.
                ($this->disabled? 'disabled="disabled"':'').'>'.
                '{literal}'.htmlentities($this->getValue(), ENT_COMPAT, 'UTF-8').'{/literal}</textarea><br/>';
    return $this->renderAcl($display).parent::renderFormInput();
  }

  public function htmlIds()
  {
    $ids    = parent::htmlIds();
    $ids[]  = $id.'_text';
    return $ids;
  }
}

class ImageAttribute extends FileAttribute
{
  protected $width;
  protected $height;
  protected $format;
  protected $forceSize;
  protected $placeholder;

  function __construct ($label, $description, $ldapName, $required = FALSE, $width = 48, $height = 48, $format = 'png', $forceSize = FALSE, $defaultValue = "", $acl = "")
  {
    parent::__construct($label, $description, $ldapName, $required, $defaultValue, $acl);
    $this->width      = $width;
    $this->height     = $height;
    $this->format     = $format;
    $this->forceSize  = $forceSize;
  }

  function setPlaceholder($placeholder)
  {
    $this->placeholder = $placeholder;
  }

  /*! \brief Update this attributes postValue depending of the $_POST values
   */
  function loadPostValue ()
  {
    $this->postValue = $this->value;
    $id = $this->getHtmlId();
    if (!$this->disabled && $this->isVisible()) {
      foreach (array_keys($_POST) as $name) {
        if (!$this->isRequired()) {
          if (preg_match('/^'.$id.'_remove_/', $name)) {
            $this->setPostValue('');
            break;
          }
        }
        if (preg_match('/^'.$id.'_upload_/', $name)) {
          parent::loadPostValue();
          break;
        }
      }
    }
  }

  function setValue ($value)
  {
    if ($value == "") {
      $this->value = "";
      return;
    }
    if (class_exists('Imagick')) {
      $im     = new Imagick();
      $modify = FALSE;
      $im->readImageBlob($value);

      $size = $im->getImageGeometry();

      if (
          ($size['width'] > 0 && $size['height'] > 0) &&
          (
            ($size['width'] < $this->width && $size['height'] < $this->height) ||
            ($size['width'] > $this->width) ||
            ($size['height'] > $this->height)
          )
        ) {
        $modify = TRUE;
        $im->resizeImage($this->width, $this->height, Imagick::FILTER_GAUSSIAN, 1, !$this->forceSize);
      }

      if ($modify || !preg_match('/^'.$this->format.'$/i', $im->getImageFormat())) {
        if ($this->format == 'jpeg') {
          $im->setImageCompression(Imagick::COMPRESSION_JPEG);
          $im->setImageCompressionQuality(90);
        }
        $im->setImageFormat($this->format);

        /* Save attribute */
        $this->value = $im->getImageBlob();
      } else {
        $this->value = $value;
      }
    } else {
      msg_dialog::display(
        _('Error'),
        _('Cannot save user picture, FusionDirectory requires the PHP module "imagick" to be installed!'),
        ERROR_DIALOG
      );
    }
  }

  function renderFormInput ()
  {
    $this->setValue($this->inputValue($this->getValue()));
    $id = $this->getHtmlId();
    srand((double)microtime() * 1000000); // Just to be sure the image is not cached
    $display  = '<img id="'.$id.'_img"'.
                ($this->disabled? 'disabled="disabled"':'').
                ' src="getbin.php?rand='.rand(0, 10000).'"'.
                ' style="border:1px solid black;"'.
                ' alt="'.$this->getDescription().'"'.
                ' title="'.$this->getDescription().'"'.
                ' /><br/>';
    $display  .= $this->renderInputField('file', $id);
    $display .= $this->renderInputField(
      'image', $id.'_upload',
      array(
        'class' => 'center',
        'src'   => 'geticon.php?context=actions&amp;icon=upload&amp;size=16',
        'title' => _('Upload'),
        'alt'   => _('Upload')
      )
    );
    if (!$this->isRequired()) {
      $display .= $this->renderInputField(
        'image', $id.'_remove',
        array(
          'class' => 'center',
          'src'   => 'geticon.php?context=actions&amp;icon=remove&amp;size=16',
          'title' => _('Remove'),
          'alt'   => _('Remove')
        )
      );
    }
    if (($this->getValue() == '') && ($this->placeholder != '')) {
      session::set('binary', $this->placeholder);
    } else {
      session::set('binary', $this->getValue());
    }
    session::set('binarytype', 'image/'.$this->format);
    return $this->renderAcl($display);
  }

  public function htmlIds()
  {
    $id = $this->getHtmlId();
    return array($id.'_img',$id,'upload'.$id);
  }
}
