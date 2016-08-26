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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
*/

/* Install event handlers */
Event.observe(window, 'load', initProgressPie);

/* Toggle checkbox that matches regex */
function chk_set_all(regex,value)
{
  for (var i = 0; i < document.mainform.elements.length; i++) {
    var _id = document.mainform.elements[i].id;
    if(_id.match(regex)) {
      document.mainform.elements[i].checked = value;
    }
  }
}

/* Toggle checkbox that matches regex */
function chk_set_all_by_class(regex,value)
{
  for (var i = 0; i < document.mainform.elements.length; i++) {
    var _class = document.mainform.elements[i].getAttribute("class");
    if(_class) {
      if(_class.match(regex)) {
        document.mainform.elements[i].checked = value;
      }
    }
  }
}

function toggle_all_(regex,state_object)
{
  state = document.getElementById(state_object).checked;
  chk_set_all(regex, state);
}


/* Scroll down the body frame */
function scrollDown2()
{
  document.body.scrollTop = document.body.scrollHeight - document.body.clientHeight;
}


/* Toggle checkbox that matches regex */
function acl_set_all(regex,value)
{
  chk_set_all(regex,value);
}

/* Toggle checkbox that matches regex */
function acl_toggle_all(regex)
{
  for (var i = 0; i < document.mainform.elements.length; i++) {
    var _id = document.mainform.elements[i].id;
    if(_id.match(regex)) {
      if (document.getElementById(_id).checked === true){
        document.getElementById(_id).checked = false;
      } else {
        document.getElementById(_id).checked = true;
      }
    }
  }
}

function inArray(p_val, array) {
  var l = array.length;
  for (var i = 0; i < l; i++) {
    if (array[i] === p_val) {
      return true;
    }
  }
  return false;
}

function changeState() {
  for (var i = 0; i < arguments.length; i++) {
    var element = $(arguments[i]);
    if (element.hasAttribute('disabled')) {
      element.removeAttribute('disabled');
    } else {
      element.setAttribute('disabled', 'disabled');
    }
  }
}

function js_check(form) {
  form.javascript.value = 'true';
}

/* Set focus to first valid input field
   avoid IExplorer warning about hidding or disabled fields
 */
function focus_field()
{
  var i       = 0;
  var found   = false;
  var element = null;

  while (focus_field.arguments[i] && !found) {
    var tmp = document.getElementsByName(focus_field.arguments[i]);
    for (var e = 0 ; e < tmp.length ; e++) {
      if ((tmp[e].disabled !== true) &&  (tmp[e].type !== "none") && (tmp[e].type !== "hidden")) {
        found = true;
        element = tmp[e];
        break;
      }
    }
    i++;
  }

  if (element && found) {
    element.blur();
    element.focus();
  }
}


/*  This function pops up messages from message queue
    All messages are hidden in html output (style='display:none;').
    This function makes single messages visible till there are no more dialogs queued.

    hidden inputs:
    current_msg_dialogs   - Currently visible dialog
    closed_msg_dialogs    - IDs of already closed dialogs
    pending_msg_dialogs   - Queued dialog IDs.
 */
function next_msg_dialog()
{
  var s_pending;
  var a_pending;
  var i_id;
  var tmp       = "";
  var ele;
  var ele2;
  var cur_id;

  if (document.getElementById('current_msg_dialogs')) {
    cur_id = document.getElementById('current_msg_dialogs').value;
    if (cur_id !== "") {
      ele = document.getElementById('e_layer' + cur_id);
      ele.onmousemove = "";
      $('e_layer' + cur_id).hide();
      document.getElementById('closed_msg_dialogs').value += "," + cur_id;
      document.getElementById('current_msg_dialogs').value = "";
    }
  }

  if (document.getElementById('pending_msg_dialogs')) {
    s_pending = document.getElementById('pending_msg_dialogs').value;
    a_pending = s_pending.split(",");
    if (a_pending.length) {
      i_id = a_pending.pop();
      for (var i = 0 ; i < a_pending.length; ++i) {
        tmp = tmp + a_pending[i] + ',';
      }
      tmp = tmp.replace(/,$/g,"");
      if (i_id !== "") {
        ele = document.getElementById('e_layer' + i_id);
        ele3 = document.getElementById('e_layerTitle' + i_id);
        ele.style.display = 'block'  ;
        document.getElementById('pending_msg_dialogs').value = tmp;
        document.getElementById('current_msg_dialogs').value = i_id;
        ele2 = document.getElementById('e_layer2') ;
        ele3.onmousedown  = start_move_div_by_cursor;
        ele2.onmouseup    = stop_move_div_by_cursor;
        ele2.onmousemove  = move_div_by_cursor;
      } else {
        ele2 = document.getElementById('e_layer2');
        ele2.style.display = "none";
      }
    }
  }
}


/* Drag & drop for message dialogs */
var enable_move_div_by_cursor = false;    // Indicates wheter the div movement is enabled or not
var mouse_x_on_div  = 0;                  //
var mouse_y_on_div  = 0;
var div_offset_x    = 0;
var div_offset_y    = 0;

