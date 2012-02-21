if (navigator.platform == "Win32" && navigator.appName == "Microsoft Internet Explorer" && window.attachEvent)
  window.attachEvent("onload", pngLoadPngs);

// parses all images //public
function pngLoadPngs()
{
  var rslt = navigator.appVersion.match(/MSIE (\d+\.\d+)/, '');
  var itsAllGood = (rslt != null && Number(rslt[1]) > 5.5);

  if (itsAllGood) {
    for (var i = 0; i < document.images.length; i++) {
      pngLoad(document.images[i]);
    }
    for (var i = 0; i < document.getElementsByTagName("input").length; i++) {
      var img = document.getElementsByTagName("input")[i];
      if(typeof img.type == "string" && img.type == "image") {
        pngLoad(img);
      }
    }
  }
}

// loads an image, src is optional // public
function pngLoad(img,src)
{
  if (typeof img == "object" && typeof img.tagName == "string" && img.tagName == "IMG")
  {
    // this is an image
    if (typeof src == "string")
    {
      // src parameter is present
      if (src.match(/\.png$/i) != null)
      {
        // this is png image
        img.style.width = null;
        img.style.height = null;
        img.style.filter = null;
        img.src = src;
        pngSwapPrepare(img);
      }
      else
      {
        // its not a png
        img.src = src;
      }
    }
    else if (img.src.match(/\.png$/i) != null)
    {
        // no src arameter, but its png -> simply swap
        pngSwapPrepare(img);
    }
  } else if (typeof img.tagName == "string" && img.tagName == "INPUT") {
    if(img.src.match(/\.png$/i) != null) {
      pngSwapPrepare(img);
    }
  }

  // swap (if complete) or shedule it to onload event // private
  function pngSwapPrepare(img)
  {
    if (img.complete)
      pngSwap(img);
    else
      img.attachEvent("onload",pngOnLoadSwap);
  }
  
  // supposed to be called when image is loaded into memory //private
  function pngOnLoadSwap()
  {
    event.srcElement.detachEvent("onload",pngOnLoadSwap);
    pngSwap(event.srcElement);
  }

  // Swaps img and background //private
  function pngSwap(img)
  {
    with (img)
    {
      style.width = width + "px";
      style.height = height + "px";
      style.filter = "progid:DXImageTransform.Microsoft.AlphaImageLoader(src='" + src + "', sizingMethod='scale') ";
      src = "images/null.gif";
    }
  }
}