/* Activates msg_dialog drag & drop
 * This function is called when clicking on a displayed msg_dialog
 */
function start_move_div_by_cursor(e)
{
  var x = 0;
  var y = 0;
  var cur_id;
  var dialog;
  var event;

  /* Get current msg_dialog position
   */
  cur_id = document.getElementById('current_msg_dialogs').value;
  if (cur_id !== "") {
    dialog = document.getElementById('e_layer' + cur_id);
    x = dialog.style.left;
    y = dialog.style.top;
    x = x.replace(/[^0-9]/g,"");
    y = y.replace(/[^0-9]/g,"");
    if(!y) y = 200;
    if(!x) x = 200;
  }

  /* Get mouse position within msg_dialog
   */
  if (window.event) {
    event = window.event;
    if (event.offsetX) {
      div_offset_x   = event.clientX -x;
      div_offset_y   = event.clientY -y;
      enable_move_div_by_cursor = true;
    }
  } else if(e) {
    event = e;
    if (event.layerX) {
      div_offset_x  = event.screenX -x;
      div_offset_y  = event.screenY -y;
      enable_move_div_by_cursor = true;
    }
  }
}


/* Deactivate msg_dialog movement
 */
function stop_move_div_by_cursor()
{
  mouse_x_on_div = 0;
  mouse_y_on_div = 0;
  div_offset_x = 0;
  div_offset_y = 0;
  enable_move_div_by_cursor = false;
}


/* Move msg_dialog with cursor */
function move_div_by_cursor(e)
{
  var event;
  var mouse_pos_x;
  var mouse_pos_y;
  var cur_div_x;
  var cur_div_y;
  var cur_id;
  var dialog;


  if ((enable_move_div_by_cursor !== undefined) && (enable_move_div_by_cursor === true)) {

    if (document.getElementById('current_msg_dialogs')) {

      /* Get mouse position on screen
       */
      if (window.event) {
        event = window.event;
        mouse_pos_x = event.clientX;
        mouse_pos_y = event.clientY;
      } else if (e) {
        event = e;
        mouse_pos_x = event.screenX;
        mouse_pos_y = event.screenY;
      } else {
        return;
      }

      /* Get id of current msg_dialog */
      cur_id = document.getElementById('current_msg_dialogs').value;
      if (cur_id !== "") {
        dialog = document.getElementById('e_layer' + cur_id);

        /* Calculate new position */
        cur_div_x = mouse_pos_x - div_offset_x;
        cur_div_y = mouse_pos_y - div_offset_y;

        /* Ensure that dialog can't be moved out of screen */
        if(cur_div_x < 0 ) cur_div_x = 0
        if(cur_div_y < 0 ) cur_div_y = 0

        /* Assign new values */
        dialog.style.left     = (cur_div_x ) + "px";
        dialog.style.top      = (cur_div_y ) + "px";
        dialog.style.margin   = "0";
        dialog.style.position = "absolute";
      }
    }
  }
}

function setProgressPie(context, percent)
{
  context.clearRect(0, 0, 22, 22);

  var r = "3D";
  var g = "AE";
  var b = "E9";

  // Fade orange
  if (percent > 50) {
    r = "F6"
    g = "74"
    b = "00";
  }

  // Fade red
  if (percent > 75) {
    r = "ED"
    g = "15"
    b = "15";
  }

  context.strokeStyle = "#" + r  + g + b
  context.fillStyle   = context.strokeStyle;

  context.beginPath();
  context.moveTo(11,11)
  context.arc(11,11,8,-Math.PI/2,-Math.PI/2 + Math.PI*percent/50,true);
  context.closePath();
  context.fill();

  context.moveTo(11,11)
  context.beginPath();
  context.arc(11,11,8,0,Math.PI*2,false);
  context.closePath();
  context.stroke();
}

function initProgressPie() {
  var canvas = $('sTimeout');

  // Check the element is in the DOM and the browser supports canvas
  if(canvas && canvas.getContext) {
    var percent = 0.01;
    var context = canvas.getContext('2d');
    setProgressPie(context, percent);

    // Extract timeout and title string out out canvas.title
    var data = canvas.title;
    var timeout = data.replace(/\|.*$/,'');
    var title = data.replace(/^.*\|/,'');
    var interval = 1;
    var time = 0;
    setInterval(
      function() {
        // Calculate percentage
        percent += (interval / timeout) * 100;

        // Increase current time by interval
        time += interval;

        // Generate title
        var minutes = parseInt((timeout-time) / 60 );
        var seconds = '' + parseInt((timeout-time) % 60);
        if(seconds.length === 1) seconds = '0' + seconds ;
        minutes = minutes + ':' + seconds;

        // Set new  canval title
        canvas.title = title.replace(/%d/ ,minutes);
        setProgressPie(context, percent);

        if (percent>99) percent = 99;
      },
      (interval * 1000)
    );
  }
}

// Global storage for baseSelector timer
var rtimer;
